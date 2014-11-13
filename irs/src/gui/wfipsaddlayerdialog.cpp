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

#include "wfipsaddlayerdialog.h"
#include "ui_wfipsaddlayerdialog.h"

WfipsAddLayerDialog::WfipsAddLayerDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::WfipsAddLayerDialog )
{
    Setup();
}

WfipsAddLayerDialog::WfipsAddLayerDialog( QString initPath, QWidget *parent) :
    QDialog( parent ),
    ui( new Ui::WfipsAddLayerDialog )
{
    this->initPath = initPath;
    Setup();
}

void WfipsAddLayerDialog::Setup()
{
    ui->setupUi(this);
    /* Connections */
    connect( ui->openToolButton, SIGNAL( clicked() ),
             this, SLOT( OpenAreaLayerFile() ) );
    connect( ui->addToolButton, SIGNAL( clicked() ),
             this, SLOT( Accept() ) );
    connect( ui->cancelToolButton, SIGNAL( clicked() ),
             this, SLOT( Cancel() ) );
    connect( ui->comboBox, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( UpdateAttributes( int ) ) );
}

WfipsAddLayerDialog::~WfipsAddLayerDialog()
{
    delete ui;
}

QString WfipsAddLayerDialog::GetFilePath()
{
    return filePath;
}

QString WfipsAddLayerDialog::GetCurrentLayer()
{
    return layer;
}

QStringList WfipsAddLayerDialog::GetLayers()
{
    return layers;
}

QStringList WfipsAddLayerDialog::GetAttributes()
{
    return attributes;
}
void WfipsAddLayerDialog::OpenAreaLayerFile()
{
    filePath =
        QFileDialog::getOpenFileName( this, tr( "Open GIS file" ), initPath, "" );
    ui->lineEdit->setText( filePath );
    ui->comboBox->clear();
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
        ui->addToolButton->setDisabled( true );
        ui->lineEdit->setText( "" );
        return;
    }
    layers.clear();
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
        ui->addToolButton->setDisabled( true );
        ui->lineEdit->setText( "" );
        GDALClose( hDS );
        return;
    }
    ui->comboBox->addItems( layers );
    OGR_DS_Destroy( hDS );
}

void WfipsAddLayerDialog::UpdateAttributes( int unused )
{
    (void)unused;

    layer = ui->comboBox->currentText();
    attributes = QStringList();
    if( layer == "" || filePath == "" )
    {
        return;
    }

    const char *pszFilename = QStringToCString( filePath );
    OGRDataSourceH hDS = OGROpen( pszFilename, FALSE, NULL );
    free( (void*)pszFilename );
    if( hDS == NULL )
    {
        return;
    }
    OGRLayerH hLyr;
    const char *pszLayer = QStringToCString( layer );
    hLyr = OGR_DS_GetLayerByName( hDS, pszLayer );
    free( (void*)pszLayer );
    if( hLyr == NULL )
    {
        return;
    }
    OGRFeatureDefnH hDefn = OGR_L_GetLayerDefn( hLyr );
    if( hDefn == NULL )
    {
        return;
    }
    OGRFieldDefnH hField;
    int n = OGR_FD_GetFieldCount( hDefn );
    for( int i = 0; i < n; i++ )
    {
        hField = OGR_FD_GetFieldDefn( hDefn, i );
        attributes.append( QString( OGR_Fld_GetNameRef( hField ) ) );
    }
    qDebug() << "Found attributes in addlayer: " << attributes;
    OGR_DS_Destroy( hDS );
}

void WfipsAddLayerDialog::Accept()
{
    this->close();
}

void WfipsAddLayerDialog::Cancel()
{
    filePath = "";
    layer = "";
    this->close();
}

