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

WfipsDispatchListView::WfipsDispatchListView( QWidget *parent ) :
    QListView( parent )
{
}

WfipsDispatchListView::~WfipsDispatchListView()
{
}

QString WfipsDispatchListView::GetDispLocName( QMouseEvent *event )
{
    QModelIndex idx;
    idx = indexAt( event->pos() );
    if( !idx.isValid() )
    {
        return QString();
    }
    QString name = idx.data().toString();
    return name;
}

void WfipsDispatchListView::mouseDoubleClickEvent( QMouseEvent *event )
{
    if( event->button() == Qt::LeftButton )
    {
        QString name = GetDispLocName( event );
        emit RightClick( name );
    }
    else
    {
        return QListView::mousePressEvent( event );
    }
}

void WfipsDispatchListView::mousePressEvent( QMouseEvent *event )
{
    if( event->button() == Qt::RightButton )
    {
        QString name = GetDispLocName( event );
        emit RightClick( name );
    }
    else
    {
        return QListView::mousePressEvent( event );
    }
}


WfipsDispatchEditDialog::WfipsDispatchEditDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::WfipsDispatchEditDialog )
{
    ui->setupUi( this );
    listView = new WfipsDispatchListView( this );
    ui->verticalLayout->insertWidget( 0, listView );
    model = NULL;
    listView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    listView->setAlternatingRowColors( true );
    connect( listView, SIGNAL( pressed( const QModelIndex & ) ),
             this, SLOT( SelectionClicked( const QModelIndex & ) ) );
    connect( ui->omitToolButton, SIGNAL( clicked() ),
             this, SLOT( Omit() ) );
    connect( ui->revertToolButton, SIGNAL( clicked() ),
             this, SLOT( Unhide() ) );
    connect( listView, SIGNAL( RightClick( QString ) ),
             this, SLOT( ShowResources( QString ) ) );
}

WfipsDispatchEditDialog::~WfipsDispatchEditDialog()
{
    delete ui;
    delete listView;
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
    //model->sort( 0 );
    listView->setModel( model );
    listView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    PopulateRescMap();
}

void WfipsDispatchEditDialog::SelectFids( QgsFeatureIds fids )
{
    QString loc;
    QSet<qint64>::iterator it = fids.begin();
    listView->clearSelection();
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
        listView->setCurrentIndex( model->index( i ) );
        listView->scrollTo( model->index( i ) );
        it++;
    }
    return;
}

QList<int> WfipsDispatchEditDialog::GetSelectedIndices()
{
    QList<int> indices;
    foreach( const QModelIndex &index, listView->selectionModel()->selectedIndexes() )
    {
        indices.append( index.row() );
    }
    qDebug() << "Selected indices: " << indices;
    return indices;
}

/*
** Hide the selected dispatch locations from the list.
*/
void WfipsDispatchEditDialog::Omit()
{
    QList<int> selectedRows;
    selectedRows = GetSelectedIndices();
    for( int i = 0; i < selectedRows.size(); i++ )
    {
        listView->setRowHidden( selectedRows[i], true );
    }
    QgsFeatureIds fids = GetVisibleFids();
    listView->clearSelection();
    emit HiddenChanged( fids );
}

void WfipsDispatchEditDialog::Unhide()
{
    for( int i = 0; i < model->rowCount(); i++ )
    {
        listView->setRowHidden( i, false );
    }
    listView->clearSelection();
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
    QStringList names;
    for( int i = 0; i < model->rowCount(); i++ )
    {
        if( !listView->isRowHidden( i ) )
        {
            names.append( model->index( i, 0).data().toString() );
        }
    }
    return GetFidsFromNames( names );
}

/*
** Get selected fids from the QListView.
*/
void WfipsDispatchEditDialog::SelectionClicked( const QModelIndex &unused )
{
    QStringList names;
    foreach( const QModelIndex &index, listView->selectionModel()->selectedIndexes() )
    {
        names.append( model->data( index, 0 ).toString() );
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
    int i, j, rc;
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

    while( it.hasNext() )
    {
        it.next();
        dl = it.value();
        rc = sqlite3_bind_text( stmt, 1, (char*)dl.toLocal8Bit().data(), -1,
                                SQLITE_TRANSIENT );
        j = 0;
        resourceList.clear();
        while( sqlite3_step( stmt ) == SQLITE_ROW )
        {
            n = sqlite3_column_int( stmt, 0 );
            name = (char*)sqlite3_column_text( stmt, 1 );
            type = (char*)sqlite3_column_text( stmt, 2 );
            resource.rowid = n;
            resource.name = name;
            resource.type = type;
            resourceList.append( resource );
            rescAtLocMap[dl][name] = type;
            j++;
        }
        rescAtLocMap2[dl] = resourceList;
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
    QMap<QString, QString>m;
    QString name, type;
    m = rescAtLocMap[dispLocName];
    qDebug() << "Resources at: " << dispLocName;
    QMapIterator<QString, QString> it( m );
    while( it.hasNext() )
    {
        it.next();
        name = it.key();
        type = it.value();
        qDebug() << "Resource:" << name << "Type:" << type;
    }
    qDebug() << "TEST##############TEST";
    WfipsResource resource;
    QList<WfipsResource> resources;
    resources = rescAtLocMap2[dispLocName];
    for( int i = 0; i < resources.size(); i++ )
    {
        resource = resources[i];
        qDebug() << "Resource:" << resource.name << "Type:" << resource.type;
    }
}

