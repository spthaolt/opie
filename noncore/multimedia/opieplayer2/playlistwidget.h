/*
                            This file is part of the Opie Project

                             Copyright (c)  2002,2003 Max Reiss <harlekin@handhelds.org>
                             Copyright (c)  2002 L. Potter <ljp@llornkcor.com>
                             Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
              =.
            .=l.
           .>+-=
 _;:,     .>    :=|.         This program is free software; you can
.> <`_,   >  .   <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.--   :           the terms of the GNU General Public
.="- .-=="i,     .._         License as published by the Free Software
 - .   .-<_>     .<>         Foundation; either version 2 of the License,
     ._= =}       :          or (at your option) any later version.
    .%`+i>       _;_.
    .i_,=:_.      -<s.       This program is distributed in the hope that
     +  .  -:.       =       it will be useful,  but WITHOUT ANY WARRANTY;
    : ..    .:,     . . .    without even the implied warranty of
    =_        +     =;=|`    MERCHANTABILITY or FITNESS FOR A
  _.=:.       :    :=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.=       =       ;      General Public License for more
++=   -.     .`     .:       details.
 :     =  ...= . :.=-
 -.   .:....=;==+<;          You should have received a copy of the GNU
  -_. . .   )=.  =           General Public License along with
    --        :-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/

#ifndef PLAY_LIST_WIDGET_H
#define PLAY_LIST_WIDGET_H


#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qpe/qcopenvelope_qws.h>

#include "playlistwidgetgui.h"

class Config;
class QListViewItem;
class QListView;
class QPoint;
class QAction;
class QLabel;
class MediaPlayerState;
class MediaPlayer;

class PlayListWidget : public PlayListWidgetGui {
    Q_OBJECT
public:
    enum TabType { CurrentPlayList, AudioFiles, VideoFiles, PlayLists };
    enum { TabTypeCount = 4 };

    struct Entry
    {
        Entry( const QString &_name, const QString &_fileName )
            : name( _name ), file( _fileName ) {}
        Entry( const QString &_fileName )
            : name( _fileName ), file( _fileName ) {}

        QString name;
        QString file;
    };

    static QString appName() { return QString::fromLatin1("opieplayer2"); }
    PlayListWidget( QWidget* parent=0, const char* name=0, WFlags = 0 );
    ~PlayListWidget();

    // retrieve the current playlist entry (media file link)
    const DocLnk *current() const;
    void useSelectedDocument();
    TabType currentTab() const;

    Entry currentEntry() const;

public slots:
    bool first();
    bool last();
    bool next();
    bool prev();
    void writeDefaultPlaylist( );
    QString currentFileListPathName() const;
protected:
   QCopChannel * channel;
    void keyReleaseEvent( QKeyEvent *e);

signals:
    void skinSelected();

private:
    int defaultSkinIndex;
/*     void readm3u(const QString &); */
/*     void readPls(const QString &); */
    void readListFromFile(const QString &);
    void initializeStates();

    bool inFileListMode() const;

private slots:
    void qcopReceive(const QCString&, const QByteArray&);
    void populateSkinsMenu();
    void skinsMenuActivated(int);
    void pmViewActivated(int);
    void writem3u();
    void writeCurrentM3u();
    void openFile();
    void openURL();
    void openDirectory();
    void setDocument( const QString& fileref );
    void addToSelection( const DocLnk& ); // Add a media file to the playlist
    void addToSelection( QListViewItem* ); // Add a media file to the playlist
    void clearList();
    void addAllToList();
    void addAllMusicToList();
    void addAllVideoToList();
    void saveList();  // Save the playlist
    void loadList( const DocLnk &);  // Load a playlist
    void playIt( QListViewItem *);
    void btnPlay(bool);
    void deletePlaylist();
    void addSelected();
    void removeSelected();
    void tabChanged(QWidget*);
    void viewPressed( int, QListViewItem *, const QPoint&, int);
    void playlistViewPressed( int, QListViewItem *, const QPoint&, int);
    void playSelected();

private:
    DocLnk addFileToPlaylist(const QString& directory, const QString& name);
    void setButtons(void);
    bool fromSetDocument;
    bool insanityBool;
    QString setDocFileRef, currentPlayList;
    int selected;
    QListView *currentFileListView;

    MediaPlayer *m_mp;
};

#endif // PLAY_LIST_WIDGET_H

