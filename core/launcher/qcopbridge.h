/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __qcopbridge_h__
#define __qcopbridge_h__

#include <qtopia/global.h>
#include <qserversocket.h>
#include <qsocket.h>
#include <qdir.h>
#include <qfile.h>
#include <qbuffer.h>

class QFileInfo;
class QCopBridgePI;
class QCopChannel;
class QTimer;

class QCopBridge : public QServerSocket
{
    Q_OBJECT

public:
    /* can not or anymore :) */
    enum Mode  { Qtopia1_7= 0x01, Sharp = 0x02, IntelliSync = 0x04 };
    QCopBridge( Q_UINT16 port, QObject *parent = 0, const char* name = 0 );
    virtual ~QCopBridge();

    void newConnection( int socket );
    void closeOpenConnections();
    void authorizeConnections();

public slots:
    void closed( QCopBridgePI *pi );
    void desktopMessage( const QCString &call, const QByteArray & );

signals:
    void connectionClosed( const QHostAddress & );

protected:
    void timerEvent( QTimerEvent * );
    void sendDesktopMessageOld( const QCString&, const QByteArray& );

private:
    QCopChannel *desktopChannel;
    QCopChannel *cardChannel;
    QList<QCopBridgePI> openConnections;
    bool sendSync;
    Mode m_mode;
};


class QCopBridgePI : public QSocket
{
    Q_OBJECT

    enum State { Connected, Wait_USER, Wait_PASS, Ready, Forbidden };

public:
    QCopBridgePI( int socket, QObject *parent = 0, const char* name = 0 );
    virtual ~QCopBridgePI();

    void sendDesktopMessage( const QString &msg );
    void sendDesktopMessage( const QCString &msg, const QByteArray& );
    void startSync() { sendSync = TRUE; }
    bool verifyAuthorised();

signals:
    void connectionClosed( QCopBridgePI *);

protected slots:
    void read();
    void send( const QString& msg );
    void process( const QString& command );
    void myConnectionClosed();

private:
    State state;
    Q_UINT16 peerport;
    QHostAddress peeraddress;
    bool sendSync;
    QTimer *timer;
};

#endif
