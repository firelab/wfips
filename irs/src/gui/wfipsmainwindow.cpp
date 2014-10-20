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

    ConstructTreeWidget();
    AssignTreeWidgetIndices( ui->treeWidget->invisibleRootItem() );
    qDebug() << "Found " << treeWidgetList.size() << " tree widget items.";

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
        AssignTreeWidgetIndices( item->child( i ) );
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
            break;
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
        case 3:
        case 4:
        case 5:
        case 6:
            ui->stackedWidget->setCurrentIndex( 0 );
            ui->mapToolFrame->setEnabled( true );
            break;
        case 7:
            ui->stackedWidget->setCurrentIndex( 1 );
            break;
        case 8:
            ui->stackedWidget->setCurrentIndex( 2 );
            break;
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
            ui->stackedWidget->setCurrentIndex( 3 );
            break;
        case 14:
        case 15:
        case 16:
            ui->stackedWidget->setCurrentIndex( 4 );
            break;
        case 17:
            ui->stackedWidget->setCurrentIndex( 5 );
            break;
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            ui->stackedWidget->setCurrentIndex( 6 );
            ui->mapToolFrame->setEnabled( true );
            break;
        /* 0 is the 'invisible root' */
        case 0:
        default:
            qDebug() << "Shouldn't ever get here, you messed up";
    }
}

