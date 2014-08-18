/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Graphical User Interface
 * Author:   Kyle Shannon <kyle@pobox.com>, see original banner below.
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#include "simpleareamaptool.h"

SimpleAreaMapTool::SimpleAreaMapTool(QgsMapCanvas *mapCanvas)
    : QgsMapTool(mapCanvas)
{
    rubberBand = new QgsRubberBand(mapCanvas, true);
    rubberBand->setColor(Qt::red);
    //rubberBand->setBrushStyle(Qt::BDiagPattern);
    rubberBand->setBrushStyle(Qt::NoBrush);
    rubberBand->setWidth(2.0);
    geometry = NULL;
}

void SimpleAreaMapTool::canvasPressEvent(QMouseEvent *e)
{
    if(e->button() != Qt::LeftButton)
    {
        return;
    }
    origin = QPoint(e->x(), e->y());
}
void SimpleAreaMapTool::canvasMoveEvent(QMouseEvent *e)
{
    //if(e->button() != Qt::LeftButton)
    if ( !( e->buttons() & Qt::LeftButton ) )
        return;
    end = QPoint(e->x(), e->y());
    rubberBand->setToCanvasRectangle(QRect(origin, end));
    rubberBand->show();
}

void SimpleAreaMapTool::canvasReleaseEvent(QMouseEvent *e)
{
    end = QPoint(e->x(), e->y());
    rubberBand->setToCanvasRectangle(QRect(origin, end));
    rubberBand->show();
    geometry = rubberBand->asGeometry();
    emit AreaSelected(geometry);
}

void SimpleAreaMapTool::deactivate()
{
    rubberBand->reset();
}

QString SimpleAreaMapTool::GetWkt()
{
    if(geometry)
    {
        return geometry->exportToWkt();
    }
    return "";
}

