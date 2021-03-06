/*
                             This file is part of the Opie Project
                             Copyright (C) Stefan Eilers (Eilers.Stefan@epost.de)
              =.             Copyright (C) The Opie Team <opie-devel@lists.sourceforge.net>
            .=l.
           .>+-=
 _;:,     .>    :=|.         This program is free software; you can
.> <`_,   >  .   <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.--   :           the terms of the GNU Library General Public
.="- .-=="i,     .._         License as published by the Free Software
 - .   .-<_>     .<>         Foundation; either version 2 of the License,
     ._= =}       :          or (at your option) any later version.
    .%`+i>       _;_.
    .i_,=:_.      -<s.       This program is distributed in the hope that
     +  .  -:.       =       it will be useful,  but WITHOUT ANY WARRANTY;
    : ..    .:,     . . .    without even the implied warranty of
    =_        +     =;=|`    MERCHANTABILITY or FITNESS FOR A
  _.=:.       :    :=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.=       =       ;      Library General Public License for more
++=   -.     .`     .:       details.
 :     =  ...= . :.=-
 -.   .:....=;==+<;          You should have received a copy of the GNU
  -_. . .   )=.  =           Library General Public License along with
    --        :-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.
*/

#include "opimmainwindow.h"

/* OPIE */
#include <opie2/odebug.h>
#include <opie2/opimresolver.h>
#include <opie2/oresource.h>

#include <qpe/categoryselect.h>
#include <qpe/ir.h>
#include <qpe/qcopenvelope_qws.h>
#include <qpe/qpeapplication.h>
#include <qpe/sound.h>

/* QT */
#include <qaction.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qcopchannel_qws.h>
#include <qdatetime.h>
#include <qmenubar.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>

namespace Opie {
OPimMainWindow::OPimMainWindow( const QString &serviceName, const QString &catName,
                                const QString &itemName, const QString &configName,
                                QWidget *parent, const char* name, WFlags f )
    : QMainWindow( parent, name, f ), m_rtti(-1), m_service( serviceName ), m_fallBack( 0l ),
      m_catGroupName( catName ), m_config( configName ), m_itemContextMenu( 0l )
{

    /*
     * let's generate our QCopChannel
     */
    m_str = QString("QPE/"+m_service).local8Bit();
    m_channel= new QCopChannel(m_str, this );
    connect(m_channel, SIGNAL(received(const QCString&,const QByteArray&)),
            this, SLOT(appMessage(const QCString&,const QByteArray&)) );
    connect(qApp, SIGNAL(appMessage(const QCString&,const QByteArray&)),
            this, SLOT(appMessage(const QCString&,const QByteArray&)) );

    /* connect flush and reload */
    connect(qApp, SIGNAL(flush() ),
            this, SLOT(flush() ) );
    connect(qApp, SIGNAL(reload() ),
            this, SLOT(reload() ) );

    m_viewMenu = NULL;
    m_itemMenu = NULL;
    m_catSelect = NULL;

    m_itemName = itemName;
    initActions();
}

OPimMainWindow::~OPimMainWindow() {
    delete m_channel;
}

QCopChannel* OPimMainWindow::channel() {
    return m_channel;
}

void OPimMainWindow::doSetDocument( const QString&  ) {

}

void OPimMainWindow::appMessage( const QCString& cmd, const QByteArray& array ) {
    bool needShow = false;
    /*
     * create demands to create
     * a new record...
     */
    QDataStream stream(array, IO_ReadOnly);
    if ( cmd == "create()" ) {
        raise();
        int uid = create();
        QCopEnvelope e(m_str, "created(int)" );
        e << uid;
        needShow = true;
    }else if ( cmd == "remove(int)" ) {
        int uid;
        stream >> uid;
        bool rem = remove( uid );
        QCopEnvelope e(m_str, "removed(bool)" );
        e << rem;
        needShow = true;
    }else if ( cmd == "beam(int)" ) {
        int uid;
        stream >> uid;
        beam( uid);
    }else if ( cmd == "show(int)" ) {
        raise();
        int uid;
        stream >> uid;
        show( uid );
        needShow = true;
    }else if ( cmd == "edit(int)" ) {
        raise();
        int uid;
        stream >> uid;
        edit( uid );
    }else if ( cmd == "add(int,QByteArray)" ) {
        int rtti;
        QByteArray byteArray;
        stream >> rtti;
        stream >> byteArray;
        m_fallBack = record(rtti, byteArray );
        if (!m_fallBack)
            return;

        add( *m_fallBack );
        delete m_fallBack;
    }else if ( cmd == "alarm(QDateTime,int)" ) {
        raise();
        QDateTime dt; int uid;
        stream >> dt;
        stream >> uid;
        QDateTime current = QDateTime::currentDateTime();
        if ( current.time().hour() != dt.time().hour() && current.time().minute() != dt.time().minute() )
            return;

        needShow = doAlarm( dt, uid );
    }

    if (needShow )
        QPEApplication::setKeepRunning();
}

/* implement the url scripting here */
void OPimMainWindow::setDocument( const QString& str) {
    doSetDocument( str );
}

/*
 * we now try to get the array demarshalled
 * check if the rtti matches this one
 */
OPimRecord* OPimMainWindow::record( int rtti,  const QByteArray& array ) {
    if ( service() != rtti )
        return 0l;

    OPimRecord* record = OPimResolver::self()->record( rtti );
    QDataStream str(array, IO_ReadOnly );
    if ( !record || !record->loadFromStream(str) ) {
        delete record;
        record = 0l;
    }

    return record;
}

/*
 * get the rtti for the service
 */
int OPimMainWindow::service() {
    if ( m_rtti == -1 )
        m_rtti  =  OPimResolver::self()->serviceId( m_service );

    return m_rtti;
}

bool OPimMainWindow::doAlarm( const QDateTime&, int ) {
    // Implement in descendant classes
    return false;
}

void OPimMainWindow::startAlarm(int count  ) {
    QCopEnvelope e( "QPE/TaskBar", "setUseAlarmVolume(bool)" );
    e << true;
    
    m_alarmCount = count;
    m_playedCount = 0;
    Sound::soundAlarm();
    m_timerId = startTimer( 5000 );
}

void OPimMainWindow::killAlarm() {
    killTimer( m_timerId );
    
    QCopEnvelope e( "QPE/TaskBar", "setUseAlarmVolume(bool)" );
    e << false;
}

void OPimMainWindow::timerEvent( QTimerEvent* e) {
    if ( m_playedCount <m_alarmCount ) {
        m_playedCount++;
        Sound::soundAlarm();
    }
    else
        killAlarm();
}

QPopupMenu *OPimMainWindow::itemContextMenu() {
    if ( !m_itemContextMenu ) {
        // Create context menu if hasn't been done before
        m_itemContextMenu = new QPopupMenu( this );
        m_itemEditAction->addTo( m_itemContextMenu );
        m_itemDuplicateAction->addTo( m_itemContextMenu );
        if(m_itemBeamAction)
            m_itemBeamAction->addTo( m_itemContextMenu );
        m_itemDeleteAction->addTo( m_itemContextMenu );
    }

    return m_itemContextMenu;
}

void OPimMainWindow::insertItemMenuItems( QActionGroup *items ) {
    // Insert menu items into Item menu
    if ( items && m_itemMenu ) {
        // Rebuild Item menu
        m_itemMenu->clear();
        m_itemMenuGroup1->addTo( m_itemMenu );
        m_itemMenu->insertSeparator();
        items->addTo( m_itemMenu );
        m_itemMenu->insertSeparator();
        m_itemMenuGroup2->addTo( m_itemMenu );
    }
}

void OPimMainWindow::insertViewMenuItems( QActionGroup *items ) {
    // Insert menu items into View menu
    if ( items && m_viewMenu ) {
        // Rebuild Item menu
        m_viewMenu->clear();
//        m_viewMenuGroup->addTo( m_viewMenu );
//        m_viewMenu->insertSeparator();
        m_viewMenuAppGroup = items;
        m_viewMenuAppGroup->addTo( m_viewMenu );
    }
}

void OPimMainWindow::setViewCategory( const QString &category ) {
    if(m_catSelect) {
        // Find category in list
        for ( int i = 0; i < m_catSelect->count(); i++ ) {
            if ( m_catSelect->text( i ) == category ) {
                m_catSelect->setCurrentItem( i );
                emit categorySelected( category );
                return;
            }
        }
    }
}

void OPimMainWindow::reloadCategories() {
    if(m_catSelect) {
        QString selected = m_catSelect->currentText();

        // Remove old categories from list
        m_catSelect->clear();

        // Add categories to list
        Categories cats;
        cats.load( categoryFileName() );
        m_catSelect->insertItem( tr( "All" ) );
        m_catSelect->insertStringList( cats.labels( m_catGroupName ) );
        m_catSelect->insertItem( tr( "Unfiled" ) );
    }
}

void OPimMainWindow::setItemNewEnabled( bool enable ) {
    m_itemNewAction->setEnabled( enable );
}

void OPimMainWindow::setItemEditEnabled( bool enable ) {
    m_itemEditAction->setEnabled( enable );
}

void OPimMainWindow::setItemDuplicateEnabled( bool enable ) {
    m_itemDuplicateAction->setEnabled( enable );
}

void OPimMainWindow::setItemDeleteEnabled( bool enable ) {
    m_itemDeleteAction->setEnabled( enable );
}

void OPimMainWindow::setItemBeamEnabled( bool enable ) {
    if(m_itemBeamAction)
        m_itemBeamAction->setEnabled( enable );
}

void OPimMainWindow::setConfigureEnabled( bool enable ) {
    m_configureAction->setEnabled( enable );
}

void OPimMainWindow::setShowCategories( bool show ) {
    if(m_catSelect) {
        show ? m_catSelect->show()
            : m_catSelect->hide();
    }
}

void OPimMainWindow::initActions() {
    // Item menu
    m_itemMenuGroup1 = new QActionGroup( this, QString::null, false );

    m_itemNewAction = new QAction( tr( "New" ),
                                  Opie::Core::OResource::loadPixmap( "new", Opie::Core::OResource::SmallIcon ),
                                  QString::null, 0, m_itemMenuGroup1, 0 );
    connect( m_itemNewAction, SIGNAL(activated()), this, SLOT(slotItemNew()) );
    m_itemNewAction->setWhatsThis( tr( "Click here to create a new item." ) );

    m_itemEditAction = new QAction( tr( "Edit" ),
                                    Opie::Core::OResource::loadPixmap( "edit", Opie::Core::OResource::SmallIcon ),
                                    QString::null, 0, m_itemMenuGroup1, 0 );
    connect( m_itemEditAction, SIGNAL(activated()), this, SLOT(slotItemEdit()) );
    m_itemEditAction->setWhatsThis( tr( "Click here to edit the selected item." ) );

    m_itemDuplicateAction = new QAction( tr( "Duplicate" ),
                                         Opie::Core::OResource::loadPixmap( "copy", Opie::Core::OResource::SmallIcon ),
                                         QString::null, 0, m_itemMenuGroup1, 0 );
    connect( m_itemDuplicateAction, SIGNAL(activated()), this, SLOT(slotItemDuplicate()) );
    m_itemDuplicateAction->setWhatsThis( tr( "Click here to duplicate the selected item." ) );

    if ( Ir::supported() ) {
        m_itemBeamAction = new QAction( tr( "Beam" ),
                                        Opie::Core::OResource::loadPixmap( "beam", Opie::Core::OResource::SmallIcon ),
                                        QString::null, 0, m_itemMenuGroup1, 0 );
        connect( m_itemBeamAction, SIGNAL(activated()), this, SLOT(slotItemBeam()) );
        m_itemBeamAction->setWhatsThis( tr( "Click here to transmit the selected item." ) );
    }
    else
        m_itemBeamAction = NULL;

    m_itemDeleteAction = new QAction( tr( "Delete" ),
                                      Opie::Core::OResource::loadPixmap( "trash", Opie::Core::OResource::SmallIcon ),
                                      QString::null, 0, m_itemMenuGroup1, 0 );
    connect( m_itemDeleteAction, SIGNAL(activated()), this, SLOT(slotItemDelete()) );
    m_itemDeleteAction->setWhatsThis( tr( "Click here to delete the selected item." ) );

    m_itemMenuGroup2 = new QActionGroup( this, QString::null, false );

    m_configureAction = new QAction( tr( "Configure" ),
                                     Opie::Core::OResource::loadPixmap( "SettingsIcon", Opie::Core::OResource::SmallIcon ),
                                     QString::null, 0, m_itemMenuGroup2, 0 );
    connect( m_configureAction, SIGNAL(activated()), this, SLOT(slotConfigure()) );
    m_configureAction->setWhatsThis( tr( "Click here to set your preferences for this application." ) );

    // View menu
    m_viewMenuAppGroup = 0l;
//    m_viewMenuGroup = new QActionGroup( this, QString::null, false );

//     QAction *a = new QAction( tr( "Filter" ), QString::null, 0, m_viewMenuGroup, 0 );
//     connect( a, SIGNAL(activated()), this, SLOT(slotViewFilter()) );
//     a->setWhatsThis( tr( "Click here to filter the items displayed." ) );
//
//     a = new QAction( tr( "Filter Settings" ), QString::null, 0, m_viewMenuGroup, 0 );
//     connect( a, SIGNAL(activated()), this, SLOT(slotViewFilterSettings()) );
//     a->setWhatsThis( tr( "Click here to modify the current filter settings." ) );


}

void OPimMainWindow::initBars() {
    QString itemStr = m_itemName.lower();

    setToolBarsMovable( false );

    // Create application menu bar
    QToolBar *mbHold = new QToolBar( this );
    mbHold->setHorizontalStretchable( true );
    QMenuBar *menubar = new QMenuBar( mbHold );
    menubar->setMargin( 0 );

    // Create application toolbar
    QToolBar *toolbar = new QToolBar( this );
    m_itemNewAction->addTo( toolbar );
    m_itemEditAction->addTo( toolbar );
    if( m_itemBeamAction )
        m_itemBeamAction->addTo( toolbar );
    m_itemDeleteAction->addTo( toolbar );

    // Create sub-menus
    m_itemMenu = new QPopupMenu( this );
    m_itemMenu->setCheckable( true );
    menubar->insertItem( m_itemName, m_itemMenu );
    m_viewMenu = new QPopupMenu( this );
    m_viewMenu->setCheckable( true );
    menubar->insertItem( tr( "View" ), m_viewMenu );

    m_itemMenuGroup1->addTo( m_itemMenu );

    m_itemMenu->insertSeparator();

    m_itemMenuGroup2->addTo( m_itemMenu );

    // Create view toolbar
    toolbar = new QToolBar( this );
    m_catSelect = new QComboBox( toolbar );
    connect( m_catSelect, SIGNAL(activated(const QString&)), this, SIGNAL(categorySelected(const QString&)) );
    QWhatsThis::add( m_catSelect, tr( "Click here to filter items by category." ) );

    // Do initial load of categories
    reloadCategories();
}

} // namespace Opie
