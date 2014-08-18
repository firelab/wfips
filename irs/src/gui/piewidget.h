//piechart/piewidget.h

#ifndef PIEWIDGET_H
#define PIEWIDGET_H

#include <QFrame>
#include <map>

class PieWidget : public QFrame    {
    Q_OBJECT
public:
    PieWidget(QWidget *parent = 0);

	~PieWidget();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void addEntry(const QString &key, double val);
    void clearEntries();

protected:
    void paintEvent(QPaintEvent *ev);

private:
    std::map<QString, double> values;
};

#endif // PIEWIDGET_H
