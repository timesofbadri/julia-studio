/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
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
****************************************************************************/

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include "coreplugin/core_global.h"
#include "coreplugin/id.h"
#include <coreplugin/actionmanager/command.h>
#include "coreplugin/icontext.h"

#include <QObject>
#include <QList>

QT_BEGIN_NAMESPACE
class QAction;
class QShortcut;
class QString;
QT_END_NAMESPACE

namespace Core {

class ActionContainer;

namespace Internal {
class ActionManagerPrivate;
class MainWindow;
}

class CORE_EXPORT ActionManager : public QObject
{
    Q_OBJECT
public:
    static ActionManager *instance();

    static ActionContainer *createMenu(const Id &id);
    static ActionContainer *createMenuBar(const Id &id);

    static Command *registerAction(QAction *action, const Id &id, const Context &context, bool scriptable = false);
    static Command *registerShortcut(QShortcut *shortcut, const Id &id, const Context &context, bool scriptable = false);

    static Command *command(const Id &id);
    static ActionContainer *actionContainer(const Id &id);

    static QList<Command *> commands();

    static void unregisterAction(QAction *action, const Id &id);
    static void unregisterShortcut(const Id &id);

    static void setPresentationModeEnabled(bool enabled);
    static bool isPresentationModeEnabled();

    static bool hasContext(int context);

signals:
    void commandListChanged();
    void commandAdded(const QString &id);

private:
    ActionManager(QObject *parent = 0);
    ~ActionManager();
    Internal::ActionManagerPrivate *d;

    friend class Core::Internal::MainWindow;
};

} // namespace Core

#endif // ACTIONMANAGER_H
