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

#ifndef WFIPSMAINWINDOW_H
#define WFIPSMAINWINDOW_H

#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

/* Our custom widgets and such */
#include "addanalysisareadialog.h"

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

/* GDAL for layer sniffing */
#include "gdal.h"
#include "ogr_api.h"

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
    void LoadCustomAnalysisArea();
    void UpdateAnalysisAreaMap( int index );

    /* Emit a warning or an error to a dialog or just qDebug() */
    void ShowMessage( const int messageType,
                      const int messageFlags,
                      const QString &message );

};

#endif // WFIPSMAINWINDOW_H
