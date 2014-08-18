// piechart/piewidget.cpp

#include <QtGui>
#include "piewidget.h"

PieWidget::PieWidget(QWidget *parent)
    : QFrame(parent)
{}

PieWidget::~PieWidget()
{}

void PieWidget::addEntry(const QString& key, double val)
{
    values.insert(std::pair<QString, double>(key, val));
}

void PieWidget::paintEvent(QPaintEvent * /*ev*/)
{
    // calculate total
    std::map<QString, double>::iterator it;
    double total = 0.0;
    for(it = values.begin(); it != values.end(); ++it)
        total += it->second;

    // prepare painter
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    // prepare colors
    QStringList colorNames = QColor::colorNames();
    int colorPos = 10; // pastel colors

    int height = rect().height();
    QRect pieRect(0, 0, height, height);

    // dedicate right half to legend
    QRect legendRect = rect();
    legendRect.setLeft(pieRect.width());
    legendRect.adjust(10, 10, -10, -10);

    int lastAngleOffset = 0;
    int currentPos = 0;

    // create an entry for every piece of the pie
    for (it = values.begin(); it != values.end(); ++it) {
        double value = it->second;
        QString text = it->first;

        int angle = (int) (16*360*value/total);
        QColor col(colorNames.at(colorPos%colorNames.count()));
        colorPos++;

        // gradient for the pie pieces
        QRadialGradient rg(pieRect.center(), pieRect.width()/2, pieRect.topLeft());
        rg.setColorAt(0, Qt::white);
        rg.setColorAt(1, col);
        p.setBrush(rg);
        QPen pen = p.pen();
        p.setPen(Qt::NoPen);
        p.drawPie(pieRect, lastAngleOffset, angle);
        lastAngleOffset += angle;

        // calculate the squares for the legend
        int fh = fontMetrics().height();
        QRect legendEntryRect(0,(fh*2)*currentPos, fh, fh);
        currentPos++;
        legendEntryRect.translate(legendRect.topLeft());

        // define gradient for the legend squares
        QLinearGradient lg(legendEntryRect.topLeft(), legendEntryRect.bottomRight());
        lg.setColorAt(0, col);
        lg.setColorAt(1, Qt::white);
        p.setBrush(QBrush(lg));
        p.drawRect(legendEntryRect);

        //draw text behind squares
        QPoint textStart = legendEntryRect.topRight();
        textStart = textStart + QPoint(fontMetrics().width('x'), 0);
        QPoint textEnd(legendRect.right(), legendEntryRect.bottom());
        QRect textEntryRect(textStart, textEnd);
        p.setPen(pen);
        p.drawText(textEntryRect, Qt::AlignVCenter, text);
    }
}

QSize PieWidget::minimumSizeHint() const
{
    int fh = fontMetrics().height();
    int height = fh*2*values.size();
    int longest = 0;
    QFontMetrics fm = fontMetrics();
    std::map<QString, double>::const_iterator it = values.begin();
    for (it = values.begin(); it != values.end(); ++it)
        longest = qMax(fm.width(it->first), longest);
    int width = height+longest+fontMetrics().width('x')+fh+(2*10);
    QSize minSize(width, height);
    return minSize;
}

QSize PieWidget::sizeHint() const
{
    return minimumSizeHint();
}

void PieWidget::clearEntries()
{
    values.clear();
}









