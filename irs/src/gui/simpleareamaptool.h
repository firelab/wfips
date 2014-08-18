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

#ifndef SIMPLE_AREA_MAP_TOOL_H_
#define SIMPLE_AREA_MAP_TOOL_H_

#include <QMouseEvent>
#include <QPoint>
#include <QRect>

#include <qgsmapcanvas.h>
#include <qgsmaptool.h>
#include <qgsrubberband.h>
#include <qgspoint.h>
#include <qgsrectangle.h>
#include <qgsmaptopixel.h>

class SimpleAreaMapTool : public QgsMapTool
{
    Q_OBJECT
public:
    SimpleAreaMapTool(QgsMapCanvas *mapCanvas);
    void canvasPressEvent(QMouseEvent *e);
    void canvasMoveEvent(QMouseEvent *e);
    void canvasReleaseEvent(QMouseEvent *e);
    void deactivate();
    QString GetWkt();
private:
    QPoint origin;
    QPoint end;
    QgsGeometry *geometry;
    QgsRubberBand *rubberBand;
signals:
    void AreaSelected(QgsGeometry*);
};

#endif /* SIMPLE_AREA_MAP_TOOL_H_ */

