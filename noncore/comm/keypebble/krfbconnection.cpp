#include "krfbconnection.h"
#include "krfblogin.h"
#include "krfbdecoder.h"
#include "krfbbuffer.h"

/* OPIE */
#include <opie2/odebug.h>
using namespace Opie::Core;

/* QT */
#include <qsocket.h>
#include <qtimer.h>

/* STD */
#include <assert.h>
#include <string.h>

KRFBConnection::KRFBConnection( QObject *parent )
		: QObject( parent, "KRFBConnection" )
{
		portBase_ = 5900;
		currentState_ = Disconnected;
		sock = 0;
		minData_ = 0;
		options_ = new KRFBServer();
		updater = 0;
		decoder_ = 0;
		buffer_ = 0;
}

KRFBConnection::~KRFBConnection()
{
		if ( ( currentState_ != Disconnected ) && ( currentState_ != Disconnecting ) && sock ) {
				disconnectDone();
		}
		delete options_;
}

void KRFBConnection::connectTo( KRFBServer server)
{
		if ( currentState_ != Disconnected )
				disconnect();

    (*options_)=server;

		sock = new QSocket( this, "rfbSocket" );
		CHECK_PTR( sock );

		// Connect to something to notice connection or error
		connect( sock, SIGNAL( error(int) ), SLOT( gotSocketError(int) ) );
		connect( sock, SIGNAL( connected() ), SLOT( gotSocketConnection() ) );

		owarn << "Connecting..." << oendl;

		currentState_ = Connecting;
		sock->connectToHost( options_->hostname.latin1(), portBase_ + options_->display );
}

void KRFBConnection::disconnect()
{
		owarn << "Disconnecting from server" << oendl;

		if ( ( currentState_ != Disconnected )
						&& ( currentState_ != Disconnecting )
						&& sock ) {
				currentState_ = Disconnecting;

				connect( sock, SIGNAL( delayedCloseFinished() ), SLOT( disconnectDone() ) );
				sock->close();

				if ( sock->state() != QSocket::Closing )
						disconnectDone();
		}
}

void KRFBConnection::disconnectDone()
{
		currentState_ = Disconnected;
		delete sock;
		sock = 0;
		minData_ = 0;
		delete updater;
		delete decoder_;
		delete buffer_;
		emit disconnected();
}

void KRFBConnection::gotSocketConnection()
{
		currentState_ = LoggingIn;

		owarn << "Connected, logging in..." << oendl;

		static QString statusMsg = tr( "Connected" );
		emit statusChanged( statusMsg );

		// Do some login stuff
		login = new KRFBLogin( this );
}

void KRFBConnection::gotRFBConnection()
{
		owarn << "Logged into server" << oendl;

		currentState_ = Connected;
		emit connected();

		// Create the decoder and start doing stuff
		decoder_ = new KRFBDecoder( this );
		CHECK_PTR( decoder_ );

		buffer_ = new KRFBBuffer( decoder_, this, "RFB Buffer" );
		CHECK_PTR( buffer_ );
		decoder_->setBuffer( buffer_ );

		connect( decoder_, SIGNAL( status(const QString&) ),
						this, SIGNAL( statusChanged(const QString&) ) );
		emit loggedIn();

		decoder_->start();

		updater = new QTimer;
		connect( updater, SIGNAL( timeout() ), SLOT( updateTimer() ) );
		updater->start( options_->updateRate );
}

void KRFBConnection::gotSocketError( int err )
{
		currentState_ = Error;

		// Do some error handling stuff
		owarn << "KRFBConnection: Socket error " << err << "" << oendl;

		static QString refused = tr( "Connection Refused" );
		static QString host = tr( "Host not found" );
		static QString read = tr( "Read Error: QSocket reported an error reading\n"
						"data, the remote host has probably dropped the\n"
						"connection." );
		static QString confused = tr( "QSocket reported an invalid error code" );

		QString msg;
		switch ( err ) {
				case QSocket::ErrConnectionRefused:
						msg = refused;
						break;
				case QSocket::ErrHostNotFound:
						msg = host;
						break;
				case QSocket::ErrSocketRead:
						msg = read;
						break;
				default:
						msg = confused;
		};

		QObject::disconnect( sock, SIGNAL( readyRead() ), this, SLOT( gotMoreData() ) );
		delete sock;
		sock = 0;
		currentState_ = Disconnected;

		emit error( msg );
}

void KRFBConnection::gotMoreData()
{
		assert( minData_ > 0 );

		if ( sock->size() >= minData_ ) {
				minData_ = 0;
				QObject::disconnect( sock, SIGNAL( readyRead() ), this, SLOT( gotMoreData() ) );
				emit gotEnoughData();
		}
}

void KRFBConnection::waitForData( unsigned int sz )
{
		assert( minData_ == 0 );
		assert( sz > 0 );
		assert( currentState_ != Error );

		if ( sock->size() >= sz ) {
				//    owarn << "No need to wait for data" << oendl;
				emit gotEnoughData();
		}
		else {
				//    owarn << "Waiting for " << sz << " bytes" << oendl;
				minData_ = sz;
				connect( sock, SIGNAL( readyRead() ), SLOT( gotMoreData() ) );
		}
}

int KRFBConnection::read( void *buf, int sz )
{
		return sock->readBlock( (char *)  buf, sz );
}

int KRFBConnection::write( void *buf, int sz )
{
		return sock->writeBlock( (const char *) buf, sz );
}

KRFBConnection::State KRFBConnection::state() const
{
		return currentState_;
}

void KRFBConnection::setPortBase( int base )
{
		portBase_ = base;
}

int KRFBConnection::portBase() const
{
		return portBase_;
}

void KRFBConnection::updateTimer()
{
		decoder_->sendUpdateRequest( true );
}

void KRFBConnection::refresh()
{
		decoder_->sendUpdateRequest( false );
}

void KRFBConnection::sendCutText( const QString &text )
{
		decoder_->sendCutEvent( text );
}

const QUrl &KRFBConnection::url()
{
		url_.setProtocol( "vnc" );
		url_.setPort( display() );
		url_.setHost( host() );
		url_.setPath( "/" );

		return url_;
}
