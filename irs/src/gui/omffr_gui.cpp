/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Graphical User Interface
 * Author:   Kyle Shannon <kyle@pobox.com>
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

#include <omffr_gui.h>



QProgressBar *globalProgress;
QLabel *globalProgressLabel;

/**
 * \brief Set up the main window.
 *
 * \param parent widget that owns the main window
 */
OmffrMainWindow::OmffrMainWindow(QWidget* parent) : QMainWindow(parent)
{
    setupUi(this);

    qgisPluginPath = QGIS_PLUGIN_DIR;
    qDebug() << "QGIS plugin path: " << qgisPluginPath;
#ifndef WIN32
    layerPath = "/home/kyle/src/omffr/trunk/data/omffr.sqlite|layername=";
#else
    layerPath = "c:/Users/ksshannon/Documents/GitHub/build/data/omffr.sqlite|layername=";
#ifdef IRS_DIANE_BUILD
    layerPath = "c:/src/omffr/data/omffr.sqlite|layername=";
#endif
#endif
    qDebug() << layerPath;
    providerName = "ogr";
    //providerName = "spatialite";
    QgsProviderRegistry::instance(qgisPluginPath);

    pointLayers << "dispatch_location" << "tanker_base";// << "scenario";
    pointLayerNames << "Dispatch Locations" << "Air Tanker Bases";// << "Ignitions";
    areaLayers << "gacc_bndry" << "fpu_bndry";// << "fwa_bndry" << "fpu_tb";
    areaLayerNames << "Region Boundaries" << "FPU Boundaries";
                   //<< "FWA Boundaries" << "Tanker Service Areas";

    SetUpMapCanvas();
    SetUpPlot();
    SetUpPreposition();
    SetUpDrawDown();
    SetUpToolButtons();
    panToolButton->click();

    CreateConnections();

    BuildRescMap();
    areaLayerComboBox->setCurrentIndex(1);
    //currentResourcesCheckBox->setChecked(true);
    //currentResourcesCheckBox->setChecked(false);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressLabel->setText("");
    globalProgress = progressBar;
    globalProgressLabel = progressLabel;

    BuildResultsTree();
    resultsTab->setEnabled(true);
    GACCwarning->setHidden(true);

    statsCached = false;
    pszGeometryCache = NULL;

    poDA = NULL;
    newIRSRun = false;
    yearsSpinBox->setRange(0, 0);
    currentResultsFile = "";
#if defined(IRS_ENABLE_THREADING) && (defined(IRS_LOAD_DATA_CONCURRENT) || defined(IRS_RUN_SCEN_CONCURRENT))
    threadSpinBox->setMaximum(QThread::idealThreadCount());
#else
    threadSpinBox->setMaximum(1);
#endif
#ifdef OMFFR_DB_DBG
    poDA = IRSDataAccess::Create(0, OMFFR_DB_DBG);
    if(poDA == NULL)
    {
        QMessageBox::warning(this, tr("Initial Response Simulator"),
                             tr("Could not open the SQLite database."),
                             QMessageBox::Ok );
    }
    dataPath = OMFFR_DB_DBG;
    UpdateFromDatatbase();
#endif
}

/**
 * \brief Delete resources related to main window.
 */
OmffrMainWindow::~OmffrMainWindow()
{
    delete mapCanvas;
    delete mapPanTool;
    delete mapZoomInTool;
    delete mapSimpleIdentifyTool;
    delete rubberBand;

    for(int i = 0;i < vectorLayers.size();i++)
    {
        delete vectorLayers[i];
    }
    for(int i = 0;i < renderers.size();i++)
    {
        delete renderers[i];
    }
    delete minCurve;
    delete maxCurve;
    delete meanCurve;
    delete resultFwaItem;
    delete resultEscapesItem;
    delete resultContainedItem;
    delete resultNoRescItem;
}

void OmffrMainWindow::BuildResultsTree()
{
    /* GONE
    resultFwaItem = new QTreeWidgetItem;
    resultEscapesItem = new QTreeWidgetItem;
    resultContainedItem = new QTreeWidgetItem;
    resultNoRescItem = new QTreeWidgetItem;
    resultEscapesItem->setText(0, tr("Escaped Fires:"));
    resultContainedItem->setText(0, tr("Contained Fires:"));
    resultNoRescItem->setText(0, tr("Fires With No Resources:"));
    resultsTreeWidget->addTopLevelItem(resultFwaItem);
    resultFwaItem->addChild(resultContainedItem);
    resultFwaItem->addChild(resultEscapesItem);
    resultFwaItem->addChild(resultNoRescItem);
    resultFwaItem->setExpanded(true);
    resultsTreeWidget->header()->resizeSection(0, 200);
    */
}

void OmffrMainWindow::BuildRescMap()
{
/*
    rescTypeMap["ATT"] = attSpinBox;
    rescTypeMap["DZR"] = dozerSpinBox;
    rescTypeMap["FBDZ"] = fbdzSpinBox;
    rescTypeMap["FRBT"] = fireboatSpinBox;
    rescTypeMap["EN"] = engineSpinBox;
    rescTypeMap["HANDCRW"] = handcrewSpinBox;
    rescTypeMap["HEAVY"] = heavySpinBox;
    rescTypeMap["HEL1"] = hel1SpinBox;
    rescTypeMap["HEL2"] = hel2SpinBox;
    rescTypeMap["HEL2E"] = hel2eSpinBox;
    rescTypeMap["HEL3"] = hel3SpinBox;
    rescTypeMap["NATHOT"] = natlshotSpinBox;
    rescTypeMap["RAPPEL"] = helitackSpinbox;
    rescTypeMap["REGHOT"] = regshotSpinBox;
    rescTypeMap["SCP"] = scooperSpinBox;
    rescTypeMap["SEAT"] = seatSpinBox;
    rescTypeMap["SJAC"] = jumperaircraftSpinBox;
    rescTypeMap["SMJR"] = jumperSpinBox;
    rescTypeMap["TP"] = tractorplowSpinBox;
    rescTypeMap["WT"] = tenderSpinBox;

    int i = 0;
    QList<QString> keys = rescTypeMap.keys();
    while(GaccMap[i].pszCode != NULL)
    {
        for(int j = 0;j < keys.size();j++)
        {
            regionRescMap.insert((int)GaccMap[i].nRegion, QMap<QString, int>());
            regionRescMap[GaccMap[i].nRegion][keys[j]] = 0;
        }
        i++;
    }
*/
}

void OmffrMainWindow::SetUpPreposition()
{
    connect(tankerPPSlider, SIGNAL(valueChanged(int)),
            tankerPPSpinBox, SLOT(setValue(int)));
    connect(tankerPPSpinBox, SIGNAL(valueChanged(int)),
            tankerPPSlider, SLOT(setValue(int)));
    connect(tankerPPCheckBox, SIGNAL(toggled(bool)),
            tankerPPSlider, SLOT(setEnabled(bool)));
    connect(tankerPPCheckBox, SIGNAL(toggled(bool)),
            tankerPPSpinBox, SLOT(setEnabled(bool)));
    connect(tankerPPCheckBox, SIGNAL(toggled(bool)),
            tankerPPSeasonCheckBox, SLOT(setEnabled(bool)));
    tankerPPCheckBox->setChecked(false);
    connect(helPPSlider, SIGNAL(valueChanged(int)),
            helPPSpinBox, SLOT(setValue(int)));
    connect(helPPSpinBox, SIGNAL(valueChanged(int)),
            helPPSlider, SLOT(setValue(int)));
    connect(helPPCheckBox, SIGNAL(toggled(bool)),
            helPPSlider, SLOT(setEnabled(bool)));
    connect(helPPCheckBox, SIGNAL(toggled(bool)),
            helPPSpinBox, SLOT(setEnabled(bool)));
    connect(helPPCheckBox, SIGNAL(toggled(bool)),
            helPPSeasonCheckBox, SLOT(setEnabled(bool)));
    helPPCheckBox->setChecked(false);
    connect(helitackPPSlider, SIGNAL(valueChanged(int)),
            helitackPPSpinBox, SLOT(setValue(int)));
    connect(helitackPPSpinBox, SIGNAL(valueChanged(int)),
            helitackPPSlider, SLOT(setValue(int)));
    connect(helitackPPCheckBox, SIGNAL(toggled(bool)),
            helitackPPSlider, SLOT(setEnabled(bool)));
    connect(helitackPPCheckBox, SIGNAL(toggled(bool)),
            helitackPPSpinBox, SLOT(setEnabled(bool)));
    connect(helitackPPCheckBox, SIGNAL(toggled(bool)),
            helitackPPSeasonCheckBox, SLOT(setEnabled(bool)));
    helitackPPCheckBox->setChecked(false);
    connect(crewPPSlider, SIGNAL(valueChanged(int)),
            crewPPSpinBox, SLOT(setValue(int)));
    connect(crewPPSpinBox, SIGNAL(valueChanged(int)),
            crewPPSlider, SLOT(setValue(int)));
    connect(crewPPCheckBox, SIGNAL(toggled(bool)),
            crewPPSlider, SLOT(setEnabled(bool)));
    connect(crewPPCheckBox, SIGNAL(toggled(bool)),
            crewPPSpinBox, SLOT(setEnabled(bool)));
    connect(crewPPCheckBox, SIGNAL(toggled(bool)),
            crewPPSeasonCheckBox, SLOT(setEnabled(bool)));
    crewPPCheckBox->setChecked(false);
    connect(enginePPSlider, SIGNAL(valueChanged(int)),
            enginePPSpinBox, SLOT(setValue(int)));
    connect(enginePPSpinBox, SIGNAL(valueChanged(int)),
            enginePPSlider, SLOT(setValue(int)));
    connect(enginePPCheckBox, SIGNAL(toggled(bool)),
            enginePPSlider, SLOT(setEnabled(bool)));
    connect(enginePPCheckBox, SIGNAL(toggled(bool)),
            enginePPSpinBox, SLOT(setEnabled(bool)));
    connect(enginePPCheckBox, SIGNAL(toggled(bool)),
            enginePPSeasonCheckBox, SLOT(setEnabled(bool)));
    enginePPCheckBox->setChecked(false);
}
void OmffrMainWindow::SetUpDrawDown()
{
    connect(tankerDDSlider, SIGNAL(valueChanged(int)),
            tankerDDSpinBox, SLOT(setValue(int)));
    connect(tankerDDSpinBox, SIGNAL(valueChanged(int)),
            tankerDDSlider, SLOT(setValue(int)));
    connect(tankerDDCheckBox, SIGNAL(toggled(bool)),
            tankerDDSlider, SLOT(setEnabled(bool)));
    connect(tankerDDCheckBox, SIGNAL(toggled(bool)),
            tankerDDSpinBox, SLOT(setEnabled(bool)));
    connect(tankerDDCheckBox, SIGNAL(toggled(bool)),
            tankerDDSeasonCheckBox, SLOT(setEnabled(bool)));
    tankerDDCheckBox->setChecked(false);
    connect(seatDDSlider, SIGNAL(valueChanged(int)),
            seatDDSpinBox, SLOT(setValue(int)));
    connect(seatDDSpinBox, SIGNAL(valueChanged(int)),
            seatDDSlider, SLOT(setValue(int)));
    connect(seatDDCheckBox, SIGNAL(toggled(bool)),
            seatDDSlider, SLOT(setEnabled(bool)));
    connect(seatDDCheckBox, SIGNAL(toggled(bool)),
            seatDDSpinBox, SLOT(setEnabled(bool)));
    connect(seatDDCheckBox, SIGNAL(toggled(bool)),
            seatDDSeasonCheckBox, SLOT(setEnabled(bool)));
    seatDDCheckBox->setChecked(false);
    /*
    connect(helDDSlider, SIGNAL(valueChanged(int)),
            helDDSpinBox, SLOT(setValue(int)));
    connect(helDDSpinBox, SIGNAL(valueChanged(int)),
            helDDSlider, SLOT(setValue(int)));
    connect(helDDCheckBox, SIGNAL(toggled(bool)),
            helDDSlider, SLOT(setEnabled(bool)));
    connect(helDDCheckBox, SIGNAL(toggled(bool)),
            helDDSpinBox, SLOT(setEnabled(bool)));
    connect(helDDCheckBox, SIGNAL(toggled(bool)),
            helDDSeasonCheckBox, SLOT(setEnabled(bool)));
    helDDCheckBox->setChecked(false);
    */
    connect(engineDDSlider, SIGNAL(valueChanged(int)),
            engineDDSpinBox, SLOT(setValue(int)));
    connect(engineDDSpinBox, SIGNAL(valueChanged(int)),
            engineDDSlider, SLOT(setValue(int)));
    connect(engineDDCheckBox, SIGNAL(toggled(bool)),
            engineDDSlider, SLOT(setEnabled(bool)));
    connect(engineDDCheckBox, SIGNAL(toggled(bool)),
            engineDDSpinBox, SLOT(setEnabled(bool)));
    connect(engineDDCheckBox, SIGNAL(toggled(bool)),
            engineDDSeasonCheckBox, SLOT(setEnabled(bool)));
    engineDDCheckBox->setChecked(false);
    connect(regCrewDDSlider, SIGNAL(valueChanged(int)),
            regCrewDDSpinBox, SLOT(setValue(int)));
    connect(regCrewDDSpinBox, SIGNAL(valueChanged(int)),
            regCrewDDSlider, SLOT(setValue(int)));
    connect(regCrewDDCheckBox, SIGNAL(toggled(bool)),
            regCrewDDSlider, SLOT(setEnabled(bool)));
    connect(regCrewDDCheckBox, SIGNAL(toggled(bool)),
            regCrewDDSpinBox, SLOT(setEnabled(bool)));
    connect(regCrewDDCheckBox, SIGNAL(toggled(bool)),
            regCrewDDSeasonCheckBox, SLOT(setEnabled(bool)));
    regCrewDDCheckBox->setChecked(false);
    /*
    connect(localCrewDDSlider, SIGNAL(valueChanged(int)),
            localCrewDDSpinBox, SLOT(setValue(int)));
    connect(localCrewDDSpinBox, SIGNAL(valueChanged(int)),
            localCrewDDSlider, SLOT(setValue(int)));
    connect(localCrewDDCheckBox, SIGNAL(toggled(bool)),
            localCrewDDSlider, SLOT(setEnabled(bool)));
    connect(localCrewDDCheckBox, SIGNAL(toggled(bool)),
            localCrewDDSpinBox, SLOT(setEnabled(bool)));
    connect(localCrewDDCheckBox, SIGNAL(toggled(bool)),
            localCrewDDSeasonCheckBox, SLOT(setEnabled(bool)));
    localCrewDDCheckBox->setChecked(false);
    */
    connect(dozerDDSlider, SIGNAL(valueChanged(int)),
            dozerDDSpinBox, SLOT(setValue(int)));
    connect(dozerDDSpinBox, SIGNAL(valueChanged(int)),
            dozerDDSlider, SLOT(setValue(int)));
    connect(dozerDDCheckBox, SIGNAL(toggled(bool)),
            dozerDDSlider, SLOT(setEnabled(bool)));
    connect(dozerDDCheckBox, SIGNAL(toggled(bool)),
            dozerDDSpinBox, SLOT(setEnabled(bool)));
    connect(dozerDDCheckBox, SIGNAL(toggled(bool)),
            dozerDDSeasonCheckBox, SLOT(setEnabled(bool)));
    dozerDDCheckBox->setChecked(false);
}

/**
 * \brief Load the default resource count into the map for editing.
 *
 * This is hacked in and uses too much sqlite, should be moved into the data
 * access stuff at some point.
 *
 */
void OmffrMainWindow::LoadDefaultResourceCounts()
{
    /*
    if(poDA == NULL || agencyComboBox->currentText() != "FS")
    {
        return;
    }

    int rc = 0;
    sqlite3 *db;

    sqlite3_stmt *stmt;
    poDA->CreateGeneralResourceDb(":memory:", &db);
    rc = sqlite3_prepare_v2(db, "SELECT count FROM general_resc "
                                "WHERE gacc=? AND type=?", -1, &stmt, NULL);

    int i = 0;
    QList<QString> keys = rescTypeMap.keys();
    int nCount;
    while(GaccMap[i].pszCode != NULL)
    {
        for(int j = 0;j < keys.size();j++)
        {
            rc = sqlite3_bind_text(stmt, 1, GaccMap[i].pszCode, -1, NULL);
            rc = sqlite3_bind_text(stmt, 2, keys[j].toStdString().c_str(), -1,
                                   SQLITE_TRANSIENT);
            rc = sqlite3_step(stmt);
            if(rc != SQLITE_ROW)
            {
                nCount = 0;
            }
            else
            {
                nCount = sqlite3_column_int(stmt, 0);
            }
            regionRescMap[GaccMap[i].nRegion][keys[j]] = nCount;
            sqlite3_reset(stmt);
        }
        i++;
    }
    sqlite3_finalize(stmt);
    stmt = NULL;
    sqlite3_close(db);
    db = NULL;
    UpdateSpinBoxes(regionComboBox->currentIndex());
    */
}

void OmffrMainWindow::SaveDefaultResourceCounts()
{
    /*
    int nRegion = IndexToRegion(regionComboBox->currentIndex());
    QList<QString> keys = rescTypeMap.keys();
    QMap<QString, QSpinBox*>::iterator it = rescTypeMap.begin();
    for(; it != rescTypeMap.end(); it++)
    {
        regionRescMap[nRegion][it.key()] = it.value()->value();
    }
    */
}

void OmffrMainWindow::UpdateSpinBoxes(int index)
{
    int nRegion = IndexToRegion(index);
    QList<QString> keys = rescTypeMap.keys();
    QMap<QString, QSpinBox*>::iterator it = rescTypeMap.begin();
    for(; it != rescTypeMap.end(); it++)
    {
        it.value()->setValue(regionRescMap[nRegion][it.key()]);
    }
}

int OmffrMainWindow::IndexToRegion(int index)
{
    return index > 5 ? index + 2 : index + 1;
}

int OmffrMainWindow::RegionToIndex(int region)
{
    return region > 6 ? region - 2 : region -1;
}

QString OmffrMainWindow::RegionName(int region)
{
    int i = 0;
    while(GaccMap[i].pszName != NULL)
    {
        if(GaccMap[i].nRegion == region)
        {
            return QString(GaccMap[i].pszCode);
        }
        i++;
    }
    return "";
}

/**
 * \brief Initialize the QGIS map widget and dependencies
 *
 * This creates map canvas and sets normal values such as background,
 * anti-aliasing, etc.  This also creates any map tools and map layers we will
 * need to load into the map.  Order is set here for layers.  All point layers
 * come first.
 */
void OmffrMainWindow::SetUpMapCanvas()
{
    mapCanvas = new QgsMapCanvas(0, 0);
    mapCanvas->enableAntiAliasing(true);
    mapCanvas->setCanvasColor(QColor(255, 255, 255));
    mapCanvas->freeze(false);
    mapCanvas->setVisible(true);
    mapCanvas->refresh();
    mapLayout = new QVBoxLayout(mapFrame);
    mapLayout->addWidget(mapCanvas);

    mapPanTool = new QgsMapToolPan(mapCanvas);
    mapZoomInTool = new QgsMapToolZoom(mapCanvas, FALSE);
    mapSimpleIdentifyTool = new SimpleIdentifyMapTool(mapCanvas);
    mapSelectTool = new QgsMapToolEmitPoint(mapCanvas);
    mapSimpleAreaTool = new SimpleAreaMapTool(mapCanvas);
    rubberBand = new QgsRubberBand(mapCanvas, true);

    QgsVectorLayer *layer;
    QgsFeatureRendererV2 *renderer;
    QList<QColor>pointLayerColors;
    pointLayerColors.append(QColor(255, 0, 0));
    QgsSymbolV2 *symbol;
    QString layerName;
    for(int i = 0; i < pointLayers.size();i++)
    {
        layerName = layerPath + pointLayers[i];
        layer = new QgsVectorLayer(layerName, "", providerName);
        qDebug() << layerName << ":" << layer << " Provider: " << providerName;
        //symbol = QgsSymbolV2::defaultSymbol(layer->geometryType());
        symbol = QgsSymbolV2::defaultSymbol(QGis::Point);
        qDebug() << "Geometry type: " << layer->geometryType() << " Symbol: " << symbol;
        symbol->setColor(QColor(255, 0, 0));
        renderer =
            QgsFeatureRendererV2::defaultRenderer(QGis::Point);
            //QgsFeatureRendererV2::defaultRenderer(layer->geometryType());
        layer->setRendererV2(renderer);
        ((QgsSingleSymbolRendererV2*)renderer)->setSymbol(symbol);
        QgsMapLayerRegistry::instance()->addMapLayer(layer, true);
        mapLayerSet.append(QgsMapCanvasLayer(layer, false));
        vectorLayers.append(layer);
        renderers.append(renderer);
    }

    dispatchLayerId = vectorLayers[0]->id();

    for(int i = 0; i < areaLayers.size();i++)
    {
        layer = new QgsVectorLayer(layerPath + areaLayers[i], "", providerName);
        //symbol = QgsSymbolV2::defaultSymbol(layer->geometryType());
        symbol = QgsSymbolV2::defaultSymbol(QGis::Polygon);
        renderer =
            QgsFeatureRendererV2::defaultRenderer(QGis::Polygon);
            //QgsFeatureRendererV2::defaultRenderer(layer->geometryType());
        symbol->setColor(QColor(115, 165, 214));
        layer->setRendererV2(renderer);
        ((QgsSingleSymbolRendererV2*)renderer)->setSymbol(symbol);
        QgsMapLayerRegistry::instance()->addMapLayer(layer, TRUE);
        mapLayerSet.append(QgsMapCanvasLayer(layer, false));
        vectorLayers.append(layer);
        renderers.append(renderer);
    }

    mapCanvas->setLayerSet(mapLayerSet);
    mapCanvas->setExtent(QgsRectangle(-129.0, 22.0, -93.0, 52.0));
    //mapCanvas->setExtent(vectorLayers[pointLayers.size()]->extent());
    mapCanvas->refresh();
    //mapCanvas->show();
    //rubberBand->show();
}

void OmffrMainWindow::SetUpPlot()
{
    yearCurve = new QwtPlotCurve("Fires per day");
    QPen yearPen(QBrush(Qt::blue), 2.0);
    yearPen.setJoinStyle(Qt::RoundJoin);
    yearCurve->setPen(yearPen);
    yearCurve->setItemAttribute(QwtPlotItem::Legend, true);

    minCurve = new QwtPlotCurve("Minimum fires per day");
    QPen minPen(QBrush(Qt::darkGreen), 2.0);
    minPen.setJoinStyle(Qt::RoundJoin);
    minCurve->setPen(minPen);
    minCurve->setItemAttribute(QwtPlotItem::Legend, true);

    maxCurve = new QwtPlotCurve("Maximum fires per day");
    QPen maxPen(QBrush(Qt::red), 2.0);
    maxPen.setJoinStyle(Qt::RoundJoin);
    maxCurve->setPen(maxPen);
    maxCurve->setItemAttribute(QwtPlotItem::Legend, true);

    meanCurve = new QwtPlotCurve("Mean fires per day");
    QPen meanPen(QBrush(Qt::darkYellow), 2.0);
    meanPen.setJoinStyle(Qt::RoundJoin);
    meanCurve->setPen(meanPen);
    meanCurve->setItemAttribute(QwtPlotItem::Legend, true);

    escapeCurve = new QwtPlotCurve("Escapes");
    QPen escapePen(QBrush(Qt::red), 2.0);
    escapePen.setJoinStyle(Qt::RoundJoin);
    escapeCurve->setPen(escapePen);
    escapeCurve->setItemAttribute(QwtPlotItem::Legend, true);

    containCurve = new QwtPlotCurve("Containments");
    QPen containPen(QBrush(Qt::blue), 2.0);
    containPen.setJoinStyle(Qt::RoundJoin);
    containCurve->setPen(containPen);
    containCurve->setItemAttribute(QwtPlotItem::Legend, true);

    norescCurve = new QwtPlotCurve("No Resources Sent");
    QPen norescPen(QBrush(Qt::darkYellow), 2.0);
    norescPen.setJoinStyle(Qt::RoundJoin);
    norescCurve->setPen(norescPen);
    norescCurve->setItemAttribute(QwtPlotItem::Legend, true);

    qwtPlot->insertLegend(new QwtLegend());
    qwtPlot->updateLayout();
 }

/**
 * \brief Set icons for tool buttons and connect tool button slot to tool
 *        selection for the map canvas.
 */
void OmffrMainWindow::SetUpToolButtons()
{
    panToolButton->setIcon(QIcon(":mActionPan.png"));
    panToolButton->setToolTip(tr("Pan"));
    zoomInToolButton->setIcon(QIcon(":mActionZoomIn.png"));
    zoomInToolButton->setToolTip(tr("Zoom In"));
    identifyToolButton->setIcon(QIcon(":mActionIdentify.png"));
    identifyToolButton->setToolTip(tr("Identify"));
    selectToolButton->setIcon(QIcon(":mActionSelect.png"));
    selectToolButton->setToolTip(tr("Select Point"));
    selectAreaToolButton->setIcon(QIcon(":mActionSelectRectangle.png"));
    selectAreaToolButton->setToolTip(tr("Select Rectangle"));
    connect(panToolButton, SIGNAL(clicked()),
            this, SLOT(SetNavigationMode()));
    connect(zoomInToolButton, SIGNAL(clicked()),
            this, SLOT(SetNavigationMode()));
    connect(identifyToolButton, SIGNAL(clicked()),
            this, SLOT(SetNavigationMode()));
    connect(selectToolButton, SIGNAL(clicked()),
            this, SLOT(SetNavigationMode()));
    connect(selectAreaToolButton, SIGNAL(clicked(bool)),
            this, SLOT(SetNavigationMode()));

    connect(mapSimpleAreaTool, SIGNAL(AreaSelected(QgsGeometry*)),
            this, SLOT(SelectGeometries(QgsGeometry*)));
    connect(mapSelectTool, SIGNAL(canvasClicked(const QgsPoint &, Qt::MouseButton)),
            this, SLOT(SelectGeometryFromPoint(const QgsPoint &, Qt::MouseButton)));
    connect(mapSimpleIdentifyTool, SIGNAL(canvasReleaseEvent(QMouseEvent*)),
            this, SLOT(SelectGeometryFromPoint(const QgsPoint &, Qt::MouseButton)));
}

/**
 * \brief Create all connections for the main window
 */
void OmffrMainWindow::CreateConnections()
{
    connect(areaLayerComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(ShowAreaLayer(int)));
    connect(pointLayerComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(ShowPointLayer(int)));
    connect(connectDbButton, SIGNAL(clicked()),
            this, SLOT(ConnectToDB()));
    connect(mapSimpleIdentifyTool,
            SIGNAL(SimpleIdentify(QList<QgsMapToolIdentify::IdentifyResult>)),
            this, SLOT(Identify(QList<QgsMapToolIdentify::IdentifyResult>)));
    //connect(loadDefaultButton, SIGNAL(clicked()),
            //this, SLOT(LoadDefaultResourceCounts()));
    //connect(saveDefaultButton, SIGNAL(clicked()),
            //this, SLOT(SaveDefaultResourceCounts()));
    //connect(regionComboBox, SIGNAL(currentIndexChanged(int)),
            //this, SLOT(UpdateSpinBoxes(int)));
    connect(figPlotMapButton, SIGNAL(clicked()),
            this, SLOT(PlotFigMap()));
    connect(figPlotGraphButton, SIGNAL(clicked()),
            this, SLOT(PlotFigGraph()));
    connect(figStartDate, SIGNAL(dateChanged(const QDate&)),
            this, SLOT(UpdateFigDate(const QDate&)));
    connect(figYearCombo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(UpdateStatsCheckBox(int)));
    connect(resourceComboBox, SIGNAL(currentIndexChanged(int)),
            resourceStackedWidget, SLOT(setCurrentIndex(int)));
    resourceComboBox->setCurrentIndex( 1 );
    resourceComboBox->setCurrentIndex( 0 );
    //connect(currentResourcesCheckBox, SIGNAL(clicked(bool)),
            //this, SLOT(SetRegionalResourcesDisabled(bool)));
    //connect(cartButton, SIGNAL(clicked()),
            //this, SLOT(ShowCartDiagram()));
    connect(resultsLoadButton, SIGNAL(clicked()),
            this, SLOT(LoadResultsFile()));
    //connect(plotResultButton, SIGNAL(clicked()),
            //this, SLOT(PlotFireResults()));

    connect(runIRSButton, SIGNAL(clicked()),
            this, SLOT(RunIRS()));
	connect(resultScaleComboBox, SIGNAL(currentIndexChanged(int)),
			this, SLOT(ScaleChanged(int)));
	connect(resultAttributeComboBox, SIGNAL(currentIndexChanged(int)),
			this, SLOT(PlotResults(int)));
    connect(resultGACCBox, SIGNAL(currentIndexChanged(int)),
        this, SLOT(PlotResults(int)));
	connect(seriesSpinBox, SIGNAL(valueChanged(int)), 
		this, SLOT(changeSeriesItem(int)));
	connect(displayTabWidget, SIGNAL(currentChanged(int)),
		this, SLOT(displayTabChanged(int)));
	connect(allSeriesButton, SIGNAL(clicked()),
			this, SLOT(SeriesPlotResults()));
        connect(exportOgrToolButton, SIGNAL(clicked()),
                this, SLOT(ExportOgr()));
        connect(exportCsvToolButton, SIGNAL(clicked()),
                this, SLOT(ExportCsv()));
        connect(autoReduceGroupBox, SIGNAL(clicked()),
                this, SLOT(ToggleAutoReduce()));
        connect(autoReduceComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(ToggleAutoReduce()));
        connect(fuelTreatGroupBox, SIGNAL(clicked()),
                this, SLOT(ToggleAutoReduce()));
        connect(rescReduceGroupBox, SIGNAL(clicked()),
                this, SLOT(ToggleAutoReduce()));
        connect(rescRedRandRadioButton, SIGNAL(toggled(bool)),
                usfsRedCheckBox, SLOT(setEnabled(bool)));
        connect(rescRedRandRadioButton, SIGNAL(toggled(bool)),
                doiRedCheckBox, SLOT(setEnabled(bool)));
        connect(rescRedRandRadioButton, SIGNAL(toggled(bool)),
                redTypeListBox, SLOT(setEnabled(bool)));
}

void OmffrMainWindow::SetRegionalResourcesDisabled(bool disable)
{
    //regionalResourcesPage->setEnabled(!disable);
}

void OmffrMainWindow::UpdateFigDate(const QDate &date)
{
    if(figEndDate->date() < date)
    {
        figEndDate->setDate(date);
    }
}

/**
 * \brief Set the navigation mode.
 *
 * Slot to catch the input from the combo box that chooses the navigation tool.
 * Pan is set as the default in the constructor.  Add any/all tools here for
 * the app.  The index of the combobox is cast to an enum of NavigationMode.
 *
 * \param type index of chosen tool
 */
void OmffrMainWindow::SetNavigationMode()
{
    if(panToolButton->isChecked())
    {
        mapCanvas->setMapTool(mapPanTool);
    }
    else if(zoomInToolButton->isChecked())
    {
        mapCanvas->setMapTool(mapZoomInTool);
    }
    else if(identifyToolButton->isChecked())
    {
        mapCanvas->setMapTool(mapSimpleIdentifyTool);
        ClearSelection();
    }
    else if(selectToolButton->isChecked())
    {
        mapCanvas->setMapTool(mapSelectTool);
        ClearSelection();
    }
    else if(selectAreaToolButton->isChecked())
    {
        mapCanvas->setMapTool(mapSimpleAreaTool);
        ClearSelection();
    }
}

OmffrMainWindow::NavigationMode OmffrMainWindow::CurrentNavigationMode()
{
    if(panToolButton->isChecked())
    {
        return panMode;
    }
    else if(zoomInToolButton->isChecked())
    {
        return zoomMode;
    }
    else if(identifyToolButton->isChecked())
    {
        return identifyMode;
    }
    else if(selectToolButton->isChecked())
    {
        return selectMode;
    }
    else if(selectAreaToolButton->isChecked())
    {
        selectAreaMode;
    }
    else
        return (NavigationMode)-1;
}

void OmffrMainWindow::SetUpFigInput()
{
    if(poDA == NULL)
    {
        return;
    }
    figYearCombo->addItem("Statistics Only");
    std::vector<int> years = poDA->GetYearIndexes();
    std::vector<int>::iterator it = years.begin();
    for(;it != years.end();it++)
    {
        figYearCombo->addItem(QString::number(*it), QVariant(*it));
    }
}

void OmffrMainWindow::ShowRubberBand()
{
    QgsPoint myPoint1 = mapCanvas->getCoordinateTransform()->toMapCoordinates(0, 100);
    rubberBand->addPoint(myPoint1);
    QgsPoint myPoint2 = mapCanvas->getCoordinateTransform()->toMapCoordinates(100, 0);
    rubberBand->addPoint(myPoint2);
    QgsPoint myPoint3 = mapCanvas->getCoordinateTransform()->toMapCoordinates(100, 100);
    rubberBand->addPoint(myPoint3);
    QgsPoint myPoint4 = mapCanvas->getCoordinateTransform()->toMapCoordinates(0, 100);
    rubberBand->addPoint(myPoint4);
}

/**
 * \brief Select and show an area based layer.
 *
 * Slot to show a certain area layer.  Since all point layers come first, the
 * index for the for loop starts at pointLayers.size().
 *
 * \param index combobox index to choose layer
 */
void OmffrMainWindow::ShowAreaLayer(int index)
{
    index += pointLayers.size();
    QgsRectangle extent = mapCanvas->extent();
    int layerIndex = -1;
    for(int i = pointLayers.size();i < mapLayerSet.size();i++ )
    {
        if(i == index - 1)
        {
            mapLayerSet[i].setVisible(true);
            layerIndex = i;
        }
        else
        {
            mapLayerSet[i].setVisible(false);
        }
    }
    if(extent.xMaximum() == 0 && extent.yMaximum() == 0 &&
       extent.xMinimum() == 0 && extent.yMinimum() == 0)
    {
        if(layerIndex == -1)
        {
            layerIndex = 0;
        }
        extent = mapLayerSet[layerIndex].layer()->extent();
    }
    mapCanvas->setLayerSet(mapLayerSet);
    mapCanvas->setExtent(extent);
    mapCanvas->refresh();
    if(layerIndex >= 0)
    {
        mapCanvas->setCurrentLayer(mapLayerSet[layerIndex].layer());
    }
    ClearSelection();
}

/**
 * \brief Select and show an point based layer.
 *
 * Slot to show a certain point layer.  Since all point layers come first, the
 * index for the for loop ends at pointLayers.size().
 *
 * \param index combobox index to choose layer
 */
void OmffrMainWindow::ShowPointLayer(int index)
{
    QgsRectangle extent = mapCanvas->extent();
    int layerIndex = -1;
    for(int i = 0;i < pointLayers.size();i++ )
    {
        if(i == index - 1)
        {
            mapLayerSet[i].setVisible(true);
            /* Ignitions */
            qDebug() << pointLayerComboBox->currentText();
            if(index == 3)
            {
                int year = figYearCombo->currentText().toInt();
                int start = figStartDate->date().toJulianDay();
                int end = figEndDate->date().toJulianDay();
                QString where;
                where = "WHERE year=" + QString::number(year) +
                                        "AND jul_day>" +
                                        QString::number(start) +
                                        "AND jul_day>" +
                                        QString::number(end);
                ((QgsVectorLayer*)
                 (mapLayerSet[i].layer()))->setSubsetString(where);
                if(CurrentNavigationMode() == selectAreaMode &&
                        mapSimpleAreaTool->GetWkt() != "")
                {
                }
            }
            layerIndex = i;
        }
        else
        {
            mapLayerSet[i].setVisible(false);
        }
    }
    mapCanvas->setLayerSet(mapLayerSet);
    mapCanvas->setExtent(extent);
    mapCanvas->refresh();
}

static int UpdateProgress(double progress, const char *message, void *unused)
{
    QCoreApplication::processEvents();
    if( progress > 0.0 )
        globalProgress->setValue((int)(progress * 100));
    else
        globalProgress->setValue(globalProgress->value());
    globalProgressLabel->setText(QString(message));
    QCoreApplication::processEvents();
    return 0;
}

int OmffrMainWindow::RunIRS()
{
    //QtConcurrent::run(this, &OmffrMainWindow::SimulateIRS);
    SimulateIRS();
    return 0;
}

void OmffrMainWindow::RunIRSConcurrent( void *pUnused )
{
    SimulateIRS();
}

int OmffrMainWindow::SimulateIRS()
{
#ifdef WIN32
#ifdef IRS_DIANE_BUILD
    poSuite = (IRSSuite*)IRSSuite::Create( "c:/src/omffr/data/omffr.sqlite", 0 );
#else
    poSuite = (IRSSuite*)IRSSuite::Create( "c:/Users/ksshannon/Documents/GitHub/build/data/omffr.sqlite", 0 );
#endif
#else
    poSuite = (IRSSuite*)IRSSuite::Create( "/home/kyle/src/omffr/trunk/data/omffr.sqlite", 0 );
#endif
	if( !poSuite )
	{
		QMessageBox::warning(this, tr("Initial Response Simulator"),
                             tr("Could not open the SQLite database."),
                             QMessageBox::Ok );
        return 1;
	}
    QString outputFile =
        QFileDialog::getSaveFileName(this, tr("Save SQLite database"), "",
                                     tr("SQLite DB Files (*.sqlite *.db)"));
    if(outputFile.isEmpty())
    {
        return 1;
    }
    QFile file(outputFile);
    if(file.exists())
    {
        QFile::remove(outputFile);
    }
    qDebug() << outputFile;

    QString osWkt = mapSimpleAreaTool->GetWkt();
    if(osWkt != "")
    {
        poSuite->SetSpatialFilter(osWkt.toStdString().c_str());
    }
    /* Check for selection */
    else
    {
        QgsVectorLayer *layer;
        layer = dynamic_cast<QgsVectorLayer*>(mapCanvas->currentLayer());
        if(layer)
        {
            QgsFeatureList featureList = layer->selectedFeatures();
            if(featureList.size() > 0)
            {
                QVariant place;
                QString placename;
                place = featureList[0].attribute("fpu_code");
                if(place.isValid())
                    poSuite->SetFpuFilter( place.toString().toStdString().c_str() );
                else
                {
                    place = featureList[0].attribute("ga_abbr");
                    if(place.isValid())
                        poSuite->SetFpuFilter( place.toString().toStdString().c_str() );
                }
            }
        }
    }

    qDebug() << "Running IRS";
    qDebug() << "Filter: " << mapSimpleAreaTool->GetWkt();

    globalProgress->setRange(0, 100);
    globalProgress->setValue(0);

    poSuite->SetQuiet( FALSE );
    poSuite->SetOutputPath( outputFile.toLocal8Bit().data() );
    int rc = SetPreposAndDrawDown();

    IRSMultiRunData sData;
    rc = FillRunData( &sData );

    //poSuite->SetOgrOutputPath( pszShapeOut );
    //poSuite->SetTreatmentProb( 0.7 );
    //poSuite->LoadData( &UpdateProgress );

    //QtConcurrent::run( poSuite, &IRSSuite::RunAllScenarios, 6, &UpdateProgress );
    mapFrame->setDisabled(true);
    tabWidget->setDisabled(true);
    int nRunCount = 1;
    if( sData.nRescRedSteps )
        nRunCount = sData.nRescRedSteps;
    else if( sData.nTreatPercSteps )
        nRunCount = sData.nTreatPercSteps;
    else
        nRunCount = 1;
    int nThreads = threadSpinBox->value();
    sData.nYearCount = yearsSpinBox->value() ? yearsSpinBox->value() : 1;

    poSuite->SetTankerCount( largeAttSpinBox->value() );

    poSuite->SetMultiRun( &sData, nRunCount, nThreads, &UpdateProgress );
    /*
    poSuite->RunAllScenarios( 6, &UpdateProgress );
    UpdateProgress(0, "Writing Outputs...", NULL );
    poSuite->PostProcessLargeFire( 0, 1.0, 50, &UpdateProgress );
    UpdateProgress(0.5, "Writing Outputs...", NULL );
    poSuite->SummarizeByFpu( &UpdateProgress );
    poSuite->ExportFpuSummary( &UpdateProgress );
    */
    UpdateProgress(1.0, "Output written.", NULL );
    IRSSuite::Destroy( poSuite );
    mapFrame->setEnabled(true);
    tabWidget->setEnabled(true);

    /*
    if(saveDetailsCheckBox->isChecked())
    {
        poDA->WriteCartFile(pszOutput, "fires.csv");
    }
    */
    tabWidget->setCurrentIndex(6);
    newIRSRun = true;
    currentResultsFile = outputFile;
    return 0;
}

void OmffrMainWindow::WriteResourceDatabase(QString file)
{
    int rc = 0;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    poDA->CreateGeneralResourceDb(file.toStdString().c_str(), &db);
    rc = sqlite3_prepare_v2(db, "UPDATE general_resc set count=? "
                                "WHERE gacc=? AND type=?", -1, &stmt, NULL);

    int i = 0;
    QList<QString> keys = rescTypeMap.keys();
    int nCount;
    while(GaccMap[i].pszCode != NULL)
    {
        for(int j = 0;j < keys.size();j++)
        {
            nCount = regionRescMap[GaccMap[i].nRegion][keys[j]];
            rc = sqlite3_bind_int(stmt, 1, nCount);
            rc = sqlite3_bind_text(stmt, 2, GaccMap[i].pszCode, -1, NULL);
            rc = sqlite3_bind_text(stmt, 3, keys[j].toStdString().c_str(), -1,
                                   SQLITE_TRANSIENT);
            rc = sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        i++;
    }
    sqlite3_finalize(stmt);
    stmt = NULL;
    sqlite3_close(db);
    db = NULL;
}

void OmffrMainWindow::UpdateFromDatatbase()
{
    SetUpFigInput();
    showStatsCheckBox->setChecked(false);
    figYearCombo->setCurrentIndex(1);
    std::vector<int> years = poDA->GetYearIndexes();
    yearsSpinBox->setRange(1, years.size());
}

void OmffrMainWindow::PlotFigMap()
{
    if(poDA != NULL)
    {
        displayTabWidget->setCurrentIndex(0);
        ShowPointLayer(3);
    }
}

void OmffrMainWindow::PlotFigGraph()
{
    if(poDA != NULL)
    {
        escapeCurve->attach(NULL);
        containCurve->attach(NULL);
        norescCurve->attach(NULL);
        displayTabWidget->setCurrentIndex(1);
        QString wkt = mapSimpleAreaTool->GetWkt();
        const char *pszGeometry;
        qDebug() << wkt;
        if(wkt != "")
        {
            pszGeometry = strdup(wkt.toStdString().c_str());
        }
        else
        {
            pszGeometry = NULL;
        }
        int year = figYearCombo->currentText().toInt();
        int start = figStartDate->date().dayOfYear();
        int end = figEndDate->date().dayOfYear();
        if(end >= 365)
        {
            end = 364;
        }
        if(start >= 365)
        {
            start = 364;
        }
        if(figYearCombo->currentIndex() > 0)
        {
            yearCurve->attach(qwtPlot);
            std::vector<int> count = poDA->GetFireCount(year, pszGeometry);
            QwtArray<double>xPoints;
            QwtArray<double>yPoints;
            //QVector<QPointF>points;
            for(int i = start;i < end;i++)
            {
                xPoints.push_back(double(i));
                yPoints.push_back(double(count[i]));
                //points.push_back(QPointF(double(i), double(count[i])));
            }
            yearData = new QwtArrayData(xPoints, yPoints);
            //yearData->setSamples(points);
            //yearCurve->setData(yearData);
            yearCurve->setData(xPoints, yPoints);
        }
        if(showStatsCheckBox->isChecked())
        {
            minCurve->attach(qwtPlot);
            maxCurve->attach(qwtPlot);
            meanCurve->attach(qwtPlot);
            std::vector<int>max, min;
            std::vector<double> mean;
            if(!statsCached || !CompareGeometry(pszGeometry, pszGeometryCache))
            {
                setCursor(Qt::BusyCursor);
                min = poDA->GetFireStatsMin(pszGeometry, NULL);
                max = poDA->GetFireStatsMax(pszGeometry, NULL);
                mean = poDA->GetFireStatsMean(pszGeometry, NULL);
                minIgnitionsCache = min;
                maxIgnitionsCache = max;
                meanIgnitionsCache = mean;
                if(pszGeometryCache != NULL)
                {
                    free((void*)pszGeometryCache);
                }
                if(pszGeometry == NULL)
                {
                    pszGeometryCache = NULL;
                }
                else
                {
                    pszGeometryCache = strdup(pszGeometry);
                }
                statsCached = true;
                setCursor(Qt::ArrowCursor);
            }
            else
            {
                min = std::vector<int>(minIgnitionsCache);
                max = std::vector<int>(maxIgnitionsCache);
                mean = std::vector<double>(meanIgnitionsCache);
            }
            //QVector<QPointF>maxPoints, minPoints;
            QwtArray<double>maxXPoints, maxYPoints, minXPoints, minYPoints, meanXPoints, meanYPoints;
            for(int i = start;i < end;i++)
            {
                minXPoints.push_back(double(i));
                minYPoints.push_back(double(min[i]));
                maxXPoints.push_back(double(i));
                maxYPoints.push_back(double(max[i]));
                meanXPoints.push_back(double(i));
                meanYPoints.push_back(double(mean[i]));
                //minPoints.push_back(QPointF(double(i), double(min[i])));
                //maxPoints.push_back(QPointF(double(i), double(max[i])));
            }
            //minData->setSamples(minPoints);
            minCurve->setData(minXPoints, minYPoints);
            //maxData->setSamples(maxPoints);
            maxCurve->setData(maxXPoints, maxYPoints);
            meanCurve->setData(meanXPoints, meanYPoints);
        }
        else
        {
            minCurve->attach(NULL);
            maxCurve->attach(NULL);
            meanCurve->attach(NULL);
        }
        qwtPlot->insertLegend(qwtPlot->legend());
        qwtPlot->replot();
    }
}

int OmffrMainWindow::ConnectToDB()
{
    if(poDA)
    {
        IRSDataAccess::Destroy( poDA );
    }
    QString path;
    QString inputDbFile;
    path = QString(getenv("OMFFR_DATA"));
    inputDbFile =
        QFileDialog::getOpenFileName(this, tr("Open SQLite database"), path,
                                     tr("SQLite DB Files (*.sqlite *.db)"));
    qDebug() << inputDbFile;
    std::string osTmp = inputDbFile.toStdString();
    const char *pszInputDb = strdup( osTmp.c_str() );
    qDebug() << pszInputDb;
    poDA = IRSDataAccess::Create(0, pszInputDb);
    //poDA = new SpatialiteDataAccess(inputDbFile.toStdString().c_str());
    if(poDA == NULL)
    {
        QMessageBox::warning(this, tr("Initial Response Simulator"),
                             tr("Could not open the SQLite database."),
                             QMessageBox::Ok );
        return 1;
    }
    dataPath = path;
    UpdateFromDatatbase();
    return 0;
}

/**
 * \brief Display information based on a user click on the map.
 *
 * Depending on settings, display basic information, or output information.
 * \param results returned results from the identify tool.
 */
void OmffrMainWindow::Identify(QList<QgsMapToolIdentify::IdentifyResult>results)
{
    if (apResults.size() == 0) return;
    
    int size = results.size();
    if(size == 0)
    {
        return;
    }
    for(int i = 0; i < size;i++)
    {
        qDebug() << results[i].mDerivedAttributes;
        qDebug() << results[i].mFeature.attributes();
    }

    QDialog *dialog = new QDialog(this, Qt::WindowStaysOnTopHint);
    QGridLayout *grid = new QGridLayout;
    QLabel *id = new QLabel;
    id->setText(results[0].mFeature.attribute(0).toString());

    grid->addWidget(id, 0, 0);

    vector<QString> attributes;
    attributes.push_back("Percent Contained");
    attributes.push_back("Large Fire Cost");
    attributes.push_back("Large Fire Acres");
    attributes.push_back("Large Fire Population");
    attributes.push_back("Resource Usage");
    vector<QString> values;

    int numYears = apResults[0]->GetNumYears();

    for (int i = 0; i < 5; i++) {
        int value = 0;

        if (i == 0) 
            value = static_cast<int>((results[0].mFeature.attribute(i + 6).toDouble() + 0.005) * 100 );
        else
            value = static_cast<int>(((results[0].mFeature.attribute(i + 6).toDouble()) + 0.5) / numYears);

        char theValue[20];
        
        int n = sprintf(theValue, "%d", value);

        values.push_back(QString(theValue));
    }

    QLabel *value1 = new QLabel();
    value1->setText(values[0]);
    grid->addWidget(value1, 1, 1);
    QLabel *id1 = new QLabel;
    id1->setText(attributes[0]);
    grid->addWidget(id1, 1, 0);
    QLabel *value2 = new QLabel();
    value2->setText(values[1]);
    grid->addWidget(value2, 2, 1);
    QLabel *id2 = new QLabel;
    id2->setText(attributes[1]);
    grid->addWidget(id2, 2, 0);
    QLabel *value3 = new QLabel();
    value3->setText(values[2]);
    grid->addWidget(value3, 3, 1);
    QLabel *id3 = new QLabel;
    id3->setText(attributes[2]);
    grid->addWidget(id3, 3, 0);
    QLabel *value4 = new QLabel();
    value4->setText(values[3]);
    grid->addWidget(value4, 4, 1);
    QLabel *id4 = new QLabel;
    id4->setText(attributes[3]);
    grid->addWidget(id4, 4, 0);
    QLabel *value5 = new QLabel();
    value5->setText(values[4]);
    grid->addWidget(value5, 5, 1);
    QLabel *id5 = new QLabel;
    id5->setText(attributes[4]);
    grid->addWidget(id5, 5, 0);

    dialog->setLayout(grid);
    //Qt::WindowFlags flags = 0;
    //flags |= Qt::WindowStaysOnTopHint;
    //dialog->setWindowFlags(flags);
    //dialog->setWindowFlags(Qt::WindowStaysOnTopHint);
    dialog->setWindowTitle("Attributes");
    dialog->show();

}

void OmffrMainWindow::ShowIdentifyResults(std::map<std::string, std::string> attributes)
{
    /*
    identifyTableWidget->clear();
    if(attributes.size() > 0)
    {
        QTableWidgetItem *item;
        QList<QTableWidgetItem*>itemList;
        std::map<std::string, std::string>::iterator it;
        it = attributes.begin();
        int i = 0;
        for(;it != attributes.end();it++)
        {
            identifyTableWidget->insertRow(identifyTableWidget->rowCount() + 1);
            item = new QTableWidgetItem();
            item->setText(QString::fromStdString(it->first));
            identifyTableWidget->setItem(i, 0, item);
            itemList.append(item);
            item = new QTableWidgetItem();
            item->setText(QString::fromStdString(it->second));
            identifyTableWidget->setItem(i, 1, item);
            itemList.append(item);
            i++;
        }
    }
    */
}

void OmffrMainWindow::DeleteTreeItems()
{
    return;
}

void OmffrMainWindow::UpdateStatsCheckBox(int index)
{
    if(index == 0)
    {
        showStatsCheckBox->setChecked(true);
    }
}

bool OmffrMainWindow::CompareGeometry(const char *pszOne, const char *pszTwo)
{
    if(pszOne == NULL && pszTwo == NULL)
    {
        return true;
    }
    else if(pszOne == NULL || pszTwo == NULL )
    {
        return false;
    }
    else if(EQUAL(pszOne, pszTwo))
    {
        return true;
    }
    return false;
}

void OmffrMainWindow::ReadFwaSummaryResults(QString inputFile)
{
    if(inputFile.isEmpty())
    {
        return;
    }
    const char *pszFile = strdup(inputFile.toStdString().c_str());
    int rc = 0;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    rc = sqlite3_open_v2(pszFile, &db, SQLITE_OPEN_READONLY, NULL);
    rc = sqlite3_prepare_v2(db, "SELECT fwa, sum(contained), sum(escapes), "
                                "sum(norescs) FROM results GROUP BY "
                                "fwa", -1, &stmt, NULL);
    std::string fwa;
    FwaResults.clear();
    std::vector<int>counts(3);
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        fwa = std::string((const char*)sqlite3_column_text(stmt, 0));
        counts[0] = sqlite3_column_int(stmt, 1);
        counts[1] = sqlite3_column_int(stmt, 2);
        counts[2] = sqlite3_column_int(stmt, 3);
        FwaResults.insert(std::pair<std::string, std::vector<int> >(fwa, counts));
    }
    sqlite3_reset(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void OmffrMainWindow::SetPPLevels()
{
    std::string rescType;
    double level;
    bool outOfSeason;

    IRSRunner->ATTpp.rescType = "ATT";
    if(tankerPPCheckBox->isChecked())
    {
        IRSRunner->ATTpp.level = tankerPPSpinBox->value();
    }
    else
    {
        IRSRunner->ATTpp.level = 0.0;
    }
    IRSRunner->ATTpp.outOfSeason = tankerPPSeasonCheckBox->isChecked();

    IRSRunner->RCRWpp.rescType = "CRW20";
    if(crewPPCheckBox->isChecked())
    {
        IRSRunner->RCRWpp.level = crewPPSpinBox->value();
    }
    else
    {
        IRSRunner->RCRWpp.level = 0.0;
    }
    IRSRunner->RCRWpp.outOfSeason = crewPPSeasonCheckBox->isChecked();

    IRSRunner->RHelpp.rescType = "HEL1";
    if(helPPCheckBox->isChecked())
    {
        IRSRunner->RHelpp.level = helPPSpinBox->value();
    }
    else
    {
        IRSRunner->RHelpp.level = 0.0;
    }
    IRSRunner->RHelpp.outOfSeason = helPPSeasonCheckBox->isChecked();

    IRSRunner->FSCRWpp.rescType = "CRW20";
    if(crewPPCheckBox->isChecked())
    {
        IRSRunner->FSCRWpp.level = crewPPSpinBox->value();
    }
    else
    {
        IRSRunner->FSCRWpp.level = 0.0;
    }
    IRSRunner->FSCRWpp.outOfSeason = crewPPSeasonCheckBox->isChecked();

    IRSRunner->DOICRWpp.rescType = "CRW20";
    if(crewPPCheckBox->isChecked())
    {
        IRSRunner->DOICRWpp.level = crewPPSpinBox->value();
    }
    else
    {
        IRSRunner->DOICRWpp.level = 0.0;
    }
    IRSRunner->DOICRWpp.outOfSeason = crewPPSeasonCheckBox->isChecked();

    IRSRunner->FSENGpp.rescType = "EN34";
    if(enginePPCheckBox->isChecked())
    {
        IRSRunner->FSENGpp.level = enginePPSpinBox->value();
    }
    else
    {
        IRSRunner->FSENGpp.level = 0.0;
    }
    IRSRunner->FSENGpp.outOfSeason = enginePPSeasonCheckBox->isChecked();

    IRSRunner->DOIENGpp.rescType = "EN34";
    if(enginePPCheckBox->isChecked())
    {
        IRSRunner->DOIENGpp.level = enginePPSpinBox->value();
    }
    else
    {
        IRSRunner->DOIENGpp.level = 0.0;
    }
    IRSRunner->DOIENGpp.outOfSeason = enginePPSeasonCheckBox->isChecked();
}

void OmffrMainWindow::SetDDLevels()
{
    IRSRunner->FSCRWdd.rescType = "CRW20";
    if(regCrewDDCheckBox->isChecked())
    {
        IRSRunner->FSCRWdd.level = regCrewDDSpinBox->value();
    }
    else
    {
        IRSRunner->FSCRWdd.level = 0.0;
    }
    IRSRunner->FSCRWdd.outOfSeason = regCrewDDCheckBox->isChecked();

    IRSRunner->DOICRWdd.rescType = "CRW20";
    if(regCrewDDCheckBox->isChecked())
    {
        IRSRunner->DOICRWdd.level = regCrewDDSpinBox->value();
    }
    else
    {
        IRSRunner->DOICRWdd.level = 0.0;
    }
    IRSRunner->DOICRWdd.outOfSeason = regCrewDDCheckBox->isChecked();

    IRSRunner->FSDZRdd.rescType = "CRW20";
    if(dozerDDCheckBox->isChecked())
    {
        IRSRunner->FSDZRdd.level = dozerDDSpinBox->value();
    }
    else
    {
        IRSRunner->FSDZRdd.level = 0.0;
    }
    IRSRunner->FSDZRdd.outOfSeason = dozerDDCheckBox->isChecked();

    IRSRunner->DOIDZRdd.rescType = "CRW20";
    if(dozerDDCheckBox->isChecked())
    {
        IRSRunner->DOIDZRdd.level = dozerDDSpinBox->value();
    }
    else
    {
        IRSRunner->DOIDZRdd.level = 0.0;
    }
    IRSRunner->DOIDZRdd.outOfSeason = dozerDDCheckBox->isChecked();

    IRSRunner->FSENGdd.rescType = "EN34";
    if(engineDDCheckBox->isChecked())
    {
        IRSRunner->FSENGdd.level = engineDDSpinBox->value();
    }
    else
    {
        IRSRunner->FSENGdd.level = 0.0;
    }
    IRSRunner->FSENGdd.outOfSeason = engineDDCheckBox->isChecked();

    IRSRunner->DOIENGdd.rescType = "EN34";
    if(engineDDCheckBox->isChecked())
    {
        IRSRunner->DOIENGdd.level = engineDDSpinBox->value();
    }
    else
    {
        IRSRunner->DOIENGdd.level = 0.0;
    }
    IRSRunner->DOIENGdd.outOfSeason = engineDDCheckBox->isChecked();

    IRSRunner->FSSEATdd.rescType = "SEAT";
    if(seatDDCheckBox->isChecked())
    {
        IRSRunner->FSSEATdd.level = seatDDSpinBox->value();
    }
    else
    {
        IRSRunner->FSSEATdd.level = 0.0;
    }
    IRSRunner->FSSEATdd.outOfSeason = seatDDCheckBox->isChecked();

    IRSRunner->DOISEATdd.rescType = "SEAT";
    if(seatDDCheckBox->isChecked())
    {
        IRSRunner->DOISEATdd.level = seatDDSpinBox->value();
    }
    else
    {
        IRSRunner->DOISEATdd.level = 0.0;
    }
    IRSRunner->DOISEATdd.outOfSeason = seatDDCheckBox->isChecked();
}

void OmffrMainWindow::ShowCartDiagram()
{
    if(poDA == NULL)
    {
        return;
    }
    poDA->WriteCartFile(currentResultsFile.toStdString().c_str(), "fires.csv");
#ifndef WIN32
    system("R --no-save < /home/kyle/src/omffr/trunk/irs/scripts/fig-tree.r");
#else
    system("\"c:/program files/r/r-3.0.2/bin/r.exe\" --no-save < c:/src/omffr/trunk/irs/scripts/fig-tree.r");
#endif
    QPixmap pixMap("fig-tree.png");
    imageLabel->setPixmap(pixMap);
    displayTabWidget->setCurrentIndex(2);
}

void OmffrMainWindow::LoadResultsFile()
{
    QString inputDbFile =
        QFileDialog::getOpenFileName(this, tr("Open SQLite database"), "",
                                     tr("SQLite DB Files (*.sqlite *.db)"));
    if( inputDbFile.isEmpty() )
        return;
    currentResultsFile = inputDbFile;
	// Open results

    apResults.clear();

        //dataPath = OMFFR_DB_DBG;
#ifdef WIN32
        dataPath = "c:/Users/ksshannon/Documents/GitHub/build/data/omffr.sqlite";
#else
        dataPath = "/home/kyle/src/omffr/trunk/data/omffr.sqlite";
#endif
	results = new IRSResult(currentResultsFile.toStdString().c_str(), dataPath.toStdString().c_str(), 1, FALSE, TRUE, NULL);

        apResults = results->GetResultArray(NULL, IRS_NEXT_RUN_DIR);
        if( apResults.size() < 1 )
            return;

	
	//Enable results plot buttons
	resultScaleLabel->setEnabled(true);
	resultAttributeLabel->setEnabled(true);
	resultScaleComboBox->setEnabled(true);
	resultAttributeComboBox->setEnabled(true);
    displayTabChanged(displayTabWidget->currentIndex());
    seriesSpinBox->setMaximum(apResults.size());
    ResultGACCs();

	/*if (apResults.size() > 1)	{
		//Series buttons
		int ResultsPage = displayTabWidget->currentIndex();
		if (ResultsPage == 0 || ResultsPage == 2)	{ 
			seriesItem->setEnabled(true);
			seriesSpinBox->setEnabled(true);
			seriesSpinBox->setValue(1);
			seriesSpinBox->setMaximum(apResults.size());
			allSeriesButton->setEnabled(false);
		}

		if(ResultsPage == 1)	{
			seriesItem->setEnabled(true);
			seriesSpinBox->setEnabled(true);
			seriesSpinBox->setValue(1);
			allSeriesButton->setEnabled(true);
			allSeriesButton->setChecked(true);
		}
	}*/
	PlotResults(1);
}

void OmffrMainWindow::PlotFireResults()
{
    yearCurve->attach(NULL);
    minCurve->attach(NULL);
    maxCurve->attach(NULL);
    meanCurve->attach(NULL);
    escapeCurve->attach(qwtPlot);
    containCurve->attach(qwtPlot);
    norescCurve->attach(qwtPlot);

    QwtArray<double>xPoints;
    QwtArray<double>yPoints;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    const char *pszDbFile = strdup(currentResultsFile.toStdString().c_str());
    rc = sqlite3_open_v2(pszDbFile, &db, SQLITE_OPEN_READONLY, NULL);
    rc = sqlite3_prepare(db, "SELECT COUNT(DISTINCT(year)) FROM full_results",
                         -1, &stmt, NULL);
    rc = sqlite3_step(stmt);
    int nYears = sqlite3_column_int(stmt, 0);
    rc = sqlite3_finalize(stmt);
    rc = sqlite3_prepare_v2(db, "SELECT jul_day, COUNT(*) FROM full_results "
                                "WHERE status IN('Escaped', 'SizeLimitExceeded', "
                                "'TimeLimitExceeded') "
                                "GROUP BY jul_day",
                            -1, &stmt, NULL);
    for(int i = 0;i < 365;i++)
    {
        xPoints.push_back(i);
        yPoints.push_back(0.0);
    }
    int julDay;
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        julDay = sqlite3_column_int(stmt, 0);
        yPoints[julDay] = sqlite3_column_double(stmt, 1) / nYears;
    }
    escapeCurve->setData(xPoints, yPoints);

    for(int i = 0;i < 365;i++)
    {
        yPoints[i] = 0;
    }

    rc = sqlite3_finalize(stmt);
    rc = sqlite3_prepare_v2(db, "SELECT jul_day, COUNT(*) FROM full_results "
                                "WHERE status = 'Contained'"
                                "GROUP BY jul_day ",
                            -1, &stmt, NULL);
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        julDay = sqlite3_column_int(stmt, 0);
        yPoints[julDay] = sqlite3_column_double(stmt, 1) / nYears;
    }
    containCurve->setData(xPoints, yPoints);

    for(int i = 0;i < 365;i++)
    {
        yPoints[i] = 0;
    }

    rc = sqlite3_finalize(stmt);
    rc = sqlite3_prepare_v2(db, "SELECT jul_day, COUNT(*) FROM full_results "
                                "WHERE status = 'No Resources Sent' "
                                "GROUP BY jul_day",
                            -1, &stmt, NULL);
    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        julDay = sqlite3_column_int(stmt, 0);
        yPoints[julDay] = sqlite3_column_double(stmt, 1) / nYears;
    }
    norescCurve->setData(xPoints, yPoints);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    qwtPlot->replot();
    displayTabWidget->setCurrentIndex(1);
}

const char * OmffrMainWindow::QStringToCString( QString s )
{
    std::string ss = s.toStdString();
    const char *p = strdup(ss.c_str());
    return p;
}

/*
** Create various output maps.

Python code for different renderers:

from qgis.core import  (QgsVectorLayer,
                        QgsMapLayerRegistry,
                        QgsGraduatedSymbolRendererV2,
                        QgsSymbolV2,
                        QgsRendererRangeV2)

myVectorLayer = QgsVectorLayer(myVectorPath, myName, 'ogr')
myTargetField = 'target_field'
myRangeList = []
myOpacity = 1
# Make our first symbol and range...
myMin = 0.0
myMax = 50.0
myLabel = 'Group 1'
myColour = QtGui.QColor('#ffee00')
mySymbol1 = QgsSymbolV2.defaultSymbol(
           myVectorLayer.geometryType())
mySymbol1.setColor(myColour)
mySymbol1.setAlpha(myOpacity)
myRange1 = QgsRendererRangeV2(
                myMin,
                myMax,
                mySymbol1,
                myLabel)
myRangeList.append(myRange1)
#now make another symbol and range...
myMin = 50.1
myMax = 100
myLabel = 'Group 2'
myColour = QtGui.QColor('#00eeff')
mySymbol2 = QgsSymbolV2.defaultSymbol(
           myVectorLayer.geometryType())
mySymbol2.setColor(myColour)
mySymbol2.setAlpha(myOpacity)
myRange2 = QgsRendererRangeV2(
                myMin,
                myMax,
                mySymbol2
                myLabel)
myRangeList.append(myRange2)
myRenderer = QgsGraduatedSymbolRendererV2(
                '', myRangeList)
myRenderer.setMode(
        QgsGraduatedSymbolRendererV2.EqualInterval)
myRenderer.setClassAttribute(myTargetField)

myVectorLayer.setRendererV2(myRenderer)
QgsMapLayerRegistry.instance().addMapLayer(myVectorLayer)

*/

void OmffrMainWindow::SelectGeometries(QgsGeometry *geometry)
{
    ClearSelection();
    if(!geometry)
        return;
    QgsVectorLayer *layer;
    layer = dynamic_cast<QgsVectorLayer*>(mapCanvas->currentLayer());
    if(!layer)
        return;
    if(layer->geometryType() != QGis::Polygon )
    {
        return;
    }
    QgsRectangle rect = geometry->boundingBox();
    ((QgsVectorLayer*)layer)->select(rect, false);
}

void OmffrMainWindow::ClearSelection()
{
    QList<QgsMapLayer*> layers = mapCanvas->layers();
    for(int i = 0; i < layers.size();i++)
    {
        QgsVectorLayer *layer = dynamic_cast<QgsVectorLayer*>(layers[i]);
        if(!layer)
            continue;
        layer->removeSelection();
    }
    mapSimpleAreaTool->deactivate();
}

#define POINT_BUF_EPSILON  0.0000001
void OmffrMainWindow::SelectGeometryFromPoint(const QgsPoint &point,
                                              Qt::MouseButton button)
{
    qDebug() << "Select Point: " << point.x() << ", " << point.y();
    QgsRectangle rect(point.x() - POINT_BUF_EPSILON,
                      point.y() - POINT_BUF_EPSILON,
                      point.x() + POINT_BUF_EPSILON,
                      point.y() + POINT_BUF_EPSILON);

    SelectGeometries(QgsGeometry::fromRect(rect));
}

void OmffrMainWindow::LoadResultAttributes()
{
    char **papszAtts;
    //papszAtts = poSuite->GetResultAttributes();
	papszAtts = results->GetResultAttributes();
    if( !papszAtts )
    {
        /* message box ? */
        return;
    }
    int i = 0;
    QString att;
    while( papszAtts[i] != NULL )
    {
        att = papszAtts[i];
        resultAttributeComboBox->addItem(att);
    }
    poSuite->FreeResultAttributes( papszAtts );
}

void OmffrMainWindow::PlotSingleRunResults()
{
	qwtPlot->detachItems(QwtPlotItem::Rtti_PlotUserItem, true);
	qwtPlot->detachItems(QwtPlotItem::Rtti_PlotCurve, true);

	//Get values to plot
	QString Scale = resultScaleComboBox->currentText();
	QString Attribute = resultAttributeComboBox->currentText();
	std::string strAttribute = Attribute.toStdString();

    if (apResults[0]->GetNumYears() > 1)    {
	    QVector<QList<double> > DataValues;
	    QVector<std::string> xLabel;
	    	xLabel.push_back("");

	    int i = seriesSpinBox->value();
	    IRSResult *results = apResults[i-1];

	    if (Scale == "Selected Area")	{
	    	std::list<double> valueList = results->NationalDistribution( strAttribute );

	    	DataValues.push_back(QList<double>::fromStdList(valueList));
	    	xLabel.push_back("Entire Area");

	    	qwtPlot->setTitle("Selected Area");
	    }

	    if (Scale == "By Region")	{
		    std::vector<std::pair<std::string, std::list<double> > > valueLists = results->GACCDistributions( strAttribute );

		    QList<double> RegionList;
		    for (int i = 0; i < valueLists.size(); i++ )	{
			    RegionList.clear();
			    RegionList = QList<double>::fromStdList(valueLists[i].second);
			    if (RegionList.size() > 0 )	{
			    	DataValues.push_back(RegionList);
			    	xLabel.push_back(valueLists[i].first);
			    }
		    }

		    qwtPlot->setTitle("By Region");
	    }
	
        if (Scale == "By FPU")	{
		    QString FPU = resultGACCBox->currentText();

            std::vector<std::pair<std::string, std::list<double> > > valueLists = results->FPUDistributions( strAttribute );
        
		    QList<double> FPUList;
            int num = 0;
		    for (int i = 0; i < valueLists.size(); i++ )	{
            //Use first 15 FPUs if GACC not Selected
                if (num < 15 || FPU != "Entire Area")       {
                   string valueFPU = valueLists[i].first;
                      valueFPU.resize(2);
                  if (valueFPU == FPU.toStdString() || FPU == "Entire Area")  {
                     FPUList.clear();
			         FPUList = QList<double>::fromStdList(valueLists[i].second);
			         if (FPUList.size() > 0 )	{
				        DataValues.push_back(FPUList);
                        std::string label = valueLists[i].first;
                        if (i/2 * 2 != i)
                            label.insert(0, "\n");
				        xLabel.push_back(label);
                     }
                   }

                   num++;
                 }
		    }

            qwtPlot->setTitle("By FPU");

	    }

   	    boxPlotItem *BoxPlot = new boxPlotItem("");

	    BoxPlot->setColor(Qt::lightGray);
	    BoxPlot->setData(DataValues);
	    BoxPlot->attach(qwtPlot);

	    QwtPlotMarker *Label = new QwtPlotMarker();
	    Label->setLabel(QwtText(""));
	    Label->attach(qwtPlot);

	    QwtDoubleRect bounds = BoxPlot->boundingRect();

	    qwtPlot->setAxisScale(QwtPlot::yLeft, bounds.top(), bounds.bottom());
	    qwtPlot->setAxisScale(QwtPlot::xBottom, 0.0, bounds.right());
	    qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, new textScaleDraw(xLabel));
	    qwtPlot->setAxisMaxMinor(QwtPlot::xBottom, bounds.right());
	    qwtPlot->setAxisMaxMajor(QwtPlot::xBottom, bounds.right());
	    qwtPlot->setAxisTitle(QwtPlot::yLeft, Attribute);
	    qwtPlot->replot();
    }

	MapResults();
    PlotPieChart();
}

void OmffrMainWindow::MapResults()	
{
	QString Scale = resultScaleComboBox->currentText();
    QString Attribute = resultAttributeComboBox->currentText();
	std::string strAttribute = Attribute.toStdString();
	
	double theMinimum = 1000000000;
	double theMaximum = 0;

	for (int i = 0; i < apResults.size(); i++ )	{
        pair<double, double> bounds;
        if (Scale == "By FPU")
            bounds =  apResults[i]->FPUMinMaxValues(strAttribute);
        else
		    bounds =  apResults[i]->MinMaxValues(strAttribute);
		
		if (bounds.first < theMinimum)
			theMinimum = bounds.first;
		if (bounds.second > theMaximum)
			theMaximum = bounds.second;
	}

	double step = (theMaximum - theMinimum) / 5;

	std::vector<double> CMinimums;
	std::vector<double> CMaximums;
	std::vector<int> Red;
	std::vector<int> Green;
	std::vector<int> Blue;

	//Very Low
	if (Attribute == "Percent Contained")	{
		CMinimums.push_back(theMaximum - step);
		CMaximums.push_back(theMaximum);
	}
	else	{
		CMinimums.push_back(theMinimum);
		CMaximums.push_back(theMinimum + step);
	}
	Red.push_back(255);
	Green.push_back(224);
	Blue.push_back(224);

	//Low
	if (Attribute == "Percent Contained")	{
		CMinimums.push_back(theMaximum - 2 * step);
		CMaximums.push_back(theMaximum - step);
	}
	else	{
		CMinimums.push_back(theMinimum + step);
		CMaximums.push_back(theMinimum + 2 * step);
	}
	Red.push_back(255);
	Green.push_back(138);
	Blue.push_back(138);

	//Medium
	if (Attribute == "Percent Contained")	{
		CMinimums.push_back(theMaximum - 3 * step);
		CMaximums.push_back(theMaximum - 2 *step);
	}
	else	{
		CMinimums.push_back(theMinimum + 2 * step);
		CMaximums.push_back(theMinimum + 3 * step);
	}
	Red.push_back(255);
	Green.push_back(8);
	Blue.push_back(8);

	//High
	if (Attribute == "Percent Contained")	{
		CMinimums.push_back(theMaximum - 4 * step);
		CMaximums.push_back(theMaximum - 3 *step);
	}
	else	{
		CMinimums.push_back(theMinimum + 3 * step);
		CMaximums.push_back(theMinimum + 4 * step);
	}
	Red.push_back(201);
	Green.push_back(6);
	Blue.push_back(6);

	//Very High
	if (Attribute == "Percent Contained")	{
		CMinimums.push_back(theMinimum);
		CMaximums.push_back(theMaximum - 4 *step);
	}
	else	{
		CMinimums.push_back(theMinimum + 4 * step);
		CMaximums.push_back(theMaximum);
	}
	Red.push_back(148);
	Green.push_back(4);
	Blue.push_back(4);

	char path[1000];
	int i = seriesSpinBox->value();
    if (Scale == "By FPU")
        int n = sprintf(path, "%s|layername=fpu_sum", apResults[i-1]->GetResultPath());
    else
	    int n = sprintf(path, "%s|layername=gacc_sum", apResults[i-1]->GetResultPath());
    QString myLayerPath(path);
    QString myLayerBaseName     = "test";
    QString myProviderName      = "ogr";

	QgsVectorLayer *gaccSingleMapLayer = new QgsVectorLayer(myLayerPath, "", myProviderName);
	QgsRendererRangeV2 *range;

	QString label("Very Low");
	QColor color(Red[0], Green[0], Blue[0]);
	QgsSymbolV2 *symbol = QgsSymbolV2::defaultSymbol(gaccSingleMapLayer->geometryType());
	qDebug() << "Symbol: " << symbol;
	if( symbol == NULL )
	{
		qDebug() << "Invalid symbol for GACC, geometryType() == " << gaccSingleMapLayer->geometryType();
		return;
	}
	symbol->setColor(color);
	range = new QgsRendererRangeV2(CMinimums[0], CMaximums[0], symbol, label);
	QgsRangeList rangeList;
	rangeList.append(QgsRendererRangeV2(CMinimums[0], CMaximums[0], symbol, label));

	label = "Low";
	color = QColor(Red[1], Green[1], Blue[1]);
	symbol = QgsSymbolV2::defaultSymbol(gaccSingleMapLayer->geometryType());
	symbol->setColor(color);
	range = new QgsRendererRangeV2(CMinimums[1], CMaximums[1], symbol, label);
	rangeList.append(QgsRendererRangeV2(CMinimums[1], CMaximums[1], symbol, label));

	label = "Medium";
	color = QColor(Red[2], Green[2], Blue[2]);
	symbol = QgsSymbolV2::defaultSymbol(gaccSingleMapLayer->geometryType());
	symbol->setColor(color);
	range = new QgsRendererRangeV2(CMinimums[2], CMaximums[2], symbol, label);
	rangeList.append(QgsRendererRangeV2(CMinimums[2], CMaximums[2], symbol, label));

	label = "High";
	color = QColor(Red[3], Green[3], Blue[3]);
	symbol = QgsSymbolV2::defaultSymbol(gaccSingleMapLayer->geometryType());
	symbol->setColor(color);
	range = new QgsRendererRangeV2(CMinimums[3], CMaximums[3], symbol, label);
	rangeList.append(QgsRendererRangeV2(CMinimums[3], CMaximums[3], symbol, label));

	label = "Very High";
	color = QColor(Red[4], Green[4], Blue[4]);
	symbol = QgsSymbolV2::defaultSymbol(gaccSingleMapLayer->geometryType());
	symbol->setColor(color);
	range = new QgsRendererRangeV2(CMinimums[4], CMaximums[4], symbol, label);
	rangeList.append(QgsRendererRangeV2(CMinimums[4], CMaximums[4], symbol, label));

	renderer = new QgsGraduatedSymbolRendererV2("", rangeList);
	renderer->setMode(QgsGraduatedSymbolRendererV2::EqualInterval);
	char ColumnChar[100];
	if (Attribute == "Percent Contained")
		int n = sprintf(ColumnChar, "%s", PERC_CONT_COLUMN);
	else if (Attribute == "Large Fire Cost" )
		int n = sprintf(ColumnChar, "%s", LF_COST_COLUMN);
	else if (Attribute == "Large Fire Acres" )
		int n = sprintf(ColumnChar, "%s", LF_ACRE_COLUMN);
	else if (Attribute == "Large Fire Population" )
		int n = sprintf(ColumnChar, "%s", LF_POP_COLUMN);
    else if (Attribute == "Resource Usage")
        int n = sprintf(ColumnChar, "%s", RESC_USAGE_COLUMN);

	QString column(ColumnChar);
	renderer->setClassAttribute(column);

	gaccSingleMapLayer->setRendererV2(renderer);

	if (gaccSingleMapLayer->isValid())
	{
		qDebug("Layer is valid");
	}
	else
	{
		qDebug("Layer is NOT valid");
		return;
	}

	// Add the Vector Layer to the Layer Registry
	QgsMapLayerRegistry::instance()->addMapLayer(gaccSingleMapLayer, TRUE);

	// Add the Layer to the Layer Set
    mapLayerSet.clear();
	mapLayerSet.append(QgsMapCanvasLayer(gaccSingleMapLayer, false));

	for (int l = 0; l < mapLayerSet.size(); l++)
	    mapLayerSet[l].setVisible(true);

	// Set the Map Canvas Layer Set
	mapCanvas->setLayerSet(mapLayerSet);
	//mpMapCanvas->setDirty(true);
	mapCanvas->refresh();
}

void OmffrMainWindow::PlotSeriesResults()
{
	qwtPlot->detachItems(QwtPlotItem::Rtti_PlotUserItem, true);
    qwtPlot->detachItems(QwtPlotItem::Rtti_PlotCurve, true);

    std::vector<QwtPlotCurve *> Curves;
    int numYears = 18;	//Maximum number of years to plot
    for (int i = 0; i < numYears; i++)
	    Curves.push_back(new QwtPlotCurve());

    QStringList colorNames = QColor::colorNames();
    int colorPos = 10;

    //Get values to plot
    QString Scale = resultScaleComboBox->currentText();
    QString Attribute = resultAttributeComboBox->currentText();
    std::string strAttribute = Attribute.toStdString();

    if (Scale == "Selected Area")	{

	    std::vector<std::vector<double> > runResults;	// a vector of the different years results for one set of results
	    std::vector<double> theResults;

	    for (int i = 0; i < apResults.size(); i++ )	{
		    std::list<double> valueList = apResults[i]->NationalDistribution( strAttribute );
		    std::list<double>::iterator it = valueList.begin();

		    if (valueList.size() < numYears )
			    numYears = valueList.size();

		    theResults.clear();
		    for (int j = 0; j < numYears; j++ )		{
			    theResults.push_back(*it);
			    it++;
		    }
		    runResults.push_back(theResults);
	    }

	    QwtArray<double> xPoints;
	    QwtArray<double> yPoints;

        int NumCurves = 0;

	    for (int i = 0; i < runResults[0].size(); i++)	{
		    xPoints.clear();
		    yPoints.clear();
		    for(int j = 0; j < runResults.size(); j++ )	{
			    xPoints.push_back(j + 1);
			    yPoints.push_back(runResults[j].at(i));
		    }

		    Curves[i]->setData(xPoints, yPoints);
		    QColor col(colorNames.at(colorPos%colorNames.count()));
		    colorPos++;

		    QPen pen(col, 3.0);
		    pen.setJoinStyle(Qt::RoundJoin);

		    Curves[i]->setPen(pen);
		    Curves[i]->attach(qwtPlot);

		    char title[20];
		    int n = sprintf(title, "Year %d", i);
		    Curves[i]->setTitle(title);
		    Curves[i]->setItemAttribute(QwtPlotItem::Legend, true);
	    }

	    qwtPlot->setTitle("Selected Area");
    }

    if (Scale == "By Region")	{

	    std::vector<std::vector<std::pair<std::string, double> > > runResults;

	    for (int s = 0; s < apResults.size(); s++ )	{
		    std::vector<std::pair<std::string, double> > valuePairs = apResults[s]->GACCSeriesAvg( strAttribute );
		    runResults.push_back(valuePairs);
	    }

	    std::vector<double> xPoints;
	    std::vector<double> yPoints;

	    for (int i = 0; i < runResults[0].size(); i++)	{
		    xPoints.clear();
		    yPoints.clear();
		    for(int j = 0; j < runResults.size(); j++ )	{
		    	xPoints.push_back(j+1);
			    yPoints.push_back(runResults[j].at(i).second);
		    }

		    QwtArray<double> Qx = QVector<double>::fromStdVector(xPoints);
		    QwtArray<double> Qy = QVector<double>::fromStdVector(yPoints);

		    Curves[i]->setData(Qx, Qy);
		    QColor col(colorNames.at(colorPos%colorNames.count()));
		    colorPos++;

		    QPen pen(col, 2.0);
		    pen.setJoinStyle(Qt::RoundJoin);

		    Curves[i]->setPen(pen);
    	    Curves[i]->attach(qwtPlot);

		    QString title = runResults[0].at(i).first.c_str();
		    Curves[i]->setTitle(title);
		    Curves[i]->setItemAttribute(QwtPlotItem::Legend, true);
		
        }

	    qwtPlot->setTitle("By Region");
    }

    if (Scale == "By FPU")	{

        QString GACC = resultGACCBox->currentText();
        string strGACC = GACC.toStdString();
	    std::vector<std::vector<std::pair<std::string, double> > > runResults;

		for (int s = 0; s < apResults.size(); s++ )	{
	        std::vector<std::pair<std::string, double> > valuePairs = apResults[s]->FPUSeriesAvg( strAttribute );
            runResults.push_back(valuePairs);
        }

	    std::vector<double> xPoints;
	    std::vector<double> yPoints;
        int numCurves = 0;

	    for (int i = 0; i < runResults[0].size(); i++)	{
		    xPoints.clear();
		    yPoints.clear();

    	    for(int j = 0; j < runResults.size(); j++ )	{
                if (numCurves < numYears-1) {
                     string fpu_id = runResults[j].at(i).first;
                     fpu_id.resize(2);
                     if (strGACC == fpu_id || strGACC == "Entire Area")    {
			            xPoints.push_back(j+1);
			            yPoints.push_back(runResults[j].at(i).second);
                     }
                 }
		    }

		    if (xPoints.size() > 0)    {
                QwtArray<double> Qx = QVector<double>::fromStdVector(xPoints);
		        QwtArray<double> Qy = QVector<double>::fromStdVector(yPoints);

                Curves[numCurves]->setData(Qx, Qy);
		        QColor col(colorNames.at(colorPos%colorNames.count()));
		        colorPos++;

		        QPen pen(col, 2.0);
		        pen.setJoinStyle(Qt::RoundJoin);

			    Curves[numCurves]->setPen(pen);
			    Curves[numCurves]->attach(qwtPlot);

			    QString title = runResults[0].at(i).first.c_str();
			    Curves[numCurves]->setTitle(title);
			    Curves[numCurves]->setItemAttribute(QwtPlotItem::Legend, true);

                 numCurves++;
            }
		
		}

		qwtPlot->setTitle("By FPU");
	}

	for (int i = 0; i < Curves.size(); i++ )
		Curves[i]->attach(qwtPlot);

	QwtDoubleRect bounds = Curves[0]->boundingRect();

	qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
	qwtPlot->setAxisAutoScale(QwtPlot::xBottom);
	qwtPlot->setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
	qwtPlot->setAxisMaxMinor(QwtPlot::xBottom, bounds.right());
	qwtPlot->setAxisMaxMajor(QwtPlot::xBottom, bounds.right());
	qwtPlot->setAxisTitle(QwtPlot::yLeft, Attribute);
	qwtPlot->replot();

	MapResults();
    PlotPieChart();
}

void OmffrMainWindow::changeSeriesItem(int i)
{
	if (allSeriesButton->isChecked())
		PlotSeriesResults();
	else 
		PlotSingleRunResults();
}

void OmffrMainWindow::PlotResults(int m)
{
    int i = seriesSpinBox->value();
    changeSeriesItem(i);
}

void OmffrMainWindow::ScaleChanged(int m)
{
	int i = seriesSpinBox->value();
    QString scale = resultScaleComboBox->currentText();
    if (scale == "By FPU")  {
        GACCLabel->setEnabled(true);
        resultGACCBox->setEnabled(true);
        if (apResults[0]->GetNumFPUs() > 15)
            GACCwarning->setVisible(true);
        changeSeriesItem(i);
    }
    else    {
        GACCLabel->setEnabled(false);
        resultGACCBox->setEnabled(false);
        GACCwarning->setHidden(true);
	    changeSeriesItem(i);
    }
}

void OmffrMainWindow::SeriesPlotResults()
{
	int i = seriesSpinBox->value();
	changeSeriesItem(i);
}

void OmffrMainWindow::displayTabChanged(int i)
{
	if (apResults.size() > 1)	{
		//Series buttons
		int ResultsPage = displayTabWidget->currentIndex();
		if (ResultsPage == 0 || ResultsPage == 2)	{ 
			seriesItem->setEnabled(true);
			seriesSpinBox->setEnabled(true);
			allSeriesButton->setEnabled(false);
		}

		if (ResultsPage == 1)	{
			seriesItem->setEnabled(true);
			seriesSpinBox->setEnabled(true);
			allSeriesButton->setEnabled(true);
		}
	}
}

void OmffrMainWindow::PlotPieChart()
{
    pieFrame->clearEntries();
	QString Scale = resultScaleComboBox->currentText();
    QString Attribute = resultAttributeComboBox->currentText();
	std::string strAttribute = Attribute.toStdString();
    int s = seriesSpinBox->value();
    
    if (Scale == "By FPU")	{
        std::vector<std::pair<std::string, double> > valuePairs = apResults[s-1]->FPUSeriesAvg( strAttribute );
        QString FPU = resultGACCBox->currentText();

        int num = 0;
	    for (int i = 0; i < valuePairs.size(); i++ )	{
            //Use first 15 FPUs if GACC not Selected
            if (num < 15 || FPU != "Entire Area")    {
                string valueFPU = valuePairs[i].first;
                valueFPU.resize(2);
                if (valueFPU == FPU.toStdString() || FPU == "Entire Area")  
	                pieFrame->addEntry(QString::fromStdString(valuePairs[i].first), valuePairs[i].second);
                num++;
            }
        }
    }
	
    else    {
        std::vector<std::pair<std::string, double> > valuePairs = apResults[s-1]->GACCSeriesAvg( strAttribute );
        for (int i = 0; i < valuePairs.size(); i++) 
             pieFrame->addEntry(QString::fromStdString(valuePairs[i].first), valuePairs[i].second);
    }
    pieFrame->update();
}

void OmffrMainWindow::ExportCsv()
{
    if(apResults.size() < 1)
        return;
    QString outputFile =
        QFileDialog::getSaveFileName(this, tr("Save CSV file"), "",
                                     tr("CSV files(*.csv)"));
    if(outputFile.isEmpty())
    {
        return;
    }
    QFile file(outputFile);
    if(file.exists())
    {
        QFile::remove(outputFile);
    }
    qDebug() << outputFile;
    int s = seriesSpinBox->value();
    apResults[s-1]->ExportFpuSummaryCsv(outputFile.toLocal8Bit());
    return;
}
void OmffrMainWindow::ExportOgr()
{
    if(apResults.size() < 1)
        return;
    QString outputFile =
        QFileDialog::getSaveFileName(this, tr("Save ESRI Shapefile"), "",
                                     tr("ESRI Shapefile (*.shp)"));
    if(outputFile.isEmpty())
    {
        return;
    }
    QFile file(outputFile);
    if(file.exists())
    {
        QFile::remove(outputFile);
    }
    qDebug() << outputFile;
    int s = seriesSpinBox->value();
    apResults[s-1]->ExportOgrSummary("ESRI Shapefile",
                                     outputFile.toLocal8Bit(),
                                     NULL);
    return;
}

void OmffrMainWindow::ResultGACCs()
{
    QStringList boxGACCs;
    boxGACCs.push_back("Entire Area");

    std::vector<std::string> GACCs = apResults[0]->ResultGACCs();
    for ( int i = 0; i < GACCs.size(); i++) 
        boxGACCs.push_back(GACCs[i].c_str());

    resultGACCBox->clear();
    resultGACCBox->addItems(boxGACCs);
}

void OmffrMainWindow::ToggleAutoReduce()
{
    bool bOn = autoReduceGroupBox->isChecked();
    if(!bOn)
    {
        if(rescReduceGroupBox->isChecked())
            rescReduceSpinBox->setEnabled(true);
        if(fuelTreatGroupBox->isChecked())
            fuelTreatSpinBox->setEnabled(true);
    }
    else
    {
        if(autoReduceComboBox->currentIndex() == 0)
        {
            if(!rescReduceGroupBox->isChecked())
                rescReduceGroupBox->setChecked(true);
            rescReduceSpinBox->setDisabled(true);
            if(fuelTreatGroupBox->isChecked())
                fuelTreatSpinBox->setEnabled(true);
        }
        else
        {
            fuelTreatGroupBox->setChecked(true);
            fuelTreatSpinBox->setDisabled(true);
            if(rescReduceGroupBox->isChecked())
                rescReduceSpinBox->setEnabled(true);
        }
    }
}

int OmffrMainWindow::FillRunData(IRSMultiRunData *psData)
{
    if(!psData)
        return IRS_INVALID_INPUT;
    int i;
    int nValue;
    double dfValue;
    nValue = 0;
    if(rescReduceGroupBox->isChecked())
    {
        nValue = 0;
        QList<QListWidgetItem*>items = redTypeListBox->selectedItems();
        for(i = 0; i < items.size(); i++)
        {
            if(items[i]->text() == "Crews")
                nValue |= CRW;
            if(items[i]->text() == "Engines")
                nValue |= ENG;
            if(items[i]->text() == "Helicopters")
                nValue |= HEL;
            if(items[i]->text() == "Helitack")
                nValue |= HELI;
        }
        if(nValue == 0)
            nValue = RESC_ALL;
        psData->nRescTypeReduction = nValue;

        /* Agency */
        nValue = 0;
        if(usfsRedCheckBox->isChecked())
            nValue |= USFS;
        if(doiRedCheckBox->isChecked())
            nValue |= DOI_ALL;
        if(nValue == 0)
            nValue |= AGENCY_ALL;
        psData->nAgencyReduction = nValue;
        /* No Region selection */
        psData->nRegionReduction = REGION_ALL;
        if( rescRedRandRadioButton->isChecked() )
            psData->nReductionMethod = RESC_REDUCE_RAND;
        else
            psData->nReductionMethod = RESC_REDUCE_PERFORM;
        psData->nRegionReduction = 0;
        QListWidgetItem *item;
        for( i = 0; i < redRegionListBox->count(); i++ )
        {
            item = redRegionListBox->item( i );
            if( item->isSelected() )
            {
                psData->nRegionReduction |= 1 << i;
            }
        }
        if( redRegionListBox->item( redRegionListBox->count() - 1 )->isSelected() )
            psData->nRegionReduction = REGION_ALL;
    }
    else
    {
        psData->nRescTypeReduction = RESC_REDUCE_RAND;
        psData->nAgencyReduction = 0;
        psData->nRegionReduction = REGION_ALL;
        psData->dfRescRedStart = 0;
        psData->dfRescRedStop = 0;
        psData->nRescRedSteps = 0;
    }

    /* MultiRun */
    if(autoReduceGroupBox->isChecked())
    {
        if(autoReduceComboBox->currentIndex() == 0)
        {
            psData->dfRescRedStart = autoStartSpinBox->value() / 100.;
            psData->dfRescRedStop = autoStopSpinBox->value() / 100.;
            psData->nRescRedSteps = autoStepSpinBox->value();
            psData->dfTreatPercStart = fuelTreatSpinBox->value() / 100.;
            psData->dfTreatPercStop = fuelTreatSpinBox->value() / 100.;
            psData->nTreatPercSteps = 0;
        }
        else
        {
            psData->dfRescRedStart = rescReduceSpinBox->value() / 100.;
            psData->dfRescRedStop = rescReduceSpinBox->value() / 100.;
            psData->nRescRedSteps = 0;
            psData->dfTreatPercStart = autoStartSpinBox->value() / 100.;
            psData->dfTreatPercStop = autoStopSpinBox->value() / 100.;
            psData->nTreatPercSteps = autoStepSpinBox->value();;
        }
    }
    else
    {
        psData->dfRescRedStart = rescReduceSpinBox->value() / 100.;
        psData->dfRescRedStop = rescReduceSpinBox->value() / 100.;
        psData->nRescRedSteps = 0;
        psData->dfTreatPercStart = fuelTreatSpinBox->value() / 100.;
        psData->dfTreatPercStop = fuelTreatSpinBox->value() / 100.;
        psData->nTreatPercSteps = 0;
    }

    psData->dfLargeFirePerc = largeFirePercentSpinBox->value() / 100.;
    psData->nLargeFireMask = 0;
    if( noRescSentCheckBox->isChecked() )
        psData->nLargeFireMask &= NO_RESC_SENT;
    if( timeLimitCheckBox->isChecked() )
        psData->nLargeFireMask &= TIME_LIMIT_EXCEED;
    if( sizeLimitCheckBox->isChecked() )
        psData->nLargeFireMask &= SIZE_LIMIT_EXCEED;
    if( exhaustCheckBox->isChecked() )
        psData->nLargeFireMask &= EXHAUSTED;
    psData->nLargeFireMinSize = largeFireMinSizeSpinBox->value();
    return 0;
}

int  OmffrMainWindow::SetPreposAndDrawDown()
{
    assert(poSuite);
    if( tankerPPCheckBox->isChecked() )
    {
        poSuite->SetPreposition( "ATT", tankerPPSpinBox->value() / 100.,
                                 tankerPPSeasonCheckBox->isChecked() );
    }
    if( helPPCheckBox->isChecked() )
    {
        poSuite->SetPreposition( "Regional Helicopter", helPPSpinBox->value() / 100.,
                                 helPPSeasonCheckBox->isChecked() );
    }
    if( helitackPPCheckBox->isChecked() )
    {
        poSuite->SetPreposition( "FS HELI", helitackPPSpinBox->value() / 100.,
                                 helitackPPSeasonCheckBox->isChecked() );
        poSuite->SetPreposition( "DOI HELI", helitackPPSpinBox->value() / 100.,
                                 helitackPPSeasonCheckBox->isChecked() );
    }
    if( crewPPCheckBox->isChecked() )
    {
        poSuite->SetPreposition( "Regional Crew", crewPPSpinBox->value() / 100.,
                                 crewPPSeasonCheckBox->isChecked() );
        poSuite->SetPreposition( "FS Crew", crewPPSpinBox->value() / 100.,
                                 crewPPSeasonCheckBox->isChecked() );
        poSuite->SetPreposition( "DOI Crew", crewPPSpinBox->value() / 100.,
                                 crewPPSeasonCheckBox->isChecked() );
    }
    if( enginePPCheckBox->isChecked() )
    {
        poSuite->SetPreposition( "FS Engine", enginePPSpinBox->value() / 100.,
                                 enginePPSeasonCheckBox->isChecked() );
        poSuite->SetPreposition( "DOI Engine", enginePPSpinBox->value() / 100.,
                                 enginePPSeasonCheckBox->isChecked() );
    }
    /*
    ** Drawdown
    **static const char * papszDrawDownKeys [] = { "FS Crew", "DOI Crew",
                                             "FS Dozer", "DOI Dozer",
                                             "FS Engine", "DOI Engine",
                                             "FS SEAT", "DOI SEAT",
                                             NULL };


    */
    if( tankerDDCheckBox->isChecked() )
    {
        poSuite->SetDrawdown( "ATT", tankerDDSpinBox->value() / 100.,
                              tankerDDSeasonCheckBox->isChecked() );
    }
    if( seatDDCheckBox->isChecked() )
    {
        poSuite->SetDrawdown( "FS SEAT", seatDDSpinBox->value() / 100.,
                              seatDDSeasonCheckBox->isChecked() );
        poSuite->SetDrawdown( "DOI SEAT", seatDDSpinBox->value() / 100.,
                              seatDDSeasonCheckBox->isChecked() );
    }
    /*
    if( helDDCheckBox->isChecked() )
    {
        poSuite->SetDrawdown( "FS", helDDSpinBox->value() / 100.,
                              helDDSeasonCheckBox->isChecked() );
    }
    */
    if( engineDDCheckBox->isChecked() )
    {
        poSuite->SetDrawdown( "FS Engine", engineDDSpinBox->value() / 100.,
                              engineDDSeasonCheckBox->isChecked() );
        poSuite->SetDrawdown( "DOI Engine", engineDDSpinBox->value() / 100.,
                              engineDDSeasonCheckBox->isChecked() );
    }
    if( regCrewDDCheckBox->isChecked() )
    {
        poSuite->SetDrawdown( "FS Crew", regCrewDDSpinBox->value() / 100.,
                              regCrewDDSeasonCheckBox->isChecked() );
        poSuite->SetDrawdown( "DOI Crew", regCrewDDSpinBox->value() / 100.,
                              regCrewDDSeasonCheckBox->isChecked() );
    }
    if( dozerDDCheckBox->isChecked() )
    {
        poSuite->SetDrawdown( "FS Dozer", dozerDDSpinBox->value() / 100.,
                              dozerDDSeasonCheckBox->isChecked() );
        poSuite->SetDrawdown( "DOI Dozer", dozerDDSpinBox->value() / 100.,
                              dozerDDSeasonCheckBox->isChecked() );
    }
    return 0;
}

