#ifndef WFIPSMAINWINDOW_H
#define WFIPSMAINWINDOW_H

#define QGISDEBUG 1

#include <QFileDialog>
#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

/*
** QGIS includes.  Try to keep these semi-organized for easier maintenance.
*/

/* Main QGIS stuff */
#include <qgsapplication.h>
#include <qgsmaplayerregistry.h>
#include <qgsproviderregistry.h>

/* Renderers and symbols */
#include <qgsgraduatedsymbolrendererv2.h>
#include <qgssinglesymbolrendererv2.h>
#include <qgscategorizedsymbolrendererv2.h>
#include <qgssymbolv2.h>

/* QGIS Map Canvas */
#include <qgsmapcanvas.h>

/* QGIS layers */
#include <qgsvectorlayer.h>

/* QGIS Map Tools */
#include <qgsmaptool.h>
#include <qgsmaptoolidentify.h>
#include <qgsmaptoolpan.h>
#include <qgsmaptoolzoom.h>

namespace Ui {
class WfipsMainWindow;
}

class WfipsMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit WfipsMainWindow(QWidget *parent = 0);
    ~WfipsMainWindow();

private:
    Ui::WfipsMainWindow *ui;
    QString qgisPluginPath;

    /* WFIPS root/default path */
    QString wfipsPath;

    /* Database paths */
    QStringList analysisAreaLayers;

    /* Tree and stack widget handling */
    void ConstructTreeWidget();
    void AssignTreeWidgetIndices( QTreeWidgetItem *item );
    /*
    ** Litte structure to know where to go when the tree is clicked().  If you
    ** add an item to the tree widget or the main stack widget (or sub stack
    ** widgets), it needs to be added in the map and AssignTreeWidgetIndices().
    */
    QList<QTreeWidgetItem*>treeWidgetList;

    /*
    ** Miscillaneous Connections.  If you don't have a home for a connection,
    ** put it in here.  This is called after all other construction.  The
    ** PostConstructionActions() is called after CreateConnections().
    */
    void CreateConnections();
    void PostConstructionActions();

    /* Tool buttons */
    void ConstructToolButtons();

    /* Analysis area */
    QgsMapCanvas *analysisAreaMapCanvas;
    QList<QgsMapCanvasLayer> analysisMapCanvasLayers;
    QgsVectorLayer *analysisLayer;

    QgsMapTool *analysisPanTool;
    QgsMapTool *analysisZoomInTool;
    QgsMapTool *analysisZoomOutTool;
    QgsMapTool *analysisSelectTool;

    QVBoxLayout *analysisAreaMapLayout;

    void ConstructAnalysisAreaWidgets();
    void AddAnalysisAreaLayer( QString path, QString layerName="" );
    void LoadAnalysisAreaLayers();

    /* Fuel treatment related */

private slots:
    /* Main path designation */
    void OpenWfipsPath();
    /* Tree and stack widget slots */
    void SetStackIndex( QTreeWidgetItem *current,
                        QTreeWidgetItem *previous );

    /* Slot for tool button to map tool mapping */
    void UpdateMapToolType();
    /* Zoom to layer extent */
    void ZoomToLayerExtent();

    /* Analysis Area */
    void AddCustomAnalysisArea();
    void UpdateAnalysisAreaMap( int index );

};

#endif // WFIPSMAINWINDOW_H
