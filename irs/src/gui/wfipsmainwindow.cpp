#include "wfipsmainwindow.h"
#include "ui_wfipsmainwindow.h"

WfipsMainWindow::WfipsMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::WfipsMainWindow)
{
    ui->setupUi(this);
    ui->treeWidget->expandAll();
}

WfipsMainWindow::~WfipsMainWindow()
{
    delete ui;
}

