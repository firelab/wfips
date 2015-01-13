#include "wfipsresourcewidget.h"
#include "ui_wfipsresourcewidget.h"

WfipsResourceWidget::WfipsResourceWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WfipsResourceWidget)
{
    ui->setupUi(this);
}

WfipsResourceWidget::~WfipsResourceWidget()
{
    delete ui;
}
