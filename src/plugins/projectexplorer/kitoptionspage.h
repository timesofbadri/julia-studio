/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: http://www.qt-project.org/
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**************************************************************************/

#ifndef KITOPTIONSPAGE_H
#define KITOPTIONSPAGE_H

#include "projectexplorer_export.h"

#include <coreplugin/dialogs/ioptionspage.h>

#include <QModelIndex>

QT_BEGIN_NAMESPACE
class QItemSelectionModel;
class QTreeView;
class QPushButton;
QT_END_NAMESPACE

namespace ProjectExplorer {

namespace Internal { class KitModel; }

class Kit;
class KitConfigWidget;
class KitFactory;
class KitManager;

// --------------------------------------------------------------------------
// KitOptionsPage:
// --------------------------------------------------------------------------

class PROJECTEXPLORER_EXPORT KitOptionsPage : public Core::IOptionsPage
{
    Q_OBJECT

public:
    KitOptionsPage();

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();
    bool matches(const QString &) const;

    void showKit(Kit *k);

private slots:
    void kitSelectionChanged();
    void addNewKit();
    void cloneKit();
    void removeKit();
    void makeDefaultKit();
    void updateState();

private:
    QModelIndex currentIndex() const;

    QTreeView *m_kitsView;
    QPushButton *m_addButton;
    QPushButton *m_cloneButton;
    QPushButton *m_delButton;
    QPushButton *m_makeDefaultButton;

    QWidget *m_configWidget;
    QString m_searchKeywords;

    Internal::KitModel *m_model;
    QItemSelectionModel *m_selectionModel;
    QWidget *m_currentWidget;

    Kit *m_toShow;
};

} // namespace ProjectExplorer

#endif // KITOPTIONSPAGE_H