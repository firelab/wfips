#ifndef WFIPSMAINWINDOW_H
#define WFIPSMAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

/*
** QGIS includes.  Try to keep these semi-organized for easier maintenance.
*/

/* QGIS Map Canvas */
#include <qgsmapcanvas.h>

/* QGIS Map Tools */
#include <qgsmaptool.h>
#include <qgsmaptoolidentify.h>
#include <qgsmaptoolpan.h>
#include <qgsmaptoolzoom.h>
#include <qgsmaptoolemitpoint.h>

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

    /* Tree and stack widget handling */
    void ConstructTreeWidget();
    void AssignTreeWidgetIndices( QTreeWidgetItem *item );
    /*
    ** Litte structure to know where to go when the tree is clicked().  If you
    ** add an item to the tree widget or the main stack widget (or sub stack
    ** widgets), it needs to be added in the map and AssignTreeWidgetIndices().
    */
    QList<QTreeWidgetItem*>treeWidgetList;

    /* Tool buttons */
    void ConstructToolButtons();

    /* Analysis area */
    QgsMapCanvas *analysisAreaMapCanvas;

    QgsMapTool *analysisPanTool;
    QgsMapTool *analysisZoomInTool;
    QgsMapTool *analysisZoomOutTool;
    QgsMapTool *analysisMapSelectTool;

    QVBoxLayout *analysisAreaMapLayout;

    void ConstructAnalysisAreaWidgets();
    void LoadAnalysisAreaLayers();

private slots:
    /* Tree and stack widget slots */
    void SetStackIndex( QTreeWidgetItem *current,
                        QTreeWidgetItem *previous );

};

#endif // WFIPSMAINWINDOW_H
