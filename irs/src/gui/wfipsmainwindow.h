#ifndef WFIPSMAINWINDOW_H
#define WFIPSMAINWINDOW_H

#include <QMainWindow>
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

};

#endif // WFIPSMAINWINDOW_H
