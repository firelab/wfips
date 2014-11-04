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
    ui->treeWidget->setAlternatingRowColors( true );
}

WfipsIdentifyDialog::~WfipsIdentifyDialog()
{
    delete ui;
}

void WfipsIdentifyDialog::Clear()
{
    ui->treeWidget->clear();
    for( int i = 0; i < items.size(); i++ )
    {
        delete items[i];
    }
}

void WfipsIdentifyDialog::ShowIdentifyResults( QList<QgsMapToolIdentify::IdentifyResult> results )
{
    QString s, attribute;
    QTreeWidgetItem *item, *subitem;
    const QgsFields *fields;
    QgsField field;
    QgsAttributes attributes;
    QgsVectorLayer *layer;
    QgsFeatureIds fids;
    Clear();

    /* Can we just grab one layer for all results?  I think so */
    layer = reinterpret_cast<QgsVectorLayer*>( results[0].mLayer );
    if( layer == NULL )
    {
        return;
    }
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
        **
        ** In order to use this properly, grab the feature and use it to access
        ** the data, not the mAttributes member.
        */
        item = new QTreeWidgetItem( QTreeWidgetItem::Type );
        s = results[i].mLabel;
        if( s == "" )
        {
            s = "Feature " + QString::number( i );
        }
        item->setText( 0, s );
        ui->treeWidget->addTopLevelItem( item );
        qDebug() << results[i].mDerivedAttributes;
        QVariant var;
        fields = results[i].mFeature.fields();
        attributes = results[i].mFeature.attributes();
        for( int j = 0;j < results[i].mFeature.attributes().size(); j++ )
        {
            field = fields->at( j );
            attribute = attributes[j].toString();
            qDebug() << "Field: " << field.name() << ". Attribute: " << attribute;
            subitem = new QTreeWidgetItem( QTreeWidgetItem::Type );
            subitem->setText( 1, field.name() );
            subitem->setText( 2, attribute );
            item->addChild( subitem );
        }
        /* Get the feature id from the derived attributes for selection display */
        fids.insert( STRING_TO_FID( results[i].mDerivedAttributes["feature id"] ) );
    }

    if( layer != NULL && results.size() > 0 )
    {
        layer->removeSelection();
        layer->select( fids );
    }
    ui->treeWidget->expandAll();
    this->show();
}
