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

#include "qgeomap_p.h"
#include "qgeomapdata_p.h"

#include "qgeotilecache_p.h"
#include "qgeotilespec_p.h"

#include "qgeocameracapabilities_p.h"
#include "qgeomapcontroller_p.h"

#include <QtPositioning/QGeoRectangle>
#include <QtPositioning/private/qgeoprojection_p.h>
#include <QtPositioning/private/qdoublevector3d_p.h>

#include "qgeocameratiles_p.h"
#include "qgeotilerequestmanager_p.h"
#include "qgeomapscene_p.h"

#include "qgeomappingmanager_p.h"

#include <QMutex>
#include <QMap>

#include <qnumeric.h>

#include <QtQuick/QSGNode>

#include <cmath>

QT_BEGIN_NAMESPACE

QGeoMap::QGeoMap(QGeoMapData *mapData, QObject *parent)
    : QObject(parent),
      mapData_(mapData)
{
    connect(mapData_, SIGNAL(cameraDataChanged(const QGeoCameraData&)), this, SIGNAL(cameraDataChanged(const QGeoCameraData&)));
    connect(mapData_, SIGNAL(updateRequired()), this, SIGNAL(updateRequired()));
    connect(mapData_, SIGNAL(activeMapTypeChanged()), this, SIGNAL(activeMapTypeChanged()));
    connect(mapData_, SIGNAL(copyrightsChanged(const QImage&, const QPoint&)), this, SIGNAL(copyrightsChanged(const QImage&, const QPoint&)));
    connect(mapData_, SIGNAL(minimumZoomChanged()), this, SIGNAL(minimumZoomChanged()));
}

QGeoMap::~QGeoMap()
{
    delete mapData_;
}

QGeoMapController *QGeoMap::mapController()
{
    return mapData_->mapController();
}

QSGNode *QGeoMap::updateSceneGraph(QSGNode *oldNode, QQuickWindow *window)
{
    return mapData_->updateSceneGraph(oldNode, window);
}

void QGeoMap::resize(int width, int height)
{
    mapData_->resize(width, height);
}

int QGeoMap::width() const
{
    return mapData_->width();
}

int QGeoMap::height() const
{
    return mapData_->height();
}

QGeoCameraCapabilities QGeoMap::cameraCapabilities() const
{
    return mapData_->cameraCapabilities();
}

QGeoRectangle QGeoMap::visibleRegion() const
{
    QGeoCoordinate tl = screenPositionToCoordinate(QDoubleVector2D(0, 0));
    QGeoCoordinate br = screenPositionToCoordinate(QDoubleVector2D(width(), height()));

    return QGeoRectangle(tl, br);
}

void QGeoMap::setCameraData(const QGeoCameraData &cameraData)
{
    mapData_->setCameraData(cameraData);
}

void QGeoMap::cameraStopped()
{
    mapData_->prefetchData();
}

QGeoCameraData QGeoMap::cameraData() const
{
    return mapData_->cameraData();
}

QGeoCoordinate QGeoMap::screenPositionToCoordinate(const QDoubleVector2D &pos, bool clipToViewport) const
{
    return mapData_->screenPositionToCoordinate(pos, clipToViewport);
}

QDoubleVector2D QGeoMap::coordinateToScreenPosition(const QGeoCoordinate &coordinate, bool clipToViewport) const
{
    return mapData_->coordinateToScreenPosition(coordinate, clipToViewport);
}

/*
    Returns the static projection of coordinate. The transformation from coordinate must not depend
    on the current map viewport (center, zoom, etc).
*/
QDoubleVector2D QGeoMap::coordinateToStaticProjection(const QGeoCoordinate &coordinate) const
{
    return mapData_->coordinateToStaticProjection(coordinate);
}

/*
    Returns the screen position of position. This function together with
    coordinateToStaticProject() is equivalent to coordinateToScreenPosition().
*/
QDoubleVector2D QGeoMap::staticProjectionToScreenPosition(const QDoubleVector2D &position) const
{
    return mapData_->staticProjectionToScreenPosition(position);
}

void QGeoMap::update()
{
    emit mapData_->update();
}

void QGeoMap::setActiveMapType(const QGeoMapType type)
{
    mapData_->setActiveMapType(type);
}

const QGeoMapType QGeoMap::activeMapType() const
{
    return mapData_->activeMapType();
}

QString QGeoMap::pluginString()
{
    return mapData_->pluginString();
}

qreal QGeoMap::minimumZoom() const
{
    return mapData_->minimumZoom();
}

QT_END_NAMESPACE
