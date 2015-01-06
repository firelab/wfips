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
    model = NULL;
    ui->listView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    ui->listView->setAlternatingRowColors( true );
    connect( ui->listView, SIGNAL( pressed( const QModelIndex & ) ),
             this, SLOT( SelectionClicked( const QModelIndex & ) ) );
    connect( ui->omitToolButton, SIGNAL( clicked() ),
             this, SLOT( Omit() ) );
    connect( ui->revertToolButton, SIGNAL( clicked() ),
             this, SLOT( Unhide() ) );
}

WfipsDispatchEditDialog::~WfipsDispatchEditDialog()
{
    delete ui;
}

void WfipsDispatchEditDialog::SetModel( const QMap<qint64, QString> &map )
{
    this->map = map;
    delete model;
    model = new QStringListModel( this );
    model->setStringList( this->map.values() );
    model->sort( 0 );
    ui->listView->setModel( model );
    ui->listView->setEditTriggers( QAbstractItemView::NoEditTriggers );
    PopulateRescMap();
}

void WfipsDispatchEditDialog::SelectFids( QgsFeatureIds fids )
{
    QString loc;
    QgsFeatureId fid;
    QSet<qint64>::iterator it = fids.begin();
    ui->listView->clearSelection();
    int i;
    while( it != fids.end() )
    {
        loc = map.value( *it );
        qDebug() << "Selecting fid: " << *it << ", name; " << loc;
        i = map.values().indexOf( loc );
        if( i < 0 )
        {
            it++;
            continue;
        }
        ui->listView->setCurrentIndex( model->index( i ) );
        it++;
    }
    return;
}

QList<int> WfipsDispatchEditDialog::GetSelectedIndices()
{
    QList<int> indices;
    foreach( const QModelIndex &index, ui->listView->selectionModel()->selectedIndexes() )
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
        ui->listView->setRowHidden( selectedRows[i], true );
    }
    QgsFeatureIds fids = GetVisibleFids();
    ui->listView->clearSelection();
    emit HiddenChanged( fids );
}

void WfipsDispatchEditDialog::Unhide()
{
    for( int i = 0; i < model->rowCount(); i++ )
    {
        ui->listView->setRowHidden( i, false );
    }
    ui->listView->clearSelection();
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
        if( !ui->listView->isRowHidden( i ) )
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
    foreach( const QModelIndex &index, ui->listView->selectionModel()->selectedIndexes() )
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
    const char *pszDispLoc;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int i, rc;
    QString dl, type;
    int n;
    if( map.size() == 0 )
    {
        return 0;
    }
    rc = sqlite3_open_v2( "disploc.db", &db, SQLITE_OPEN_READONLY, NULL );
    rc = sqlite3_exec( db, "ATTACH 'resc.db' as resc", NULL, NULL, NULL );
    rc = sqlite3_prepare_v2( db, "SELECT resc_type, count(*) FROM disploc " \
                                 "JOIN resource ON disploc.name=resource.disploc " \
                                 "WHERE disploc.name=? GROUP BY disploc.name, " \
                                 "resc_type",
                             -1, &stmt, NULL );

    rescAtLocMap.clear();
    QMapIterator<qint64, QString>it( map );

    while( it.hasNext() )
    {
        it.next();
        dl = it.value();
        rc = sqlite3_bind_text( stmt, 1, (char*)dl.toLocal8Bit().data(), -1,
                                SQLITE_TRANSIENT );
        rc = sqlite3_step( stmt );
        if( rc != SQLITE_ROW )
        {
            sqlite3_reset( stmt );
            continue;
        }
        type = (char*)sqlite3_column_text( stmt, 0 );
        n = sqlite3_column_int( stmt, 1 );

        rescAtLocMap[dl][type] = n;
        rc = sqlite3_reset( stmt );
    }
    rc = sqlite3_finalize( stmt );
    rc = sqlite3_close( db );
    qDebug() << "Found resources at " << rescAtLocMap.size() << " dispatch locations.";
    return rescAtLocMap.size();
}


