#ifndef EDITTREELEVEL_H
#define EDITTREELEVEL_H

#include <QDialog>

namespace Ui {
class EditTreeLevel;
}

class EditTreeLevel : public QDialog
{
    Q_OBJECT

public:
    explicit EditTreeLevel(QWidget *parent = 0);
    ~EditTreeLevel();

private slots:
    void changeChecked();
    void editLayer();
    void setDOIAgencies(int value);

private:
    Ui::EditTreeLevel *ui;
};

#endif // EDITTREELEVEL_H
