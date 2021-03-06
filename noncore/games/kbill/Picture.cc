/***************************************************************************
                          Picture.cc  -  description
                             -------------------
    begin                : Thu Dec 30 1999
    copyright            : (C) 1999 by Jurrien Loonstra
    email                : j.h.loonstra@st.hanze.nl
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "Picture.h"

#include <opie2/odebug.h>
#include <opie2/oresource.h>
#ifdef KDEVER
#include <kapp.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#endif
void Picture::load(const char *name, int index) {
#ifdef KDEVER
	KStandardDirs dirs;
	QString file;

	if (index>=0) {
	//kdDebug() << "Here";
	QString sindex;
	sindex.setNum(index);
	 // kdDebug() << "kbill/pixmaps/" + QString::fromLocal8Bit(name) + "_" + sindex + ".xpm";
	   file = dirs.findResource("data","kbill/pixmaps/" + QString::fromLocal8Bit(name) + "_" + sindex + ".xpm");
	} else {
	  file = dirs.findResource("data","kbill/pixmaps/" + QString::fromLocal8Bit(name) + ".xpm");
	}
	kdDebug() << file << std::endl;
	pix = new QPixmap();
	if (pix->load(file) == FALSE)
		oerr << "cannot open " << file << oendl;
	width = pix->width();
	height = pix->height();
#endif
QString sindex;
//pix = new QPixmap();
sindex.setNum(index);
if (index>=0)
    pix = new QPixmap( Opie::Core::OResource::loadPixmap( "kbill/pixmaps/" + QString::fromLocal8Bit(name) +"_"+ sindex ) );
else
    pix = new QPixmap( Opie::Core::OResource::loadPixmap( "kbill/pixmaps/" + QString::fromLocal8Bit(name) ) );

	width = pix->width();
	height = pix->height();

}

QPixmap* Picture::getPixmap() {
	return pix;
}
