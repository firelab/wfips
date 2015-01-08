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

#ifndef WFIPSDISPATCHEDITDIALOG_H
#define WFIPSDISPATCHEDITDIALOG_H

#include <QDebug>
#include <QDialog>
#include <QListView>
#include <QMouseEvent>
#include <QString>
#include <QStringListModel>
#include <sqlite3.h>

#include <qgsfeature.h>

#include "ui_wfipsdispatcheditdialog.h"

namespace Ui {
class WfipsDispatchEditDialog;
}

class WfipsDispatchListView : public QListView
{
    Q_OBJECT

public:
    explicit WfipsDispatchListView( QWidget *parent = 0 );
    ~WfipsDispatchListView();

protected slots:
    void mousePressEvent( QMouseEvent *event );
    void mouseDoubleClickEvent( QMouseEvent *event );

signals:
    //void RightClick( QModelIndex &index );
};

class WfipsDispatchEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WfipsDispatchEditDialog( QWidget *parent = 0 );
    WfipsDispatchEditDialog( QStringList stringList, QWidget *parent = 0 );
    ~WfipsDispatchEditDialog();

    void SetModel( const QMap<QgsFeatureId, QString> &map );
    void SelectFids( QgsFeatureIds fids );

private:
    Ui::WfipsDispatchEditDialog *ui;

    WfipsDispatchListView *listView;

    QStringListModel *model;
    QMap<QgsFeatureId, QString>map;
    QList<int> GetSelectedIndices();
    QgsFeatureIds GetVisibleFids();
    QgsFeatureIds GetFidsFromNames( QStringList names );

    QMap< QString, QMap<QString, int> > rescAtLocMap;
    int PopulateRescMap();

protected:
    void hideEvent( QHideEvent *event );

private slots:
    void SelectionClicked( const QModelIndex & );
    void Omit();
    void Unhide();

signals:
    void SelectionChanged( const QgsFeatureIds &fids );
    void Hiding();
    void HiddenChanged( const QgsFeatureIds &fids );
};

#endif /* WFIPSDISPATCHEDITDIALOG_H */

