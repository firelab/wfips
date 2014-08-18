#include <list>
#include <qstring.h>
#include <QList>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_interval_data.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>
#include <qwt_symbol.h>
#include "boxplotitem.h"

boxPlotItem::boxPlotItem(const QString &title) :
    QwtPlotItem(QwtText(title))
{
    //init();
}

boxPlotItem::~boxPlotItem()
{}

void boxPlotItem::init()
{
    reference = 0.0;
    attributes = boxPlotItem::Auto;

    setItemAttribute(QwtPlotItem::AutoScale, true);
    setItemAttribute(QwtPlotItem::Legend, true);
}

int boxPlotItem::rtti() const
{
	return QwtPlotItem::Rtti_PlotUserItem;
}

void boxPlotItem::calcBoxValues(QVector<QList<double> > dataLists)
{
    double midpt;
    double q1;
    double q3;
    double w1;
    double w3;

    for (int i = 0; i < dataLists.size(); i++)  {

        midpt = q1 = q3 = w1 = w3 = 0;

        std::list<double> dataSet = dataLists[i].toStdList();
        std::list<double>::iterator it;
        std::list<double> topHalf;
        std::list<double> bottomHalf;

        dataSet.sort();

        int count = dataSet.size();
        if (count == 0 || count == 1)
            return;


        it = dataSet.begin();
        double last = 0;
        for (int j = 0; j < count / 2; j++)    {
            bottomHalf.push_back(*it);
            last = (*it);
            it++;
        }

        if (count % 2 == 0)
            midpt = (last + (*it)) / 2;
        else   {
            midpt = (*it);
            it++;
        }

        while (it != dataSet.end()) {
            topHalf.push_back(*it);
            it++;
        }

        int qcount = bottomHalf.size();

        it = bottomHalf.begin();
        for (int j = 0; j < qcount / 2; j++) {
            last = (*it);
            it++;
        }

        if (qcount % 2 == 0)
            q1 = (last + (*it)) / 2;
        else
            q1 = (*it);

        qcount = topHalf.size();

        it = topHalf.begin();
        for (int j = 0; j < qcount / 2; j++) {
            last = (*it);
            it++;
        }

        if (qcount % 2 == 0)
            q3 = (last + (*it)) / 2;
        else
            q3 = (*it);

        double interQRange = q3 - q1;

        w1 = q1 - 1.5 * interQRange;
        w3 = q3 + 1.5 * interQRange;

        it = dataSet.begin();
        if (w1 < (*it))
            w1 = (*it);
        else    {
            while (w1 > (*it))  {
                std::pair<double, double> thePair = std::make_pair(i + 1, (*it));
                outliers.push_back(thePair);
                it++;
            }
        }

        it = dataSet.end();
        it--;
        if (w3 > (*it))
            w3 = (*it);
        else    {
            while (w3 < (*it))  {
                std::pair<double, double> thePair = std::make_pair(i + 1, (*it));
                outliers.push_back(thePair);
                it--;
            }
        }

        bottomWhiskers.push_back(w1);
        Q1s.push_back(q1);
        medians.push_back(midpt);
        Q3s.push_back(q3);
        topWhiskers.push_back(w3);

    }

}

void boxPlotItem::setBoxPlotAttribute(boxPlotItemAttribute attribute, bool on)
{
    if ( bool(attributes & attribute) == on )
        return;

    if ( on )
        attributes |= attribute;
    else
        attributes &= ~attribute;

    itemChanged();
}

bool boxPlotItem::testBoxPlotAttribute(boxPlotItemAttribute attribute) const
{
    return attributes & attribute;
}

void boxPlotItem::setData(QVector<QList<double> > newdata)
{
    dataLists = newdata;
    calcBoxValues(dataLists);
    itemChanged();
}


QVector<QList<double> > boxPlotItem::data()
{
    return dataLists;
}

void boxPlotItem::setColor(const QColor &newcolor)
{
    if ( plotColor != newcolor )
    {
        plotColor = newcolor;
        itemChanged();
    }
}

QColor boxPlotItem::color() const
{
    return plotColor;
}

QwtDoubleRect boxPlotItem::boundingRect() const
{
    double minX = 0;
    double maxX = dataLists.size() + 1;

    if (maxX < 1 || bottomWhiskers.size() < 1 || topWhiskers.size() < 1)
      return QwtDoubleRect(1.0, 1.0, -2.0, -2.0);

    double minY = bottomWhiskers[0];
    double maxY = topWhiskers[0];

    for (int i = 0; i < bottomWhiskers.size(); i++ )    {
        if (minY > bottomWhiskers[i])
            minY = bottomWhiskers[i];
        if (maxY < topWhiskers[i])
            maxY = topWhiskers[i];

        for (int j = 0; j < outliers.size(); j++)   {
            if (minY > outliers[j].second)
                minY = outliers[j].second;
            if (maxY < outliers[j].second)
                maxY = outliers[j].second;
        }
    }

    double yMinValue = minY - 0.05 * (maxY - minY);
    double yMaxValue = maxY + 0.05 * (maxY - minY);

    QwtDoubleRect rect = QwtDoubleRect( minX, yMinValue,
                                    maxX - minX, yMaxValue - yMinValue );

    return rect;
}

void boxPlotItem::draw(QPainter *painter, const QwtScaleMap &xMap,
    const QwtScaleMap &yMap, const QRect &) const
{
    painter->setPen(QPen(plotColor));

    // Draw the whiskers
    for (int i = 0; i < (int)topWhiskers.size(); i++)
    {
        const int y1 = yMap.transform(bottomWhiskers[i]);
        const int y2 = yMap.transform(topWhiskers[i]);

        const int x1 = xMap.transform (i + 0.9);
        const int x2 = xMap.transform (i + 1.1);

        QRect r(x1, y1, x2-x1, y2-y1);

        painter->setPen(QPen(Qt::black, 2));
        QwtPainter::drawLine(painter,
           r.left(), r.top(), r.right(), r.top());
        QwtPainter::drawLine(painter,
           r.left(), r.bottom(), r.right(), r.bottom());

        double mid = (r.right() + r.left()) / 2;
        QwtPainter::drawLine(painter,
           mid, r.top(), mid, r.bottom());
    }

    // Draw the box
    for (int i = 0; i < (int)medians.size(); i++)
    {
        const int y1 = yMap.transform(Q1s[i]);
        const int y2 = yMap.transform(Q3s[i]);

        const int x1 = xMap.transform(i + 0.7);
        const int x2 = xMap.transform(i + 1.3);

        QRect rect(x1, y1, x2 - x1, y2 - y1);

        drawBox(painter, Qt::Vertical, rect);

        // Draw mean and lines around box
        double median = yMap.transform(medians[i]);
        painter->setPen(QPen(Qt::black, 2));
        QwtPainter::drawLine(painter,
           rect.left() + 2, median, rect.right() - 1, median);
    }

    // Draw the outliers
    painter->setPen(QPen(Qt::black, 1));

    for (int i = 0; i < outliers.size(); i++)
    {
        int x = xMap.transform(outliers[i].first);
        int y = yMap.transform(outliers[i].second);

        painter->drawEllipse(x - 3, y - 3, 6, 6);
    }

}

void boxPlotItem::drawBox(QPainter *painter,
   Qt::Orientation, const QRect& rect) const
{
   painter->save();

   const QRect r = rect.normalized();

   painter->setBrush(plotColor);
   painter->setPen(Qt::NoPen);
   QwtPainter::drawRect(painter, r.x() + 1, r.y() + 1,
      r.width() - 2, r.height() - 2);
   painter->setBrush(Qt::NoBrush);
}
