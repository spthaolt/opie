/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
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
#ifndef QTPALMTOP_RECORD_H
#define QTPALMTOP_RECORD_H
#include <qglobal.h>
#include "qpcglobal.h"
#include "palmtopuidgen.h"
#include <qarray.h>
#include <qmap.h>

#if defined(QPC_TEMPLATEDLL)
// MOC_SKIP_BEGIN
QPC_TEMPLATEEXTERN template class QPC_EXPORT QMap<QString, QString>;
// MOC_SKIP_END
#endif

class QRegExp;
/**
 * @short Qtopia namespace
 * The namespace of Qtopia
 */
namespace Qtopia {

class RecordPrivate;
/**
 * @short The base class of all Records
 *
 * The base class for Records in Qtopia
 * @see Task
 * @see Event
 * @see Contact
 */
class QPC_EXPORT Record
{
public:
    Record() : mUid(0), mCats(), d( 0 ) { }
    Record( const Record &c )
        : mUid( c.mUid ), mCats ( c.mCats ), customMap(c.customMap), d( 0 ) { }
    /**
     * @todo make non inline in regard to KDE BC guide
     */
    virtual ~Record() { }

    Record &operator=( const Record &c );

    virtual bool match( const QRegExp & ) const { return FALSE; }

    void setCategories( const QArray<int> &v ) { mCats = v; mCats.sort(); }
    void setCategories( int single );
    const QArray<int> &categories() const { return mCats; }

    void reassignCategoryId( int oldId, int newId )
    {
        int index = mCats.find( oldId );
        if ( index >= 0 )
            mCats[index] = newId;
    }

    int uid() const { return mUid; };
    virtual void setUid( int i ) { mUid = i; uidGen().store( mUid ); }
    bool isValidUid() const { return mUid != 0; }
    void assignUid() { setUid( uidGen().generate() ); }

    virtual QString customField(const QString &) const;
    virtual void setCustomField(const QString &, const QString &);
    virtual void removeCustomField(const QString &);

    virtual bool operator == ( const Record &r ) const
{ return mUid == r.mUid; }
    virtual bool operator != ( const Record &r ) const
{ return mUid != r.mUid; }

    // convenience methods provided for loading and saving to xml
    static QString idsToString( const QArray<int> &ids );
    // convenience methods provided for loading and saving to xml
    static QArray<int> idsFromString( const QString &str );

    // for debugging
    static void dump( const QMap<int, QString> &map );

protected:
    virtual UidGen &uidGen() = 0;
    virtual QString customToXml() const;
private:
    int mUid;
    QArray<int> mCats;
    QMap<QString, QString> customMap;
    RecordPrivate *d;
};

}

#endif

