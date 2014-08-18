#include "edittreelevel.h"
#include "ui_edittreelevel.h"

EditTreeLevel::EditTreeLevel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditTreeLevel)
{
    ui->setupUi(this);

    //Setup GUI
    ui->stackedWidget->setCurrentIndex(0);
    ui->editButton->setEnabled(false);
    ui->acceptButton->setVisible(false);
    ui->stackedWidget_2->setVisible(false);

    //Connections
    connect (ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect (ui->agencyButton, SIGNAL(clicked()), this, SLOT(changeChecked()));
    connect (ui->areaButton, SIGNAL(clicked()), this, SLOT(changeChecked()));
    connect (ui->rescTypeButton, SIGNAL(clicked()), this, SLOT(changeChecked()));
    connect (ui->editButton, SIGNAL(clicked()), this, SLOT(editLayer()));
    connect (ui->DOIBox, SIGNAL(valueChanged(int)), this, SLOT(setDOIAgencies(int)));

}

EditTreeLevel::~EditTreeLevel()
{
    delete ui;
}

void EditTreeLevel::changeChecked()
{
    if (ui->agencyButton->isChecked())  {
        ui->stackedWidget->setCurrentIndex(1);
        ui->editButton->setEnabled(true);
    }
    else if (ui->areaButton->isChecked())   {
        ui->stackedWidget->setCurrentIndex(2);
        ui->editButton->setEnabled(true);
    }
    else if (ui->rescTypeButton->isChecked())   {
        ui->stackedWidget->setCurrentIndex(3);
        ui->editButton->setEnabled(true);
    }
    else    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->editButton->setEnabled(false);
    }
}

void EditTreeLevel::editLayer()
{
    ui->editButton->setEnabled(false);
    ui->acceptButton->setVisible(true);
    ui->acceptButton->setEnabled(true);

    ui->stackedWidget_2->setVisible(true);
    if (ui->agencyButton->isChecked())
        ui->stackedWidget_2->setCurrentIndex(0);
    else if (ui->areaButton->isChecked() && ui->regionButton->isChecked())
        ui->stackedWidget_2->setCurrentIndex(1);
    else if (ui->areaButton->isChecked() && ui->stateButton->isChecked())
        ui->stackedWidget_2->setCurrentIndex(2);
    else if (ui->rescTypeButton->isChecked())
        ui->stackedWidget_2->setCurrentIndex(3);

    ui->layerBox->setEnabled(false);
    ui->stackedWidget->setEnabled(false);


}

void EditTreeLevel::setDOIAgencies(int value)
{
    ui->BIABox->setValue(value);
    ui->BLMBox->setValue(value);
    ui->FWSBox->setValue(value);
    ui->NPSBox->setValue(value);
}
