#ifndef WFIPSEXPORTDIALOG_H_
#define WFIPSEXPORTDIALOG_H_

#include <QByteArray>
#include <QDebug>
#include <QDialog>
#include <QFileDialog>

#include "cpl_string.h"
#include "ogr_api.h"


namespace Ui {
class WfipsExportDialog;
}

class WfipsExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WfipsExportDialog( QWidget *parent = 0 );
    ~WfipsExportDialog();

    QString GetFormat() {return driver;}
    QString GetFilename() {return filename;}
    int GetExportLevel() {return exportLevel;}

private:
    Ui::WfipsExportDialog *ui;
    QString filename, driver;
    int exportLevel;

private slots:
    void SaveAs();
};

#endif /* WFIPSEXPORTDIALOG_H */

