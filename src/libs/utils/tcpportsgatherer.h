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

#ifndef TCPPORTSGATHERER_H
#define TCPPORTSGATHERER_H

#include "portlist.h"

namespace Utils {
namespace Internal {
class TcpPortsGathererPrivate;
}

class QTCREATOR_UTILS_EXPORT TcpPortsGatherer
{
public:
    enum NetworkLayerProtocol {
        IPv4Protocol = 0x1,
        IPv6Protocol = 0x2,
        AnyIPProcol = IPv4Protocol | IPv6Protocol
    };
    Q_DECLARE_FLAGS(ProtocolFlags, NetworkLayerProtocol)

    TcpPortsGatherer(ProtocolFlags flags);
    ~TcpPortsGatherer();

    void update();

    PortList usedPorts() const;
    quint16 getNextFreePort(PortList *port);

private:
    Internal::TcpPortsGathererPrivate *d;
};

} // namespace Utils

#endif // TCPPORTSGATHERER_H
