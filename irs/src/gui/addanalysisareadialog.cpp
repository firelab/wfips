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

#include "addanalysisareadialog.h"
#include "ui_addanalysisareadialog.h"

AddAnalysisAreaDialog::AddAnalysisAreaDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddAnalysisAreaDialog)
{
    ui->setupUi(this);
    /* Connections */
    connect( ui->openLayerFileToolButton, SIGNAL( clicked() ),
             this, SLOT( OpenAreaLayerFile() ) );
    connect( ui->addLayerToolButton, SIGNAL( clicked() ),
             this, SLOT( Accept() ) );
    connect( ui->cancelToolButton, SIGNAL( clicked() ),
             this, SLOT( Cancel() ) );
}

AddAnalysisAreaDialog::~AddAnalysisAreaDialog()
{
    delete ui;
}

QString AddAnalysisAreaDialog::GetFilePath()
{
    return filePath;
}

QString AddAnalysisAreaDialog::GetLayerName()
{
    layerName = ui->addLayerComboBox->currentText();
    return layerName;
}

void AddAnalysisAreaDialog::OpenAreaLayerFile()
{
    filePath =
        QFileDialog::getOpenFileName( this, tr( "Open GIS file" ), "", "" );
    ui->addLayerLineEdit->setText( filePath );
    ui->addLayerComboBox->clear();
    if( filePath== "" ) 
    {
        return;
    }
    const char *pszFilename = QStringToCString( filePath );
    OGRDataSourceH hDS = OGROpen( pszFilename, FALSE, NULL );
    free( (void*)pszFilename );
    if( hDS == NULL )
    {
        qDebug() << "Could not identify layer file";
        ui->addLayerToolButton->setDisabled( true );
        ui->addLayerLineEdit->setText( "" );
        return;
    }
    QStringList layers;
    int i = 0;
    OGRLayerH hLyr;
    while( i < OGR_DS_GetLayerCount( hDS ) )
    {
        hLyr = OGR_DS_GetLayer( hDS, i );
        if( OGR_L_GetGeomType( hLyr ) == wkbPolygon ||
            OGR_L_GetGeomType( hLyr ) == wkbMultiPolygon )
        {
            layers << OGR_L_GetName( OGR_DS_GetLayer( hDS, i ) );
        }
        i++;
    }
    if( layers.size() < 1 )
    {
        ui->addLayerToolButton->setDisabled( true );
        ui->addLayerLineEdit->setText( "" );
        GDALClose( hDS );
        return;
    }
    ui->addLayerComboBox->addItems( layers );
    OGR_DS_Destroy( hDS );
}

void AddAnalysisAreaDialog::Accept()
{
    this->close();
}

void AddAnalysisAreaDialog::Cancel()
{
    filePath = "";
    layerName = "";
    this->close();
}

