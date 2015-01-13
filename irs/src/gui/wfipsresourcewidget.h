#ifndef WFIPSRESOURCEWIDGET_H
#define WFIPSRESOURCEWIDGET_H

#include <QWidget>

namespace Ui {
class WfipsResourceWidget;
}

class WfipsResourceWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit WfipsResourceWidget(QWidget *parent = 0);
    ~WfipsResourceWidget();
    
private:
    Ui::WfipsResourceWidget *ui;
};

#endif // WFIPSRESOURCEWIDGET_H
