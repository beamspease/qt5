/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Components project.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qtexclusivegroup_p.h"

#include <QtCore/QVariant>
#include "qdebug.h"

#define CHECKED_PROPERTY "checked"

static const char *checkableSignals[] = {
    CHECKED_PROPERTY"Changed()",
    "toggled(bool)",
    "toggled()",
    0
};

QtExclusiveGroup::QtExclusiveGroup(QObject *parent)
    : QObject(parent)
{
    int index = metaObject()->indexOfMethod("updateCurrent()");
    m_updateCurrentMethod = metaObject()->method(index);
}

void QtExclusiveGroup::setCurrent(QObject * o)
{
    if (m_current == o)
        return;

    if (m_current)
        m_current->setProperty(CHECKED_PROPERTY, QVariant(false));
    m_current = o;
    if (m_current)
        m_current->setProperty(CHECKED_PROPERTY, QVariant(true));
    emit currentChanged();
}

void QtExclusiveGroup::updateCurrent()
{
    QObject *checkable = sender();
    QVariant checkedVariant = checkable->property(CHECKED_PROPERTY);
    if (checkedVariant.isValid() && checkedVariant.toBool())
        setCurrent(checkable);
}

void QtExclusiveGroup::registerCheckable(QObject *o)
{
    for (const char **signalName = checkableSignals; *signalName; signalName++) {
        int signalIndex = o->metaObject()->indexOfSignal(*signalName);
        if (signalIndex != -1) {
            QMetaMethod signalMethod = o->metaObject()->method(signalIndex);
            connect(o, signalMethod, this, m_updateCurrentMethod, Qt::UniqueConnection);
            connect(o, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterCheckable(QObject*)), Qt::UniqueConnection);
            return;
        }
    }

    qWarning() << "QtExclusiveGroup::registerCheckable(): Cannot register" << o;
}

void QtExclusiveGroup::unregisterCheckable(QObject *o)
{
    for (const char **signalName = checkableSignals; *signalName; signalName++) {
        int signalIndex = o->metaObject()->indexOfSignal(*signalName);
        if (signalIndex != -1) {
            QMetaMethod signalMethod = o->metaObject()->method(signalIndex);
            if (disconnect(o, signalMethod, this, m_updateCurrentMethod)) {
                disconnect(o, SIGNAL(destroyed(QObject*)), this, SLOT(unregisterCheckable(QObject*)));
                break;
            }
        }
    }
}
