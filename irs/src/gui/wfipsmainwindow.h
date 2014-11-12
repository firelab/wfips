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

#include <QtConcurrentRun>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QProgressBar>
#include <QSettings>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

/* Our custom widgets and such */
#include "wfipsaddlayerdialog.h"

#include "wfipsidentifymaptool.h"
#include "wfipsidentifydialog.h"
#include "wfipsselectmaptool.h"

/*
** QGIS includes.  Try to keep these semi-organized for easier maintenance.
*/

/* Main QGIS stuff */
#include <qgsapplication.h>
#include <qgsmaplayerregistry.h>
#include <qgsproviderregistry.h>
#include <qgsvectordataprovider.h>

/* QGIS feature and related */
#include <qgsfeature.h>
#include <qgsfeaturerequest.h>
#include <qgsgeometry.h>

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
#include <qgsmaptoolemitpoint.h>

/* CRS for QGIS */
#include <qgscoordinatereferencesystem.h>

/* GDAL for layer sniffing */
#include "gdal.h"
#include "cpl_string.h"
#include "cpl_conv.h"
#include "cpl_port.h"
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

    /* CRS for map canvases */
    QgsCoordinateReferenceSystem crs;
    QgsCoordinateTransform transform;

    /* WFIPS root/default path */
    QString wfipsPath;
    QString customLayerPath;

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
    QList<QgsMapLayer*> analysisLayers;
    QgsVectorLayer *analysisLayer;

    QgsMapTool *analysisPanTool;
    QgsMapTool *analysisZoomInTool;
    QgsMapTool *analysisZoomOutTool;
    QgsMapTool *analysisIdentifyTool;
    QgsMapTool *analysisSelectTool;

    QVBoxLayout *analysisAreaMapLayout;

    void ConstructAnalysisAreaWidgets();
    void AddAnalysisAreaLayer( QString path, QString layerName="",
                               bool useExtent=false );
    void LoadAnalysisAreaLayers();

    void AddAnalysisLayerToCanvases();

    QgsFeatureIds selectedFids;
    QgsFeatureList selectedFeatures;

    /*
    ** Mutable layer for our buffering and editing, and displaying in other
    ** canvases
    */
    QgsVectorLayer *analysisAreaMemLayer;
    QgsVectorLayer *dispatchLocationMemLayer;

    /* Dispatch location canvas */
    QVBoxLayout *dispatchMapLayout;
    QgsMapCanvas *dispatchMapCanvas;
    QList<QgsMapCanvasLayer> dispatchMapCanvasLayers;
    QList<QgsMapLayer*> dispatchLayers;

    void ConstructDispatchWidgets();

    /* Identify results viewer */
    WfipsIdentifyDialog *identifyDialog;

    /* Handle to current mapcanvas */
    QgsMapCanvas *currentMapCanvas;

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

    /* Identify a map layer feature */
    void Identify( QList<QgsMapToolIdentify::IdentifyResult> result );
    void Select( QgsFeatureIds fids );

    /* Analysis Area */
    void AddCustomAnalysisArea();
    void UpdateAnalysisAreaMap( int index );

    void SetAnalysisArea();
    void ClearAnalysisAreaSelection();

    void ReadSettings();
    void WriteSettings();

    /* Emit a warning or an error to a dialog or just qDebug() */
    void ShowMessage( const int messageType,
                      const int messageFlags,
                      const QString &message );

protected:
    void closeEvent( QCloseEvent *event );

};

#endif // WFIPSMAINWINDOW_H
