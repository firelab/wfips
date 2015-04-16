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

#include "wfipsdispatcheditdialog.h"

WfipsDispatchEditDialog::WfipsDispatchEditDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::WfipsDispatchEditDialog )
{
    ui->setupUi( this );
    treeWidget = new QTreeWidget( this );
    ui->verticalLayout->insertWidget( 0, treeWidget );
    model = NULL;
    treeWidget->setSelectionMode( QAbstractItemView::ExtendedSelection );
    treeWidget->setAlternatingRowColors( true );
    treeWidget->setColumnCount( 3 );
    QStringList labels;
    labels << "Dispatch Location" << "Resource" << "Type";
    treeWidget->setHeaderLabels( labels );

    connect( treeWidget, SIGNAL( pressed( const QModelIndex & ) ),
             this, SLOT( SelectionClicked( const QModelIndex & ) ) );
    connect( ui->omitToolButton, SIGNAL( clicked() ),
             this, SLOT( Omit() ) );
    connect( ui->revertToolButton, SIGNAL( clicked() ),
             this, SLOT( Unhide() ) );
    connect( treeWidget, SIGNAL( RightClick( QString ) ),
             this, SLOT( ShowResources( QString ) ) );
    connect( ui->removeEmptyButton, SIGNAL( clicked() ),
             this, SLOT( ClearEmptyLocations() ) );
    connect( ui->saveButton, SIGNAL( clicked() ),
             this, SLOT( SaveAs() ) );
}

WfipsDispatchEditDialog::~WfipsDispatchEditDialog()
{
    delete ui;
    delete treeWidget;
}

void WfipsDispatchEditDialog::SetDataPath( QString path )
{
    wfipsDataPath = path;
    rescTypes = WfipsGetRescTypes( path );
}

void WfipsDispatchEditDialog::SetModel( const QMap<qint64, QString> &map,
                                        const int agencyFilter )
{
    this->map = map;
    PopulateRescMap( agencyFilter );
}

void WfipsDispatchEditDialog::Clear()
{
    map.clear();
    rescAtLocMap.clear();
    treeWidget->clear();
}

void WfipsDispatchEditDialog::SelectFids( QgsFeatureIds fids )
{
    if( rescAtLocMap.size() == 0 )
        return;
    QString loc;
    QSet<qint64>::iterator it = fids.begin();
    QList<QTreeWidgetItem*> items;
    treeWidget->clearSelection();
    int i;
    while( it != fids.end() )
    {
        loc = map.value( *it );
        qDebug() << "Selecting fid: " << *it << ", name; " << loc;
        qDebug() << map.values().indexOf( loc );
        i = map.values().indexOf( loc );
        if( i < 0 )
        {
            it++;
            continue;
        }
        items = treeWidget->findItems( loc, Qt::MatchExactly );
        treeWidget->setCurrentItem( items[0] );
        //treeWidget->scrollTo( model->index( i ) );
        it++;
    }
    return;
}

/*
** Hide the selected dispatch locations from the list.
*/
void WfipsDispatchEditDialog::Omit()
{
    QList<QTreeWidgetItem*> items;
    items = treeWidget->selectedItems();
    for( int i = 0; i < items.size(); i++ )
    {
        items[i]->setHidden( true );
    }
    QgsFeatureIds fids = GetVisibleFids();
    treeWidget->clearSelection();
    UpdateCost();
    emit HiddenChanged( fids );
}

void WfipsDispatchEditDialog::Unhide()
{
    QTreeWidgetItemIterator it(treeWidget, QTreeWidgetItemIterator::Hidden );
    while( *it )
    {
        (*it)->setHidden( false );
        it++;
    }
    treeWidget->clearSelection();
    UpdateCost();
    emit HiddenChanged( QgsFeatureIds() );
}

/*
** Find fids for names from the map.
*/
QgsFeatureIds WfipsDispatchEditDialog::GetFidsFromNames( QStringList names )
{
    /* Find the fids.  We should probably fix this */
    QgsFeatureIds fids;
    QgsFeatureId fid;
    for( int i = 0; i < names.size(); i++ )
    {
        fid = map.key( names[i], -1 );
        if( fid < 0 )
        {
            continue;
        }
        fids.insert( fid );
    }
    return fids;
}

QgsFeatureIds WfipsDispatchEditDialog::GetVisibleFids()
{
    QTreeWidgetItemIterator it(treeWidget, QTreeWidgetItemIterator::NotHidden );
    QStringList names;
    while( *it )
    {
        if( (*it)->data( 0, 0 ) != "" )
        {
            names.append( (*it)->data( 0, 0 ).toString() );
        }
        it++;
    }
    return GetFidsFromNames( names );
}

/*
** Get selected fids from the QListView.
*/
void WfipsDispatchEditDialog::SelectionClicked( const QModelIndex &unused )
{
    QStringList names;
    QTreeWidgetItemIterator it( treeWidget, QTreeWidgetItemIterator::Selected );
    while( *it )
    {
        if( (*it)->data( 0, 0 ) != "" )
        {
            names.append( (*it)->data( 0, 0 ).toString() );
        }
        it++;
    }
    emit SelectionChanged( GetFidsFromNames( names ) );
}

void WfipsDispatchEditDialog::hideEvent( QHideEvent *event )
{
    /* This is false so it can uncheck a button */
    emit Hiding( false );
}

int WfipsDispatchEditDialog::PopulateRescMap( int nAgencyFlag )
{
    int i, j, rc;
    QString dl, name, type;
    int n;
    if( map.size() == 0 )
    {
        qDebug() << "Dispatch location map empty";
        return 0;
    }
    char *pszDataPath = QStringToCString( wfipsDataPath );
    WfipsData *poData = new WfipsData( pszDataPath );
    poData->Open();
    free( pszDataPath );
    int nDispLocCount;
    nDispLocCount = map.size();
    std::vector<int> anDispLocs;
    QMapIterator<qint64, QString>it( map );
    rescAtLocMap.clear();
    i = 0;
    while( it.hasNext() )
    {
        it.next();
        anDispLocs.push_back(it.key());
        rescAtLocMap[it.value()] = QList<WfipsResource>();
    }

    std::vector<WfipsResc>aoResc = poData->GetAssociatedResources( anDispLocs, nAgencyFlag );
    WfipsResource resource;
    QTreeWidgetItem *item;
    QTreeWidgetItem *subitem;
    for( i = 0; i < aoResc.size(); i++ )
    {
        n = aoResc[i].nId;
        dl = QString::fromStdString( aoResc[i].osDispLoc );
        name = QString::fromStdString( aoResc[i].osName );
        type = QString::fromStdString( aoResc[i].osType );
        resource.rowid = n;
        resource.name = name;
        resource.type = type;
        rescAtLocMap[dl].append( resource );
    }
    QMapIterator< QString, QList<WfipsResource> > it2( rescAtLocMap );
    QList<WfipsResource> rescList;
    while( it2.hasNext() )
    {
        it2.next();
        item = new QTreeWidgetItem( treeWidget );
        item->setText( 0, it2.key() );
        rescList = it2.value();
        for( i = 0; i < rescList.size(); i++ )
        {
            subitem = new QTreeWidgetItem( item );
            subitem->setText( 1, rescList[i].name );
            subitem->setText( 2, rescList[i].type );
            item->addChild( subitem );
        }
    }
    for( j = 0; j < 3; j++ )
    {
        treeWidget->resizeColumnToContents( j );
    }
    UpdateCost();
    return rescAtLocMap.size();
}

void WfipsDispatchEditDialog::ShowResources( QString dispLocName )
{
    WfipsResource resource;
    QList<WfipsResource> resources;
    resources = rescAtLocMap[dispLocName];
    for( int i = 0; i < resources.size(); i++ )
    {
        resource = resources[i];
        qDebug() << "ID:" << resource.rowid
                 << "Resource:" << resource.name
                 << "Type:" << resource.type;
    }
}

QgsFeatureIds WfipsDispatchEditDialog::GetResourceFids( int subset )
{
    QgsFeatureIds fids;
    QTreeWidgetItemIterator::IteratorFlags flag = QTreeWidgetItemIterator::All;
    if( subset == WFIPS_RESC_SUBSET_OMIT )
        flag = QTreeWidgetItemIterator::Hidden;
    else if( subset == WFIPS_RESC_SUBSET_INCLUDE )
        flag = QTreeWidgetItemIterator::NotHidden;

    int i, j;
    QString disploc, resc;
    QList<WfipsResource> resources;
    WfipsResource resource;
    QTreeWidgetItemIterator it( treeWidget, flag );
    while( *it )
    {
        i = 0;
        disploc = (*it)->data( 0, 0 ).toString();
        resources = rescAtLocMap.value( disploc );
        if( resources.size() == 0 )
        {
            it++;
            continue;
        }
        while( (*it)->child( i ) != NULL )
        {
            if( !(*it)->child( i )->isHidden() )
            {
                resc = (*it)->child( i )->data( 1, 0 ).toString();
                j = 0;
                while( resources[j].name != resc && j < resources.size() )
                    j++;
                if( j < resources.size() )
                    fids.insert( resources[j].rowid );
            }
            i++;
        }
        it++;
    }
    qDebug() << "Resource FIDs:" << fids;
    return fids;
}

/*
** Iterate through the top level items and remove any childless locations.
*/

void WfipsDispatchEditDialog::ClearEmptyLocations()
{
    QTreeWidgetItem *item;

    int i = 0;
    while( (item = treeWidget->topLevelItem( i )) != NULL )
    {
        if( item->childCount() == 0 )
        {
            item->setSelected( false );
            item->setHidden( true );
        }
        i++;
    }
    QgsFeatureIds fids = GetVisibleFids();
    UpdateCost();
    emit HiddenChanged( fids );
}

void WfipsDispatchEditDialog::UpdateCost()
{
    int rc = SQLITE_OK;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    QgsFeatureIds fids;
    fids = GetResourceFids( WFIPS_RESC_SUBSET_INCLUDE );
    char *pszFidSet = QStringToCString( BuildFidSet( "", fids ) );
    char *pszPath = QStringToCString( wfipsDataPath );
    char *pszRescDbPath = sqlite3_mprintf( "%s/resc.db", pszPath );
    char *pszCostDbPath = sqlite3_mprintf( "%s/cost.db", pszPath );
    char *pszSql = sqlite3_mprintf( "ATTACH %Q AS cost", pszCostDbPath );

    rc = sqlite3_open_v2( pszRescDbPath, &db, SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_exec( db, pszSql, NULL, NULL, NULL );
    sqlite3_free( pszSql );
    pszSql = sqlite3_mprintf( "SELECT SUM(nat) FROM resource " \
                              "LEFT JOIN cost USING(resc_type) " \
                              "WHERE resource.ROWID%s", pszFidSet );

    qDebug() << "SQL for costs:" << pszSql;

    rc = sqlite3_prepare_v2( db, pszSql, -1, &stmt, NULL );
    rc = sqlite3_step( stmt );
    if( rc == SQLITE_ROW )
    {
        ui->costSpinBox->setValue( sqlite3_column_int( stmt, 0 ) );
    }
    free( pszFidSet );
    free( pszPath );
    sqlite3_free( pszSql );
    sqlite3_free( pszRescDbPath );
    sqlite3_free( pszCostDbPath );
    rc = sqlite3_finalize( stmt );
    rc = sqlite3_close( db );
}

void WfipsDispatchEditDialog::SaveAs()
{
    QString rescOutPath =
        QFileDialog::getSaveFileName( this, tr("Save resource data"),
                                      ".", tr("WFIPS database file (*.db)") );
    qDebug() << rescOutPath;
    if( rescOutPath == "" )
        return;
    QgsFeatureIds fids;
    fids = GetResourceFids( WFIPS_RESC_SUBSET_INCLUDE );
    int n = fids.size();
    int *panIds = (int*)malloc( sizeof( int ) * n );

    QSetIterator<qint64>it( fids );
    int i = 0;
    while( it.hasNext() )
    {
        panIds[i++] = it.next();
    }
    char *pszOutputFile = QStringToCString( rescOutPath );
    char *pszPath;
    pszPath = QStringToCString( wfipsDataPath + "/" );
    WfipsData oData( pszPath );
    oData.Open();
    oData.WriteRescDb( pszOutputFile, panIds, NULL, n );
    oData.Close();
    emit SaveResourcesAs( rescOutPath );
    free( (void*)panIds );
    free( (void*)pszOutputFile );
    free( (void*)pszPath );
    accept();
}

