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

    /* Call *after* construction */
    CreateConnections();
    PostConstructionActions();
}

WfipsMainWindow::~WfipsMainWindow()
{
    //QgsMapLayerRegistry::instance()->removeAllMapLayers();
    //delete analysisLayer;
    //delete analysisSymbol;
    //delete analysisRenderer;
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

    /* Load custom layers for analysis */
    connect( ui->customAnalysisAreaOpenToolButton, SIGNAL( clicked() ),
             this, SLOT( AddCustomAnalysisArea() ) );
}

void WfipsMainWindow::PostConstructionActions()
{
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

void WfipsMainWindow::AddAnalysisAreaLayer( QString path, QString layerName )
{
    if( path == "" )
    {
        qDebug() << "Invalid layer";
        return;
    }
    QString dataSource = path;
    path += "|layername=";
    if( layerName == "" )
    {
        layerName = QFileInfo( path ).baseName();
    }
    path += layerName;
    qDebug() << "Loading layer: " << layerName;
    analysisLayer = new QgsVectorLayer( dataSource, "", "ogr", true );
    if( !analysisLayer->isValid() )
    {
        qDebug() << "Invalid layer";
        return;
    }
    analysisLayer->setReadOnly( true );
    QgsMapLayerRegistry::instance()->addMapLayer( analysisLayer, false, false );
    analysisMapCanvasLayers.append( QgsMapCanvasLayer( analysisLayer, false ) );
    ui->analysisAreaComboBox->addItem( layerName.toUpper() );
}

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
    return;
}

void WfipsMainWindow::AddCustomAnalysisArea()
{
    QString layerFile =
        QFileDialog::getOpenFileName( this, tr( "Open GIS file" ), "", "" );
    ui->customAnalysisAreaLineEdit->setText( layerFile );
    if( layerFile == "" )
    {
        return;
    }
    QgsMapLayer *layer = new QgsVectorLayer( layerFile, "", "ogr", false );
    if( !layer->isValid() )
    {
        qDebug() << "Could not identify layer file";
        return;
    }
    QStringList layers = layer->subLayers();
    ui->customAnalysisAreaLayerComboBox->addItems( layers );
    delete layer;
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

