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


WfipsMainWindow::WfipsMainWindow( QWidget *parent ) :
    QMainWindow( parent ),
    ui( new Ui::WfipsMainWindow )
{
    ui->setupUi( this );
    ui->treeWidget->expandAll();

    /* Initialize QGIS registry */
    QString qgisPluginPath = QGIS_PLUGIN_DIR;
    QgsProviderRegistry::instance( qgisPluginPath );
    qDebug() << "QGIS plugin path:" << qgisPluginPath;

    currentMapCanvas = NULL;
    poData = NULL;

    ConstructToolButtons();
    ConstructAnalysisAreaWidgets();
    ConstructDispatchWidgets();

    ConstructTreeWidget();
    AssignTreeWidgetIndices( ui->treeWidget->invisibleRootItem() );
    qDebug() << "Found" << treeWidgetList.size() << "tree widget items.";

    identifyDialog = new WfipsIdentifyDialog( this );

    /* Initialize global mem layers for free */
    analysisAreaMemLayer = NULL;
    dispatchLocationMemLayer = NULL;

    dispatchEditDialog = new WfipsDispatchEditDialog( this );

    /* Call *after* construction */
    CreateConnections();
    PostConstructionActions();
    ui->threadSpinBox->setRange( 1, QThread::idealThreadCount() );
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

    /* Analysis Area */
    delete analysisAreaMapCanvas;
    delete analysisAreaMapLayout;

    delete analysisPanTool;
    delete analysisZoomInTool;
    delete analysisZoomOutTool;
    delete analysisIdentifyTool;
    delete analysisSelectTool;

    delete analysisAreaMemLayer;

    /* Dispatch Locations */
    delete dispatchMapCanvas;
    delete dispatchPanTool;
    delete dispatchZoomInTool;
    delete dispatchZoomOutTool;
    delete dispatchIdentifyTool;
    delete dispatchSelectTool;

    delete dispatchLocationMemLayer;

    delete dispatchEditDialog;
    delete poData;
}

void WfipsMainWindow::WriteSettings()
{
    QSettings settings( QSettings::NativeFormat, QSettings::UserScope,  "firelab", "wfips" );
    settings.setValue( "wfipsdatapath", wfipsPath );
    settings.setValue( "customlayerpath", customLayerPath );
    settings.setValue( "analysisbuffer", ui->bufferAnalysisSpinBox->value() );
    settings.setValue( "useanalysisbuffer", ui->bufferAnalysisCheckBox->isChecked() );
    if( currentMapCanvas != NULL )
    {
        QgsRectangle extent = currentMapCanvas->extent();
        settings.setValue( "xmin", extent.xMinimum() );
        settings.setValue( "xmax", extent.xMaximum() );
        settings.setValue( "ymin", extent.yMinimum() );
        settings.setValue( "ymax", extent.yMaximum() );
    }
    QString lyr = ui->analysisAreaComboBox->currentText();
    if( lyr != "ANALYSIS AREA" )
    {
        settings.setValue( "defaultlayer", lyr );
    }
    int nThreadCount = ui->threadSpinBox->value();
    settings.setValue( "threadcount", ui->threadSpinBox->value() );
    settings.setValue( "advancedrun", ui->advancedRunGroupBox->isChecked() );
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
            dispatchEditDialog->SetDataPath( wfipsPath );
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
    if( settings.contains( "defaultlayer" ) )
    {
        QString lyr = settings.value( "defaultlayer" ).toString();
        int i = ui->analysisAreaComboBox->findText( lyr );
        if( i < 0 )
        {
            i = 0;
        }
        ui->analysisAreaComboBox->setCurrentIndex( i );
    }
    if( settings.contains( "threadcount" ) )
    {
        ui->threadSpinBox->setValue( settings.value( "threadcount" ).toInt() );
    }
    if( settings.contains( "advancedrun" ) )
    {
        ui->advancedRunGroupBox->setChecked( settings.value( "advancedrun" ).toBool() );
    }
    /* last */
    if( settings.contains( "xmin" ) &&
        settings.contains( "xmax" ) &&
        settings.contains( "ymin" ) &&
        settings.contains( "ymax" ) )
    {
        QgsRectangle extent( settings.value( "xmin" ).toDouble(),
                             settings.value( "ymin" ).toDouble(),
                             settings.value( "xmax" ).toDouble(),
                             settings.value( "ymax" ).toDouble() );
        analysisAreaMapCanvas->setExtent( extent );
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

    /* Tree navigation buttons */
    ui->treeNextButton->setIcon( QIcon( ":/go-next" ) );
    connect( ui->treeNextButton, SIGNAL( clicked() ),
             this, SLOT( NextTreeWidgetItem() ) );
    ui->treePreviousButton->setIcon( QIcon( ":/go-previous" ) );
    connect( ui->treePreviousButton, SIGNAL( clicked() ),
             this, SLOT( PrevTreeWidgetItem() ) );
    connect( ui->runButton, SIGNAL( clicked() ),
             this, SLOT( RunIrs() ) );

}

void WfipsMainWindow::PostConstructionActions()
{
    /* Make sure one of the map tools gets initialized */
    ui->mapPanToolButton->click();
    ui->treeWidget->setCurrentItem( ui->treeWidget->topLevelItem( 0 ) );
    /* When we select locations from the list, update the map */
    connect( dispatchEditDialog,
             SIGNAL( SelectionChanged( const QgsFeatureIds & ) ),
             this,
             SLOT( UpdateSelectedDispatchLocations( const QgsFeatureIds & ) ) );

    /*
    ** Toggle tool button for dispatch location editor when we close the dialog
    */
    connect( dispatchEditDialog, SIGNAL( Hiding() ),
             ui->dispatchEditToolButton, SLOT( toggle() ) );
    /*
    ** Hide dispatch locations on the analysis layer if they are omitted from
    ** the list.
    */
    connect( dispatchEditDialog, SIGNAL( HiddenChanged( QgsFeatureIds ) ),
             this, SLOT( HideDispatchLocations( QgsFeatureIds ) ) );

    /*
    ** Bump the fuel combo to align the enabler.
    */
    connect( ui->fuelComboBox, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( EnableCustomFuelMask( int ) ) );
    ui->fuelComboBox->setCurrentIndex( 1 );
    ui->fuelComboBox->setCurrentIndex( 0 );
    connect( ui->fuelMaskToolButton, SIGNAL( clicked() ),
             this, SLOT( SelectFuelMask() ) );
    connect( ui->fuelAttComboBox, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( EnableFuelMaskAttr( int ) ) );
    /* Toggle a radio button the ignitions to enable the proper widgets */
    ui->randomYearsRadioButton->click();
    ui->singleYearRadioButton->click();
    ui->figAllRadioButton->click();
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

    /* Dispatch location tool buttons */
    ui->dispatchEditToolButton->setIcon( QIcon( ":/action" ) );
    connect( ui->dispatchEditToolButton, SIGNAL( clicked() ),
             this, SLOT( ShowDispatchEditDialog() ) );
    ui->dispatchEditToolButton->setIcon( QIcon( ":/properties" ) );

    /* Fuel button */
    ui->fuelMaskToolButton->setIcon( QIcon( ":/add_layer" ) );
}

void WfipsMainWindow::ConstructAnalysisAreaWidgets()
{
    analysisAreaMapCanvas = new QgsMapCanvas( 0, 0 );
    analysisAreaMapCanvas->enableAntiAliasing( true );
    analysisAreaMapCanvas->setCanvasColor( Qt::white );
    analysisAreaMapCanvas->freeze( false );
    analysisAreaMapCanvas->setWheelAction( QgsMapCanvas::WheelZoomToMouseCursor );
    analysisAreaMapCanvas->setVisible( true );
    /*
    ** XXX: Need to set dst crs for warping on the fly, but this may mess with
    ** how we handle extents and zoom and selection.
    */
    crs.createFromSrid( 4269 );
    qDebug() << "CRS valid:" << crs.isValid();
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
    qDebug() << "Loading layer:" << layerName;
    analysisLayer = new QgsVectorLayer( path, layerName, "ogr", true );
    AddAnalysisAreaLayer( analysisLayer, useExtent );
}

void WfipsMainWindow::AddAnalysisAreaLayer( QgsVectorLayer *layer, bool useExtent )
{
    if( !analysisLayer->isValid() )
    {
        delete analysisLayer;
        qDebug() << "Invalid layer";
        return;
    }
    analysisLayer->setReadOnly( true );
    QgsMapLayerRegistry::instance()->addMapLayer( layer, true );
    analysisLayers.append( layer );
    analysisMapCanvasLayers.append( QgsMapCanvasLayer( layer, false ) );
    ui->analysisAreaComboBox->addItem( layer->name().toUpper() );
    if( useExtent )
    {
        transform.setSourceCrs( analysisLayer->crs() );
        analysisAreaMapCanvas->setExtent( transform.transformBoundingBox( layer->extent() ) );
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
    /*
    ** XXX
    ** This should probably be moved, or this fx renamed, as we do some stuff
    ** here just because we have a path.
    */
    poData = new WfipsData( wfipsPath.toLocal8Bit().data() );
    poData->Open();
    if( poData->Valid() == 0 )
    {
        /* Panic */
        delete poData;
        poData = NULL;
    }
    if( poData )
    {
        int nCount, *panIndices;
        nCount = poData->GetScenarioIndices( &panIndices );
        if( nCount > 0 )
        {
            ui->randomYearsSpinBox->setRange( 1, nCount );
            ui->singleYearComboBox->clear();
            for( int i = 0; i < nCount; i++ )
            {
                ui->singleYearComboBox->addItem( QString::number( panIndices[i] ) );
            }
            WfipsData::Free( panIndices );
        }
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
    layerNames << "gacc" << "forest" << "district" << "us_state" << "us_county"
               << "fpu";
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
        qDebug() << "Failed to load any layers from:" << wfipsPath;
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
    qDebug() << "Loading" << dialog.GetFilePath() << "," << dialog.GetCurrentLayer();
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
    //SetAnalysisArea();
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
    dispatchMapCanvas->setWheelAction( QgsMapCanvas::WheelZoomToMouseCursor );
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
    connect( dispatchSelectTool, SIGNAL( WfipsSelect( QgsFeatureIds ) ),
             this, SLOT( SelectDispatchLocations( QgsFeatureIds ) ) );
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
    qDebug() << "tree widget index:" << i;
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
        /* Resource map */
        case 3:
        case 4:
            ui->stackedWidget->setCurrentIndex( 2 );
            ui->mapToolFrame->setEnabled( true );
            currentMapCanvas = dispatchMapCanvas;
            break;
        case 5:
            ui->stackedWidget->setCurrentIndex( 3 );
            break;
        case 6:
            ui->stackedWidget->setCurrentIndex( 4 );
            break;
        case 7:
            ui->stackedWidget->setCurrentIndex( 5 );
            break;
        case 8:
            ui->stackedWidget->setCurrentIndex( 6 );
            break;
        case 9:
            ui->stackedWidget->setCurrentIndex( 7 );
            break;
        case 10:
            ui->stackedWidget->setCurrentIndex( 8 );
            break;
        case 11:
            ui->stackedWidget->setCurrentIndex( 9 );
            break;
        case 12:
            ui->stackedWidget->setCurrentIndex( 10 );
            break;
        case 13:
            ui->stackedWidget->setCurrentIndex( 11 );
            break;
        case 14:
            ui->stackedWidget->setCurrentIndex( 12 );
            break;
        case 15:
        case 16:
        case 17:
        case 18:
            break;
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
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
    dispatchEditDialog->SetDataPath( wfipsPath );
    if( wfipsPath == "" )
        return;
    qDebug() << "Using" << wfipsPath << "for data path.";
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
    if( ui->setAnalysisAreaToolButton->isChecked() )
    {
        qDebug() << "Clear selection before selecting new analysis area";
        return;
    }
    qDebug() << "Selecting fids:" << fids;
    QgsVectorLayer *layer;
    layer =
        dynamic_cast<QgsVectorLayer*>( analysisAreaMapCanvas->currentLayer() );
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
                qDebug() << "Found layer:" << layer->name();
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
                qDebug() << "Found layer:" << layer->name();
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
        layer = dynamic_cast<QgsVectorLayer*>( mapLayers[i] );
        mapLayer = dynamic_cast<QgsMapLayer*>( mapLayers[i] );
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
        if( i == 0 )
        {
            extent = lextent;
        }
        else
        {
            qDebug() << "Combining:" << extent.xMinimum() << ","
                                      << extent.xMaximum() << ","
                                      << extent.yMinimum() << ","
                                      << extent.yMaximum();
            qDebug() << "with:" << lextent.xMinimum() << ","
                                 << lextent.xMaximum() << ","
                                 << lextent.yMinimum() << ","
                                 << lextent.yMaximum();

            extent.combineExtentWith( &lextent );
        }
    }
    extent.scale( 1.1 );
    qDebug() << "Setting extent:" << extent.xMinimum() << ","
                                   << extent.xMaximum() << ","
                                   << extent.yMinimum() << ","
                                   << extent.yMaximum();
    currentMapCanvas->setExtent( extent );
    currentMapCanvas->refresh();
}

void WfipsMainWindow::ClearAnalysisAreaSelection()
{
    int rc;
    rc = QMessageBox::warning( this, tr("WFIPS" ),
                               tr( "All settings associated with the run will " \
                                   "be cleared, are you sure?" ),
                               QMessageBox::Ok | QMessageBox::Cancel );
    if( rc == QMessageBox::Cancel )
    {
        /* re-check the button */
        ui->setAnalysisAreaToolButton->setChecked( true );
        ui->setAnalysisAreaToolButton->setText( "Clear Analysis Area" );
        return;
    }
    QgsVectorLayer *layer;
    for( int i = 0; i < analysisMapCanvasLayers.size(); i++ )
    {
        layer = dynamic_cast<QgsVectorLayer*>( analysisMapCanvasLayers[i].layer() );
        if( layer != NULL )
        {
            layer->removeSelection();
            layer->setSubsetString( "" );
        }
    }

    int index = -1;
    for( int i = 0; i < analysisMapCanvasLayers.size(); i++ )
    {
        layer = dynamic_cast<QgsVectorLayer*>( analysisMapCanvasLayers[i].layer() );
        if( layer == analysisAreaMemLayer )
        {
            index = i;
            break;
        }
    }
    if( index != -1 )
    {
        analysisMapCanvasLayers.removeAt( index );
        QgsMapLayerRegistry::instance()->removeMapLayer( analysisAreaMemLayer->id() );
    }

    index = ui->analysisAreaComboBox->findText( "ANALYSIS AREA" );
    ui->analysisAreaComboBox->removeItem( index );
    analysisAreaMapCanvas->refresh();

    ((WfipsSelectMapTool*)analysisSelectTool)->clear();
    dispatchEditDialog->Clear();

    /* Dispatch Layer */
    if( dispatchMapCanvasLayers.size() > 0 )
    {
        dispatchLocationMap.clear();
        dispatchMapCanvasLayers.clear();
        QgsMapLayerRegistry::instance()->removeMapLayer( dispatchLocationMemLayer->id() );
        dispatchMapCanvas->refresh();
    }
    ui->setAnalysisAreaToolButton->setText( "Set Analysis Area" );
    ui->setAnalysisAreaToolButton->setChecked( false );
    ui->bufferAnalysisCheckBox->setEnabled( true );
    ui->bufferAnalysisSpinBox->setEnabled( true );
    selectedFids.clear();
    analysisAreaMapCanvas->refresh();
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
    memLayer->startEditing();
    QgsVectorDataProvider *provider = memLayer->dataProvider();
    for( int i = 0; i < fields.size(); i++ )
    {
        qDebug() << "Copying field:" << fields[i].name();
    }
    QgsField oidField( "ofid", QVariant::LongLong );
    fields.append( oidField );
    rc = provider->addAttributes( fields.toList() );
    assert( (bool)rc == true );
    assert( provider->fields().size() == layer->dataProvider()->fields().size() + 1 );

    //memLayer->commitChanges();
    QgsFeature feature;
    QgsFeature newFeature;
    QgsFeatureList features;
    QgsFeatureRequest request;
    if( fids.size() > 0 )
    {
        request.setFilterFids( fids );
    }
    QgsFeatureId fid;
    /* request default is QgsFeatureRequest(), so we should be good here */
    QgsFeatureIterator fit = layer->getFeatures( request );
    while( fit.nextFeature( feature ) )
    {
        fid = feature.id();
        newFeature = QgsFeature();
        newFeature.setFields( &fields, true );
        /*
        ** XXX Make sure fields are ordered.
        */
        for( int i = 0; i < fields.size() - 1; i++ )
        {
            rc = newFeature.setAttribute( i, feature.attribute( i ) );
        }
        rc = newFeature.setAttribute( fields.size() - 1, fid );
        newFeature.setGeometry( new QgsGeometry( *(feature.geometry()) ) );
        features.append( newFeature );
    }
    qDebug() << "Fetched" << features.size() << "features from" << layer->name();
    provider->addFeatures( features );
    memLayer->updateExtents();
    memLayer->commitChanges();

    return memLayer;
}

/*
** Dummy fx for miles to degrees.  Replace with a projection based system.
** This uses 78.71 km == 1.0 degrees longitude @ 45N.
*/
const static double MilesToDegrees( double miles )
{
    static const double kmToDeg = 78.71; /* longitude @ 45N */
    //static const double kmToDeg = 111.32 /* latitude */
    double km = miles / 0.62;
    return km / kmToDeg;
}

/*
** Use the selected feature and set the other maps.
*/
void WfipsMainWindow::SetAnalysisArea()
{
    if( !ui->setAnalysisAreaToolButton->isChecked() )
    {
        ClearAnalysisAreaSelection();
        return;
    }
    QgsVectorLayer *layer;
    layer =
        dynamic_cast<QgsVectorLayer*>( analysisAreaMapCanvas->currentLayer() );
    if( layer == NULL || !layer->isValid() || selectedFids.size() < 1 ||
        (selectedFids.size() == 1 && selectedFids.contains( -1 )  ) )
    {
        int rc;
        rc = QMessageBox::warning( this, tr("WFIPS" ),
                               tr( "No features have been selected on a valid " \
                                   "layer." ),
                               QMessageBox::Ok );
        ui->setAnalysisAreaToolButton->setChecked( false );

        return;
    }
    qDebug() << "Setting analysis area using fids:" << selectedFids;
    this->setDisabled( true );
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
    QgsGeometry *newGeometry = NULL;
    QFuture<QgsGeometry*>future;
    QgsFeature analysisFeature;
    if( ui->bufferAnalysisCheckBox->isChecked() && ui->bufferAnalysisSpinBox->value() > 0 )
    {
        /* Qt Concurrent */
        double m, d;
        m = ui->bufferAnalysisSpinBox->value();
        d = MilesToDegrees( m );
        qDebug() << "Using" << m << "miles," << d << "degrees for buffer";
        this->statusBar()->showMessage( "Buffering  geometries..." );
        ui->progressBar->setRange( 0, 0 );
        ui->progressBar->setValue( 0 );
        future = QtConcurrent::run( BufferGeomConcurrent, multi, d, 2 );
        int i = 0;
        while( !future.isFinished() && i < 1000 )
        {
            CPLSleep( 0.1 );
            ui->progressBar->setValue( 0 );
            QCoreApplication::processEvents();
            i++;
        }
        future.waitForFinished();
        ui->progressBar->setRange( 0, 100 );
        this->statusBar()->showMessage( "Buffering finished.", 1500 );
        ui->progressBar->reset();
        newGeometry = future.results()[0];
        //analysisFeature.setGeometry( future.results()[0] );
    }
    else
    {
        newGeometry = multi;
    }
    if( layer->crs() != crs )
    {
        newGeometry->transform( transform );
    }
    analysisFeature.setGeometry( newGeometry );
    QgsGeometry *analysisAreaGeometry = new QgsGeometry( *(analysisFeature.geometry()) );
    features.append( analysisFeature );

    analysisAreaMemLayer = new QgsVectorLayer( "MultiPolygon?crs=EPSG:4269", "Analysis Area", "memory", true );
    assert( analysisAreaMemLayer->isValid() );
    QgsVectorDataProvider *provider;
    provider = analysisAreaMemLayer->dataProvider();
    analysisAreaMemLayer->setReadOnly( true );

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
    int i, n, progress;
    i = 0; n = layer->featureCount();
    this->statusBar()->showMessage( "Searching for dispatch locations..." );
    dispatchLocationMap.clear();
    int *panLocIds, nLocCount;
    QString wkt = analysisAreaGeometry->exportToWkt();
    char *pszWkt = QStringToCString( wkt );
    if( poData == NULL )
    {
        /* panic */
    }
    QFuture<int>nFuture;
    this->statusBar()->showMessage( "Searching for dispatch locations...", 1500 );
    ui->progressBar->setRange( 0, 0 );
    nFuture = QtConcurrent::run( poData, &WfipsData::GetAssociatedDispLoc, pszWkt, &panLocIds, &nLocCount );
    poData->SetAnalysisAreaMask( pszWkt );
    i = 0;
    while( !nFuture.isFinished() && i < 1000 )
    {
        CPLSleep( 0.1 );
        ui->progressBar->setValue( 0 );
        QCoreApplication::processEvents();
        i++;
    }
    nFuture.waitForFinished();
    ui->progressBar->setRange( 0, 100 );
    this->statusBar()->showMessage( "Found locations.", 1500 );
    ui->progressBar->reset();
    for( int i = 0; i < nLocCount; i++ )
    {
        fids.insert( panLocIds[i] );
    }
    WfipsData::Free( panLocIds );
    free( pszWkt );
    request.setFilterFids( fids );
    fit = layer->getFeatures( request );
    while( fit.nextFeature( feature ) )
    {
        dispatchLocationMap.insert( feature.id(),
                                    feature.attribute( "name" ).toString() );
    }
    this->statusBar()->showMessage( "Adding new layers to map canvases..." );
    ui->progressBar->reset();
    this->statusBar()->showMessage( "Found" + fids.size() + QString( " locations." ), 3000 );
    qDebug() << "Found" << fids.size() << "dispatch locations within the analysis area";
    int agencies = 0;
    if( ui->agencyRescUsfsCheckBox->isChecked() )
        agencies |= USFS;
    if( ui->agencyRescDoiCheckBox->isChecked() )
        agencies |= DOI_ALL;
    if( ui->agencyRescStateCheckBox->isChecked() )
        agencies |= STATE_LOCAL;
    dispatchEditDialog->SetModel( dispatchLocationMap, agencies );

    dispatchLocationMemLayer = WfipsCopyToMemLayer( layer, fids );
    assert( dispatchLocationMemLayer->isValid() );
    QgsMapLayerRegistry::instance()->addMapLayer( dispatchLocationMemLayer, true );
    delete analysisAreaGeometry;
    for( int i = 0; i < newGeometries.size(); i++ )
    {
        delete newGeometries[i];
    }

    AddAnalysisLayerToCanvases();

    QgsRectangle extent = analysisAreaMemLayer->extent();
    if( layer->crs() != crs )
    {
        transform.setSourceCrs( layer->crs() );
        extent = transform.transformBoundingBox( analysisAreaGeometry->boundingBox() );
    }
    extent.scale( 1.1 );

    analysisAreaMapCanvas->setExtent( extent );
    analysisAreaMapCanvas->refresh();

    delete layer;

    ui->setAnalysisAreaToolButton->setText( "Clear Analysis Area" );
    ui->bufferAnalysisCheckBox->setDisabled( true );
    ui->bufferAnalysisSpinBox->setDisabled( true );
    ui->analysisAreaComboBox->setCurrentIndex( ui->analysisAreaComboBox->count() - 1 );
    this->statusBar()->showMessage( "Done.", 3000 );
    this->setEnabled( true );
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

    AddAnalysisAreaLayer( analysisAreaMemLayer, true );
}

void WfipsMainWindow::ShowDispatchEditDialog()
{
    if( !ui->dispatchEditToolButton->isChecked() )
    {
        dispatchEditDialog->hide();
        return;
    }
    dispatchEditDialog->show();
    return;
}

void WfipsMainWindow::SelectDispatchLocations( QgsFeatureIds fids )
{
    QgsFeatureIds dlfids;
    QgsFeature feature;
    QgsFeatureRequest request;
    request.setFilterFids( fids );
    QgsFeatureId fid;
    QgsFeatureIterator fit;
    if( dispatchLocationMemLayer == NULL || !dispatchLocationMemLayer->isValid() )
    {
        return;
    }
    dispatchLocationMemLayer->removeSelection();
    dispatchLocationMemLayer->select( fids );
    fit = dispatchLocationMemLayer->getFeatures( request );
    while( fit.nextFeature( feature ) )
    {
        fid = feature.attribute( "ofid" ).toLongLong();
        dlfids.insert( fid );
        qDebug() << "Local fid:" << feature.id() << ", original:" << fid;
    }
    dispatchEditDialog->SelectFids( dlfids );
    if( !ui->dispatchEditToolButton->isChecked() )
    {
        ui->dispatchEditToolButton->click();
    }
    return;
}

static QString JoinFids( QgsFeatureIds fids )
{
    QString join;
    QSet<qint64>::iterator it;
    for( it = fids.begin(); it != fids.end(); it++ )
    {
        join += QString::number( *it ) + ",";
    }
    join += "-1";
    return join;
}

/*
** Select the dispatch locations on the map.
*/
void WfipsMainWindow::UpdateSelectedDispatchLocations( const QgsFeatureIds &fids )
{
    QgsVectorLayer *layer = dynamic_cast<QgsVectorLayer*>( dispatchLocationMemLayer );
    if( layer == NULL || !layer->isValid() )
    {
        return;
    }
    QgsFeatureIterator fit;
    QgsFeature feature;
    QgsFeatureRequest request;
    QgsFeatureId fid;
    QgsFeatureIds newfids;

    qDebug() << "Selecting OFIDS:" << fids;
    QString sql = "ofid IN(";
    sql += JoinFids( fids );
    sql += ")";
    qDebug() << "SQL filter:" << sql;
    request.setFilterExpression( sql );
    fit = layer->getFeatures( request );
    while( fit.nextFeature( feature ) )
    {
        fid = feature.id();
        newfids.insert( fid );
    }
    qDebug() << "Found mem fids:" << newfids;
    layer->removeSelection();
    layer->select( newfids );
    dispatchMapCanvas->refresh();
}


void WfipsMainWindow::HideDispatchLocations( QgsFeatureIds fids )
{
    QgsVectorLayer *layer = dynamic_cast<QgsVectorLayer*>( dispatchLocationMemLayer );
    if( layer == NULL )
    {
        return;
    }
    if( fids.empty() )
    {
        layer->removeSelection();
        layer->setSubsetString( "" );
        return;
    }
    QString where = BuildFidSet( "ofid", fids );
    qDebug() << "Limiting location set:" << where;
    layer->setSubsetString( where );
    analysisAreaMapCanvas->refresh();
}

void WfipsMainWindow::EnableCustomFuelMask( int index )
{
    ui->fuelStackWidget->setCurrentIndex( index );
    bool enable = index == 1;
    ui->fuelMaskToolButton->setEnabled( enable );
}

void WfipsMainWindow::EnableFuelMaskAttr( int index )
{
    bool enable = !(bool)index;
    ui->fuelProbSpinBox->setEnabled( enable );
}

void WfipsMainWindow::SelectFuelMask()
{
    /* Use custom layer path for now, but maybe add a setting */
    WfipsAddLayerDialog dialog( customLayerPath, this );
    dialog.exec();
    qDebug() << "Loading" << dialog.GetFilePath() << "," << dialog.GetCurrentLayer();
    if( dialog.GetFilePath() == "" || dialog.GetCurrentLayer() == "" )
    {
        qDebug() << "Invalid Layer file or layer name!";
        ui->fuelMaskSourceLineEdit->setText( "" );
        ui->fuelMaskLayerLineEdit->setText( "" );
        return;
    }
    QFileInfo info(dialog.GetFilePath() );
    fuelMaskSource = dialog.GetFilePath();
    fuelMaskLayer = dialog.GetCurrentLayer();
    qDebug() << "Using file:" << fuelMaskSource << ", layer:" << fuelMaskLayer;
    ui->fuelMaskSourceLineEdit->setText( info.baseName() );
    ui->fuelMaskLayerLineEdit->setText( fuelMaskLayer );
    customLayerPath = info.absolutePath();
    QgsVectorLayer layer(fuelMaskSource + "|layername=" + fuelMaskLayer, "", "ogr" );
    if( !layer.isValid() )
    {
        fuelMaskSource = "";
        fuelMaskLayer = "";
        return;
    }
    QgsFields fields;
    fields = layer.pendingFields();
    /*
    ** XXX
    ** Disable attribute selection for now, just use fixed.
    ** XXX
    */
    return;
    for( int i = 0; i < fields.size(); i++ )
    {
        qDebug() << fields[i].name();
        ui->fuelAttComboBox->addItem( fields[i].name() );
    }
    return;
}

double WfipsMainWindow::GetPrepositionValue( QComboBox *c )
{
    int i = c->currentIndex();
    double d;
    switch( i )
    {
        case 0:
            d = 0.5;
        case 1:
            d = 0.6;
        case 2:
            d = 0.7;
        case 3:
            d = 0.8;
        case 4:
            d = 0.9;
        default:
            d = 0.5;
    }
    return d;
}

char * WfipsMainWindow::GetTreatWkt()
{
    QgsVectorLayer layer(fuelMaskSource + "|layername=" + fuelMaskLayer, "", "ogr" );

    QgsFeatureIterator fit = layer.getFeatures();
    QList<QgsGeometry*>geometries;
    QgsFeature feature;
    while( fit.nextFeature( feature ) )
    {
        geometries.append( new QgsGeometry( *(feature.geometry()) ) );
    }
    QgsGeometry *multi = QgsGeometry::unaryUnion( geometries );
    QString wkt = multi->exportToWkt();
    delete multi;
    if( wkt != "" )
    {
        return QStringToCString( wkt );
    }
    return NULL;
}

void WfipsMainWindow::UpdateAsyncProgress( QFuture<int>&future )
{
    int i = 0;
    ui->progressBar->setRange( 0, 0 );
    while( !future.isFinished() && i < 1000 )
    {
        CPLSleep( 0.1 );
        ui->progressBar->setValue( 0 );
        QCoreApplication::processEvents();
        i++;
    }
    future.waitForFinished();
    ui->progressBar->setRange( 0, 100 );
    ui->progressBar->reset();
}

int WfipsMainWindow::RunIrs()
{
    int rc, i;
    QString outputFile =
        QFileDialog::getSaveFileName( this, tr( "Save simulation as..." ),
                                      "", tr( "WFIPS Database files (*.db)" ) );
    if( outputFile == "" )
        return 0;
    char *pszPath = NULL;
    char *pszDataPath = NULL;
    pszPath = QStringToCString( outputFile );
    this->setDisabled( true );

    /* Get all parameters from GUI */
    /* Prepositioning */
    double dfPpEng, dfPpCrw, dfPpHeli;
    /* correct default? */
    dfPpEng = dfPpCrw = dfPpHeli = 0.5;
    if( ui->prePositionGroupBox->isChecked() )
    {
        dfPpEng = GetPrepositionValue( ui->enginePPComboBox );
        dfPpCrw = GetPrepositionValue( ui->crewPPComboBox );
        dfPpHeli = GetPrepositionValue( ui->helitackPPComboBox );
    }
    poData->SetPrepositioning( dfPpEng, dfPpCrw, dfPpHeli );

    /* Fuel Treatment */
    double dfTreatProb = 0.0;
    const char *pszTreatWkt = NULL;
    int nWfpMask = 0;
    double adfWfpProb[4] = { 0.0, 0.0, 0.0, 0.0 };
    if( ui->treatGroupBox->isChecked() )
    {
        if( ui->fuelComboBox->currentIndex() == 0 )
        {
            dfTreatProb = ui->fuelTreatSpinBox->value() / 100.;
        }
        else if( ui->fuelComboBox->currentIndex() == 1 )
        {
            pszTreatWkt = GetTreatWkt();
            dfTreatProb = ui->fuelProbSpinBox->value() / 100.;
        }
        else if( ui->fuelComboBox->currentIndex() == 2 )
        {
            if( ui->wfpCat1CheckBox->isChecked() )
            {
                nWfpMask |= WFP_PRIORITY_1;
                adfWfpProb[0] = ui->wfpCat1SpinBox->value() / 100.;
            }
            if( ui->wfpCat2CheckBox->isChecked() )
            {
                nWfpMask |= WFP_PRIORITY_2;
                adfWfpProb[1] = ui->wfpCat2SpinBox->value() / 100.;
            }
            if( ui->wfpCat3CheckBox->isChecked() )
            {
                nWfpMask |= WFP_PRIORITY_3;
                adfWfpProb[2] = ui->wfpCat3SpinBox->value() / 100.;
            }
            if( ui->wfpCat4CheckBox->isChecked() )
            {
                nWfpMask |= WFP_PRIORITY_4;
                adfWfpProb[3] = ui->wfpCat4SpinBox->value() / 100.;
            }
            dfTreatProb = 0;
        }
    }

    int nIgnOwnership = 0;
    if( ui->agencyIgnUsfsCheckBox->isChecked() )
        nIgnOwnership |= USFS;
    if( ui->agencyIgnDoiCheckBox->isChecked() )
        nIgnOwnership |= DOI_ALL;
    if( ui->agencyIgnStateCheckBox->isChecked() )
        nIgnOwnership |= AGENCY_OTHER;
    /* Need some ignitions */
    assert( nIgnOwnership );

    double dfModRespProb = 0;
    if( ui->manageGroupBox->isChecked() )
    {
        dfModRespProb = ui->manageSpinBox->value() / 100.;
    }

    QFuture<int>future;
    if( 0 )
    {
        this->statusBar()->showMessage( "Loading data..." );
        future = QtConcurrent::run( poData, &WfipsData::LoadIrsData );
        UpdateAsyncProgress( future );
        this->statusBar()->showMessage( "Data loaded." );
        rc = future.results()[0];
        this->statusBar()->showMessage( "Loading Fires..." );
        /* FIXME */
        /*
        future = QtConcurrent::run( poData, &WfipsData::LoadScenario, 5, NULL,
                                                                      0.0, 0,
                                                                      WFP_NO_TREAT,
                                                                      0, 0 );
        UpdateAsyncProgress( future );
        */
        /*
WfipsData::LoadScenario( int nYearIdx, const char *pszTreatWkt,
                         double dfTreatProb, int nWfpTreatMask,
                         double *padfWfpTreatProb, double dfStratProb,
                         int nAgencyFilter )
                         */
        rc = poData->LoadScenario( 5, (const char*)pszTreatWkt, dfTreatProb,
                                   nWfpMask, adfWfpProb, dfModRespProb,
                                   nIgnOwnership );
        this->statusBar()->showMessage( "Fires loaded." );
        //rc = future.results()[0];
        this->statusBar()->showMessage( "Running Scenario..." );
        future = QtConcurrent::run( poData, &WfipsData::RunScenario, 0 );
        this->statusBar()->showMessage( "Simulation finished.", 1500 );
        rc = future.results()[0];
    }
    else
    {
        this->statusBar()->showMessage( "Loading data..." );
        rc = poData->LoadIrsData();
        this->statusBar()->showMessage( "Data loaded." );
        this->statusBar()->showMessage( "Loading fires..." );
        rc = poData->LoadScenario( 5, (const char*)pszTreatWkt, dfTreatProb,
                                   nWfpMask, adfWfpProb, dfModRespProb,
                                   nIgnOwnership );
        this->statusBar()->showMessage( "Fires loaded." );
        rc = poData->SetResultPath( pszPath );
        this->statusBar()->showMessage( "Running Scenario..." );
        rc = poData->RunScenario( 0 );
        this->statusBar()->showMessage( "Simulation finished." );
        this->statusBar()->showMessage( "Writing output..." );
        rc = poData->WriteResults();
        this->statusBar()->showMessage( "Output written." );
        this->statusBar()->showMessage( "Simulating Large Fires..." );
        //rc = poData->SimulateLargeFire();
        this->statusBar()->showMessage( "Large Fire Simulation finished." );
        this->statusBar()->showMessage( "Writing Spatial Summary Results..." );
        rc = poData->SpatialSummary( "fpu" );
        this->statusBar()->showMessage( "Done." );
    }
    free( (void*)pszTreatWkt );
    free( (void*)pszPath );
    /* We should probably make our own, and leave this one alone */
    poResults = poData->GetResults();
    EnableResultsWidgets( outputFile );
    this->setEnabled( true );
    return rc;
}

void WfipsMainWindow::EnableResultsWidgets( QString resultsFile )
{
    ui->resultsFileEdit->setText( resultsFile );
}




/*
** XXX: Works when items aren't disabled.  Needs to be checked for disabled
**      items as well as invisible items.
*/
QTreeWidgetItem * WfipsMainWindow::FindLastVisibleChild( QTreeWidgetItem *item )
{
    if( !item->isHidden() && item->childCount() == 0 )
    {
        return item;
    }
    return FindLastVisibleChild( item->child( item->childCount() - 1 ) );
}

void WfipsMainWindow::FindTreeWidget( int down )
{
    QTreeWidgetItem *current, *next;
    current = ui->treeWidget->currentItem();
    next = current;
    do
    {
        if( down )
            next = ui->treeWidget->itemBelow( next );
        else
            next = ui->treeWidget->itemAbove( next );
        if( next == NULL )
        {
            if( down )
                next = ui->treeWidget->topLevelItem( 0 );
            else
            {
                next = ui->treeWidget->topLevelItem( ui->treeWidget->topLevelItemCount() - 1 );
                next = FindLastVisibleChild( next );
            }
        }
    } while( next->isDisabled() );

    ui->treeWidget->setCurrentItem( next );
}

void WfipsMainWindow::NextTreeWidgetItem()
{
    return FindTreeWidget( 1 );
}

void WfipsMainWindow::PrevTreeWidgetItem()
{
    return FindTreeWidget( 0 );
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

