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

#include "wfipsidentifydialog.h"
#include "ui_wfipsidentifydialog.h"

WfipsIdentifyDialog::WfipsIdentifyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WfipsIdentifyDialog)
{
    ui->setupUi(this);
}

WfipsIdentifyDialog::~WfipsIdentifyDialog()
{
    delete ui;
}

void WfipsIdentifyDialog::ShowIdentifyResults( QList<QgsMapToolIdentify::IdentifyResult> results )
{
    ui->treeWidget->clear();
    for( int i = 0; i < results.size(); i++ )
    {
        /*
        ** Results have a variety of data lets just display the label and the
        ** attributes.  Below is the layout of the result struct
        **
        ** QgsMapLayer * mLayer
        ** QString mLabel
        ** QgsFields mFields
        ** QgsFeature mFeature
        ** QMap<QString, QString> mAttributes
        ** QMap<QString, QString> mDerivedAttributes
        ** QMap<QString, QVariant> mParams
        */
        qDebug() << results[i].mLabel;
        qDebug() << results[i].mAttributes;
    }
}
