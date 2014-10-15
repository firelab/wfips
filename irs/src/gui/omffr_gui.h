/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Graphical User Interface
 * Author:   Kyle Shannon <kyle@pobox.com>, see original banner below.
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
#ifndef OMFFR_GUI_H_
#define OMFFR_GUI_H_

#undef HAVE_TOUCH

/* QGIS includes */
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsgraduatedsymbolrendererv2.h>
//#include <qgssinglesymbolrenderer.h>
//for upgrade
#include <qgssinglesymbolrendererv2.h>
#include <qgscategorizedsymbolrendererv2.h>
#include <qgsmaplayerregistry.h>
#include <qgsvectorlayer.h>
#include <qgsmaptoolpan.h>
#include <qgsmaptoolzoom.h>
#include <qgsmaptoolidentify.h>
#include <qgsmaptoolemitpoint.h>
#include <qgsmapcanvas.h>
#include <qgsmaptool.h>
#include <qgsrubberband.h>
#include <qgssinglesymbolrendererv2.h>
#include <qgssymbolv2.h>

/* Qt */
#include <QtGui>
#include <QMainWindow>
#include <QtCore>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPointF>
#include <QVector>
#include <QAbstractItemModel>
/* QWT */
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
//#include <qwt_series_data.h>
#include <qwt_data.h>
#include <qwt_legend.h>
#include <qwt_plot_marker.h>

/* Qt UI */
#include "ui_omffr_gui.h"

/* Other IRS GUI includes */
#include "simpleidentifymaptool.h"
#include "simpleareamaptool.h"
#include "boxplotitem.h"
#include "textscaledraw.h"
#include "piewidget.h"


#ifdef _OPENMP
#include <omp.h>
#endif

/* IRS */
#include "irs.h"
#include "irs_const.h"
#include "data_load.h"

static int UpdateProgress(double progress, const char* message, void* unused);

class ResultTreeModel;

class OmffrMainWindow : public QMainWindow, private Ui::OmffrMainWindow
{
    Q_OBJECT;

public:
    OmffrMainWindow(QWidget* parent = 0);
    ~OmffrMainWindow();

private:
    IRSSuite *poSuite;

    enum NavigationMode
    {
        panMode,
        zoomMode,
        identifyMode,
        selectMode,
        selectAreaMode
    };

    QVBoxLayout *mapLayout;
    QToolBar *mapToolBar;

    QString qgisPluginPath;
    QString layerPath;
    QString providerName;

    QwtPlotCurve *yearCurve;
    QwtPlotCurve *minCurve;
    QwtPlotCurve *maxCurve;
    QwtPlotCurve *meanCurve;
    QwtPlotCurve *escapeCurve;
    QwtPlotCurve *containCurve;
    QwtPlotCurve *norescCurve;
    QwtArrayData* yearData;
    QwtArrayData* minData;
    QwtArrayData* maxData;
    QwtArrayData* meanData;

    bool statsCached;
    const char *pszGeometryCache;
    int startCache;
    int endCache;
    std::vector<int>minIgnitionsCache;
    std::vector<int>maxIgnitionsCache;
    std::vector<double>meanIgnitionsCache;

    QString currentResultsFile;
    QTreeWidgetItem *resultFwaItem;
    QTreeWidgetItem *resultEscapesItem;
    QTreeWidgetItem *resultContainedItem;
    QTreeWidgetItem *resultNoRescItem;
    QString dispatchLayerId;

    QList<QTreeWidgetItem*>resourceTreeWidgets;
    QList< QList<QTreeWidgetItem*> >resourceTreeSubWidgets;

    QgsMapCanvas *mapCanvas;
    QList<QgsVectorLayer*>vectorLayers;
    QList<QgsFeatureRendererV2*>renderers;
    QStringList pointLayers;
    QStringList pointLayerNames;
    QStringList areaLayers;
    QStringList areaLayerNames;
    QgsVectorLayer *gaccMapLayer;
    QgsVectorLayer *fpuMapLayer;
    QgsVectorLayer *fwaMapLayer;
    QgsVectorLayer *dispatchMapLayer;
    QgsVectorLayer *airTankerMapLayer;
    QList<QgsMapCanvasLayer> mapLayerSet;

    QgsMapTool *mapPanTool;
    QgsMapTool *mapZoomInTool;
    QgsMapTool *mapZoomOutTool;
    QgsMapTool *mapZoomLayerTool;
    SimpleIdentifyMapTool *mapSimpleIdentifyTool;
    QgsMapTool *mapSelectTool;
    SimpleAreaMapTool *mapSimpleAreaTool;
    QgsRubberBand *rubberBand;

    QString dataPath;
	std::vector<IRSResult*>apResults;
    QMap<QString, QSpinBox*>rescTypeMap;
    QMap< int, QMap<QString, int> >regionRescMap;

    IRSDataAccess *poDA;
    int numYears;
    bool newIRSRun;
    CRunScenario *IRSRunner;
    std::vector< std::vector<CResults> >IRSResults;
    std::map< std::string, std::vector< int > > FwaResults;
	IRSResult * results;
	QgsGraduatedSymbolRendererV2 *renderer;

    void SetUpMapCanvas();
    void SetUpPlot();
    void SetUpToolButtons();
    void SetUpFigInput();
    void SetUpPreposition();
    void SetUpDrawDown();
    void BuildResultsTree();
    void CreateConnections();
    void BuildRescMap();
    int IndexToRegion(int index);
    int RegionToIndex(int region);
    QString RegionName(int region);
    NavigationMode CurrentNavigationMode();
    void WriteResourceDatabase(QString file);
    bool CompareGeometry(const char *pszOne, const char *pszTwo);
    void ReadFwaSummaryResults(QString resultsFile);
    void SetPPLevels();
    void SetDDLevels();
    int FillRunData(IRSMultiRunData *psData);
    int  SetPreposAndDrawDown();

private slots:
    void SetNavigationMode();
    void ShowAreaLayer(int index);
    void ShowPointLayer(int index);
    void ShowRubberBand();
    int ConnectToDB();
    int RunIRS();
    int SimulateIRS();
    void Identify(QList<QgsMapToolIdentify::IdentifyResult>results);
    void LoadDefaultResourceCounts();
    void UpdateSpinBoxes(int index);
    void SaveDefaultResourceCounts();
    void UpdateFromDatatbase();
    void PlotFigMap();
    void PlotFigGraph();
    void ShowIdentifyResults(std::map<std::string, std::string>attributes);
    void DeleteTreeItems();
    void UpdateFigDate(const QDate &date);
    void UpdateStatsCheckBox(int index);
    void SetRegionalResourcesDisabled(bool disable);
    void ShowCartDiagram();
    void LoadResultsFile();
    void PlotFireResults();
	void PlotSingleRunResults();
	void MapResults();
	void PlotSeriesResults();
	void PlotResults(int i);
    void ScaleChanged(int i);
	void SeriesPlotResults();
    void PlotPieChart();
    void ExportCsv();
    void ExportOgr();
    void ResultGACCs();

    const char * QStringToCString( QString s );

    void SelectGeometries(QgsGeometry*);
    void SelectGeometryFromPoint(const QgsPoint &point, Qt::MouseButton button);
    void ClearSelection();

    void LoadResultAttributes();
	void changeSeriesItem(int i);
	void displayTabChanged(int i);
    void RunIRSConcurrent(void*);

    void ToggleAutoReduce();

    void zoomLayer();

signals:
    //void SendProgress(double progress, const char *message, void *notUsed);

};

#endif /* OMFFR_GUI_H_ */

