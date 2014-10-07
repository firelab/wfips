#ifndef WFIPSMAINWINDOW_H
#define WFIPSMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class WfipsMainWindow;
}

class WfipsMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit WfipsMainWindow(QWidget *parent = 0);
    ~WfipsMainWindow();

private:
    Ui::WfipsMainWindow *ui;
};

#endif // WFIPSMAINWINDOW_H
