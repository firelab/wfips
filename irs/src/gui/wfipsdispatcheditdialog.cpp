#include "wfipsdispatcheditdialog.h"
#include "ui_wfipsdispatcheditdialog.h"

WfipsDispatchEditDialog::WfipsDispatchEditDialog( QWidget *parent ) :
    QDialog( parent ),
    ui( new Ui::WfipsDispatchEditDialog )
{
    ui->setupUi( this );
}

WfipsDispatchEditDialog::~WfipsDispatchEditDialog()
{
    delete ui;
}
