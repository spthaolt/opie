/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qtopia Environment.
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
#include "battery.h"
#include "batterystatus.h"

/* OPIE */
#include <opie2/otaskbarapplet.h>

#include <qpe/qpeapplication.h>
#include <qpe/applnk.h>
#include <qpe/config.h>
#include <qpe/power.h>

/* QT */
#include <qpainter.h>
#include <qtimer.h>


using namespace Opie::Ui;
BatteryMeter::BatteryMeter( QWidget *parent )
: QWidget( parent ), charging(false), m_ignoreRelease(false) {
    ps = new PowerStatus;
    startTimer( 10000 );

    setFixedWidth( QMAX(AppLnk::smallIconSize()*3/4, 6) );
    setFixedHeight( AppLnk::smallIconSize() );

    chargeTimer = new QTimer( this );
    connect( chargeTimer, SIGNAL(timeout()), this, SLOT(chargeTimeout()) );
    timerEvent(0);
    QPEApplication::setStylusOperation( this, QPEApplication::RightOnHold );
    Config c( "qpe" );
    c.setGroup( "Battery" );
    style = c.readNumEntry( "Style", 0 ) ? true : false;
}

BatteryMeter::~BatteryMeter() {
    delete ps;
}

QSize BatteryMeter::sizeHint() const {
    return QSize(QMAX(AppLnk::smallIconSize()*3/4, 6), height() );
}

void BatteryMeter::mousePressEvent( QMouseEvent* e ) {
    if ( e->button() == RightButton ) {
        style = !style;
        Config c( "qpe" );
        c.setGroup( "Battery" );
        c.writeEntry( "Style", style ? 1 : 0 );
        repaint( true );
        m_ignoreRelease = true;
    }
    QWidget::mousePressEvent( e );
}

void BatteryMeter::mouseReleaseEvent( QMouseEvent *e ) {
    if ( e->button() == LeftButton ) {
        if( m_ignoreRelease ) {
            m_ignoreRelease = false;
        }
        else {
            if ( batteryView && batteryView->isVisible() ) {
                batteryView->hide();
            } else {
                if ( !batteryView ) {
                    batteryView = new BatteryStatus( ps, this, WStyle_StaysOnTop | WType_Popup );
                    batteryView->setFrameStyle( QFrame::PopupPanel | QFrame::Raised );
                }

                batteryView->UpdateBatteryStatus();
                QRect r(batteryView->pos(),batteryView->sizeHint());
                QPoint curPos = this->mapToGlobal ( rect().topLeft() );

                int lp = qApp->desktop()->width() - batteryView->sizeHint().width();
                r.moveTopLeft( QPoint(lp, curPos.y()-batteryView->sizeHint().height()-1) );
                batteryView->setGeometry(r);

                batteryView->raise();
                batteryView->show();
            }
        }
    }
}


void BatteryMeter::timerEvent( QTimerEvent * ) {
    PowerStatus prev = *ps;

    *ps = PowerStatusManager::readStatus();

    if ( prev != *ps ) {
        percent = ps->batteryPercentRemaining();
        if ( !charging && ps->batteryStatus() == PowerStatus::Charging ) {
            percent = 0;
            charging = true;
            chargeTimer->start( 500 );
        } else if ( charging && ps->batteryStatus() != PowerStatus::Charging ) {
            charging = false;
            chargeTimer->stop();
            if ( batteryView )
                batteryView->updatePercent( percent );
        }
        repaint( style );
        if ( batteryView )
            batteryView->repaint();
    }
}

void BatteryMeter::chargeTimeout() {
    percent += 20;
    if ( percent > 100 )
        percent = 0;

    repaint(FALSE);
    if ( batteryView )
        batteryView->updatePercent( percent );
}

void BatteryMeter::paintEvent( QPaintEvent* ) {

    if ( style == 1 ) {
        QPainter p(this);
        QFont f( "Fixed", AppLnk::smallIconSize()/2 );
        QFontMetrics fm( f );
        //Avoid text overlapping
        p.eraseRect( 0, 0, 20, 20 );
        p.setFont( f );
        p.drawText( 0, height()/2, QString::number( percent ) );
        p.drawText( width()/4, height(), "%" );
        return;
    }

    QPainter p(this);
    QColor color;
    QColor g = gray.light( 160 );
    switch ( ps->acStatus() ) {
    case PowerStatus::Offline:
        color = blue.light( 150 );
        break;
    case PowerStatus::Online:
        color = green.dark( 130 ).light( 180 );
        break;
    default:
        color = red.light( 160 );
    }

    int w = height() / 2;
    if ( !(w%2) )
        w--; // should have an odd value to get a real middle line
    int h = height() - 4;
    int pix = (percent * h) / 100;
    int y2 = height() -2;
    int y = y2 - pix;
    int x1 = (width() - w ) / 2;

    p.setPen(QColor(80,80,80));
    p.drawLine(x1+w/4,0,x1+w/4+w/2+1,0);  // header
    p.drawRect(x1,1,w,height()-1);      // corpus
    p.setBrush(color);

    //int extra = ((percent * h) % 100)/(100/4);

    int middle = w/2;
    for ( int i = 0; i < middle; i++ ) {
        p.setPen( gray.dark( 100+i*20 ) );
        p.drawLine( x1+middle-i, 2, x1+middle-i, y-1 );
        p.drawLine( x1+middle+i, 2, x1+middle+i, y-1 );
        p.setPen( color.dark( 100+i*20 ) );
        p.drawLine( x1+middle-i, y, x1+middle-i, y2 );
        p.drawLine( x1+middle+i, y, x1+middle+i, y2 );
    }
}

int BatteryMeter::position() {
    return 8;
}

EXPORT_OPIE_APPLET_v1( BatteryMeter )

