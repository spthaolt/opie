#include <qtextbrowser.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qaction.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <qapplication.h>

#include <qpe/config.h>

#include <opie/ofiledialog.h>

#include "settings.h"
#include "composemail.h"
#include "viewmail.h"
#include "abstractmail.h"
#include "accountview.h"
#include "mailtypes.h"

AttachItem::AttachItem(QListView * parent,QListViewItem *after, const QString&mime,const QString&desc,const QString&file,
    const QString&fsize,int num)
	: QListViewItem(parent,after),_partNum(num)
{
	setText(0, mime);
	setText(1, desc);
	setText(2, file);
    setText(3, fsize);
}

void ViewMail::setBody( RecBody body ) {

m_body = body;
m_mail[2] = body.Bodytext();
attachbutton->setEnabled(body.Parts().count()>0);
attachments->setEnabled(body.Parts().count()>0);
if (body.Parts().count()==0) {
    return;
}
AttachItem * curItem=0;
QString type=body.Description().Type()+"/"+body.Description().Subtype();
QString desc,fsize;
double s = body.Description().Size();
int w;
w=0;

while (s>1024) {
    s/=1024;
    ++w;
    if (w>=2) break;
}

QString q="";
switch(w) {
case 1:
    q="k";
    break;
case 2:
    q="M";
    break;
default:
    break;
}

{
    /* I did not found a method to make a CONTENT reset on a QTextStream
       so I use this construct that the stream will re-constructed in each
       loop. To let it work, the textstream is packed into a own area of
       code is it will be destructed after finishing its small job.
    */
    QTextOStream o(&fsize);
    if (w>0) o.precision(2); else o.precision(0);
    o.setf(QTextStream::fixed);
    o << s << " " << q << "Byte";
}

curItem=new AttachItem(attachments,curItem,type,"Mailbody","",fsize,-1);
QString filename = "";
for (unsigned int i = 0; i < body.Parts().count();++i) {
    type = body.Parts()[i].Type()+"/"+body.Parts()[i].Subtype();
    part_plist_t::ConstIterator it = body.Parts()[i].Parameters().begin();
    for (;it!=body.Parts()[i].Parameters().end();++it) {
        qDebug(it.key());
        if (it.key().lower()=="name") {
            filename=it.data();
        }
    }
    s = body.Parts()[i].Size();
    w = 0;
    while (s>1024) {
        s/=1024;
        ++w;
        if (w>=2) break;
    }
    switch(w) {
    case 1:
        q="k";
        break;
    case 2:
        q="M";
        break;
    default:
        q="";
        break;
    }
    QTextOStream o(&fsize);
    if (w>0) o.precision(2); else o.precision(0);
    o.setf(QTextStream::fixed);
    o << s << " " << q << "Byte";
    desc = body.Parts()[i].Description();
    curItem=new AttachItem(attachments,curItem,type,desc,filename,fsize,i);
}
}


void ViewMail::slotShowHtml( bool state ) {
    m_showHtml = state;
    setText();
}

void ViewMail::slotItemClicked( QListViewItem * item , const QPoint & point, int ) {
    if (!item )
        return;

    if (  ( ( AttachItem* )item )->Partnumber() == -1 ) {
        setText();
        return;
    }
    QPopupMenu *menu = new QPopupMenu();
  int ret=0;

  if ( item->text( 0 ).left( 5 ) == "text/" || item->text(0)=="message/rfc822" )  {
     menu->insertItem( tr( "Show Text" ), 1 );
  }
  menu->insertItem( tr( "Save Attachment" ),  0 );
  menu->insertSeparator(1);

  ret = menu->exec( point, 0 );

  switch(ret) {
        case 0:
             { MimeTypes types;
             types.insert( "all", "*" );
             QString str = OFileDialog::getSaveFileName( 1,
                  "/", item->text( 2 )  , types, 0 );

             if( !str.isEmpty() ) {
               encodedString*content = m_recMail.Wrapper()->fetchDecodedPart( m_recMail, m_body.Parts()[ ( ( AttachItem* )item )->Partnumber() ] );
               if (content) {
                 QFile output(str);
                 output.open(IO_WriteOnly);
                 output.writeBlock(content->Content(),content->Length());
                 output.close();
                 delete content;
               }
             }
             }
        break ;

        case 1:
             if (  ( ( AttachItem* )item )->Partnumber() == -1 ) {
                setText();
             } else {
                if (  m_recMail.Wrapper() != 0l )  { // make sure that there is a wrapper , even after delete or simular actions
                browser->setText( m_recMail.Wrapper()->fetchTextPart( m_recMail, m_body.Parts()[ ( ( AttachItem* )item )->Partnumber() ] ) );
                }
             }
        break;
 }
 delete menu;
}


void ViewMail::setMail( RecMail mail ) {

    m_recMail = mail;

    m_mail[0] = mail.getFrom();
    m_mail[1] = mail.getSubject();
    m_mail[3] = mail.getDate();
    m_mail[4] = mail.Msgid();

    m_mail2[0] = mail.To();
    m_mail2[1] = mail.CC();
    m_mail2[2] = mail.Bcc();

    setText();
}



ViewMail::ViewMail( QWidget *parent, const char *name, WFlags fl)
	: ViewMailBase(parent, name, fl), _inLoop(false)
{
	m_gotBody = false;
        deleted = false;

	connect( reply, SIGNAL(activated()), SLOT(slotReply()));
	connect( forward, SIGNAL(activated()), SLOT(slotForward()));
        connect( deleteMail, SIGNAL( activated() ),  SLOT( slotDeleteMail(  ) ) );
        connect( showHtml, SIGNAL( toggled( bool ) ), SLOT( slotShowHtml( bool ) ) );

	attachments->setEnabled(m_gotBody);
        connect( attachments,  SIGNAL( clicked ( QListViewItem *, const QPoint & , int ) ), SLOT( slotItemClicked(  QListViewItem *,  const QPoint & , int ) ) );

        readConfig();

}

void ViewMail::readConfig() {
    Config cfg( "mail" );
    cfg.setGroup( "Settings" );
    m_showHtml =  cfg.readBoolEntry( "showHtml", false );
    showHtml->setOn( m_showHtml );
}

void ViewMail::setText()
{

    QString toString;
    QString ccString;
    QString bccString;

    for ( QStringList::Iterator it = ( m_mail2[0] ).begin(); it != ( m_mail2[0] ).end(); ++it ) {
        toString += (*it);
    }
    for ( QStringList::Iterator it = ( m_mail2[1] ).begin(); it != ( m_mail2[1] ).end(); ++it ) {
        ccString += (*it);
    }
    for ( QStringList::Iterator it = ( m_mail2[2] ).begin(); it != ( m_mail2[2] ).end(); ++it ) {
        bccString += (*it);
    }

    setCaption( caption().arg( m_mail[0] ) );

    m_mailHtml = "<html><body>"
                 "<table width=\"100%\" border=\"0\"><tr bgcolor=\"#FFDD76\"><td>"
                 "<div align=left><b>" + deHtml( m_mail[1] ) + "</b></div>"
                 "</td></tr><tr bgcolor=\"#EEEEE6\"><td>"
                 "<b>" + tr( "From" ) + ": </b><font color=#6C86C0>" + deHtml( m_mail[0] ) + "</font><br>"
                 "<b>" + tr(  "To" ) + ": </b><font color=#6C86C0>" + deHtml( toString ) + "</font><br><b>" +
                 tr( "Cc" ) + ": </b>" + deHtml( ccString ) + "<br>"
                 "<b>" + tr( "Date" ) + ": </b> " +  m_mail[3] +
                 "</td></tr></table><font face=fixed>";

    if ( !m_showHtml ) {
	browser->setText( QString( m_mailHtml) + deHtml( m_mail[2] ) + "</font></html>" );
    } else {
       	browser->setText( QString( m_mailHtml) + m_mail[2] + "</font></html>" );
    }
    // remove later in favor of a real handling
    m_gotBody = true;
}


ViewMail::~ViewMail()
{
    m_recMail.Wrapper()->cleanMimeCache();
	hide();
}

void ViewMail::hide()
{
	QWidget::hide();

	if (_inLoop) {
		_inLoop = false;
		qApp->exit_loop();

        }

}

void ViewMail::exec()
{
	show();

	if (!_inLoop) {
		_inLoop = true;
		qApp->enter_loop();
	}

}

QString ViewMail::deHtml(const QString &string)
{
	QString string_ = string;
	string_.replace(QRegExp("&"), "&amp;");
	string_.replace(QRegExp("<"), "&lt;");
	string_.replace(QRegExp(">"), "&gt;");
	string_.replace(QRegExp("\\n"), "<br>");
	return string_;
}

void ViewMail::slotReply()
{
	if (!m_gotBody) {
		QMessageBox::information(this, tr("Error"), tr("<p>The mail body is not yet downloaded, so you cannot reply yet."), tr("Ok"));
		return;
	}

	QString rtext;
	rtext += QString("* %1 wrote on %2:\n")		// no i18n on purpose
		.arg(  m_mail[0] )
		.arg( m_mail[3] );

	QString text = m_mail[2];
	QStringList lines = QStringList::split(QRegExp("\\n"), text);
        QStringList::Iterator it;
	for (it = lines.begin(); it != lines.end(); it++) {
		rtext += "> " + *it + "\n";
	}
	rtext += "\n";

	QString prefix;
	if ( m_mail[1].find(QRegExp("^Re: *$")) != -1) prefix = "";
	else prefix = "Re: ";				// no i18n on purpose

        Settings *settings = new Settings();
        ComposeMail composer( settings ,this, 0, true);
        composer.setTo( m_mail[0] );
        composer.setSubject( "Re: " + m_mail[1] );
	composer.setMessage( rtext );
	composer.showMaximized();
    if ( QDialog::Accepted==composer.exec()) {
        m_recMail.Wrapper()->answeredMail(m_recMail);
    }
}

void ViewMail::slotForward()
{
	if (!m_gotBody) {
		QMessageBox::information(this, tr("Error"), tr("<p>The mail body is not yet downloaded, so you cannot forward yet."), tr("Ok"));
		return;
	}

	QString ftext;
	ftext += QString("\n----- Forwarded message from %1 -----\n\n")
		.arg( m_mail[0] );
	if (!m_mail[3].isNull())
	 	ftext += QString("Date: %1\n")
		 	.arg( m_mail[3] );
	if (!m_mail[0].isNull())
	 	ftext += QString("From: %1\n")
		 	.arg( m_mail[0] );
	if (!m_mail[1].isNull())
	 	ftext += QString("Subject: %1\n")
		 	.arg( m_mail[1] );

	ftext += QString("\n%1\n")
	 	.arg( m_mail[2]);

	ftext += QString("----- End forwarded message -----\n");

        Settings *settings = new Settings();
        ComposeMail composer( settings ,this, 0, true);
        composer.setSubject( "Fwd: " + m_mail[1] );
	composer.setMessage( ftext );
	composer.showMaximized();
    if ( QDialog::Accepted==composer.exec()) {

    }
}

void ViewMail::slotDeleteMail( )
{
    if ( QMessageBox::warning(this, tr("Delete Mail"), QString( tr("<p>Do you really want to delete this mail? <br><br>" ) + m_mail[0] + " - " + m_mail[1] ) , QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) {
        m_recMail.Wrapper()->deleteMail( m_recMail );
        hide();
        deleted = true;
    }
}
