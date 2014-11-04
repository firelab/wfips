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

    selectedFid = -1;

    identifyDialog = new WfipsIdentifyDialog( this );

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
    delete identifyDialog;

    delete analysisAreaMapCanvas;
    delete analysisAreaMapLayout;

    delete analysisPanTool;
    delete analysisZoomInTool;
    delete analysisZoomOutTool;
    delete analysisIdentifyTool;
    delete analysisSelectTool;
}

void WfipsMainWindow::CreateConnections()
{
    /* Open the root path to most of the data */
    connect( ui->openWfipsPathToolButton, SIGNAL( clicked() ),
             this, SLOT( OpenWfipsPath() ) );
    /* Update the analysis layer based on the combo box choice */
    connect( ui->analysisAreaComboBox, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( UpdateAnalysisAreaMap( int ) ) );

    /* Set Analysis Area */
    connect( ui->setAnalysisAreaToolButton, SIGNAL( clicked() ),
             this, SLOT( SetAnalysisArea() ) );
}

void WfipsMainWindow::PostConstructionActions()
{
    /* Make sure one of the map tools gets initialized */
    ui->mapPanToolButton->click();
}

void WfipsMainWindow::ConstructToolButtons()
{
    /* Map navigation tool buttons */
    ui->mapPanToolButton->setIcon( QIcon( ":/pan" ) );
    connect( ui->mapPanToolButton, SIGNAL( clicked() ),
             this, SLOT( UpdateMapToolType() ) );
    ui->mapZoomInToolButton->setIcon( QIcon( ":/zoom_in" ) );
    connect( ui->mapZoomInToolButton, SIGNAL( clicked() ),
             this, SLOT( UpdateMapToolType() ) );
    ui->mapZoomOutToolButton->setIcon( QIcon( ":/zoom_out" ) );
    connect( ui->mapZoomOutToolButton, SIGNAL( clicked() ),
             this, SLOT( UpdateMapToolType() ) );
    ui->mapSelectToolButton->setIcon( QIcon( ":/select" ) );
    connect( ui->mapSelectToolButton, SIGNAL( clicked() ),
             this, SLOT( UpdateMapToolType() ) );
    ui->mapIdentifyToolButton->setIcon( QIcon( ":/identify" ) );
    connect( ui->mapIdentifyToolButton, SIGNAL( clicked() ),
             this, SLOT( UpdateMapToolType() ) );
    ui->mapZoomToLayerToolButton->setIcon( QIcon( ":/zoom_layer" ) );
    connect( ui->mapZoomToLayerToolButton, SIGNAL( clicked() ),
             this, SLOT( ZoomToLayerExtent() ) );

    /* Add custom analysis layer tool button */
    ui->customAnalysisAreaOpenToolButton->setIcon( QIcon( ":/add_layer" ) );
    connect( ui->customAnalysisAreaOpenToolButton, SIGNAL( clicked() ),
             this, SLOT( AddCustomAnalysisArea() ) );
}

void WfipsMainWindow::ConstructAnalysisAreaWidgets()
{
    analysisAreaMapCanvas = new QgsMapCanvas( 0, 0 );
    analysisAreaMapCanvas->enableAntiAliasing( true );
    analysisAreaMapCanvas->setCanvasColor( Qt::white );
    analysisAreaMapCanvas->freeze( false );
    analysisAreaMapCanvas->setVisible( true );
    /*
    ** XXX: Need to set dst crs for warping on the fly, but this may mess with
    ** how we handle extents and zoom and selection.
    */
    /*
    QgsCoordinateReferenceSystem crs;
    crs.createFromSrid( 4269 );
    qDebug() << "CRS valid: " << crs.isValid();
    analysisAreaMapCanvas->setDestinationCrs( crs );
    analysisAreaMapCanvas->setCrsTransformEnabled( true );
    */
    analysisAreaMapCanvas->refresh();
    analysisAreaMapLayout = new QVBoxLayout( ui->analysisAreaMapFrame );
    analysisAreaMapLayout->addWidget( analysisAreaMapCanvas );

    analysisPanTool = new QgsMapToolPan( analysisAreaMapCanvas );
    analysisZoomInTool = new QgsMapToolZoom( analysisAreaMapCanvas, FALSE );
    analysisZoomOutTool = new QgsMapToolZoom( analysisAreaMapCanvas, TRUE);
    analysisIdentifyTool = new WfipsIdentifyMapTool( analysisAreaMapCanvas );
    connect( analysisIdentifyTool, SIGNAL( WfipsIdentify( QList<QgsMapToolIdentify::IdentifyResult> ) ),
             this, SLOT( Identify( QList<QgsMapToolIdentify::IdentifyResult> ) ) );
    analysisSelectTool = new WfipsSelectMapTool( analysisAreaMapCanvas );
    connect( analysisSelectTool, SIGNAL( WfipsSelect( qint64 ) ),
             this, SLOT( SelectPoint( qint64 ) ) );
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
void WfipsMainWindow::AddAnalysisAreaLayer( QString path, QString layerName,
                                            bool useExtent )
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
    analysisLayer = new QgsVectorLayer( path, layerName, "ogr", true );
    if( !analysisLayer->isValid() )
    {
        delete analysisLayer;
        qDebug() << "Invalid layer";
        return;
    }
    analysisLayer->setReadOnly( true );
    QgsMapLayerRegistry::instance()->addMapLayer( analysisLayer, false );
    analysisLayers.append( analysisLayer );
    analysisMapCanvasLayers.append( QgsMapCanvasLayer( analysisLayer, true ) );
    ui->analysisAreaComboBox->addItem( layerName.toUpper() );
    if( useExtent )
    {
        analysisAreaMapCanvas->setExtent( analysisLayer->extent() );
    }
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
    bool useExtent = false;
    for( int i = 0; i < layerNames.size(); i++ )
    {
        AddAnalysisAreaLayer( wfipsPath + "/" + layerNames[i] + ".db" );
    }
    analysisAreaMapCanvas->setLayerSet( analysisMapCanvasLayers );
    analysisAreaMapCanvas->setExtent( ((QgsVectorLayer*)(QgsMapLayerRegistry::instance()->mapLayers().values().last()))->extent() );
    analysisAreaMapCanvas->refresh();
}

/*
** Allow the user to add a custom layer to run the simulation.  This may be any
** OGR supported datasource, and one layer may be selected.
*/
void WfipsMainWindow::AddCustomAnalysisArea()
{
    WfipsAddLayerDialog dialog( this );
    dialog.exec();
    qDebug() << "Loading " << dialog.GetFilePath() << ", " << dialog.GetLayerName();
    if( dialog.GetFilePath() == "" || dialog.GetLayerName() == "" )
    {
        qDebug() << "Invalid Layer file or layer name!";
        return;
    }
    AddAnalysisAreaLayer( dialog.GetFilePath(), dialog.GetLayerName(), true );
    if( analysisMapCanvasLayers.size() == 1 )
        analysisAreaMapCanvas->setLayerSet( analysisMapCanvasLayers );
    ui->analysisAreaComboBox->setCurrentIndex( ui->analysisAreaComboBox->count() - 1 );
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
    analysisAreaMapCanvas->setCurrentLayer( analysisLayers[index] );
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
            ui->stackedWidget->setCurrentIndex( 0 );
            break;
        case 2:
            ui->stackedWidget->setCurrentIndex( 1 );
            ui->mapToolFrame->setEnabled( true );
            break;
        case 3:
            ui->stackedWidget->setCurrentIndex( 2 );
            break;
        case 4:
            ui->stackedWidget->setCurrentIndex( 3 );
            break;
        case 5:
            ui->stackedWidget->setCurrentIndex( 4 );
            ui->mapToolFrame->setEnabled( true );
            break;
        case 6:
            ui->stackedWidget->setCurrentIndex( 5 );
            break;
        case 7:
            ui->stackedWidget->setCurrentIndex( 6 );
            ui->mapToolFrame->setEnabled( true );
            break;
        case 8:
            ui->stackedWidget->setCurrentIndex( 7 );
            break;
        case 9:
            ui->stackedWidget->setCurrentIndex( 8 );
            break;
        case 10:
            ui->stackedWidget->setCurrentIndex( 9 );
            break;
        case 11:
            ui->stackedWidget->setCurrentIndex( 10 );
            break;
        case 12:
            ui->stackedWidget->setCurrentIndex( 11 );
            break;
        case 13:
            ui->stackedWidget->setCurrentIndex( 12 );
            break;
        case 14:
            ui->stackedWidget->setCurrentIndex( 13 );
            break;
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            break;
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
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
    /*
    ** Clear our selections on current layer
    ** XXX: Should we clear all layers?
    ** XXX: Should we add a fx that does this, it is probably worth it.
    */
    QgsVectorLayer *layer;
    layer =
        reinterpret_cast<QgsVectorLayer*>( analysisAreaMapCanvas->currentLayer() );
    if( layer != NULL )
    {
        layer->removeSelection();
        selectedFid = -1;
    }
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
    else if( ui->mapIdentifyToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to identify";
        analysisAreaMapCanvas->setMapTool( analysisIdentifyTool );
    }
    else if( ui->mapSelectToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to select";
        analysisAreaMapCanvas->setMapTool( analysisSelectTool );
    }
}

void WfipsMainWindow::Identify( QList<QgsMapToolIdentify::IdentifyResult> results )
{
    if( results.size() > 0 )
    {
        identifyDialog->ShowIdentifyResults( results );
    }
}

void WfipsMainWindow::SelectPoint( qint64 fid )
{
    qDebug() << "Selecting fid: " << fid;
    QgsVectorLayer *layer;
    layer =
        reinterpret_cast<QgsVectorLayer*>( analysisAreaMapCanvas->currentLayer() );
    if( layer == NULL )
    {
        selectedFid = -1;
        return;
    }
    layer->removeSelection();
    /* 
    ** fid -1 if results are invalid, but we still want to clear the selection
    */
    if( fid > -1 )
    {
        layer->select( fid );
    }
    selectedFid = fid;
}

void WfipsMainWindow::ZoomToLayerExtent()
{
    qDebug() << "Zoom to layer extent";
    QgsVectorLayer *layer;
    layer =
        reinterpret_cast<QgsVectorLayer*>( analysisAreaMapCanvas->currentLayer() );
    if( layer == NULL )
    {
        return;
    }
    QgsRectangle rectangle = layer->extent();
    analysisAreaMapCanvas->setExtent( rectangle );
    analysisAreaMapCanvas->refresh();
}

/*
** Use the selected feature and set the other maps.
*/
void WfipsMainWindow::SetAnalysisArea()
{
    if( selectedFid < 0 )
    {
        return;
    }
    qDebug() << "Setting analysis area using fid: " << selectedFid;
    QgsVectorLayer *layer;
    layer =
        reinterpret_cast<QgsVectorLayer*>( analysisAreaMapCanvas->currentLayer() );
    if( layer == NULL )
    {
        qDebug() << "Invalid layer in SetAnalysisArea()";
        return;
    }
    QgsFeatureList features;
    features = layer->selectedFeatures();
    if( features.size() < 1 )
    {
        qDebug() << "No selected features";
        return;
    }
    QgsFeature feature = features[0];
    layer->setSubsetString( "" );
    layer->setSubsetString( QString( "FID=" ) + QString::number( selectedFid ) );
    QgsRectangle extent = feature.geometry()->boundingBox();
    extent.scale( 1.1 );
    analysisAreaMapCanvas->setExtent( extent );
    analysisAreaMapCanvas->refresh();
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

