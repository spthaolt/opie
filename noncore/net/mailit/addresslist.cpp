/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Palmtop Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#include <qfile.h>
#include <qtextstream.h>

#include "addresslist.h"

AddressList::AddressList(QString file)
{
	addresses.setAutoDelete(TRUE);
	filename = file;
	read();
	dirty = FALSE;
}

AddressList::~AddressList()
{
	write();
	addresses.clear();
}

void AddressList::addContact(QString email, QString name)
{
	//skip if not a valid email address,
	if (email.find( '@') == -1)
		return;
		
	if ( ! containsEmail(email) ) {
		Contact *in = new Contact;
		in->email = email;
		in->name = name;
		addresses.append(in);
		dirty = TRUE;
	}
}

bool AddressList::containsEmail(QString email)
{
	return ( getEmailRef(email) != -1 );
}

bool AddressList::containsName(QString name)
{
	return ( getNameRef(name) != -1 );
}

QString AddressList::getNameByEmail(QString email)
{
	int pos = getEmailRef(email);
	if (pos != -1) {
		Contact *ptr = addresses.at(pos);
		return ptr->name;
	}
	
	return NULL;
}

QString AddressList::getEmailByName(QString name)
{
	int pos = getNameRef(name);
	if (pos != -1) {
		Contact *ptr = addresses.at(pos);
		return ptr->email;
	}
	
	return NULL;
}

int AddressList::getEmailRef(QString email)
{
	int pos = 0;
	Contact *ptr;
	
	for (ptr = addresses.first(); ptr != 0; ptr = addresses.next() ) {
		if (ptr->email == email)
			return pos;
		pos++;
	}
	return -1;
}

int AddressList::getNameRef(QString name)
{
	int pos = 0;
	Contact *ptr;
	
	for (ptr = addresses.first(); ptr != 0; ptr = addresses.next() ) {
		if (ptr->name == name)
			return pos;
		pos++;
	}
	return -1;
}

QList<Contact>* AddressList::getContactList()
{
	return &addresses;
}

void AddressList::read()
{
	QFile f(filename);
	QString lineEmail, lineName, email, name;
	
	if (! f.open(IO_ReadOnly) )
		return;
	
	QTextStream stream(&f);
	
	while (! stream.atEnd() ) {
		lineEmail = stream.readLine();
		if (! stream.atEnd() )
			lineName = stream.readLine();
		else return;
		
		email = getRightString(lineEmail);
		name = getRightString(lineName);
		addContact(email, name);
	}
	f.close();
}

QString AddressList::getRightString(QString in)
{
	QString out = "";
	
	int pos = in.find('=');
	if (pos != -1) {
		out = in.mid(pos+1).stripWhiteSpace();
	}
	return out;
}

void AddressList::write()
{
	if ( (addresses.count() == 0) || (!dirty) )
		return;
		
	QFile f(filename);
	if (! f.open(IO_WriteOnly) )
		return;
	
	QTextStream stream(&f);
	Contact *ptr;
	for (ptr = addresses.first(); ptr != 0; ptr = addresses.next() ) {
		stream << "email = " + ptr->email + "\n";
		stream << "name = " + ptr->name + "\n";
	}
	f.close();
}
