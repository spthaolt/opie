/*
 * todayconfig.cpp
 *
 * copyright   : (c) 2002 by Maximilian Rei�
 * email       : harlekin@handhelds.org
 *
 */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "todayconfig.h"

#include <qpe/config.h>
#include <qpe/resource.h>
#include <qpe/qcopenvelope_qws.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qheader.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtoolbutton.h>

class ToolButton : public QToolButton {

public:
    ToolButton( QWidget *parent, const char *name, const QString& icon, QObject *handler, const QString& slot, bool t = FALSE )
        : QToolButton( parent, name ) {
        // setTextLabel( name );
        setPixmap( Resource::loadPixmap( icon ) );
        setAutoRaise( TRUE );
        setFocusPolicy( QWidget::NoFocus );
        setToggleButton( t );
        connect( this, t ? SIGNAL( toggled(bool) ) : SIGNAL( clicked() ), handler, slot );
    }
};


/**
 * The class has currently quite some duplicate code.
 * By that way it would be real easy to have it as seperate app in settings tab
 *
 */
TodayConfig::TodayConfig( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ) {

    setCaption( tr( "Today config" ) );

    QVBoxLayout *layout = new QVBoxLayout( this );
    TabWidget3 = new QTabWidget( this, "TabWidget3" );
    TabWidget3->setAutoMask( FALSE );
    TabWidget3->setTabShape( QTabWidget::Rounded );
    layout->addWidget( TabWidget3 );

    tab_2 = new QWidget( TabWidget3, "tab_2" );
    QVBoxLayout *tab2Layout = new QVBoxLayout( tab_2, 4 ,4 );
    QLabel *l = new QLabel( tr( "Load which plugins in what order:" ), tab_2 );
    tab2Layout->addWidget( l );
    QHBox *hbox1 = new QHBox( tab_2 );
    m_appletListView = new QListView( hbox1 );
    m_appletListView->addColumn( "PluginList" );
    m_appletListView->header()->hide();
    QVBox *vbox1 = new QVBox( hbox1 );
    new ToolButton( vbox1, tr( "Move Up" ), "opieplayer/up",  this , SLOT( moveSelectedUp() ) );
    new ToolButton( vbox1, tr( "Move Down" ),"opieplayer/down", this , SLOT( moveSelectedDown() ) );
    tab2Layout->addWidget( hbox1 );

    TabWidget3->insertTab( tab_2, tr( "active/order" ) );
    tab_3 = new QWidget( TabWidget3, "tab_3" );
    QVBoxLayout *tab3Layout = new QVBoxLayout( tab_3 );
    QHBox *hbox_clip = new QHBox( tab_3 );
    TextLabel1 = new QLabel( hbox_clip, "TextLabel1" );
    TextLabel1->setText( tr( "Clip after how\n"
                             "many letters" ) );
    SpinBox7 = new QSpinBox( hbox_clip, "SpinBox7" );
    SpinBox7->setMaxValue( 80 );
    QHBox *hbox_auto = new QHBox( tab_3 );
    TextLabel2 = new QLabel( hbox_auto, "AutoStart" );
    TextLabel2->setText( tr( "autostart on \nresume? (Opie only)" ) );
    CheckBoxAuto = new QCheckBox( hbox_auto, "CheckBoxAuto" );
    QHBox *hbox_inactive = new QHBox( tab_3 );
    TimeLabel = new QLabel( hbox_inactive  , "TimeLabel" );
    TimeLabel->setText( tr( "minutes inactive" ) );
    SpinBoxTime = new QSpinBox(  hbox_inactive, "TimeSpinner");
    tab3Layout->addWidget( hbox_clip );
    tab3Layout->addWidget( hbox_auto );
    tab3Layout->addWidget( hbox_inactive );
    TabWidget3->insertTab( tab_3, tr( "Misc" ) );

    m_applets_changed = false;

    connect ( m_appletListView , SIGNAL( clicked ( QListViewItem * ) ), this, SLOT( appletChanged ( ) ) );

    readConfig();
    showMaximized();
}


/**
 * Autostart, uses the new (opie only) autostart method in the launcher code.
 * If registered against that today ist started on each resume.
 */
void TodayConfig::setAutoStart() {
    Config cfg( "today" );
    cfg.setGroup( "Autostart" );
    int autostart = cfg.readNumEntry( "autostart", 1);
    if ( autostart ) {
        QCopEnvelope e( "QPE/System", "autoStart(QString,QString,QString)" );
        e << QString( "add" );
        e << QString( "today" );
        e << m_autoStartTimer;
    } else {
        QCopEnvelope e( "QPE/System", "autoStart(QString,QString)" );
        e << QString( "remove" );
        e << QString( "today" );
    }
}

/**
 * Read the config part
 */
void TodayConfig::readConfig() {
    Config cfg( "today" );
    cfg.setGroup( "Autostart" );
    m_autoStart = cfg.readNumEntry( "autostart", 1 );
    CheckBoxAuto->setChecked( m_autoStart );
    m_autoStartTimer = cfg.readEntry( "autostartdelay", "0" );
    SpinBoxTime->setValue( m_autoStartTimer.toInt() );

    cfg.setGroup( "Applets" );
    m_excludeApplets = cfg.readListEntry( "ExcludeApplets", ',' );
}

/**
 * Write the config part
 */
void TodayConfig::writeConfig() {
    Config cfg( "today" );
    cfg. setGroup ( "Applets" );
    if ( m_applets_changed ) {
        QStringList exclude;
        QStringList include;
        QMap <QString, QCheckListItem *>::Iterator it;
        for ( it = m_applets.begin(); it != m_applets. end (); ++it ) {
            if ( !(*it)-> isOn () ) {
                exclude << it.key();
            } else {
                include << it.key();
            }
        }
        cfg.writeEntry( "ExcludeApplets", exclude, ',' );
        cfg.writeEntry( "IncludeApplets", include, ',' );
    }

    cfg.setGroup( "Autostart" );
    m_autoStart = CheckBoxAuto->isChecked();
    cfg.writeEntry( "autostart",  m_autoStart  );
    m_autoStartTimer = SpinBoxTime->value();
    cfg.readEntry( "autostartdelay", m_autoStartTimer );
}


void TodayConfig::moveSelectedUp() {
    QListViewItem *item = m_appletListView->selectedItem();
    if ( item && item->itemAbove() )  {
        item->itemAbove()->moveItem( item );
    }
}


void TodayConfig::moveSelectedDown() {
    QListViewItem *item = m_appletListView->selectedItem();
    if ( item && item->itemBelow() ) {
        item->moveItem( item->itemBelow() );
    }
}


/**
 * Set up the
 */
void TodayConfig::pluginManagement( QString libName, QString name, QPixmap icon ) {

	QCheckListItem *item;
    	item = new QCheckListItem( m_appletListView, name, QCheckListItem::CheckBox );

        if ( !icon.isNull() ) {
	    item->setPixmap( 0, icon );
        }

        qDebug (" SUCHNAME: " + name );
        if ( m_excludeApplets.find( libName ) == m_excludeApplets.end() ) {
	    item->setOn( TRUE );
        }
	m_applets[libName] = item;
}


void TodayConfig::appletChanged() {
    m_applets_changed = true;
}


TodayConfig::~TodayConfig() {
}

