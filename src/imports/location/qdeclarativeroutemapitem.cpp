/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qdeclarativeroutemapitem_p.h"
#include "qdeclarativepolylinemapitem_p.h"
#include "qgeocameracapabilities_p.h"
#include "qdeclarativegeoroute_p.h"
#include <QtQml/QQmlInfo>
#include <QtGui/QPainter>
#include <QtPositioning/QGeoRectangle>

/*!
    \qmltype MapRoute
    \instantiates QDeclarativeRouteMapItem
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since Qt Location 5.0

    \brief The MapRoute type displays a Route on a Map.

    The MapRoute type displays a Route obtained through a RouteModel or
    other means, on the Map as a Polyline following the path of the Route.

    MapRoute is really a \l MapPolyline, but with the path specified using the
    \l route property instead of directly in \l {coordinate}{coordinates}.

    By default, the route is displayed as a 1-pixel thick black line. This can
    be changed using the \l line.width and \l line.color properties.

    \section2 Performance

    For notes about the performance on MapRoute, refer to the documentation for
    \l MapPolyline.

    \section2 Example Usage

    Here is how to draw a \l{Route}{route} on a \l{Map}{map}:

    \snippet declarative/maps.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/maps.qml MapRoute
*/


QDeclarativeRouteMapItem::QDeclarativeRouteMapItem(QQuickItem *parent):
    QDeclarativeGeoMapItemBase(parent),
    route_(0)
{
    setFlag(ItemHasContents, true);
    line_.setWidth(3.0);
    QObject::connect(&line_, SIGNAL(colorChanged(QColor)),
                     this, SLOT(updateAfterLinePropertiesChanged()));
    QObject::connect(&line_, SIGNAL(widthChanged(qreal)),
                     this, SLOT(updateAfterLinePropertiesChanged()));
}

QDeclarativeRouteMapItem::~QDeclarativeRouteMapItem()
{
}

/*!
    \internal
*/
void QDeclarativeRouteMapItem::updateAfterLinePropertiesChanged()
{
    // mark dirty just in case we're a width change
    geometry_.markSourceDirty();
    polish();
}

/*!
    \internal
*/
void QDeclarativeRouteMapItem::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    QDeclarativeGeoMapItemBase::setMap(quickMap,map);
    if (map) {
        geometry_.markSourceDirty();
        polish();
    }
}

/*!
    \qmlproperty Route MapRoute::route

    This property holds the route to be drawn which can be used
    to represent one geographical route.
*/
QDeclarativeGeoRoute *QDeclarativeRouteMapItem::route() const
{
    return route_;
}

void QDeclarativeRouteMapItem::setRoute(QDeclarativeGeoRoute *route)
{
    if (route_ == route)
        return;

    route_ = route;

    m_path.clear();
    m_staticProjectionPath.clear();

    if (route_) {
        m_path.reserve(m_path.length());
        m_staticProjectionPath.reserve(m_path.length());

        foreach (const QGeoCoordinate &coordinate, route_->routePath()) {
            // Check coordinate validitity here instead of in
            // QGeoMapPolylineGeometry::updateSourcePoints() called from updatePolish()
            if (!coordinate.isValid())
                continue;

            m_path.append(coordinate);
            m_staticProjectionPath.append(map()->coordinateToStaticProjection(coordinate));
        }
    }

    geometry_.markSourceDirty();
    polish();
    emit routeChanged(route_);

}

/*!
    \internal
*/
QSGNode *QDeclarativeRouteMapItem::updateMapItemPaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);

    MapPolylineNode *node = static_cast<MapPolylineNode *>(oldNode);
    if (!visibleOnMap()) {
        delete node;
        return 0;
    }

    if (!node)
        node = new MapPolylineNode;

    //TODO: update only material
    if (geometry_.isScreenDirty() || dirtyMaterial_ || !oldNode) {
        geometry_.setPreserveGeometry(false);
        node->update(line_.color(), &geometry_);
        geometry_.markClean();
        dirtyMaterial_ = false;
    }
    return node;
}

/*!
    \qmlpropertygroup Location::MapRoute::line
    \qmlproperty int MapRoute::line.width
    \qmlproperty color MapRoute::line.color

    This property is part of the line property group.
    The line property group holds the width and color used to draw the line.

    The width is in pixels and is independent of the zoom level of the map.
    The default values correspond to a black border with a width of 1 pixel.

    For no line, use a width of 0 or a transparent color.
*/

QDeclarativeMapLineProperties *QDeclarativeRouteMapItem::line()
{
    return &line_;
}

/*!
    \internal
*/
void QDeclarativeRouteMapItem::updatePolish()
{
    if (!map() || m_path.isEmpty() || m_staticProjectionPath.isEmpty())
        return;

    setVisibleOnMap(map()->visibleRegion().intersects(route_->bounds()));
    if (!visibleOnMap())
        return;

    geometry_.updateSourcePoints(*map(), m_path, m_staticProjectionPath);
    geometry_.updateScreenPoints(*map(), line_.width());

    setWidth(geometry_.sourceBoundingBox().width());
    setHeight(geometry_.sourceBoundingBox().height());

    setPositionOnMap(m_path.at(0), -1 * geometry_.sourceBoundingBox().topLeft());

    update();
}

/*!
    \internal
*/
void QDeclarativeRouteMapItem::afterViewportChanged(const QGeoMapViewportChangeEvent &event)
{
    // if the scene is tilted, we must regenerate our geometry every frame
    if (map()->cameraCapabilities().supportsTilting()
            && (event.cameraData.tilt() > 0.1
                || event.cameraData.tilt() < -0.1)) {
        geometry_.markSourceDirty();
    }

    // if the scene is rolled, we must regen too
    if (map()->cameraCapabilities().supportsRolling()
            && (event.cameraData.roll() > 0.1
                || event.cameraData.roll() < -0.1)) {
        geometry_.markSourceDirty();
    }

    // otherwise, only regen on rotate, resize and zoom
    if (event.bearingChanged || event.mapSizeChanged || event.zoomLevelChanged) {
        geometry_.markSourceDirty();
    }

    geometry_.setPreserveGeometry(true, geometry_.geoLeftBound());
    geometry_.markScreenDirty();
    polish();
}

/*!
    \internal
*/
bool QDeclarativeRouteMapItem::contains(const QPointF &point) const
{
    return geometry_.contains(point);
}
