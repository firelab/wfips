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

void WfipsDispatchEditDialog::SetModel( const QMap<qint64, QString> &map )
{
    this->map = map;
    PopulateRescMap();
}

void WfipsDispatchEditDialog::SelectFids( QgsFeatureIds fids )
{
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
    emit Hiding();
}

int WfipsDispatchEditDialog::PopulateRescMap()
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int j, rc;
    QString dl, name, type;
    int n;
    if( map.size() == 0 )
    {
        qDebug() << "Dispatch location map empty";
        return 0;
    }
    char zSql[8192];
    const char *zDataPath = QStringToCString( wfipsDataPath );
    sqlite3_snprintf( 8192, zSql, (const char *)"%s/resc.db", zDataPath );
    rc = sqlite3_open_v2( zSql, &db, SQLITE_OPEN_READONLY, NULL );
    if( rc != SQLITE_OK || db == NULL )
    {
        qDebug() << "Failed to open resc.db";
        return 0;
    }
    rc = sqlite3_prepare_v2( db, "SELECT ROWID, name, resc_type FROM resource " \
                                 "WHERE disploc=?", -1, &stmt, NULL );
    if( rc != SQLITE_OK )
    {
        sqlite3_close( db );
        qDebug() << "Failed to prep resc statment.";
        return 0;
    }
    rescAtLocMap.clear();
    QMapIterator<qint64, QString>it( map );

    WfipsResource resource;
    QList<WfipsResource>resourceList;

    QTreeWidgetItem *item;
    QTreeWidgetItem *subitem;

    while( it.hasNext() )
    {
        it.next();
        dl = it.value();
        rc = sqlite3_bind_text( stmt, 1, (char*)dl.toLocal8Bit().data(), -1,
                                SQLITE_TRANSIENT );
        j = 0;
        resourceList.clear();
        item = new QTreeWidgetItem( treeWidget );
        item->setText( 0, dl );
        while( sqlite3_step( stmt ) == SQLITE_ROW )
        {
            n = sqlite3_column_int( stmt, 0 );
            name = (char*)sqlite3_column_text( stmt, 1 );
            type = (char*)sqlite3_column_text( stmt, 2 );
            resource.rowid = n;
            resource.name = name;
            resource.type = type;
            resourceList.append( resource );
            subitem = new QTreeWidgetItem( item );
            subitem->setText( 1, name );
            subitem->setText( 2, type );
            item->addChild( subitem );
            j++;
        }
        rescAtLocMap[dl] = resourceList;
        if( j == 0 )
        {
            qDebug() << "Found no resources at :" << dl;
        }
        rc = sqlite3_reset( stmt );
    }
    rc = sqlite3_finalize( stmt );
    rc = sqlite3_close( db );
    qDebug() << "Found resources at " << rescAtLocMap.size() << " dispatch locations.";
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
        resources = rescAtLocMap[disploc];
        while( (*it)->child( i ) != NULL )
        {
            if( !(*it)->child( i )->isHidden() )
            {
                resc = (*it)->child( i )->data( 1, 0 ).toString();
                j = 0;
                while( resources[j].name != resc )
                    j++;
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

