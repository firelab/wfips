#include "wfipsmainwindow.h"
#include "ui_wfipsmainwindow.h"

WfipsMainWindow::WfipsMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WfipsMainWindow)
{
    ui->setupUi(this);
    ui->treeWidget->expandAll();

    ConstructToolButtons();
    ConstructAnalysisAreaWidgets();

}

WfipsMainWindow::~WfipsMainWindow()
{
    delete ui;
}

void WfipsMainWindow::ConstructToolButtons()
{
    ui->mapPanToolButton->setIcon( QIcon( ":/pan" ) );
    ui->mapZoomInToolButton->setIcon( QIcon( ":/zoom_in" ) );
    ui->mapZoomOutToolButton->setIcon( QIcon( ":/zoom_out" ) );
    ui->mapZoomToLayerToolButton->setIcon( QIcon( ":/zoom_layer" ) );
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
}

void WfipsMainWindow::LoadAnalysisAreaLayers()
{
    return;
}
