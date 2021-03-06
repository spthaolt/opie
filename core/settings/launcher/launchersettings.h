/*
               =.            This file is part of the OPIE Project
             .=l.            Copyright (c)  2002 Robert Griebl <sandman@handhelds.org>
           .>+-=
 _;:,     .>    :=|.         This file is free software; you can
.> <`_,   >  .   <=          redistribute it and/or modify it under
:`=1 )Y*s>-.--   :           the terms of the GNU General Public
.="- .-=="i,     .._         License as published by the Free Software
 - .   .-<_>     .<>         Foundation; either version 2 of the License,
     ._= =}       :          or (at your option) any later version.
    .%`+i>       _;_.
    .i_,=:_.      -<s.       This file is distributed in the hope that
     +  .  -:.       =       it will be useful, but WITHOUT ANY WARRANTY;
    : ..    .:,     . . .    without even the implied warranty of
    =_        +     =;=|`    MERCHANTABILITY or FITNESS FOR A
  _.=:.       :    :=>`:     PARTICULAR PURPOSE. See the GNU General
..}^=.=       =       ;      Public License for more details.
++=   -.     .`     .:
 :     =  ...= . :.=-        You should have received a copy of the GNU
 -.   .:....=;==+<;          General Public License along with this file;
  -_. . .   )=.  =           see the file COPYING. If not, write to the
    --        :-=`           Free Software Foundation, Inc.,
                             59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.

*/

#ifndef __LAUNCHER_SETTINGS_H__
#define __LAUNCHER_SETTINGS_H__

#include <qdialog.h>

class TabsSettings;
class TaskbarSettings;
class MenuSettings;
class InputMethodSettings;
class DocTabSettings;

class LauncherSettings : public QDialog {
	Q_OBJECT

public:
	static QString appName() { return QString::fromLatin1("launchersettings"); }
	LauncherSettings (QWidget *parent = 0, const char* name = 0, WFlags fl = 0 );

	virtual void accept ( );
	virtual void done ( int r );

private:
	TabsSettings *m_tabs;
	TaskbarSettings *m_taskbar;
	MenuSettings *m_menu;
	InputMethodSettings* m_imethods;
	DocTabSettings *m_doctab;
};

#endif
