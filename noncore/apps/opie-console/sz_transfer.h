#ifndef OPIE_FL_SZ_H
#define OPIE_FL_SZ_H

#include "file_layer.h"
#include <opie2/oprocess.h>

class SzTransfer : public FileTransferLayer {

    Q_OBJECT

public:
    enum Type {
      SZ=0,
      SX,
      SY
    };

    SzTransfer( Type t, IOLayer * );
    ~SzTransfer();

public slots:
    /**
     * send a file over the layer
     */
    void sendFile( const QString& file ) ;
    void sendFile( const QFile& );
    void sent();

private slots:
    void SzReceivedStdout(Opie::Core::OProcess *, char *, int);
    void SzReceivedStderr(Opie::Core::OProcess *, char *, int);
    void receivedStdin(const QByteArray &);

private:
    Opie::Core::OProcess *proc;
    Type m_t;

};

#endif
