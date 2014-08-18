#include "textscaledraw.h"
#include <string>
#include <QString>

textScaleDraw::textScaleDraw() : QwtScaleDraw()
{
}

textScaleDraw::textScaleDraw(QVector<std::string> labels) : QwtScaleDraw()
{
    axisLabels = labels;
}

QwtText textScaleDraw::label(double Double) const
{
    int i = (int)Double;
    QString qqLabel = "";
    if (i < axisLabels.size() && i > -1) {
        std::string label = axisLabels[i];
        QString qLabel(QString::fromStdString(label));
        qqLabel = qLabel;
    }

    return QwtText(qqLabel);
}
