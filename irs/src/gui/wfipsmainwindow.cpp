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
    ConstructDispatchWidgets();

    ConstructTreeWidget();
    AssignTreeWidgetIndices( ui->treeWidget->invisibleRootItem() );
    qDebug() << "Found " << treeWidgetList.size() << " tree widget items.";

    identifyDialog = new WfipsIdentifyDialog( this );

    /* Initialize global mem layers for free */
    analysisAreaMemLayer = NULL;
    dispatchLocationMemLayer = NULL;

    /* Call *after* construction */
    CreateConnections();
    PostConstructionActions();
    ReadSettings();
    this->setWindowIcon( QIcon( ":/osu" ) );
    ui->progressBar->setRange( 0, 100 );
    ui->progressBar->setValue( 0 );
    this->statusBar()->showMessage( "Welcome to WFIPS...", 5000 );
}

WfipsMainWindow::~WfipsMainWindow()
{
    //QgsMapLayerRegistry::instance()->removeAllMapLayers();
    delete ui;
    delete identifyDialog;

    delete analysisAreaMapCanvas;
    delete analysisAreaMapLayout;

    delete analysisPanTool;
    delete analysisZoomInTool;
    delete analysisZoomOutTool;
    delete analysisIdentifyTool;
    delete analysisSelectTool;

    delete analysisAreaMemLayer;
    delete dispatchLocationMemLayer;
}

void WfipsMainWindow::WriteSettings()
{
    QSettings settings( QSettings::NativeFormat, QSettings::UserScope,  "firelab", "wfips" );
    settings.setValue( "wfipsdatapath", wfipsPath );
    settings.setValue( "customlayerpath", customLayerPath );
    settings.setValue( "analysisbuffer", ui->bufferAnalysisSpinBox->value() );
    settings.setValue( "useanalysisbuffer", ui->bufferAnalysisCheckBox->isChecked() );
}

void WfipsMainWindow::ReadSettings()
{
    QSettings settings( QSettings::NativeFormat, QSettings::UserScope,  "firelab", "wfips" );
    if( settings.contains( "wfipsdatapath" ) )
    {
        wfipsPath = settings.value( "wfipsdatapath" ).toString();
        if( wfipsPath != "" )
        {
            ui->openWfipsPathLineEdit->setText( wfipsPath );
            LoadAnalysisAreaLayers();
        }
    }
    if( settings.contains( "customlayerpath" ) )
    {
        customLayerPath = settings.value( "customlayerpath" ).toString();
    }
    if( settings.contains( "analysisbuffer" ) )
    {
        ui->bufferAnalysisSpinBox->setValue( settings.value( "analysisbuffer" ).toDouble() );
    }
    if( settings.contains( "useanalysisbuffer" ) )
    {
        ui->bufferAnalysisCheckBox->setChecked( settings.value( "useanalysisbuffer" ).toBool() );
    }
}

void WfipsMainWindow::closeEvent( QCloseEvent *event )
{
    WriteSettings();
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
    ui->treeWidget->setCurrentItem( ui->treeWidget->topLevelItem( 0 ) );
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
    crs.createFromSrid( 4269 );
    qDebug() << "CRS valid: " << crs.isValid();
    transform.setDestCRS( crs );
    analysisAreaMapCanvas->setDestinationCrs( crs );
    analysisAreaMapCanvas->setCrsTransformEnabled( true );
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
    connect( analysisSelectTool, SIGNAL( WfipsSelect( QgsFeatureIds ) ),
             this, SLOT( Select( QgsFeatureIds ) ) );
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
    QgsMapLayerRegistry::instance()->addMapLayer( analysisLayer, true );
    analysisLayers.append( analysisLayer );
    analysisMapCanvasLayers.append( QgsMapCanvasLayer( analysisLayer, false ) );
    ui->analysisAreaComboBox->addItem( layerName.toUpper() );
    if( useExtent )
    {
        transform.setSourceCrs( analysisLayer->crs() );
        analysisAreaMapCanvas->setExtent( transform.transformBoundingBox( analysisLayer->extent() ) );
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
    ui->analysisAreaComboBox->clear();
    /* Clean up and remove existing layers */
    QString layerId;
    for( int i = 0; i < analysisMapCanvasLayers.size(); i++ )
    {
        layerId = analysisMapCanvasLayers[i].layer()->id();
        QgsMapLayerRegistry::instance()->removeMapLayer( layerId );
    }
    analysisLayers.clear();
    analysisMapCanvasLayers.clear();
    QStringList layerNames;
    layerNames << "gacc" << "forest" << "district" << "us_state" << "us_county";
    bool useExtent = false;
    for( int i = 0; i < layerNames.size(); i++ )
    {
        AddAnalysisAreaLayer( wfipsPath + "/" + layerNames[i] + ".db" );
    }
    analysisAreaMapCanvas->setLayerSet( analysisMapCanvasLayers );
    if( analysisMapCanvasLayers.size() > 0 )
    {
        analysisAreaMapCanvas->setExtent( ((QgsVectorLayer*)(QgsMapLayerRegistry::instance()->mapLayers().values().last()))->extent() );
        /* XXX: Touchy */
        ui->treeWidget->setCurrentItem( ui->treeWidget->itemBelow( ui->treeWidget->currentItem() ) );
    }
    else
    {
        qDebug() << "Failed to load any layers from: " << wfipsPath;
        wfipsPath = "";
    }
    analysisAreaMapCanvas->refresh();
}

/*
** Allow the user to add a custom layer to run the simulation.  This may be any
** OGR supported datasource, and one layer may be selected.
*/
void WfipsMainWindow::AddCustomAnalysisArea()
{
    WfipsAddLayerDialog dialog( customLayerPath, this );
    dialog.exec();
    qDebug() << "Loading " << dialog.GetFilePath() << ", " << dialog.GetCurrentLayer();
    if( dialog.GetFilePath() == "" || dialog.GetCurrentLayer() == "" )
    {
        qDebug() << "Invalid Layer file or layer name!";
        return;
    }
    customLayerPath = QFileInfo( dialog.GetFilePath() ).absolutePath();
    AddAnalysisAreaLayer( dialog.GetFilePath(), dialog.GetCurrentLayer(), true );
    if( analysisMapCanvasLayers.size() == 1 )
        analysisAreaMapCanvas->setLayerSet( analysisMapCanvasLayers );
    ui->analysisAreaComboBox->setCurrentIndex( ui->analysisAreaComboBox->count() - 1 );
    if( ui->setAnalysisAreaToolButton->isChecked() )
    {
        SetAnalysisArea();
    }
    analysisAreaMapCanvas->refresh();
}

void WfipsMainWindow::UpdateAnalysisAreaMap( int index )
{
    if( index < 0 )
    {
        return;
    }
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
    /* Unselect, clear, etc. */
    SetAnalysisArea();
    analysisAreaMapCanvas->refresh();
}

void WfipsMainWindow::ConstructDispatchWidgets()
{
    dispatchMapCanvas = new QgsMapCanvas( 0, 0 );
    dispatchMapCanvas->enableAntiAliasing( true );
    dispatchMapCanvas->setCanvasColor( Qt::white );
    dispatchMapCanvas->setDestinationCrs( crs );
    dispatchMapCanvas->freeze( false );
    dispatchMapCanvas->setVisible( true );
    dispatchMapCanvas->refresh();
    dispatchMapLayout = new QVBoxLayout( ui->dispatchMapFrame );
    dispatchMapLayout->addWidget( dispatchMapCanvas );

    /* Map tools */
    dispatchPanTool = new QgsMapToolPan( dispatchMapCanvas );
    dispatchZoomInTool = new QgsMapToolZoom( dispatchMapCanvas, FALSE );
    dispatchZoomOutTool = new QgsMapToolZoom( dispatchMapCanvas, TRUE);
    dispatchIdentifyTool = new WfipsIdentifyMapTool( dispatchMapCanvas );
    connect( dispatchIdentifyTool, SIGNAL( WfipsIdentify( QList<QgsMapToolIdentify::IdentifyResult> ) ),
             this, SLOT( Identify( QList<QgsMapToolIdentify::IdentifyResult> ) ) );
    dispatchSelectTool = new WfipsSelectMapTool( dispatchMapCanvas );
    //connect( dispatchSelectTool, SIGNAL( WfipsSelect( QgsFeatureIds ) ),
             //this, SLOT( Select( QgsFeatureIds ) ) );
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
            currentMapCanvas = analysisAreaMapCanvas;
            break;
        case 3:
            ui->stackedWidget->setCurrentIndex( 2 );
            break;
        case 4:
            ui->stackedWidget->setCurrentIndex( 3 );
            ui->mapToolFrame->setEnabled( true );
            currentMapCanvas = dispatchMapCanvas;
            break;
        case 5:
            ui->stackedWidget->setCurrentIndex( 4 );
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
    if( ui->mapPanToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to pan";
        analysisAreaMapCanvas->setMapTool( analysisPanTool );
        dispatchMapCanvas->setMapTool( dispatchPanTool );
    }
    else if( ui->mapZoomInToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to zoom in";
        analysisAreaMapCanvas->setMapTool( analysisZoomInTool );
        dispatchMapCanvas->setMapTool( dispatchZoomInTool );
    }
    else if( ui->mapZoomOutToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to zoom out";
        analysisAreaMapCanvas->setMapTool( analysisZoomOutTool );
        dispatchMapCanvas->setMapTool( dispatchZoomOutTool );
    }
    else if( ui->mapIdentifyToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to identify";
        analysisAreaMapCanvas->setMapTool( analysisIdentifyTool );
        dispatchMapCanvas->setMapTool( dispatchIdentifyTool );
    }
    else if( ui->mapSelectToolButton->isChecked() )
    {
        qDebug() << "Setting map tool to select";
        analysisAreaMapCanvas->setMapTool( analysisSelectTool );
        dispatchMapCanvas->setMapTool( dispatchSelectTool );
    }
}

void WfipsMainWindow::Identify( QList<QgsMapToolIdentify::IdentifyResult> results )
{
    if( results.size() > 0 )
    {
        identifyDialog->ShowIdentifyResults( results );
    }
}

void WfipsMainWindow::Select( QgsFeatureIds fids )
{
    qDebug() << "Selecting fids: " << fids;
    QgsVectorLayer *layer;
    layer =
        reinterpret_cast<QgsVectorLayer*>( analysisAreaMapCanvas->currentLayer() );
    if( layer == NULL )
    {
        ClearAnalysisAreaSelection();
        return;
    }
    layer->removeSelection();
    if( fids.size() > 0 )
    {
        layer->select( fids );
        ui->setAnalysisAreaToolButton->setEnabled( true );
    }
    else
    {
        ui->setAnalysisAreaToolButton->setDisabled( true );
    }
    selectedFids = fids;
}

int WfipsMainWindow::WfipsIsVisible( QgsMapLayer *layer )
{
    int i = 0;
    if( currentMapCanvas == analysisAreaMapCanvas )
    {
        qDebug() << "Checking for visible layers in analysis area canvas";
        for( i = 0; i < analysisMapCanvasLayers.size(); i++ )
        {
            if( layer == analysisMapCanvasLayers[i].layer() )
            {
                qDebug() << "Found layer: " << layer->name();
                return 1;
            }
        }
    }
    else if( currentMapCanvas == dispatchMapCanvas )
    {
        qDebug() << "Checking for visible layers in dispatch canvas";
        for( i = 0; i < dispatchMapCanvasLayers.size(); i++ )
        {
            if( layer == dispatchMapCanvasLayers[i].layer() )
            {
                qDebug() << "Found layer: " << layer->name();
                return 1;
            }
        }
    }
    return 0;
}

void WfipsMainWindow::ZoomToLayerExtent()
{
    qDebug() << "Zoom to layer extent";
    QgsVectorLayer *layer;
    QgsMapLayer *mapLayer;
    QgsRectangle lextent;
    QgsRectangle extent;
    QList<QgsMapLayer*>mapLayers = currentMapCanvas->layers();
    for( int i = 0; i < mapLayers.size(); i++ )
    {
        layer = reinterpret_cast<QgsVectorLayer*>( mapLayers[i] );
        mapLayer = reinterpret_cast<QgsMapLayer*>( mapLayers[i] );
        if( !WfipsIsVisible( mapLayer ) )
        {
            continue;
        }
        if( layer->crs() != crs )
        {
            transform.setSourceCrs( layer->crs() );
            lextent = transform.transformBoundingBox( layer->extent() );
        }
        else
        {
            lextent = layer->extent();
        }
        extent.combineExtentWith( &lextent );
    }
    extent.scale( 1.1 );
    currentMapCanvas->setExtent( extent );
    currentMapCanvas->refresh();
}

/*
** Couldn't find a way to try to get this in QGIS, so we have an OGR stub.
** This apparently doesn't work all the time, although I am unclear on why OGR
** Drivers don't usually reture a valid column.
*/
static const char * OGRGetFIDColumn( const char *pszUrl )
{
    char **papszTokens =
        CSLTokenizeString2( pszUrl, "|=", CSLT_STRIPENDSPACES | CSLT_STRIPLEADSPACES );
    int n = CSLCount( papszTokens );
    if( n < 1 )
    {
        return NULL;
    }
    OGRDataSourceH hDS;
    OGRLayerH hLayer;

    hDS = OGROpen( papszTokens[0], FALSE, NULL );
    if( hDS == NULL )
    {
        CSLDestroy( papszTokens );
        return NULL;
    }
    if( n > 2 && EQUAL( "layername", papszTokens[1] ) )
    {
        hLayer = OGR_DS_GetLayerByName( hDS, papszTokens[2] );
    }
    else
    {
        hLayer = OGR_DS_GetLayer( hDS, 0 );
    }
    if( hLayer == NULL )
    {
        CSLDestroy( papszTokens );
        return NULL;
    }
    CSLDestroy( papszTokens );
    const char *pszFidCol = OGR_L_GetFIDColumn( hLayer );
    if( !EQUAL( pszFidCol, "" ) )
    {
        pszFidCol = strdup( pszFidCol );
    }
    else
    {
        /* For now, default to OGC_FID */
        pszFidCol = strdup( "OGC_FID" );
    }
    OGR_DS_Destroy( hDS );
    /* To be free'd, ahole */
    return pszFidCol;
}

static QString BuildFidSet( const char *pszFidCol, QgsFeatureIds fids )
{
    qDebug() << "Setting filter using col: " << pszFidCol;
    QString fidset = QString( pszFidCol ) + " IN (";
    QSetIterator<qint64>it( fids );
    while( it.hasNext() )
    {
        fidset += FID_TO_STRING( it.next() );
        if( it.hasNext()  )
        {
            fidset += ",";
        }
    }
    fidset += ")";
    return fidset;
}

void WfipsMainWindow::ClearAnalysisAreaSelection()
{
    QgsVectorLayer *layer;
    for( int i = 0; i < analysisMapCanvasLayers.size(); i++ )
    {
        layer = reinterpret_cast<QgsVectorLayer*>( analysisMapCanvasLayers[i].layer() );
        if( layer != NULL )
        {
            layer->removeSelection();
            layer->setSubsetString( "" );
        }
    }
    ((WfipsSelectMapTool*)analysisSelectTool)->clear();

    /* Dispatch Layer */
    if( dispatchMapCanvasLayers.size() > 0 )
    {
        dispatchMapCanvasLayers.clear();
        QgsMapLayerRegistry::instance()->removeMapLayer( analysisAreaMemLayer->id() );
        dispatchMapCanvas->refresh();
    }
}

static QgsGeometry * BufferGeomConcurrent( QgsGeometry *geometry, const double buf,
                                           int segmentApprox )
{
    return geometry->buffer( buf, segmentApprox );
}

/*
** Copy a layer or a subset of a layer to an in memory datasource.  Currently
** we only use this for the dispatch location layer, so we only use it for
** Point layers.  Some logic needs to be added for other layers, if needed.
**
** Subsetting is done by supplying feature ids to be selected.
*/

static QgsVectorLayer * WfipsCopyToMemLayer( QgsVectorLayer *layer,
                                             QgsFeatureIds fids )
{
    if( !layer->isValid() )
    {
        return NULL;
    }
    int rc;
    /*
    ** XXX: Handle different geometry types.  Map from wkb to memory provider
    */
    QString uri = "Point";
    /* crs is our NAD 83, index for the hell of it */
    uri += "?crs=EPSG:4269&index=yes";
    /* Copy the fields */
    QgsFields fields = layer->dataProvider()->fields();

    QgsVectorLayer *memLayer = new QgsVectorLayer( uri, layer->name(), "memory", true );
    assert( memLayer->isValid() );
    QgsVectorDataProvider *provider = memLayer->dataProvider();
    rc = provider->addAttributes( fields.toList() );
    assert( rc == true );
    for( int i = 0; i < fields.size(); i++ )
    {
        qDebug() << "Copying field: " << fields[i].name();
    }

    memLayer->commitChanges();
    QgsFeature feature;
    QgsFeatureList features;
    QgsFeatureRequest request;
    if( fids.size() > 0 )
    {
        request.setFilterFids( fids );
    }
    /* request default is QgsFeatureRequest(), so we should be good here */
    QgsFeatureIterator fit = layer->getFeatures( request );
    while( fit.nextFeature( feature ) )
    {
        features.append( feature );
    }
    qDebug() << "Fetched " << features.size() << " features from " << layer->name();
    provider->addFeatures( features );
    memLayer->updateExtents();
    memLayer->commitChanges();

    return memLayer;
}

/*
** Use the selected feature and set the other maps.
*/
void WfipsMainWindow::SetAnalysisArea()
{
    QgsVectorLayer *layer;
    layer =
        reinterpret_cast<QgsVectorLayer*>( analysisAreaMapCanvas->currentLayer() );
    if( layer == NULL )
    {
        qDebug() << "Invalid layer in SetAnalysisArea()";
        ui->setAnalysisAreaToolButton->setText( "Set Analysis Area" );
        ui->setAnalysisAreaToolButton->setChecked( false );
        return;
    }
    if( !ui->setAnalysisAreaToolButton->isChecked() )
    {
        layer->setSubsetString( "" );
        ClearAnalysisAreaSelection();
        analysisAreaMapCanvas->refresh();
        ui->setAnalysisAreaToolButton->setText( "Set Analysis Area" );
        ui->setAnalysisAreaToolButton->setChecked( false );
        return;
    }
    if( selectedFids.size() < 1 )
    {
        ui->setAnalysisAreaToolButton->setText( "Set Analysis Area" );
        ui->setAnalysisAreaToolButton->setChecked( false );
        ClearAnalysisAreaSelection();
        return;
    }
    qDebug() << "Setting analysis area using fids: " << selectedFids;
    if( selectedFids.size() < 1 )
    {
        qDebug() << "No selected features";
        ui->setAnalysisAreaToolButton->setText( "Set Analysis Area" );
        ui->setAnalysisAreaToolButton->setChecked( false );
        ClearAnalysisAreaSelection();
        return;
    }

    QgsFeatureList features;
    QgsFeatureRequest request;
    request.setFilterFids( selectedFids );
    QgsFeatureIterator fit = layer->getFeatures( request );
    QList<QgsGeometry*>newGeometries;
    QgsFeature feature;
    while( fit.nextFeature( feature ) )
    {
        newGeometries.append( new QgsGeometry( *(feature.geometry()) ) );
    }
    QgsGeometry *multi = QgsGeometry::unaryUnion( newGeometries );
    QFuture<QgsGeometry*>future;
    QgsFeature analysisFeature;
    if( ui->bufferAnalysisCheckBox->isChecked() && ui->bufferAnalysisSpinBox->value() > 0 )
    {
        this->statusBar()->showMessage( "Buffering  geometries..." );
        /* Just busy */
        ui->progressBar->setRange( 0, 0 );
        ui->progressBar->setValue( 1 );
        future = QtConcurrent::run( BufferGeomConcurrent, multi, ui->bufferAnalysisSpinBox->value(), 2 );
        //BufferGeomConcurrent( multi, ui->bufferAnalysisSpinBox->value(), 2 );
        //buffered = multi->buffer( ui->bufferAnalysisSpinBox->value(), 2 );
        future.waitForFinished();
        ui->progressBar->setRange( 0, 100 );
        ui->progressBar->setValue( 0 );
        this->statusBar()->showMessage( "Buffering finished.", 1500 );
        analysisFeature.setGeometry( future.results()[0] );
    }
    else
    {
        analysisFeature.setGeometry( multi );
    }
    QgsGeometry *analisysAreaGeometry = new QgsGeometry( *(analysisFeature.geometry()) );
    features.append( analysisFeature );

    analysisAreaMemLayer = new QgsVectorLayer( "MultiPolygon?crs=EPSG:4269", "Analysis Area", "memory", true );
    assert( analysisAreaMemLayer->isValid() );
    QgsVectorDataProvider *provider;
    provider = analysisAreaMemLayer->dataProvider();
    analysisAreaMemLayer->setReadOnly( true );

    //QgsFields fields = layer->dataProvider()->fields();
    //provider->addAttributes( fields.toList() );
    provider->addFeatures( features );
    analysisAreaMemLayer->updateExtents();
    QgsMapLayerRegistry::instance()->addMapLayer( analysisAreaMemLayer, true );

    /* Dispatch Location mem layer */
    QString dlUri = wfipsPath + "/disploc.db|layername=disploc";
    layer = new QgsVectorLayer( dlUri, "Dispatch Locations", "ogr", true );
    assert( layer->isValid() );
    layer->setReadOnly( true );

    /*
    ** Subset the dispatch locations.  We may have to do this by hand?  Iterate
    ** through the features and grab the FIDs of the contained locations.
    */
    fit = layer->getFeatures();
    QgsFeatureIds fids;
    int i, n;
    i = 0; n = layer->featureCount();
    this->statusBar()->showMessage( "Searching for dispatch locations..." );
    while( fit.nextFeature( feature ) )
    {
        if( analisysAreaGeometry->boundingBox().contains( feature.geometry()->asPoint() ) )
        {
            if( analisysAreaGeometry->contains( feature.geometry() ) )
            {
                fids.insert( feature.id() );
            }
        }
        i++;
        ui->progressBar->setValue( (int)((float)i / n) * 100 );
        QCoreApplication::processEvents();
    }
    ui->progressBar->reset();
    this->statusBar()->showMessage( "Found " + fids.size() + QString( " locations." ), 3000 );
    qDebug() << "Found " << fids.size() << " dispatch locations within the analysis area";

    dispatchLocationMemLayer = WfipsCopyToMemLayer( layer, fids );
    assert( dispatchLocationMemLayer->isValid() );
    QgsMapLayerRegistry::instance()->addMapLayer( dispatchLocationMemLayer, true );
    delete analisysAreaGeometry;
    delete layer;
    for( int i = 0; i < newGeometries.size(); i++ )
    {
        delete newGeometries[i];
    }

    AddAnalysisLayerToCanvases();

    QgsRectangle extent = analysisAreaMemLayer->extent();
    if( layer->crs() != crs )
    {
        transform.setSourceCrs( layer->crs() );
        extent = transform.transformBoundingBox( analisysAreaGeometry->boundingBox() );
    }
    extent.scale( 1.1 );

    analysisAreaMapCanvas->setExtent( extent );
    analysisAreaMapCanvas->refresh();

    ui->setAnalysisAreaToolButton->setText( "Clear Analysis Area" );
}

void WfipsMainWindow::AddAnalysisLayerToCanvases()
{
    dispatchMapCanvasLayers.append( QgsMapCanvasLayer( dispatchLocationMemLayer, true ) );
    dispatchMapCanvasLayers.append( QgsMapCanvasLayer( analysisAreaMemLayer, true ) );
    dispatchMapCanvas->setLayerSet( dispatchMapCanvasLayers );

    QgsRectangle extent = analysisAreaMemLayer->extent();
    extent.scale( 1.1 );
    dispatchMapCanvas->setExtent( extent );
    dispatchMapCanvas->setCurrentLayer( dispatchMapCanvasLayers[0].layer() );
    dispatchMapCanvas->refresh();
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

