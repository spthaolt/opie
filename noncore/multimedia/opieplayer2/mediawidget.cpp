/*
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#include "mediawidget.h"

extern MediaPlayerState *mediaPlayerState;

MediaWidget::MediaWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    connect( mediaPlayerState, SIGNAL( displayTypeChanged( MediaPlayerState::DisplayType ) ),
             this, SLOT( setDisplayType( MediaPlayerState::DisplayType ) ) );
    connect( mediaPlayerState, SIGNAL( lengthChanged( long ) ),
             this, SLOT( setLength( long ) ) );
    connect( mediaPlayerState, SIGNAL( playingToggled( bool ) ),
             this, SLOT( setPlaying( bool ) ) );
}

MediaWidget::~MediaWidget()
{
}

/* vim: et sw=4 ts=4
 */
