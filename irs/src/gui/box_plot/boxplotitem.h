#ifndef BOXPLOTITEM_H
#define BOXPLOTITEM_H

#include <qwt_plot_item.h>
#include <QList>

class QString;

class boxPlotItem : public QwtPlotItem
{
public:
    boxPlotItem(const QString &title = QString::null);
    ~boxPlotItem();

    void setData(QVector<QList<double>> newdata);
    QVector<QList<double>> data();

    void setColor(const QColor &newColor);
    QColor color() const;

    virtual QwtDoubleRect boundingRect() const;

    virtual void draw(QPainter *, const QwtScaleMap &xMap,
        const QwtScaleMap &yMap, const QRect &) const;

    void setBaseline(double reference);
    double baseline() const;

    enum boxPlotItemAttribute
    {
        Auto = 0,
        Xfy = 1
    };

    void setBoxPlotAttribute(boxPlotItemAttribute, bool on = true);
    bool testBoxPlotAttribute(boxPlotItemAttribute) const;

protected:
    void drawBox(QPainter *,
        Qt::Orientation o, const QRect &) const;

private:
    void init();
    void calcBoxValues(QVector<QList<double>> dataList);

    int attributes;
    QVector<QList<double>> dataLists;
    QVector<std::pair<double, double>> outliers;
    QVector<double> bottomWhiskers;
    QVector<double> Q1s;
    QVector<double> medians;
    QVector<double> Q3s;
    QVector<double> topWhiskers;
    QColor plotColor;
    double reference;
};

#endif // BOXPLOTITEM_H
