/**********************************************************************
** Copyright (C) 2001 Trolltech AS.  All rights reserved.
** Copyright (C) 2003 zecke Introduce Sharp to the glory of default arguments
**
** This file is part of Qtopia Environment.
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

#ifndef __CATEGORYCOMBO_H__
#define __CATEGORYCOMBO_H__

#include <qcombobox.h>
#include <qhbox.h>
#include <qstring.h>
#include <qarray.h>

extern QString categoryFileName();

class QToolButton;

class CategoryComboPrivate;
class CategoryCombo : public QComboBox
{
    Q_OBJECT

public:
    CategoryCombo( QWidget *parent, const char* name = 0, int width = 0);

    ~CategoryCombo();

    int currentCategory() const;
    void setCurrentCategory( int id );
    // depreciated.
    void initCombo( const QArray<int> &recCats, const QString &appName );
    void initCombo( const QArray<int> &recCats, const QString &appName,
		    const QString &visibleName /* = appName */ );

    QArray<int> initComboWithRefind( const QArray<int> &recCats, const QString &appName );

signals:
    void sigCatChanged( int newUid );

private slots:
    void slotValueChanged( int );

private:
    void setCurrentText( const QString &str );
    CategoryComboPrivate *d;
};

#endif

class CategorySelectPrivate;
class CategorySelect : public QHBox
{
    Q_OBJECT
public:
    // we need two constructors, the first gets around designer limitations

    /*
     * The Sharp libqpe does have two c'tor and no default argument for int width
     * to stay BC cause the Kompany have the original headers and don't compile
     * against the normal SDK we need to provide the two symbols as well
     * -zecke
     */
    CategorySelect( QWidget* parent  =  0, const char* name = 0 );
    CategorySelect( QWidget *parent /*= 0 */, const char *name/* = 0*/ , int width /* = 0 if we break bc -zecke */ );

    CategorySelect( const QArray<int> &vlCats, const QString &appName,
		    QWidget *parent = 0, const char *name = 0,
		    int width = 0);
    CategorySelect( const QArray<int> &vlCats, const QString &appName,
		    const QString &visibleName, QWidget *parent = 0,
		    const char *name = 0 , int width = 0);

    ~CategorySelect();

    const QArray<int> &currentCategories() const;
    int currentCategory() const;
    void setCurrentCategory( int newCatUid );
    // pretty much if you don't set it the constructor, you need to
    // call it here ASAP!
    // however this call is depreciated...

    QString setCategories( const QArray<int> &vlCats, const QString &appName );
    QString setCategories( const QArray<int> &vlCats, const QString &appName,
			const QString &visibleName );
    // these were added for find dialog.
    void setRemoveCategoryEdit( bool remove );
    void setAllCategories( bool add );

    void setFixedWidth(int width);
signals:
    void signalSelected( int );

private slots:
    void slotDialog();

public slots:
    void slotNewCat( int id );

private:
    void init(int width=0);
    QString mStrAppName;
    CategoryCombo *cmbCat;
    QToolButton *cmdCat;
    CategorySelectPrivate *d;
};
