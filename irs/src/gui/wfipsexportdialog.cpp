#include "wfipsexportdialog.h"
#include "ui_wfipsexportdialog.h"

WfipsExportDialog::WfipsExportDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::WfipsExportDialog )
{
    ui->setupUi( this );
    connect( ui->buttonBox, SIGNAL( accepted() ),
             this, SLOT( SaveAs() ) );
}

WfipsExportDialog::~WfipsExportDialog()
{
    delete ui;
}

void WfipsExportDialog::SaveAs()
{
    exportLevel = ui->comboBox->currentIndex();
    /* Get our OGR available drivers for output */
    QStringList formats;
    QByteArray format;
    QStringList exts;
    QStringList files;
    QString exportFile;
    OGRDataSourceH hSrcDS, hDstDS;
    OGRLayerH hSrcLyr, hDstLyr;
    OGRFeatureH hSrcFeat, hDstFeat;
    OGRFeatureDefnH hFeatDefn;
    const char *pszFormat;
    OGRRegisterAll();
    int i;
    OGRSFDriverH hDrv;
    if( exportLevel > 0 )
    {
        formats << "CSV files *.txt";
    }
    else
    {
        for( i = 0; i < OGRGetDriverCount(); i++ )
        {
            hDrv = OGRGetDriver( i );
            if( OGR_Dr_TestCapability( hDrv, ODrCCreateDataSource ) )
            {
                formats << OGR_Dr_GetName( hDrv );
            }
        }
    }
    QFileDialog dialog( this );
    dialog.setNameFilters( formats );
    dialog.setFileMode( QFileDialog::AnyFile );
    dialog.exec();
    files = dialog.selectedFiles();
    if( files.size() < 1 )
        return;
    exportFile = files[0];
    format = dialog.selectedNameFilter().toLocal8Bit();
    pszFormat = CPLSPrintf( "%s", (const char*)format.data() );
    qDebug() << "Using OGR to write" << exportFile << "as a" << pszFormat;
    hDrv = OGRGetDriverByName( pszFormat );
    if( hDrv == NULL )
    {/* report and cancel */}
    driver = pszFormat;
    filename = exportFile;
    return;
}

