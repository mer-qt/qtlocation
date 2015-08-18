/****************************************************************************
**
** Copyright (C) 2015 Jolla Ltd.
** Contact: Aaron McCarthy <aaron.mccarthy@jollamobile.com>
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

#ifndef QDECLARATIVEGEOMAPITEMVIEW_H
#define QDECLARATIVEGEOMAPITEMVIEW_H

#include <QtCore/QModelIndex>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QQmlIncubator>
#include <QtQml/qqml.h>

QT_BEGIN_NAMESPACE

class QAbstractItemModel;
class QQmlComponent;
class QQuickItem;
class QDeclarativeGeoMap;
class QDeclarativeGeoMapItemBase;
class QQmlOpenMetaObject;
class QQmlOpenMetaObjectType;
class MapItemViewDelegateIncubator;

class QDeclarativeGeoMapItemView : public QObject, public QQmlParserStatus
{
    Q_OBJECT

    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QVariant model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
    Q_PROPERTY(bool autoFitViewport READ autoFitViewport WRITE setAutoFitViewport NOTIFY autoFitViewportChanged)

public:
    explicit QDeclarativeGeoMapItemView(QQuickItem *parent = 0);
    ~QDeclarativeGeoMapItemView();

    QVariant model() const;
    void setModel(const QVariant &);

    QQmlComponent *delegate() const;
    void setDelegate(QQmlComponent *);

    bool autoFitViewport() const;
    void setAutoFitViewport(const bool &);

    void setMapData(QDeclarativeGeoMap *);
    void repopulate();
    void removeInstantiatedItems();
    void instantiateAllItems();

    qreal zValue();
    void setZValue(qreal zValue);

    // From QQmlParserStatus
    virtual void componentComplete();
    void classBegin() {}

Q_SIGNALS:
    void modelChanged();
    void delegateChanged();
    void autoFitViewportChanged();

protected:
    void incubatorStatusChanged(MapItemViewDelegateIncubator *incubator,
                                QQmlIncubator::Status status);

private Q_SLOTS:
    void itemModelReset();
    void itemModelRowsInserted(const QModelIndex &index, int start, int end);
    void itemModelRowsRemoved(const QModelIndex &index, int start, int end);
    void itemModelRowsMoved(const QModelIndex &parent, int start, int end,
                            const QModelIndex &destination, int row);
    void itemModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                              const QVector<int> &roles);

private:
    struct ItemData {
        ItemData()
        :   incubator(0), item(0), context(0), modelData(0), modelDataMeta(0)
        {
        }

        ~ItemData();

        MapItemViewDelegateIncubator *incubator;
        QDeclarativeGeoMapItemBase *item;
        QQmlContext *context;
        QObject *modelData;
        QQmlOpenMetaObject *modelDataMeta;
    };

    void createItemForIndex(const QModelIndex &index);
    void fitViewport();

    bool componentCompleted_;
    QQmlComponent *delegate_;
    QAbstractItemModel *itemModel_;
    QDeclarativeGeoMap *map_;
    QVector<ItemData *> m_itemData;
    bool fitViewport_;

    QQmlOpenMetaObjectType *m_metaObjectType;

    friend class QTypeInfo<ItemData>;
    friend class MapItemViewDelegateIncubator;
};

QT_END_NAMESPACE

Q_DECLARE_TYPEINFO(QDeclarativeGeoMapItemView::ItemData, Q_MOVABLE_TYPE);

QML_DECLARE_TYPE(QDeclarativeGeoMapItemView)

#endif
