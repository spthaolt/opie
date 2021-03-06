/* $Id: dundialog.cpp,v 1.4 2006-04-05 19:29:19 korovkin Exp $ */
/* DUN connection dialog */
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "dundialog.h"
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qstring.h>
#include <qcombobox.h>
#include <qdir.h>
#include <opie2/oprocess.h>
#include <opie2/odebug.h>
using namespace Opie::Core;

using namespace OpieTooth;

using namespace Opie::Core;

DunDialog::DunDialog( const QString& device, int port, QWidget* parent,
    const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl ) {
    QDir d("/etc/ppp/peers/"); //Dir we search files in
    d.setFilter( QDir::Files);
    d.setSorting( QDir::Size | QDir::Reversed );

    if ( !name )
        setName( "DUNDialog" );
    setCaption( tr( "DUN connection " ) ) ;

    m_device = device;
    m_port = port;

    m_dunConnect = NULL;
    layout = new QVBoxLayout( this );

    QLabel* info = new QLabel( this );
    info->setText( tr("Enter an ppp script name:") );

    cmdLine = new QComboBox( this );
    cmdLine->setEditable(true);

    outPut = new QMultiLineEdit( this );
    QFont outPut_font(  outPut->font() );
    outPut_font.setPointSize( 8 );
    outPut->setFont( outPut_font );
    outPut->setWordWrap( QMultiLineEdit::WidgetWidth );

    connectButton = new QPushButton( this );
    connectButton->setText( tr( "Connect" ) );

    doEncryption = new QCheckBox(this, "encrypt");
    doEncryption->setText( tr( "encrypt" ) );

    persist = new QCheckBox(this, "persist");
    persist->setText( tr( "persist" ) );

    layout->addWidget(info);
    layout->addWidget(cmdLine);
    layout->addWidget(doEncryption);
    layout->addWidget(persist);
    layout->addWidget(outPut);
    layout->addWidget(connectButton);

    connect( connectButton, SIGNAL( clicked() ), this,  SLOT( connectToDevice() ) );
    //And fill cmdLine with ppp script filenames
    cmdLine->insertStringList(d.entryList());
}

DunDialog::~DunDialog() {
}

void DunDialog::connectToDevice() {
    bool doEnc = doEncryption->isChecked();
    bool doPersist = persist->isChecked();

    if (cmdLine->currentText() == "")
        return;
    if (m_dunConnect) {
        outPut->append(tr("Work in progress"));
        return;
    }
    m_dunConnect = new OProcess();
    outPut->clear();

    // Fill process command line
    *m_dunConnect << "dund"
            << "--connect" << m_device
            << "--channel" << QString::number(m_port)
            << "--nodetach"; // no tr on any of these
    if (doEnc)
        *m_dunConnect << "--encrypt";
    if (doPersist)
        *m_dunConnect << "--persist";
    *m_dunConnect << "call"
            << cmdLine->currentText();
    if (!m_dunConnect->start(OProcess::NotifyOnExit,
        OProcess::All)) {
        outPut->append(tr("Couldn't start"));
        delete m_dunConnect;
        m_dunConnect = NULL;
    }
    else
    {
        m_dunConnect->resume();
        outPut->append(tr("Started"));
        connect(m_dunConnect,
		    SIGNAL(receivedStdout(Opie::Core::OProcess*, char*, int)),
            this, SLOT(fillOutPut(Opie::Core::OProcess*, char*, int)));
        connect(m_dunConnect,
            SIGNAL(receivedStderr(Opie::Core::OProcess*, char*, int)),
            this,    SLOT(fillErr(Opie::Core::OProcess*, char*, int)));
        connect(m_dunConnect,
            SIGNAL(processExited(Opie::Core::OProcess*)),
            this, SLOT(slotProcessExited(Opie::Core::OProcess*)));
    }
}

void DunDialog::fillOutPut( OProcess*, char* cha, int len ) {
    QCString str(cha, len);
    outPut->append(str);
}

void DunDialog::fillErr(OProcess*, char* buf, int len)
{
    QCString str(buf, len);
    outPut->append(str);
}

void DunDialog::slotProcessExited(OProcess* proc) {
    if (m_dunConnect->normalExit()) {
        outPut->append( tr("Finished with result ") );
        outPut->append( QString::number(proc->exitStatus()) );
    }
    else
        outPut->append( tr("Exited abnormally") );
    delete m_dunConnect;
    m_dunConnect = NULL;
}

void DunDialog::closeEvent(QCloseEvent* e)
{
    if (m_dunConnect && m_dunConnect->isRunning())
        m_dunConnect->kill();
    QDialog::closeEvent(e);
}

//eof
