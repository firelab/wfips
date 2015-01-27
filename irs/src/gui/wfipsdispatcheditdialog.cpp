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

    connect( treeWidget, SIGNAL( pressed( const QModelIndex & ) ),
             this, SLOT( SelectionClicked( const QModelIndex & ) ) );
    connect( ui->omitToolButton, SIGNAL( clicked() ),
             this, SLOT( Omit() ) );
    connect( ui->revertToolButton, SIGNAL( clicked() ),
             this, SLOT( Unhide() ) );
    connect( treeWidget, SIGNAL( RightClick( QString ) ),
             this, SLOT( ShowResources( QString ) ) );
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
    delete model;
    model = new QStringListModel( this );
    model->setStringList( this->map.values() );
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

