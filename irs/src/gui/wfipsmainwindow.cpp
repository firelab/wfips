/******************************************************************************
 *
 * Project:  Wildland Fire Investment Planning System
 * Purpose:  Graphical User Interface
 * Author:   Kyle Shannon <kyle at pobox dot com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include "wfipsmainwindow.h"
#include "ui_wfipsmainwindow.h"

/*
** Helper to bounce between QStrings and C style strings.  Please free the
** results with free(), as the return value is owned by you, the caller.
*/
static char * QStringToCString( const QString &s )
{
    int n = s.size() + 1;
    char *p = (char*)malloc( sizeof( char ) * n );
    strncpy( p, s.toLocal8Bit().data(), n );
    return p;
}

WfipsMainWindow::WfipsMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WfipsMainWindow)
{
    ui->setupUi(this);
    ui->treeWidget->expandAll();

    /* Initialize QGIS registry */
    QString qgisPluginPath = QGIS_PLUGIN_DIR;
    QgsProviderRegistry::instance( qgisPluginPath );
    qDebug() << "QGIS plugin path: " << qgisPluginPath;

    ConstructToolButtons();
    ConstructAnalysisAreaWidgets();

    ConstructTreeWidget();
    AssignTreeWidgetIndices( ui->treeWidget->invisibleRootItem() );
    qDebug() << "Found " << treeWidgetList.size() << " tree widget items.";

    /* Call *after* construction */
    CreateConnections();
    PostConstructionActions();
}

WfipsMainWindow::~WfipsMainWindow()
{
    /*
    ** We may need to clean up a little for QGIS, but I don't think so.
    QgsMapLayerRegistry::instance()->removeAllMapLayers();
    delete analysisLayer;
    delete analysisSymbol;
    delete analysisRenderer;
    */
    delete ui;
}

void WfipsMainWindow::CreateConnections()
{
    /* Open the root path to most of the data */
    connect( ui->openWfipsPathToolButton, SIGNAL( clicked() ),
             this, SLOT( OpenWfipsPath() ) );
    /* Update the analysis layer based on the combo box choice */
    connect( ui->analysisAreaComboBox, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( UpdateAnalysisAreaMap( int ) ) );
    /* Open and load custom layers for analysis */
    connect( ui->customAnalysisAreaOpenToolButton, SIGNAL( clicked() ),
             this, SLOT( AddCustomAnalysisArea() ) );
    connect( ui->customAnalysisAreaAddLayerToolButton, SIGNAL( clicked() ),
             this, SLOT( LoadCustomAnalysisArea() ) );
}

void WfipsMainWindow::PostConstructionActions()
{
    /* Make sure one of the map tools gets initialized */
    ui->mapPanToolButton->click();
}

void WfipsMainWindow::ConstructToolButtons()
{
    ui->mapPanToolButton->setIcon( QIcon( ":/pan" ) );
    connect( ui->mapPanToolButton, SIGNAL( clicked() ),
             this, SLOT( UpdateMapToolType() ) );
    ui->mapZoomInToolButton->setIcon( QIcon( ":/zoom_in" ) );
    connect( ui->mapZoomInToolButton, SIGNAL( clicked() ),
             this, SLOT( UpdateMapToolType() ) );
    ui->mapZoomOutToolButton->setIcon( QIcon( ":/zoom_out" ) );
    connect( ui->mapZoomOutToolButton, SIGNAL( clicked() ),
             this, SLOT( UpdateMapToolType() ) );
    ui->mapZoomToLayerToolButton->setIcon( QIcon( ":/select" ) );
    connect( ui->mapSelectToolButton, SIGNAL( clicked() ),
             this, SLOT( UpdateMapToolType() ) );
    ui->mapZoomToLayerToolButton->setIcon( QIcon( ":/zoom_layer" ) );
    connect( ui->mapZoomToLayerToolButton, SIGNAL( clicked() ),
             this, SLOT( ZoomToLayerExtent() ) );
}

void WfipsMainWindow::ConstructAnalysisAreaWidgets()
{
    analysisAreaMapCanvas = new QgsMapCanvas( 0, 0 );
    analysisAreaMapCanvas->enableAntiAliasing( true );
    analysisAreaMapCanvas->setCanvasColor( Qt::white );
    analysisAreaMapCanvas->freeze( false );
    analysisAreaMapCanvas->setVisible( true );
    analysisAreaMapCanvas->refresh();
    analysisAreaMapLayout = new QVBoxLayout( ui->analysisAreaMapFrame );
    analysisAreaMapLayout->addWidget( analysisAreaMapCanvas );

    analysisPanTool = new QgsMapToolPan( analysisAreaMapCanvas );
    analysisZoomInTool = new QgsMapToolZoom( analysisAreaMapCanvas, FALSE );
    analysisZoomOutTool = new QgsMapToolZoom( analysisAreaMapCanvas, TRUE);
    //analysisSelectTool = new QgsMapToolSelect( analysisAreaMapCanvas, TRUE);
}

/*
** Add one layer to the analysis area widget.  The path to the file and the
** name of the layer are needed.  If the layer name is not provided, the fx
** attempts to find a layer that is the same as the basename of the file (eg,
** /abc/def/ghi.xyz -> ghi).  The base name is also added to the combo box that
** allows the user to select which layer to display.
**
** The created layers use default symbology/rendering and ownership is passed
** to the map layer registry.
*/
void WfipsMainWindow::AddAnalysisAreaLayer( QString path, QString layerName )
{
    if( path == "" )
    {
        qDebug() << "Invalid layer";
        return;
    }
    path += "|layername=";
    if( layerName == "" )
    {
        layerName = QFileInfo( path ).baseName();
    }
    path += layerName;
    qDebug() << "Loading layer: " << layerName;
    analysisLayer = new QgsVectorLayer( path, "", "ogr", true );
    if( !analysisLayer->isValid() )
    {
        delete analysisLayer;
        qDebug() << "Invalid layer";
        return;
    }
    analysisLayer->setReadOnly( true );
    QgsMapLayerRegistry::instance()->addMapLayer( analysisLayer, false );
    analysisMapCanvasLayers.append( QgsMapCanvasLayer( analysisLayer, false ) );
    ui->analysisAreaComboBox->addItem( layerName.toUpper() );
}

/*
** Load the default analysis area related map layers.  These are admin layers
** from the base wfips directory.  Various USFS admin boundaries are available
** including geographic areas, forests and districts.
*/
void WfipsMainWindow::LoadAnalysisAreaLayers()
{
    if( wfipsPath == "" )
    {
        qDebug() << "The data path has not been provided, no layers";
        return;
    }
    QStringList layerNames;
    layerNames << "gacc" << "forest" << "district";
    for( int i = 0; i < layerNames.size(); i++ )
    {
        AddAnalysisAreaLayer( wfipsPath + "/" + layerNames[i] + ".db" );
    }
    analysisAreaMapCanvas->setLayerSet( analysisMapCanvasLayers );
    analysisAreaMapCanvas->setExtent( QgsRectangle( -129.0, 22.0, -93.0, 52.0 ) );
    analysisAreaMapCanvas->refresh();
}

/*
** Allow the user to add a custom layer to run the simulation.  This may be any
** OGR supported datasource, and one layer may be selected.
*/
void WfipsMainWindow::AddCustomAnalysisArea()
{
    QString layerFile =
        QFileDialog::getOpenFileName( this, tr( "Open GIS file" ), "", "" );
    ui->customAnalysisAreaLineEdit->setText( layerFile );
    ui->customAnalysisAreaLayerComboBox->clear();
    if( layerFile == "" ) 
    {
        return;
    }
    const char *pszFilename = QStringToCString( layerFile );
    OGRDataSourceH hDS = OGROpen( pszFilename, FALSE, NULL );
    free( (void*)pszFilename );
    if( hDS == NULL )
    {
        qDebug() << "Could not identify layer file";
        ui->customAnalysisAreaAddLayerToolButton->setDisabled( true );
        ui->customAnalysisAreaLineEdit->setText( "" );
        return;
    }
    QStringList layers;
    int i = 0;
    while( i < OGR_DS_GetLayerCount( hDS ) )
    {
        layers << OGR_L_GetName( GDALDatasetGetLayer( hDS, i ) );
        i++;
    }
    if( layers.size() < 1 )
    {
        ui->customAnalysisAreaAddLayerToolButton->setDisabled( true );
        ui->customAnalysisAreaLineEdit->setText( "" );
        GDALClose( hDS );
        return;
    }
    ui->customAnalysisAreaLayerComboBox->addItems( layers );
    OGR_DS_Destroy( hDS );
    ui->customAnalysisAreaAddLayerToolButton->setEnabled( true );
}

void WfipsMainWindow::LoadCustomAnalysisArea()
{
    if( ui->customAnalysisAreaLineEdit->text() == "" ||
        ui->customAnalysisAreaLayerComboBox->count() < 1 )
    {
        qDebug() << "No datasource or layer selected";
        return;
    }
    AddAnalysisAreaLayer( ui->customAnalysisAreaLineEdit->text(),
                          ui->customAnalysisAreaLayerComboBox->currentText() );
    analysisAreaMapCanvas->refresh();
}

void WfipsMainWindow::UpdateAnalysisAreaMap( int index )
{
    assert( index < analysisMapCanvasLayers.size() );
    QgsRectangle extent = analysisAreaMapCanvas->extent();
    for( int i = 0; i < analysisMapCanvasLayers.size(); i++ )
    {
        analysisMapCanvasLayers[i].setVisible( false );
    }
    analysisMapCanvasLayers[index].setVisible( true );
    analysisAreaMapCanvas->setExtent( extent );
    analysisAreaMapCanvas->setLayerSet( analysisMapCanvasLayers );
    analysisAreaMapCanvas->refresh();
}

void WfipsMainWindow::ConstructTreeWidget()
{
    connect( ui->treeWidget,
             SIGNAL( currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * ) ),
             this,
             SLOT( SetStackIndex( QTreeWidgetItem *, QTreeWidgetItem * ) ) );
}

void WfipsMainWindow::AssignTreeWidgetIndices( QTreeWidgetItem *item )
{
    treeWidgetList << item;
    for( int i = 0; i < item->childCount(); i++ )
    {
        AssignTreeWidgetIndices( item->child( i ) );
    }
}

void WfipsMainWindow::SetStackIndex( QTreeWidgetItem *current,
                                     QTreeWidgetItem *previous )
{
    if( current == previous )
        return;
    int i = 0;
    while( i < treeWidgetList.size() )
    {
        if( current == treeWidgetList[i] )
        {
            break;
        }
        i++;
    }
    qDebug() << "tree widget index: " << i;
    /*
    ** Set the correct page for the stack widget.  If there is no map on the
    ** page, disable the map tool frame.
    */
    ui->mapToolFrame->setDisabled( true );
    switch( i )
    {
        case 1:
        case 2:
            ui->stackedWidget->setCurrentIndex( 0 );
            break;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            ui->stackedWidget->setCurrentIndex( 1 );
            ui->mapToolFrame->setEnabled( true );
            break;
        case 9:
            ui->stackedWidget->setCurrentIndex( 2 );
            break;
        case 10:
            ui->stackedWidget->setCurrentIndex( 3 );
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            ui->stackedWidget->setCurrentIndex( 4 );
            break;
        case 17:
        case 18:
        case 19:
            ui->stackedWidget->setCurrentIndex( 5 );
            break;
        case 20:
            ui->stackedWidget->setCurrentIndex( 6 );
            break;
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
            ui->stackedWidget->setCurrentIndex( 7 );
            ui->mapToolFrame->setEnabled( true );
            break;
        /* 0 is the 'invisible root' */
        case 0:
        default:
            qDebug() << "Shouldn't ever get here, you messed up";
            break;
    }
    analysisAreaMapCanvas->refresh();
}

/*
** Action based stuff.  Open files, etc.  Any private slot *not* involved with
** basic loading/construction.
*/

void WfipsMainWindow::OpenWfipsPath()
{
    wfipsPath = 
        QFileDialog::getExistingDirectory( this, tr("Open wfips data path"),
                                           "", QFileDialog::ShowDirsOnly |
                                               QFileDialog::DontResolveSymlinks );
    ui->openWfipsPathLineEdit->setText( wfipsPath );
    if( wfipsPath == "" )
        return;
    qDebug() << "Using " << wfipsPath << " for data path.";
    /* Find the analysis layers */
    LoadAnalysisAreaLayers();
    /* Find resource layers */

    return;
}

void WfipsMainWindow::UpdateMapToolType()
{
    if( ui->mapPanToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to pan";
        analysisAreaMapCanvas->setMapTool( analysisPanTool );
    }
    else if( ui->mapZoomInToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to zoom in";
        analysisAreaMapCanvas->setMapTool( analysisZoomInTool );
    }
    else if( ui->mapZoomOutToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to zoom out";
        analysisAreaMapCanvas->setMapTool( analysisZoomOutTool );
    }
    else if( ui->mapZoomOutToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to select";
        analysisAreaMapCanvas->setMapTool( analysisSelectTool );
    }
}

void WfipsMainWindow::ZoomToLayerExtent()
{
    qDebug() << "Zoom to layer extent";
}

void WfipsMainWindow::ShowMessage( const int messageType,
                                   const int messageFlags,
                                   const QString &message )
{
#define WFIPS_CONSOLE_ERR
#ifdef WFIPS_CONSOLE_ERR
    QString s;
    if( messageType == QMessageBox::Question )
        s += "QUESTION: ";
    else if( messageType == QMessageBox::Information )
        s += "MESSAGE: ";
    else if( messageType == QMessageBox::Warning )
        s += "WARNING: ";
    else if( messageType == QMessageBox::Critical )
        s += "CRITICAL: ";
    s += message;
    qDebug() << s;

#else /* WFIPS_CONSOLE_ERR */
    QMessageBox msgBox;
#endif /* WFIPS_CONSOLE_ERR */
}

