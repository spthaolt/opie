/*
 * VCard Backend for the OPIE-Contact Database.
 *
 * Copyright (C) 2000 Trolltech AS.  All rights reserved.
 * Copyright (c) 2002 by Stefan Eilers (Eilers.Stefan@epost.de)
 *
 * =====================================================================
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Library General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * =====================================================================
 * ToDo:
 *
 * =====================================================================
 * Version: $Id: ocontactaccessbackend_vcard.cpp,v 1.1 2004-11-16 21:46:07 mickeyl Exp $
 * =====================================================================
 * History:
 * $Log: ocontactaccessbackend_vcard.cpp,v $
 * Revision 1.1  2004-11-16 21:46:07  mickeyl
 * libopie1 goes into unsupported
 *
 * Revision 1.11  2003/08/01 12:30:16  eilers
 * Merging changes from BRANCH_1_0 to HEAD
 *
 * Revision 1.10.4.3  2003/07/23 08:54:37  eilers
 * Default email was added to the list of all emails, which already contains
 * the default email..
 * This closes bug #1045
 *
 * Revision 1.10.4.2  2003/07/23 08:44:45  eilers
 * Importing of Notes in vcard files wasn't implemented.
 * Closes bug #1044
 *
 * Revision 1.10.4.1  2003/06/02 13:37:49  eilers
 * Fixing memory leak
 *
 * Revision 1.10  2003/04/13 18:07:10  zecke
 * More API doc
 * QString -> const QString&
 * QString = 0l -> QString::null
 *
 * Revision 1.9  2003/03/21 10:33:09  eilers
 * Merged speed optimized xml backend for contacts to main.
 * Added QDateTime to querybyexample. For instance, it is now possible to get
 * all Birthdays/Anniversaries between two dates. This should be used
 * to show all birthdays in the datebook..
 * This change is sourcecode backward compatible but you have to upgrade
 * the binaries for today-addressbook.
 *
 * Revision 1.8  2003/02/21 16:52:49  zecke
 * -Remove old Todo classes they're deprecated and today I already using the
 * new API
 * -Guard against self assignment in OTodo
 * -Add test apps for OPIM
 * -Opiefied Event classes
 * -Added TimeZone handling and pinning of TimeZones to OEvent
 * -Adjust ORecur and the widget to better timezone behaviour
 *
 * Revision 1.7  2003/02/16 22:25:46  zecke
 * 0000276 Fix for that bug.. or better temp workaround
 * A Preferred Number is HOME|VOICE
 * A CellPhone is HOME|VOICE|CELL the type & HOME|VOICE test
 * triggers both
 * and the cell phone number overrides the other entries..
 *
 * as a temp I check that it's not equal to HOME|VOICE|CELL before setting the
 * number
 *
 * The right and final fix would be to reorder the if statement to make it
 * if else based and the less common thing put to the bottom
 *
 * OTodoAccessVcal fix the date for beaming
 *
 * Revision 1.6  2003/01/13 15:49:31  eilers
 * Fixing crash when businesscard.vcf is missing..
 *
 * Revision 1.5  2002/12/07 13:26:22  eilers
 * Fixing bug in storing anniversary..
 *
 * Revision 1.4  2002/11/13 14:14:51  eilers
 * Added sorted for Contacts..
 *
 * Revision 1.3  2002/11/11 16:41:09  kergoth
 * no default arguments in implementation
 *
 * Revision 1.2  2002/11/10 15:41:53  eilers
 * Bugfixes..
 *
 * Revision 1.1  2002/11/09 14:34:52  eilers
 * Added VCard Backend.
 *
 */
#include "ocontactaccessbackend_vcard.h"
#include "../../library/backend/vobject_p.h"
#include "../../library/backend/qfiledirect_p.h"

#include <qpe/timeconversion.h>

#include <qfile.h>

OContactAccessBackend_VCard::OContactAccessBackend_VCard ( const QString& , const QString& filename ):
	m_dirty( false ),
	m_file( filename )
{
	load();
}


bool OContactAccessBackend_VCard::load ()
{
	m_map.clear();
	m_dirty = false;

	VObject* obj = 0l;

	if ( QFile::exists(m_file) ){
		obj = Parse_MIME_FromFileName( QFile::encodeName(m_file).data() );
		if ( !obj )
			return false;
	}else{
		qWarning("File \"%s\" not found !", m_file.latin1() );
		return false;
	}

	while ( obj ) {
		OContact con = parseVObject( obj );
		/*
		 * if uid is 0 assign a new one
		 * this at least happens on
		 * Nokia6210
		 */
		if ( con.uid() == 0 ){
			con.setUid( 1 );
			qWarning("assigned new uid %d",con.uid() );
		}

		m_map.insert( con.uid(), con );

		VObject *t = obj;
		obj = nextVObjectInList(obj);
		cleanVObject( t );
	}

	return true;

}
bool OContactAccessBackend_VCard::reload()
{
	return load();
}
bool OContactAccessBackend_VCard::save()
{
	if (!m_dirty )
		return true;

	QFileDirect file( m_file );
	if (!file.open(IO_WriteOnly ) )
		return false;

	VObject *obj;
	obj = newVObject( VCCalProp );
	addPropValue( obj, VCVersionProp, "1.0" );

	VObject *vo;
	for(QMap<int, OContact>::ConstIterator it=m_map.begin(); it !=m_map.end(); ++it ){
		vo = createVObject( *it );
		writeVObject( file.directHandle() , vo );
		cleanVObject( vo );
	}
	cleanStrTbl();
	deleteVObject( obj );

	m_dirty = false;
	return true;


}
void OContactAccessBackend_VCard::clear ()
{
	m_map.clear();
	m_dirty = true; // ??? sure ? (se)
}

bool OContactAccessBackend_VCard::add ( const OContact& newcontact )
{
	m_map.insert( newcontact.uid(), newcontact );
	m_dirty = true;
	return true;
}

bool OContactAccessBackend_VCard::remove ( int uid )
{
	m_map.remove( uid );
	m_dirty = true;
	return true;
}

bool OContactAccessBackend_VCard::replace ( const OContact &contact )
{
	m_map.replace( contact.uid(), contact );
	m_dirty = true;
	return true;
}

OContact OContactAccessBackend_VCard::find ( int uid ) const
{
    return m_map[uid];
}

QArray<int> OContactAccessBackend_VCard::allRecords() const
{
	QArray<int> ar( m_map.count() );
	QMap<int, OContact>::ConstIterator it;
	int i = 0;
	for ( it = m_map.begin(); it != m_map.end(); ++it ) {
		ar[i] = it.key();
		i++;
	}
	return ar;
}

// Not implemented
QArray<int> OContactAccessBackend_VCard::queryByExample ( const OContact&, int, const QDateTime& )
{
	QArray<int> ar(0);
	return ar;
}

// Not implemented
QArray<int> OContactAccessBackend_VCard::matchRegexp(  const QRegExp&  ) const
{
	QArray<int> ar(0);
	return ar;
}

const uint OContactAccessBackend_VCard::querySettings()
{
	return 0; // No search possible
}

bool OContactAccessBackend_VCard::hasQuerySettings (uint ) const
{
	return false; // No search possible, therefore all settings invalid ;)
}

bool OContactAccessBackend_VCard::wasChangedExternally()
{
	return false; // Don't expect concurrent access
}

// Not implemented
QArray<int> OContactAccessBackend_VCard::sorted( bool , int, int, int )
{
	QArray<int> ar(0);
	return ar;
}

// *** Private stuff ***


OContact OContactAccessBackend_VCard::parseVObject( VObject *obj )
{
	OContact c;

	VObjectIterator it;
	initPropIterator( &it, obj );
	while( moreIteration( &it ) ) {
		VObject *o = nextVObject( &it );
		QCString name = vObjectName( o );
		QCString value = vObjectStringZValue( o );
		if ( name == VCNameProp ) {
			VObjectIterator nit;
			initPropIterator( &nit, o );
			while( moreIteration( &nit ) ) {
				VObject *o = nextVObject( &nit );
				QCString name = vObjectTypeInfo( o );
				QString value = vObjectStringZValue( o );
				if ( name == VCNamePrefixesProp )
					c.setTitle( value );
				else if ( name == VCNameSuffixesProp )
					c.setSuffix( value );
				else if ( name == VCFamilyNameProp )
					c.setLastName( value );
				else if ( name == VCGivenNameProp )
					c.setFirstName( value );
				else if ( name == VCAdditionalNamesProp )
					c.setMiddleName( value );
			}
		}
		else if ( name == VCAdrProp ) {
			bool work = TRUE; // default address is work address
			QString street;
			QString city;
			QString region;
			QString postal;
			QString country;

			VObjectIterator nit;
			initPropIterator( &nit, o );
			while( moreIteration( &nit ) ) {
				VObject *o = nextVObject( &nit );
				QCString name = vObjectName( o );
				QString value = vObjectStringZValue( o );
				if ( name == VCHomeProp )
					work = FALSE;
				else if ( name == VCWorkProp )
					work = TRUE;
				else if ( name == VCStreetAddressProp )
					street = value;
				else if ( name == VCCityProp )
					city = value;
				else if ( name == VCRegionProp )
					region = value;
				else if ( name == VCPostalCodeProp )
					postal = value;
				else if ( name == VCCountryNameProp )
					country = value;
			}
			if ( work ) {
				c.setBusinessStreet( street );
				c.setBusinessCity( city );
				c.setBusinessCountry( country );
				c.setBusinessZip( postal );
				c.setBusinessState( region );
			} else {
				c.setHomeStreet( street );
				c.setHomeCity( city );
				c.setHomeCountry( country );
				c.setHomeZip( postal );
				c.setHomeState( region );
			}
		}
		else if ( name == VCTelephoneProp ) {
			enum {
				HOME = 0x01,
				WORK = 0x02,
				VOICE = 0x04,
				CELL = 0x08,
				FAX = 0x10,
				PAGER = 0x20,
				UNKNOWN = 0x80
			};
			int type = 0;

			VObjectIterator nit;
			initPropIterator( &nit, o );
			while( moreIteration( &nit ) ) {
				VObject *o = nextVObject( &nit );
				QCString name = vObjectTypeInfo( o );
				if ( name == VCHomeProp )
					type |= HOME;
				else if ( name == VCWorkProp )
					type |= WORK;
				else if ( name == VCVoiceProp )
					type |= VOICE;
				else if ( name == VCCellularProp )
					type |= CELL;
				else if ( name == VCFaxProp )
					type |= FAX;
				else if ( name == VCPagerProp )
					type |= PAGER;
				else  if ( name == VCPreferredProp )
					;
				else
					type |= UNKNOWN;
			}
			if ( (type & UNKNOWN) != UNKNOWN ) {
				if ( ( type & (HOME|WORK) ) == 0 ) // default
					type |= HOME;
				if ( ( type & (VOICE|CELL|FAX|PAGER) ) == 0 ) // default
					type |= VOICE;

                                qWarning("value %s %d", value.data(), type );
				if ( (type & (VOICE|HOME) ) == (VOICE|HOME) && (type & (CELL|HOME) ) != (CELL|HOME) )
					c.setHomePhone( value );
				if ( ( type & (FAX|HOME) ) == (FAX|HOME) )
					c.setHomeFax( value );
				if ( ( type & (CELL|HOME) ) == (CELL|HOME) )
					c.setHomeMobile( value );
				if ( ( type & (VOICE|WORK) ) == (VOICE|WORK) && (type & (CELL|WORK) ) != (CELL|WORK) )
					c.setBusinessPhone( value );
				if ( ( type & (FAX|WORK) ) == (FAX|WORK) )
					c.setBusinessFax( value );
				if ( ( type & (CELL|WORK) ) == (CELL|WORK) )
					c.setBusinessMobile( value );
				if ( ( type & (PAGER|WORK) ) == (PAGER|WORK) )
					c.setBusinessPager( value );
			}
		}
		else if ( name == VCEmailAddressProp ) {
			QString email = vObjectStringZValue( o );
			bool valid = TRUE;
			VObjectIterator nit;
			initPropIterator( &nit, o );
			while( moreIteration( &nit ) ) {
				VObject *o = nextVObject( &nit );
				QCString name = vObjectTypeInfo( o );
				if ( name != VCInternetProp && name != VCHomeProp &&
				     name != VCWorkProp &&
				     name != VCPreferredProp )
					// ### preffered should map to default email
					valid = FALSE;
			}
			if ( valid ) {
				c.insertEmail( email );
			}
		}
		else if ( name == VCURLProp ) {
			VObjectIterator nit;
			initPropIterator( &nit, o );
			while( moreIteration( &nit ) ) {
				VObject *o = nextVObject( &nit );
				QCString name = vObjectTypeInfo( o );
				if ( name == VCHomeProp )
					c.setHomeWebpage( value );
				else if ( name == VCWorkProp )
					c.setBusinessWebpage( value );
			}
		}
		else if ( name == VCOrgProp ) {
			VObjectIterator nit;
			initPropIterator( &nit, o );
			while( moreIteration( &nit ) ) {
				VObject *o = nextVObject( &nit );
				QCString name = vObjectName( o );
				QString value = vObjectStringZValue( o );
				if ( name == VCOrgNameProp )
					c.setCompany( value );
				else if ( name == VCOrgUnitProp )
					c.setDepartment( value );
				else if ( name == VCOrgUnit2Prop )
					c.setOffice( value );
			}
		}
		else if ( name == VCTitleProp ) {
			c.setJobTitle( value );
		}
		else if ( name == "X-Qtopia-Profession" ) {
			c.setProfession( value );
		}
		else if ( name == "X-Qtopia-Manager" ) {
			c.setManager( value );
		}
		else if ( name == "X-Qtopia-Assistant" ) {
			c.setAssistant( value );
		}
		else if ( name == "X-Qtopia-Spouse" ) {
			c.setSpouse( value );
		}
		else if ( name == "X-Qtopia-Gender" ) {
			c.setGender( value );
		}
		else if ( name == "X-Qtopia-Anniversary" ) {
			c.setAnniversary( convVCardDateToDate( value ) );
		}
		else if ( name == "X-Qtopia-Nickname" ) {
			c.setNickname( value );
		}
		else if ( name == "X-Qtopia-Children" ) {
			c.setChildren( value );
		}
		else if ( name == VCBirthDateProp ) {
			// Reading Birthdate regarding RFC 2425 (5.8.4)
			c.setBirthday( convVCardDateToDate( value ) );

		}
		else if ( name == VCCommentProp ) {
			c.setNotes( value );
		}
#if 0
		else {
			printf("Name: %s, value=%s\n", name.data(), vObjectStringZValue( o ) );
			VObjectIterator nit;
			initPropIterator( &nit, o );
			while( moreIteration( &nit ) ) {
				VObject *o = nextVObject( &nit );
				QCString name = vObjectName( o );
				QString value = vObjectStringZValue( o );
				printf(" subprop: %s = %s\n", name.data(), value.latin1() );
			}
		}
#endif
	}
	c.setFileAs();
	return c;
}


VObject* OContactAccessBackend_VCard::createVObject( const OContact &c )
{
	VObject *vcard = newVObject( VCCardProp );
	safeAddPropValue( vcard, VCVersionProp, "2.1" );
	safeAddPropValue( vcard, VCLastRevisedProp, TimeConversion::toISO8601( QDateTime::currentDateTime() ) );
	safeAddPropValue( vcard, VCUniqueStringProp, QString::number(c.uid()) );

	// full name
	safeAddPropValue( vcard, VCFullNameProp, c.fullName() );

	// name properties
	VObject *name = safeAddProp( vcard, VCNameProp );
	safeAddPropValue( name, VCFamilyNameProp, c.lastName() );
	safeAddPropValue( name, VCGivenNameProp, c.firstName() );
	safeAddPropValue( name, VCAdditionalNamesProp, c.middleName() );
	safeAddPropValue( name, VCNamePrefixesProp, c.title() );
	safeAddPropValue( name, VCNameSuffixesProp, c.suffix() );

	// home properties
	VObject *home_adr= safeAddProp( vcard, VCAdrProp );
	safeAddProp( home_adr, VCHomeProp );
	safeAddPropValue( home_adr, VCStreetAddressProp, c.homeStreet() );
	safeAddPropValue( home_adr, VCCityProp, c.homeCity() );
	safeAddPropValue( home_adr, VCRegionProp, c.homeState() );
	safeAddPropValue( home_adr, VCPostalCodeProp, c.homeZip() );
	safeAddPropValue( home_adr, VCCountryNameProp, c.homeCountry() );

	VObject *home_phone = safeAddPropValue( vcard, VCTelephoneProp, c.homePhone() );
	safeAddProp( home_phone, VCHomeProp );
	home_phone = safeAddPropValue( vcard, VCTelephoneProp, c.homeMobile() );
	safeAddProp( home_phone, VCHomeProp );
	safeAddProp( home_phone, VCCellularProp );
	home_phone = safeAddPropValue( vcard, VCTelephoneProp, c.homeFax() );
	safeAddProp( home_phone, VCHomeProp );
	safeAddProp( home_phone, VCFaxProp );

	VObject *url = safeAddPropValue( vcard, VCURLProp, c.homeWebpage() );
	safeAddProp( url, VCHomeProp );

	// work properties
	VObject *work_adr= safeAddProp( vcard, VCAdrProp );
	safeAddProp( work_adr, VCWorkProp );
	safeAddPropValue( work_adr, VCStreetAddressProp, c.businessStreet() );
	safeAddPropValue( work_adr, VCCityProp, c.businessCity() );
	safeAddPropValue( work_adr, VCRegionProp, c.businessState() );
	safeAddPropValue( work_adr, VCPostalCodeProp, c.businessZip() );
	safeAddPropValue( work_adr, VCCountryNameProp, c.businessCountry() );

	VObject *work_phone = safeAddPropValue( vcard, VCTelephoneProp, c.businessPhone() );
	safeAddProp( work_phone, VCWorkProp );
	work_phone = safeAddPropValue( vcard, VCTelephoneProp, c.businessMobile() );
	safeAddProp( work_phone, VCWorkProp );
	safeAddProp( work_phone, VCCellularProp );
	work_phone = safeAddPropValue( vcard, VCTelephoneProp, c.businessFax() );
	safeAddProp( work_phone, VCWorkProp );
	safeAddProp( work_phone, VCFaxProp );
	work_phone = safeAddPropValue( vcard, VCTelephoneProp, c.businessPager() );
	safeAddProp( work_phone, VCWorkProp );
	safeAddProp( work_phone, VCPagerProp );

	url = safeAddPropValue( vcard, VCURLProp, c.businessWebpage() );
	safeAddProp( url, VCWorkProp );

	VObject *title = safeAddPropValue( vcard, VCTitleProp, c.jobTitle() );
	safeAddProp( title, VCWorkProp );


	QStringList emails = c.emailList();
	// emails.prepend( c.defaultEmail() ); Fix for bugreport #1045
	for( QStringList::Iterator it = emails.begin(); it != emails.end(); ++it ) {
		VObject *email = safeAddPropValue( vcard, VCEmailAddressProp, *it );
		safeAddProp( email, VCInternetProp );
	}

	safeAddPropValue( vcard, VCNoteProp, c.notes() );

	// Exporting Birthday regarding RFC 2425 (5.8.4)
	if ( c.birthday().isValid() ){
		qWarning("Exporting birthday as: %s", convDateToVCardDate( c.birthday() ).latin1() );
		safeAddPropValue( vcard, VCBirthDateProp, convDateToVCardDate( c.birthday() ) );
	}

	if ( !c.company().isEmpty() || !c.department().isEmpty() || !c.office().isEmpty() ) {
		VObject *org = safeAddProp( vcard, VCOrgProp );
		safeAddPropValue( org, VCOrgNameProp, c.company() );
		safeAddPropValue( org, VCOrgUnitProp, c.department() );
		safeAddPropValue( org, VCOrgUnit2Prop, c.office() );
	}

	// some values we have to export as custom fields
	safeAddPropValue( vcard, "X-Qtopia-Profession", c.profession() );
	safeAddPropValue( vcard, "X-Qtopia-Manager", c.manager() );
	safeAddPropValue( vcard, "X-Qtopia-Assistant", c.assistant() );

	safeAddPropValue( vcard, "X-Qtopia-Spouse", c.spouse() );
	safeAddPropValue( vcard, "X-Qtopia-Gender", c.gender() );
	if ( c.anniversary().isValid() ){
		qWarning("Exporting anniversary as: %s", convDateToVCardDate( c.anniversary() ).latin1() );
		safeAddPropValue( vcard, "X-Qtopia-Anniversary", convDateToVCardDate( c.anniversary() ) );
	}
	safeAddPropValue( vcard, "X-Qtopia-Nickname", c.nickname() );
	safeAddPropValue( vcard, "X-Qtopia-Children", c.children() );

	return vcard;
}

QString OContactAccessBackend_VCard::convDateToVCardDate( const QDate& d ) const
{
		QString str_rfc2425 = QString("%1-%2-%3")
			.arg( d.year() )
			.arg( d.month(), 2 )
			.arg( d.day(), 2 );
		// Now replace spaces with "0"...
		int pos = 0;
		while ( ( pos = str_rfc2425.find (' ')  ) > 0 )
			str_rfc2425.replace( pos, 1, "0" );

		return str_rfc2425;
}

QDate OContactAccessBackend_VCard::convVCardDateToDate( const QString& datestr )
{
	int monthPos = datestr.find('-');
	int dayPos = datestr.find('-', monthPos+1 );
	int sep_ignore = 1;
	if ( monthPos == -1 || dayPos == -1 ) {
		qDebug("fromString didn't find - in str = %s; mpos = %d ypos = %d", datestr.latin1(), monthPos, dayPos );
		// Ok.. No "-" found, therefore we will try to read other format ( YYYYMMDD )
		if ( datestr.length() == 8 ){
			monthPos   = 4;
			dayPos     = 6;
			sep_ignore = 0;
			qDebug("Try with follwing positions str = %s; mpos = %d ypos = %d", datestr.latin1(), monthPos, dayPos );
		} else {
			return QDate();
		}
	}
	int y = datestr.left( monthPos ).toInt();
	int m = datestr.mid( monthPos + sep_ignore, dayPos - monthPos - sep_ignore ).toInt();
	int d = datestr.mid( dayPos + sep_ignore ).toInt();
	qDebug("TimeConversion::fromString ymd = %s => %d %d %d; mpos = %d ypos = %d", datestr.latin1(), y, m, d, monthPos, dayPos);
	QDate date ( y,m,d );
	return date;
}

VObject* OContactAccessBackend_VCard::safeAddPropValue( VObject *o, const char *prop, const QString &value )
{
	VObject *ret = 0;
	if ( o && !value.isEmpty() )
		ret = addPropValue( o, prop, value.latin1() );
	return ret;
}

VObject* OContactAccessBackend_VCard::safeAddProp( VObject *o, const char *prop)
{
	VObject *ret = 0;
	if ( o )
		ret = addProp( o, prop );
	return ret;
}
