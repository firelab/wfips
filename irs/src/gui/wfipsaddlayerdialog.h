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

#ifndef WFIPSADDAREADIALOG_H
#define WFIPSADDAREADIALOG_H

#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QString>

#include "wfipsguiutil.h"

#include "gdal.h"
#include "ogr_api.h"

namespace Ui {
class WfipsAddLayerDialog;
}

class WfipsAddLayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WfipsAddLayerDialog( QWidget *parent = 0 );
    WfipsAddLayerDialog( QString initPath, QWidget *parent = 0);
    ~WfipsAddLayerDialog();

    QString GetFilePath();
    QString GetCurrentLayer();
    QStringList GetLayers();
    QStringList GetAttributes();

private:
    Ui::WfipsAddLayerDialog *ui;

    void Setup();

    QString filePath, layer;
    QStringList layers;
    QString initPath;
    QStringList attributes;

private slots:
    void OpenAreaLayerFile();
    void UpdateAttributes( int unused );
    void Accept();
    void Cancel();
};

#endif /* WFIPSADDAREADIALOG_H */
