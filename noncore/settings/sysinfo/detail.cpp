/**********************************************************************
** Detail
**
** Display module information
**
** Copyright (C) 2002, Michael Lauer
**                    mickey@tm.informatik.uni-frankfurt.de
**                    http://www.Vanille.de
**
** Based on ProcessDetail by Dan Williams <williamsdr@acm.org>
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
**********************************************************************/

#include "detail.h"

#include <qtextview.h>
#include <qlayout.h>

Detail::Detail( QWidget* parent,  const char* name, WFlags )
    : QWidget( parent, name, WStyle_ContextHelp )
{
    QVBoxLayout *layout = new QVBoxLayout( this );

    detailView = new QTextView( this );
    detailView->setTextFormat( PlainText );

    layout->addWidget( detailView );
}

Detail::~Detail()
{
}


