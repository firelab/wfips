#ifndef TEXTSCALEDRAW_H
#define TEXTSCALEDRAW_H

#include <qwt_scale_draw.h>
#include <string>
#include <QVector>


class textScaleDraw : public QwtScaleDraw
{
public:
    textScaleDraw();
    textScaleDraw(QVector<std::string> labels);

    virtual ~textScaleDraw() {}
    virtual  QwtText label(double) const;

private:
    QVector<std::string> axisLabels;

};

#endif // TEXTSCALEDRAW_H
