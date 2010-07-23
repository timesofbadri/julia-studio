/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#ifndef DEBUGGER_WATCHHANDLER_H
#define DEBUGGER_WATCHHANDLER_H

#include "watchdata.h"

#include <QtCore/QPointer>
#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <QtCore/QAbstractItemModel>

namespace Debugger {
namespace Internal {

class DebuggerEngine;
class WatchItem;
class WatchHandler;
class WatchData;

enum WatchType
{
    ReturnWatch,
    LocalsWatch,
    WatchersWatch,
    TooltipsWatch
};

enum IntegerFormat
{
    DecimalFormat = 0, // Keep that at 0 as default.
    HexadecimalFormat,
    BinaryFormat,
    OctalFormat,
};

class WatchModel : public QAbstractItemModel
{
    Q_OBJECT

private:
    explicit WatchModel(WatchHandler *handler, WatchType type);
    virtual ~WatchModel();

public:
    int rowCount(const QModelIndex &idx) const;
    int columnCount(const QModelIndex &idx) const;

private:
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QModelIndex index(int, int, const QModelIndex &idx) const;
    QModelIndex parent(const QModelIndex &idx) const;
    bool hasChildren(const QModelIndex &idx) const;
    Qt::ItemFlags flags(const QModelIndex &idx) const;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    friend class WatchHandler;
    friend class GdbEngine;

    WatchItem *watchItem(const QModelIndex &) const;
    QModelIndex watchIndex(const WatchItem *needle) const;
    QModelIndex watchIndexHelper(const WatchItem *needle,
        const WatchItem *parentItem, const QModelIndex &parentIndex) const;

    void insertData(const WatchData &data);
    void insertBulkData(const QList<WatchData> &data);
    WatchItem *findItem(const QByteArray &iname, WatchItem *root) const;
    void reinitialize();
    void removeOutdated();
    void removeOutdatedHelper(WatchItem *item);
    WatchItem *rootItem() const;
    void destroyItem(WatchItem *item);

    void emitDataChanged(int column,
        const QModelIndex &parentIndex = QModelIndex());
    void beginCycle(); // Called at begin of updateLocals() cycle.
    void endCycle(); // Called after all results have been received.

    friend QDebug operator<<(QDebug d, const WatchModel &m);

    void dump();
    void dumpHelper(WatchItem *item);
    void emitAllChanged();

signals:
    void enableUpdates(bool);

private:
    QString niceType(const QString &typeIn) const;
    void formatRequests(QByteArray *out, const WatchItem *item) const;
    DebuggerEngine *engine() const;

    WatchHandler *m_handler;
    WatchType m_type;
    WatchItem *m_root;
    QSet<QByteArray> m_fetchTriggered;
};

class WatchHandler : public QObject
{
    Q_OBJECT

public:
    explicit WatchHandler(DebuggerEngine *engine);
    WatchModel *model(WatchType type) const;
    WatchModel *modelForIName(const QByteArray &iname) const;

    void cleanup();
    void watchExpression(const QString &exp);
    void removeWatchExpression(const QString &exp);
    Q_SLOT void emitAllChanged();

    void beginCycle(); // Called at begin of updateLocals() cycle
    void updateWatchers(); // Called after locals are fetched
    void endCycle(); // Called after all results have been received
    void showEditValue(const WatchData &data);

    void insertData(const WatchData &data);
    void insertBulkData(const QList<WatchData> &data);
    void removeData(const QByteArray &iname);
    WatchData *findItem(const QByteArray &iname) const;

    void loadSessionData();
    void saveSessionData();

    void initializeFromTemplate(WatchHandler *other);
    void storeToTemplate(WatchHandler *other);

    bool isExpandedIName(const QByteArray &iname) const
        { return m_expandedINames.contains(iname); }
    QSet<QByteArray> expandedINames() const
        { return m_expandedINames; }
    QStringList watchedExpressions() const;
    QHash<QByteArray, int> watcherNames() const
        { return m_watcherNames; }

    QByteArray expansionRequests() const;
    QByteArray typeFormatRequests() const;
    QByteArray individualFormatRequests() const;

    int format(const QByteArray &iname) const;

    void addTypeFormats(const QString &type, const QStringList &formats);

    QByteArray watcherName(const QByteArray &exp);

private:
    friend class WatchModel;

    void loadWatchers();
    void saveWatchers();

    void loadTypeFormats();
    void saveTypeFormats();
    void setFormat(const QString &type, int format);
    void updateWatchersWindow();

    bool m_inChange;

    // QWidgets and QProcesses taking care of special displays.
    typedef QMap<QString, QPointer<QObject> > EditHandlers;
    EditHandlers m_editHandlers;

    QHash<QByteArray, int> m_watcherNames;
    QHash<QString, int> m_typeFormats;
    QHash<QByteArray, int> m_individualFormats; // Indexed by iname.
    QHash<QString, QStringList> m_reportedTypeFormats;

    // Items expanded in the Locals & Watchers view.
    QSet<QByteArray> m_expandedINames;

    WatchModel *m_return;
    WatchModel *m_locals;
    WatchModel *m_watchers;
    WatchModel *m_tooltips;
    DebuggerEngine *m_engine;
};

} // namespace Internal
} // namespace Debugger

#endif // DEBUGGER_WATCHHANDLER_H
