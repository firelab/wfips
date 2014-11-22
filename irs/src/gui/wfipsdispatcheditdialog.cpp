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
#include "ui_wfipsdispatcheditdialog.h"

WfipsDispatchEditDialog::WfipsDispatchEditDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::WfipsDispatchEditDialog )
{
    ui->setupUi( this );
    model = NULL;
    ui->listView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    ui->listView->setAlternatingRowColors( true );
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
    ui->listView->setModel( model );
    ui->listView->setEditTriggers( QAbstractItemView::NoEditTriggers );
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

