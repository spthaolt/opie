/*
 *            kPPP: A pppd front end for the KDE project
 *
 * $Id: general.cpp,v 1.11 2004-10-14 01:44:27 zecke Exp $
 *
 *            Copyright (C) 1997 Bernd Johannes Wuebben
 *                   wuebben@math.cornell.edu
 *
 * based on EzPPP:
 * Copyright (C) 1997  Jay Painter
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "general.h"
#include "interfaceppp.h"
#include "modeminfo.h"
#include "modemcmds.h"
#include "pppdata.h"

/* OPIE */
#include <opie2/odebug.h>
#include <qpe/config.h>
#include <qpe/qpeapplication.h>
using namespace Opie::Core;

/* QT */
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

/* STD */
#include <termios.h>
#include <string.h>


ModemWidget::ModemWidget( PPPData *pd, QWidget *parent, const char *name )
        : QWidget(parent, name), _pppdata(pd)
{
    int k;

    QGridLayout *tl = new QGridLayout(this, 8, 2, 0 );//, KDialog::spacingHint());

    QLabel *label1;

    label1 = new QLabel(tr("Modem &name:"), this);
    tl->addWidget(label1, 0, 0);

    modemname = new QLineEdit(this, "modemName");
    modemname->setText( _pppdata->devname() );
    label1->setBuddy(modemname);
    tl->addWidget(modemname, 0, 1);

    label1 = new QLabel(tr("Modem de&vice:"), this);
    tl->addWidget(label1, 1, 0);

    modemdevice = new QComboBox(false, this);
    modemdevice->setEditable( true );
    modemdevice->setDuplicatesEnabled ( false );
    modemdevice->setInsertionPolicy( QComboBox::AtTop );
    label1->setBuddy(modemdevice);

    Config cfg("NetworkSetupPPP");
    cfg.setGroup("Devices_General");
    QStringList devs = cfg.readListEntry("devices",',');
    if (devs.isEmpty()) devs << "/dev/modem" << "/dev/ircomm0" << "/dev/ttyS0";
    modemdevice->insertStringList( devs );
    tl->addWidget(modemdevice, 1, 1);

    //   connect(modemdevice, SIGNAL(activated(int)),
    // 	  SLOT(setmodemdc(int)));
    //   connect(modemdevice, SIGNAL(textChanged(const QString&) ),
    //           SLOT( setmodemdc(const QString&) ) );

    QString tmp = tr("This specifies the serial port your modem is attached \n"
                     "to. On Linux/x86, typically this is either /dev/ttyS0 \n"
                     "(COM1 under DOS) or /dev/ttyS1 (COM2 under DOS).\n"
                     "\n"
                     "If you have an internal ISDN card with AT command\n"
                     "emulation (most cards under Linux support this), you\n"
                     "should select one of the /dev/ttyIx devices.");

    QWhatsThis::add(label1,tmp);
    QWhatsThis::add(modemdevice,tmp);


    label1 = new QLabel(tr("&Flow control:"), this);
    tl->addWidget(label1, 2, 0);

    flowcontrol = new QComboBox(false, this);
    label1->setBuddy(flowcontrol);
    flowcontrol->insertItem(tr("Hardware [CRTSCTS]"));
    flowcontrol->insertItem(tr("Software [XON/XOFF]"));
    flowcontrol->insertItem(tr("None"));
    tl->addWidget(flowcontrol, 2, 1);
    //   connect(flowcontrol, SIGNAL(activated(int)),
    // 	  SLOT(setflowcontrol(int)));

    tmp = tr("<p>Specifies how the serial port and modem\n"
             "communicate. You should not change this unless\n"
             "you know what you are doing.\n"
             "\n"
             "<b>Default</b>: CRTSCTS");

    QWhatsThis::add(label1,tmp);
    QWhatsThis::add(flowcontrol,tmp);

    QLabel *labelenter = new QLabel(tr("&Line termination:"), this);
    tl->addWidget(labelenter, 3, 0);

    enter = new QComboBox(false, this);
    labelenter->setBuddy(enter);
    enter->insertItem("CR");
    enter->insertItem("LF");
    enter->insertItem("CR/LF");
    tl->addWidget(enter, 3, 1);
    //  connect(enter, SIGNAL(activated(int)), SLOT(setenter(int)));
    tmp = tr("<p>Specifies how AT commands are sent to your\n"
             "modem. Most modems will work fine with the\n"
             "default <i>CR/LF</i>. If your modem does not react\n"
             "to the init string, you should try different\n"
             "settings here\n"
             "\n"
             "<b>Default</b>: CR/LF");

    QWhatsThis::add(labelenter,tmp);
    QWhatsThis::add(enter, tmp);

    QLabel *baud_label = new QLabel(tr("Co&nnection speed:"), this);
    tl->addWidget(baud_label, 4, 0);
    baud_c = new QComboBox(this);
    baud_label->setBuddy(baud_c);

    static const char *baudrates[] =
        {

#ifdef B460800
            "460800",
#endif

#ifdef B230400
            "230400",
#endif

#ifdef B115200
            "115200",
#endif

#ifdef B57600
            "57600",
#endif

            "38400",
            "19200",
            "9600",
            "2400",
            0
        };

    for(k = 0; baudrates[k]; k++)
        baud_c->insertItem(baudrates[k]);

    baud_c->setCurrentItem(3);
    //   connect(baud_c, SIGNAL(activated(int)),
    // 	  this, SLOT(speed_selection(int)));
    tl->addWidget(baud_c, 4, 1);

    tmp = tr("Specifies the speed your modem and the serial\n"
             "port talk to each other. You should begin with\n"
             "the default of 38400 bits/sec. If everything\n"
             "works you can try to increase this value, but to\n"
             "no more than 115200 bits/sec (unless you know\n"
             "that your serial port supports higher speeds).");

    QWhatsThis::add(baud_label,tmp);
    QWhatsThis::add(baud_c,tmp);

    tl->addRowSpacing(5, 10);

    //Modem Lock File
    modemlockfile = new QCheckBox(tr("&Use lock file"), this);
    modemlockfile->setChecked(_pppdata->modemLockFile());
    //   connect(modemlockfile, SIGNAL(toggled(bool)),
    //           SLOT(modemlockfilechanged(bool)));
    tl->addMultiCellWidget(modemlockfile, 6, 6, 0, 1);
    //  l12->addStretch(1);
    QWhatsThis::add(modemlockfile,
                        tr("<p>To prevent other programs from accessing the\n"
                           "modem while a connection is established, a\n"
                           "file can be created to indicate that the modem\n"
                           "is in use. On Linux an example file would be\n"
                           "<tt>/var/lock/LCK..ttyS1</tt>\n"
                           "Here you can select whether this locking will\n"
                           "be done.\n"
                           "\n"
                           "<b>Default</b>: On"));

    // Modem Timeout Line Edit Box
    QHBoxLayout *timeoutLayout = new QHBoxLayout( this );
    QLabel *timeoutlabel = new QLabel( tr("Modem timeout:") ,this, "timeout" );
    modemtimeout = new QSpinBox( 1, 120, 1, this, "modemTimeout" );
    //   modemtimeout = new KIntNumInput(_pppdata->modemTimeout(), this);
    //   modemtimeout->setLabel(tr("Modem &timeout:"));
    //  modemtimeout->setRange(1, 120, 1);
    modemtimeout->setSuffix(tr(" sec"));
    modemtimeout->setValue( _pppdata->modemTimeout() );
    //   connect(modemtimeout, SIGNAL(valueChanged(int)),
    // 	  SLOT(modemtimeoutchanged(int)));
    timeoutLayout->addWidget(timeoutlabel);
    timeoutLayout->addWidget(modemtimeout);
    tl->addMultiCellLayout(timeoutLayout, 7, 7, 0, 1);

    QWhatsThis::add(modemtimeout,
                        tr("This specifies how long <i>kppp</i> waits for a\n"
                           "<i>CONNECT</i> response from your modem. The\n"
                           "recommended value is 30 seconds."));

    //set stuff from gpppdata
    enter->setCurrentItem( static_cast<int>(_pppdata->enter()) );
    flowcontrol->setCurrentItem( static_cast<int>( _pppdata->flowcontrol() ) );

    for(int i=0; i <= modemdevice->count()-1; i++)
    {
        if(_pppdata->modemDevice() == modemdevice->text(i))
            modemdevice->setCurrentItem(i);
    }

    //set the modem speed
    for(int i=0; i < baud_c->count(); i++)
        if(baud_c->text(i) == _pppdata->speed())
            baud_c->setCurrentItem(i);

    tl->setRowStretch(1, 1);
}

ModemWidget::~ModemWidget()
{
    QStringList devs;

    for (int i=0;i<modemdevice->count();i++)
    {
        QString s = modemdevice->text(i);
        s.simplifyWhiteSpace();
        if (! s.isEmpty() ) devs << s;
    }


    QString edited = modemdevice->currentText();
    if ( !( edited ).isEmpty() )
    {
        edited.simplifyWhiteSpace();
        if ( devs.contains( edited ) == 0 )
        {
            devs << edited;
        }
    }


    Config cfg("NetworkSetupPPP");
    cfg.setGroup("Devices_General");
    cfg.writeEntry("devices",devs,',');

}

// void ModemWidget::speed_selection(int) {
//   _pppdata->setSpeed(baud_c->text(baud_c->currentItem()));
// }


// void ModemWidget::setenter(int ) {
//   _pppdata->setEnter(enter->text(enter->currentItem()));
// }


// void ModemWidget::setmodemdc(int i) {
//   _pppdata->setModemDevice(modemdevice->text(i));
// }

// void ModemWidget::setmodemdc( const QString &string )  {
//     _pppdata->setModemDevice( string );
// }

// void ModemWidget::setflowcontrol(int i) {
//   _pppdata->setFlowcontrol(flowcontrol->text(i));
// }


// void ModemWidget::modemlockfilechanged(bool set) {
//   _pppdata->setModemLockFile(set);
// }


// void ModemWidget::modemtimeoutchanged(int n) {
//   _pppdata->setModemTimeout(n);
// }



bool ModemWidget::save()
{
    //first check to make sure that the device name is unique!
    if(modemname->text().isEmpty() ||
            !_pppdata->isUniqueDevname(modemname->text()))
        return false;

    _pppdata->setDevname( modemname->text() );
    _pppdata->setModemDevice( modemdevice->currentText() );
    _pppdata->setFlowcontrol(static_cast<PPPData::FlowControl>(flowcontrol->currentItem()));
    _pppdata->setEnter( static_cast<PPPData::LineTermination>(enter->currentItem()));
    _pppdata->setSpeed(baud_c->currentText());
    _pppdata->setModemLockFile( modemlockfile->isChecked());
    _pppdata->setModemTimeout( modemtimeout->value() );
    return true;

}

void ModemWidget::slotBeforeModemQuery()
{
    m_oldModemDev     = _pppdata->modemDevice();
    m_oldFlowControl  = static_cast<int>( _pppdata->flowcontrol() );
    m_oldSpeed        = _pppdata->speed();
    m_oldModemLock    = _pppdata->modemLockFile();
    m_oldModemTimeout = _pppdata->modemTimeout();
    m_oldLineEnd      = static_cast<int>( _pppdata->enter() );


    _pppdata->setModemDevice( modemdevice->currentText() );
    _pppdata->setFlowcontrol(static_cast<PPPData::FlowControl>(flowcontrol->currentItem()));
    _pppdata->setEnter(static_cast<PPPData::LineTermination>(enter->currentItem()));
    _pppdata->setSpeed(baud_c->currentText());
    _pppdata->setModemLockFile( modemlockfile->isChecked());
    _pppdata->setModemTimeout( modemtimeout->value() );
}


void ModemWidget::slotAfterModemQuery()
{
    _pppdata->setModemDevice( m_oldModemDev );
    _pppdata->setFlowcontrol( static_cast<PPPData::FlowControl>(m_oldFlowControl) );
    _pppdata->setEnter( static_cast<PPPData::LineTermination>(m_oldLineEnd) );
    _pppdata->setSpeed( m_oldSpeed );
    _pppdata->setModemLockFile( m_oldModemLock );
    _pppdata->setModemTimeout( m_oldModemTimeout );
}


ModemWidget2::ModemWidget2( PPPData *pd, InterfacePPP *ip, QWidget *parent,
                            const char *name)
        : QWidget(parent, name), _pppdata(pd), _ifaceppp(ip)
{
    QVBoxLayout *l1 = new QVBoxLayout(this, 0 );//, KDialog::spacingHint());


    waitfordt = new QCheckBox(tr("&Wait for dial tone before dialing"), this);
    waitfordt->setChecked(_pppdata->waitForDialTone());
    //  connect(waitfordt, SIGNAL(toggled(bool)), SLOT(waitfordtchanged(bool)));
    l1->addWidget(waitfordt);
    QWhatsThis::add(waitfordt,
                        tr("<p>Normally the modem waits for a dial tone\n"
                           "from your phone line, indicating that it can\n"
                           "start to dial a number. If your modem does not\n"
                           "recognize this sound, or your local phone system\n"
                           "does not emit such a tone, uncheck this option\n"
                           "\n"
                           "<b>Default:</b>: On"));

    QHBoxLayout *waitLayout = new QHBoxLayout( this );
    QLabel *waitLabel = new QLabel( tr("Busy wait:"), this, "busyWait" );
    busywait = new QSpinBox( 0, 300, 5, this, "busyWait" );
    //   busywait = new KIntNumInput(_pppdata->busyWait(), this);
    //   busywait->setLabel(tr("B&usy wait:"));
    //   busywait->setRange(0, 300, 5, true);
    busywait->setSuffix(tr(" sec"));
    //  connect(busywait, SIGNAL(valueChanged(int)), SLOT(busywaitchanged(int)));
    waitLayout->addWidget(waitLabel);
    waitLayout->addWidget(busywait);
    l1->addLayout( waitLayout );

    QWhatsThis::add(busywait,
                        tr("Specifies the number of seconds to wait before\n"
                           "redial if all dialed numbers are busy. This is\n"
                           "necessary because some modems get stuck if the\n"
                           "same number is busy too often.\n"
                           "\n"
                           "The default is 0 seconds, you should not change\n"
                           "this unless you need to."));

    l1->addSpacing(10);

    QHBoxLayout *hbl = new QHBoxLayout;
    hbl->setSpacing(2);//KDialog::spacingHint());

    QLabel *volumeLabel = new QLabel(tr("Modem &volume:"), this);
    hbl->addWidget(volumeLabel);
    volume = new QSlider(0, 2, 1, _pppdata->volume(),
                         QSlider::Horizontal, this);
    volumeLabel->setBuddy(volume);
    volume->setTickmarks(QSlider::Below);
    hbl->addWidget(volume);

    l1->addLayout(hbl);

    //   connect(volume, SIGNAL(valueChanged(int)),
    // 	  this, SLOT(volumeChanged(int)));
    QString tmp = tr("Most modems have a speaker which makes\n"
                     "a lot of noise when dialing. Here you can\n"
                     "either turn this completely off or select a\n"
                     "lower volume.\n"
                     "\n"
                     "If this does not work for your modem,\n"
                     "you must modify the modem volume command.");

    QWhatsThis::add(volumeLabel,tmp);
    QWhatsThis::add(volume, tmp);

    l1->addSpacing(20);

#if 0
    chkbox1 = new QCheckBox(tr("Modem asserts CD line"), this);
    chkbox1->setChecked(_pppdata->UseCDLine());
    connect(chkbox1,SIGNAL(toggled(bool)),
            this,SLOT(use_cdline_toggled(bool)));
    l12->addWidget(chkbox1);
    l12->addStretch(1);
    l1->addStretch(1);
    QWhatsThis::add(chkbox1,
                        tr("This controls how <i>kppp</i> detects that the modem\n"
                           "is not responding. Unless you are having\n"
                           "problems with this, do not modify this setting.\n"
                           "\n"
                           "<b>Default</b>: Off"));
#endif

    modemcmds = new QPushButton(tr("Mod&em Commands..."), this);
    QWhatsThis::add(modemcmds,
                        tr("Allows you to change the AT command for\n"
                           "your modem."));

    modeminfo_button = new QPushButton(tr("&Query Modem..."), this);
    QWhatsThis::add(modeminfo_button,
                        tr("Most modems support the ATI command set to\n"
                           "find out vendor and revision of your modem.\n"
                           "\n"
                           "Press this button to query your modem for\n"
                           "this information. It can be useful to help\n"
                           "you setup the modem"));

    //   terminal_button = new QPushButton(tr("&Terminal..."), this);
    //   QWhatsThis::add(terminal_button,
    // 		  tr("Opens the built-in terminal program. You\n"
    // 		       "can use this if you want to play around\n"
    // 		       "with your modem's AT command set"));

    QHBoxLayout *hbox = new QHBoxLayout();
    l1->addLayout(hbox);
    hbox->addStretch(1);
    QVBoxLayout *vbox = new QVBoxLayout();
    hbox->addLayout(vbox);

    vbox->addWidget(modemcmds);
    vbox->addWidget(modeminfo_button);
    //  vbox->addWidget(terminal_button);

    hbox->addStretch(1);
    l1->addStretch(1);

    connect(modemcmds, SIGNAL(clicked()),
            SLOT(modemcmdsbutton()));
    connect(modeminfo_button, SIGNAL(clicked()),
            SLOT(query_modem()));
    //   connect(terminal_button, SIGNAL(clicked()),
    // 	  SLOT(terminal()));
}


void ModemWidget2::modemcmdsbutton()
{
    ModemCommands mc(_ifaceppp->data(), this, "commands" , true, Qt::WStyle_ContextHelp);

    QPEApplication::execDialog( &mc );
}


void ModemWidget2::query_modem()
{
    emit sig_beforeQueryModem();

    ModemTransfer mt(_ifaceppp->modem(), this);
    mt.exec();

    emit sig_afterQueryModem();
}


// void ModemWidget2::terminal() {
//   MiniTerm terminal(NULL,NULL);
//   terminal.exec();
// }


// #if 0
// void ModemWidget2::use_cdline_toggled(bool on) {
//     _pppdata->setUseCDLine(on);
// }
// #endif

// void ModemWidget2::waitfordtchanged(bool b) {
//   _pppdata->setWaitForDialTone((int)b);
// }

// void ModemWidget2::busywaitchanged(int n) {
//   _pppdata->setbusyWait(n);
// }


// void ModemWidget2::volumeChanged(int v) {
//   _pppdata->setVolume(v);
// }

bool ModemWidget2::save()
{
    _pppdata->setWaitForDialTone(waitfordt->isChecked());
    _pppdata->setbusyWait(busywait->value());
    _pppdata->setVolume(volume->value());
    return true;
}

