/*
   rdesktop: A Remote Desktop Protocol client.
   User interface services - X Window System
   Copyright (C) Matthew Chapman 1999-2002
   qt.cpp by Jay Sorg

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "rdesktop.h"
#include <qpe/qpeapplication.h>
#include <qmainwindow.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qimage.h>
#include <qsocketnotifier.h>
#include <qscrollview.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qcheckbox.h>
#include <qpopupmenu.h>
#include "qtwin.h"
#include <stdlib.h>

uint32 flags;
char server[64] = "";
char domain[16] = "";
char password[16] = "";
char shell[128] = "";
char directory[32] = "";

extern int g_width;
extern int g_height;
extern int server_bpp;
extern BOOL fullscreen;
extern char username[];
int global_sock;

QSocketNotifier* SocketNotifier;
QPEApplication* App;
QMyMainWindow* MW;
QMyScrollView* SV;
struct QColorMap
{
  uint32 RGBColors[256];
  int NumColors;
};
QColorMap* CM;
uint8* BS;
int clipx;
int clipy;
int clipcx;
int clipcy;

struct bitmap
{
  int w;
  int h;
  uint8* data;
};

BOOL owncolmap = False;

//*****************************************************************************
void CleanString(QString* Item)
{
  int i;

  i = Item->length() - 1;
  while (i >= 0)
  {
    if (Item->at(i) == 10 || Item->at(i) == 13)
      Item->remove(i, 1);
    i--;
  }
}

//*****************************************************************************
QMyDialog::QMyDialog(QWidget* parent) : QDialog(parent, "Settings", true)
{
  setCaption( tr( "Configuration" ) );
  int i, j;
  char* home;
  char Text[256];
  QString Line;
  QString ItemName;
  QString ItemValue;

  // resize dialog
  resize(230, 270);
  // main list box
  ListBox = new QListBox(this);
  ListBox->move(10, 10);
  ListBox->resize(200, 100);
  connect(ListBox, SIGNAL(selectionChanged()), this, SLOT(ListBoxChanged()));
  connect(ListBox, SIGNAL(selected(int)), this, SLOT(ListBoxSelected(int)));
  // server
  Label1 = new QLabel(this);
  Label1->setText( tr("Server Desc") );
  Label1->move(10, 120);
  Label1->resize(100, 20);
  ServerNameEdit = new QLineEdit(this);
  ServerNameEdit->move(75, 120);
  ServerNameEdit->resize(100, 20);
  // username
  Label2 = new QLabel(this);
  Label2->setText( tr("User Name") );
  Label2->move(10, 150);
  Label2->resize(100, 20);
  UserNameEdit = new QLineEdit(this);
  UserNameEdit->move(75, 150);
  UserNameEdit->resize(100, 20);
  // ip
  Label3 = new QLabel(this);
  Label3->setText( tr("Server Address") );
  Label3->move(10, 180);
  Label3->resize(100, 20);
  IPEdit = new QLineEdit(this);
  IPEdit->move(75, 180);
  IPEdit->resize(100, 20);
  // width and height
  WidthHeightBox = new QComboBox(this);
  WidthHeightBox->move(10, 210);
  WidthHeightBox->resize(100, 20);
  WidthHeightBox->insertItem("240x320");
  WidthHeightBox->insertItem("640x480");
  WidthHeightBox->insertItem("800x600");
  connect(WidthHeightBox, SIGNAL(activated(int)), this, SLOT(ComboChanged(int)));
  WidthHeightBox->setCurrentItem(1);
  WidthEdit = new QLineEdit(this);
  WidthEdit->move(110, 210);
  WidthEdit->resize(30, 20);
  WidthEdit->setText("800");
  HeightEdit = new QLineEdit(this);
  HeightEdit->move(140, 210);
  HeightEdit->resize(30, 20);
  HeightEdit->setText("600");
  // add to list button
  AddButton = new QPushButton(this);
  AddButton->move(180, 120);
  AddButton->resize(50, 20);
  AddButton->setText(tr("Add", "Add Connection"));
  connect(AddButton, SIGNAL(clicked()), this, SLOT(AddClicked()));
  // change list item button
  EditButton = new QPushButton(this);
  EditButton->move(180, 140);
  EditButton->resize(50, 20);
  EditButton->setText(tr("Edit"));
  connect(EditButton, SIGNAL(clicked()), this, SLOT(EditClicked()));
  // save to file button
  SaveButton = new QPushButton(this);
  SaveButton->move(180, 160);
  SaveButton->resize(50, 20);
  SaveButton->setText(tr("Save"));
  connect(SaveButton, SIGNAL(clicked()), this, SLOT(SaveClicked()));
  // remove an item button
  RemoveButton = new QPushButton(this);
  RemoveButton->move(180, 180);
  RemoveButton->resize(50, 20);
  RemoveButton->setText(tr("Remove"));
  connect(RemoveButton, SIGNAL(clicked()), this, SLOT(RemoveClicked()));
  // full screen check box
  FullScreenCheckBox = new QCheckBox(this, "Full Screen");
  FullScreenCheckBox->setText(tr("Full Screen"));
  FullScreenCheckBox->move(10, 230);
  // ok button

  for (i = 0; i < 10; i++)
  {
    ConnectionList[i] = new QMyConnectionItem;
    ConnectionList[i]->ServerName = "";
    ConnectionList[i]->UserName = "";
    ConnectionList[i]->ServerIP = "";
    ConnectionList[i]->Width = 0;
    ConnectionList[i]->Height = 0;
    ConnectionList[i]->FullScreen = 0;
  }
  home = getenv("HOME");
  if (home != NULL)
  {
    snprintf(Text, 255 - 13, "%s/rdesktop.ini", home);
    QFile* File = new QFile(Text);
    if (File->open(IO_ReadOnly))
    {
      i = -1;
      while (!File->atEnd())
      {
        File->readLine(Line, 255);
        j = Line.find("=");
        if (j > 0)
        {
          ItemName = Line.mid(0, j);
          CleanString(&ItemName);
          ItemValue = Line.mid(j + 1);
          CleanString(&ItemValue);
          if (ItemName == "Server")
          {
            i++;
            ConnectionList[i]->ServerName = ItemValue;
            ListBox->insertItem(ItemValue);
          }
          else if (ItemName == "UserName" && i > -1)
            ConnectionList[i]->UserName = ItemValue;
          else if (ItemName == "Width" && i > -1)
            ConnectionList[i]->Width = ItemValue.toInt();
          else if (ItemName == "Height" && i > -1)
            ConnectionList[i]->Height = ItemValue.toInt();
          else if (ItemName == "IP" && i > -1)
            ConnectionList[i]->ServerIP = ItemValue;
          else if (ItemName == "FullScreen" && i > -1)
            ConnectionList[i]->FullScreen = (ItemValue != "0");
        }
      }
    }
    delete File;
  }
}

//*****************************************************************************
QMyDialog::~QMyDialog()
{
  QMyConnectionItem* Item;
  int i;

  for (i = 0; i < 10; i++)
  {
    Item = ConnectionList[i];
    delete Item;
  }
}

//*****************************************************************************
void QMyDialog::ComboChanged(int index)
{
  if (index == 0)
  {
    WidthEdit->setText("240");
    HeightEdit->setText("320");
  }
  if (index == 1)
  {
    WidthEdit->setText("640");
    HeightEdit->setText("480");
  }
  else if (index == 2)
  {
    WidthEdit->setText("800");
    HeightEdit->setText("600");
  }
}

//*****************************************************************************
void QMyDialog::accept()
{
  ServerName = ServerNameEdit->text();
  UserName = UserNameEdit->text();
  Width = WidthEdit->text().toInt();
  Height = HeightEdit->text().toInt();
  ServerIP = IPEdit->text();
  FullScreen = FullScreenCheckBox->isChecked();

  QDialog::accept();
}


//*****************************************************************************
void QMyDialog::AddClicked()
{
  int i;
  QMyConnectionItem* Item;

  i = ListBox->count();
  if (i < 10)
  {
    ListBox->insertItem(ServerNameEdit->text());
    Item = ConnectionList[i];
    Item->ServerName = ServerNameEdit->text();
    Item->UserName = UserNameEdit->text();
    Item->Width = WidthEdit->text().toInt();
    Item->Height = HeightEdit->text().toInt();
    Item->ServerIP = IPEdit->text();
    Item->FullScreen = FullScreenCheckBox->isChecked();
  }
}

//*****************************************************************************
void QMyDialog::EditClicked()
{
  int i;
  QMyConnectionItem* Item;

  i = ListBox->currentItem();
  if (i >= 0)
  {
    Item = ConnectionList[i];
    Item->ServerName = ServerNameEdit->text();
    Item->UserName = UserNameEdit->text();
    Item->Width = WidthEdit->text().toInt();
    Item->Height = HeightEdit->text().toInt();
    Item->ServerIP = IPEdit->text();
    Item->FullScreen = FullScreenCheckBox->isChecked();
    ListBox->changeItem(ServerNameEdit->text(), i);
  }
}

//*****************************************************************************
void WriteString(QFile* File, QString* Line)
{
  File->writeBlock((const char*)(*Line), Line->length());
}

//*****************************************************************************
void QMyDialog::SaveClicked()
{
  int i, j;
  QMyConnectionItem* Item;
  QString Line;
  char* home;
  char Text[256];
  QFile* File;

  home = getenv("HOME");
  if (home != NULL)
  {
    snprintf(Text, 255 - 13, "%s/rdesktop.ini", home);
    File = new QFile(Text);
    if (File->open(IO_Truncate | IO_ReadWrite))
    {
      i = ListBox->count();
      for (j = 0; j < i; j++)
      {
        Item = ConnectionList[j];
        Line = "Server=";
        Line += Item->ServerName;
        Line += (char)10;
        WriteString(File, &Line);
        Line = "UserName=";
        Line += Item->UserName;
        Line += (char)10;
        WriteString(File, &Line);
        Line = "Width=";
        sprintf(Text, "%d", Item->Width);
        Line += Text;
        Line += (char)10;
        WriteString(File, &Line);
        Line = "Height=";
        sprintf(Text, "%d", Item->Height);
        Line += Text;
        Line += (char)10;
        WriteString(File, &Line);
        Line = "IP=";
        Line += Item->ServerIP;
        Line += (char)10;
        WriteString(File, &Line);
        Line = "FullScreen=";
        if (Item->FullScreen)
          Line += "1";
        else
          Line += "0";
        Line += (char)10;
        WriteString(File, &Line);
      }
    }
    File->flush();
    File->close();
    delete File;
  }
}

//*****************************************************************************
void QMyDialog::RemoveClicked()
{
  int i, j, c;
  QMyConnectionItem* Item1;
  QMyConnectionItem* Item2;

  i = ListBox->currentItem();
  if (i >= 0)
  {
    c = ListBox->count();
    for (j = i; j < c - 1; j++)
    {
      Item1 = ConnectionList[i];
      Item2 = ConnectionList[i + 1];
      Item1->ServerName = Item2->ServerName;
      Item1->UserName = Item2->UserName;
      Item1->Width = Item2->Width;
      Item1->Height = Item2->Height;
      Item1->ServerIP = Item2->ServerIP;
      Item1->FullScreen = Item2->FullScreen;
    }
    ListBox->removeItem(i);
  }
}

//*****************************************************************************
void QMyDialog::ListBoxChanged()
{
  int i;
  QMyConnectionItem* Item;
  char Text[100];

  i = ListBox->currentItem();
  if (i >= 0 && i < 10)
  {
    Item = ConnectionList[i];
    ServerNameEdit->setText(Item->ServerName);
    UserNameEdit->setText(Item->UserName);
    sprintf(Text, "%d", Item->Width);
    WidthEdit->setText(Text);
    sprintf(Text, "%d", Item->Height);
    HeightEdit->setText(Text);
    IPEdit->setText(Item->ServerIP);
    FullScreenCheckBox->setChecked(Item->FullScreen != 0);
  }
}

//*****************************************************************************
void QMyDialog::ListBoxSelected(int /*index*/)
{
}

//*****************************************************************************
void GetScanCode(QKeyEvent* e, int* ScanCode, int* code)
{
  int key;
  int mod;
  int ascii;

  key = e->key();
  mod = e->state();
  ascii = e->ascii();

  *ScanCode = 0;
  *code = mod; // 8 shift, 16 control, 32 alt

  switch (key)
  {
    case 4096: // esc
    case 4097: // tab
    case 4099: // backspace
    case 4100: // enter
    case 4101: // enter
    case 4103: // delete
      ascii = 0;
  }

  if (ascii == 0)
  {
    switch (key)
    {
      case 4096: *ScanCode = 0x01; break; // esc
      case 4097: *ScanCode = 0x0f; break; // tab
      case 4099: *ScanCode = 0x0e; break; // backspace
      case 4100: *ScanCode = 0x1c; break; // enter
      case 4101: *ScanCode = 0x1c; break; // enter
      case 4112: *ScanCode = 0xc7; break; // home
      case 4113: *ScanCode = 0xcf; break; // end
      case 4102: *ScanCode = 0xd2; break; // insert
      case 4103: *ScanCode = 0xd3; break; // delete
      case 4118: *ScanCode = 0xc9; break; // page up
      case 4119: *ScanCode = 0xd1; break; // page down
      case 4117: *ScanCode = 0xd0; break; // down arrow
      case 4115: *ScanCode = 0xc8; break; // up arrow
      case 4114: *ScanCode = 0xcb; break; // left arrow
      case 4116: *ScanCode = 0xcd; break; // right arrow
      case 4128: *ScanCode = 0x2a; break; // shift
      case 4131: *ScanCode = 0x38; break; // alt
      case 4129: *ScanCode = 0x1d; break; // ctrl
    }
    if (*ScanCode != 0)
      return;
  }

  switch (ascii)
  {
    // first row
    case 'q':  *ScanCode = 0x10; break;
    case 'Q':  *ScanCode = 0x10; *code |= 8; break;
    case '1':  *ScanCode = 0x02; break;
    case 'w':  *ScanCode = 0x11; break;
    case 'W':  *ScanCode = 0x11; *code |= 8; break;
    case '2':  *ScanCode = 0x03; break;
    case 'e':  *ScanCode = 0x12; break;
    case 'E':  *ScanCode = 0x12; *code |= 8; break;
    case '3':  *ScanCode = 0x04; break;
    case 'r':  *ScanCode = 0x13; break;
    case 'R':  *ScanCode = 0x13; *code |= 8; break;
    case '4':  *ScanCode = 0x05; break;
    case 't':  *ScanCode = 0x14; break;
    case 'T':  *ScanCode = 0x14; *code |= 8; break;
    case '5':  *ScanCode = 0x06; break;
    case 'y':  *ScanCode = 0x15; break;
    case 'Y':  *ScanCode = 0x15; *code |= 8; break;
    case '6':  *ScanCode = 0x07; break;
    case 'u':  *ScanCode = 0x16; break;
    case 'U':  *ScanCode = 0x16; *code |= 8; break;
    case '7':  *ScanCode = 0x08; break;
    case 'i':  *ScanCode = 0x17; break;
    case 'I':  *ScanCode = 0x17; *code |= 8; break;
    case '8':  *ScanCode = 0x09; break;
    case 'o':  *ScanCode = 0x18; break;
    case 'O':  *ScanCode = 0x18; *code |= 8; break;
    case '9':  *ScanCode = 0x0a; break;
    case 'p':  *ScanCode = 0x19; break;
    case 'P':  *ScanCode = 0x19; *code |= 8; break;
    case '0':  *ScanCode = 0x0b; break;
    // second row
    case 'a':  *ScanCode = 0x1e; break;
    case 'A':  *ScanCode = 0x1e; *code |= 8; break;
    case '!':  *ScanCode = 0x02; *code |= 8; break;
    case 's':  *ScanCode = 0x1f; break;
    case 'S':  *ScanCode = 0x1f; *code |= 8; break;
    case '@':  *ScanCode = 0x03; *code |= 8; break;
    case 'd':  *ScanCode = 0x20; break;
    case 'D':  *ScanCode = 0x20; *code |= 8; break;
    case '#':  *ScanCode = 0x04; *code |= 8; break;
    case 'f':  *ScanCode = 0x21; break;
    case 'F':  *ScanCode = 0x21; *code |= 8; break;
    case '$':  *ScanCode = 0x05; *code |= 8; break;
    case 'g':  *ScanCode = 0x22; break;
    case 'G':  *ScanCode = 0x22; *code |= 8; break;
    case '%':  *ScanCode = 0x06; *code |= 8; break;
    case 'h':  *ScanCode = 0x23; break;
    case 'H':  *ScanCode = 0x23; *code |= 8; break;
    case '_':  *ScanCode = 0x0c; *code |= 8; break;
    case 'j':  *ScanCode = 0x24; break;
    case 'J':  *ScanCode = 0x24; *code |= 8; break;
    case '&':  *ScanCode = 0x08; *code |= 8; break;
    case 'k':  *ScanCode = 0x25; break;
    case 'K':  *ScanCode = 0x25; *code |= 8; break;
    case '*':  *ScanCode = 0x09; *code |= 8; break;
    case 'l':  *ScanCode = 0x26; break;
    case 'L':  *ScanCode = 0x26; *code |= 8; break;
    case '(':  *ScanCode = 0x0a; *code |= 8; break;
//    case 8:    *ScanCode = 0x0e; break; // backspace
    // third row
    case 'z':  *ScanCode = 0x2c; break;
    case 'Z':  *ScanCode = 0x2c; *code |= 8; break;
    case 'x':  *ScanCode = 0x2d; break;
    case 'X':  *ScanCode = 0x2d; *code |= 8; break;
    case 'c':  *ScanCode = 0x2e; break;
    case 'C':  *ScanCode = 0x2e; *code |= 8; break;
    case 'v':  *ScanCode = 0x2f; break;
    case 'V':  *ScanCode = 0x2f; *code |= 8; break;
    case 'b':  *ScanCode = 0x30; break;
    case 'B':  *ScanCode = 0x30; *code |= 8; break;
    case '-':  *ScanCode = 0x0c; break;
    case 'n':  *ScanCode = 0x31; break;
    case 'N':  *ScanCode = 0x31; *code |= 8; break;
    case '+':  *ScanCode = 0x0d; *code |= 8; break;
    case 'm':  *ScanCode = 0x32; break;
    case 'M':  *ScanCode = 0x32; *code |= 8; break;
    case '=':  *ScanCode = 0x0d; break;
    case ',':  *ScanCode = 0x33; break;
    case ';':  *ScanCode = 0x27; break;
    case ')':  *ScanCode = 0x0b; *code |= 8; break;
    // fourth row
//    case 9:    *ScanCode = 0x0f; break; // tab
    case '/':  *ScanCode = 0x35; break;
    case '?':  *ScanCode = 0x35; *code |= 8; break;
    case ' ':  *ScanCode = 0x39; break;
    case '\'': *ScanCode = 0x28; break;
    case '"':  *ScanCode = 0x28; *code |= 8; break;
    case '~':  *ScanCode = 0x29; *code |= 8; break;
    case '.':  *ScanCode = 0x34; break;
    case ':':  *ScanCode = 0x27; *code |= 8; break;
    case '<':  *ScanCode = 0x33; *code |= 8; break;
//    case 13:   *ScanCode = 0x1c; break; // enter
    case '>':  *ScanCode = 0x34; *code |= 8; break;
    // others
//    case 27:   *ScanCode = 0x01; break; // esc
    case '`':  *ScanCode = 0x29; break;
    case '^':  *ScanCode = 0x07; *code |= 8; break;
    case '[':  *ScanCode = 0x1a; break;
    case '{':  *ScanCode = 0x1a; *code |= 8; break;
    case ']':  *ScanCode = 0x1b; break;
    case '}':  *ScanCode = 0x1b; *code |= 8; break;
    case '\\': *ScanCode = 0x2b; break;
    case '|':  *ScanCode = 0x2b; *code |= 8; break;
    // ctrl keys
    case 1:    *ScanCode = 0x1e; *code |= 16; break; // a
    case 2:    *ScanCode = 0x30; *code |= 16; break; // b
  }

  if (*ScanCode == 0 && key < 3000)
    printf("unknown key %d mod %d ascii %d\n", key, mod, ascii);

}

//*****************************************************************************
QMyScrollView::QMyScrollView() : QScrollView()
{
}

//*****************************************************************************
QMyScrollView::~QMyScrollView()
{
}

//*****************************************************************************
void QMyScrollView::keyPressEvent(QKeyEvent* e)
{
  int ScanCode, code;
  GetScanCode(e, &ScanCode, &code);
  if (ScanCode != 0)
  {
    if (code & 8) // send shift
      rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYPRESS, 0x2a, 0);
    if (code & 16) // send control
      rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYPRESS, 0x1d, 0);
    if (code & 32) // send alt
      rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYPRESS, 0x38, 0);
    rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYPRESS, ScanCode, 0);
    e->accept();
  }
}

//*****************************************************************************
void QMyScrollView::keyReleaseEvent(QKeyEvent* e)
{
  int ScanCode, code;
  GetScanCode(e, &ScanCode, &code);
  if (ScanCode != 0)
  {
    rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYRELEASE, ScanCode, 0);
    if (code & 8) // send shift
      rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYRELEASE, 0x2a, 0);
    if (code & 16) // send control
      rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYRELEASE, 0x1d, 0);
    if (code & 32) // send alt
      rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYRELEASE, 0x38, 0);
    e->accept();
  }
}

//*****************************************************************************
void QMyScrollView::showEvent(QShowEvent* e)
{
  QScrollView::showEvent(e);
}

//*****************************************************************************
void QMyScrollView::show()
{
  QScrollView::show();
}

//*****************************************************************************
void QMyScrollView::polish()
{
  QScrollView::polish();
}

//*****************************************************************************
void QMyScrollView::timerEvent(QTimerEvent* e)
{
  QScrollView::timerEvent(e);
  killTimer(timer_id);
  QMyDialog* d;
  QWidget* Desktop;
  int dw;
  int dh;

  d = new QMyDialog(this);
  if (d->exec() == 1) // ok clicked
  {
    flags = RDP_LOGON_NORMAL;
    g_width = d->Width;
    g_height = d->Height;
    fullscreen = d->FullScreen;
    sprintf(server, "%s", (const char*)d->ServerIP);
    sprintf(username, "%s", (const char*)d->UserName);
    if (!rdp_connect(server, flags, domain, password, shell, directory))
    {
      delete d;
      SV->close();
      return;
    }
    BS = (uint8*)xmalloc(g_width * g_height);
    memset(BS, 0, g_width * g_height);
    clipx = 0;
    clipy = 0;
    clipcx = g_width;
    clipcy = g_height;
    CM = (QColorMap*)xmalloc(sizeof(struct QColorMap));
    memset(CM, 0, sizeof(struct QColorMap));
    CM->NumColors = 256;
    MW = new QMyMainWindow();
    MW->resize(g_width, g_height);
    MW->show();
    SV->addChild(MW);
    MW->setMouseTracking(true);
    SocketNotifier = new QSocketNotifier(global_sock, QSocketNotifier::Read, MW);
    MW->connect(SocketNotifier, SIGNAL(activated(int)), MW, SLOT(dataReceived()));
    if (fullscreen)
    {
      Desktop = App->desktop();
      dw = Desktop->width();
      dh = Desktop->height();
      if (dw == g_width && dh == g_height)
        MW->resize(g_width - 4, g_height - 4);
      SV->showFullScreen();
    }
    delete d;
  }
  else // cancel clicked
  {
    delete d;
    SV->close();
  }
}

//*****************************************************************************
QMyMainWindow::QMyMainWindow() : QWidget(SV->viewport())
{
  PopupMenu = new QPopupMenu(this);
  PopupMenu->insertItem("Right click", 1, 0);
  PopupMenu->insertItem("Toggle fullscreen", 2, 1);
  PopupMenu->insertItem("Reset keyboard", 3, 2);
  PopupMenu->insertItem("Double click", 4, 3);
  connect(PopupMenu, SIGNAL(activated(int)), this, SLOT(MemuClicked(int)));
}

//*****************************************************************************
QMyMainWindow::~QMyMainWindow()
{
  delete PopupMenu;
}

//*****************************************************************************
void QMyMainWindow::timerEvent(QTimerEvent* e)
{
  QWidget::timerEvent(e);
  if (e->timerId() == timer_id)
  {
    // send mouse up
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON1, mx, my);
    // if in fullscreen, take it out or the menu won't work
    if (fullscreen)
    {
      fullscreen = 0;
      SV->showNormal();
      SV->showMaximized();
    }
    else
      PopupMenu->popup(mapToGlobal(QPoint(mx, my)));
  }
  killTimer(timer_id);
}

//*****************************************************************************
void QMyMainWindow::MemuClicked(int MenuID)
{
  QWidget* Desktop;
  int dw;
  int dh;

  if (MenuID == 1) // right click
  {
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_DOWN | MOUSE_FLAG_BUTTON2, mx, my);
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON2, mx, my);
  }
  else if (MenuID == 2) // toggle full screen
  {
    fullscreen = ~fullscreen;
    if (fullscreen)
    {
      Desktop = App->desktop();
      dw = Desktop->width();
      dh = Desktop->height();
      if (dw == g_width && dh == g_height)
        MW->resize(g_width - 4, g_height - 4);
      SV->showFullScreen();
    }
    else
    {
      SV->showNormal();
      SV->showMaximized();
      MW->resize(g_width, g_height);
    }
  }
  else if (MenuID == 3) // reset keyboard
  {
    rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYRELEASE, 0x2a, 0); // shift
    rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYRELEASE, 0x1d, 0); // control
    rdp_send_input(0, RDP_INPUT_SCANCODE, RDP_KEYRELEASE, 0x38, 0); // alt
  }
  else if (MenuID == 4) // double click
  {
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_DOWN | MOUSE_FLAG_BUTTON1, mx, my);
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON1, mx, my);
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_DOWN | MOUSE_FLAG_BUTTON1, mx, my);
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON1, mx, my);
  }
}

//*****************************************************************************
void QMyMainWindow::mouseMoveEvent(QMouseEvent* e)
{
  int x;
  int y;

  x = e->x();
  y = e->y();

  if (timer_id)
  {
    x = x - mx;
    y = y - my;
    if (x < -10 || x > 10 || y < -10 || y > 10)
    {
      killTimer(timer_id);
      timer_id = 0;
    }
  }
  rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_MOVE, e->x(), e->y());
}

//*****************************************************************************
void QMyMainWindow::mousePressEvent(QMouseEvent* e)
{
  timer_id = startTimer(1000);
  mx = e->x();
  my = e->y();
  if (e->button() == LeftButton)
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_DOWN | MOUSE_FLAG_BUTTON1, e->x(), e->y());
  else if (e->button() == RightButton)
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_DOWN | MOUSE_FLAG_BUTTON2, e->x(), e->y());
  else if (e->button() == MidButton)
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_DOWN | MOUSE_FLAG_BUTTON3, e->x(), e->y());
}

//*****************************************************************************
void QMyMainWindow::mouseReleaseEvent(QMouseEvent* e)
{
  killTimer(timer_id);
  timer_id = 0;
  if (e->button() == LeftButton)
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON1, e->x(), e->y());
  else if (e->button() == RightButton)
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON2, e->x(), e->y());
  else if (e->button() == MidButton)
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON3, e->x(), e->y());
}

//*****************************************************************************
void QMyMainWindow::wheelEvent(QWheelEvent* e)
{
  if (e->delta() > 0)
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON4, e->x(), e->y());
  else if (e->delta() < 0)
    rdp_send_input(0, RDP_INPUT_MOUSE, MOUSE_FLAG_BUTTON5, e->x(), e->y());
}

#define NOT(x) (255-(x))

//*****************************************************************************
uint8 rop(int rop, uint8 src, uint8 dst)
{
  switch (rop)
  {
    case 0x0: return 0;
    case 0x1: return NOT (src | dst);
    case 0x2: return NOT (src) & dst;
    case 0x3: return NOT (src);
    case 0x4: return src & NOT (dst);
    case 0x5: return NOT (dst);
    case 0x6: return src ^ dst;
    case 0x7: return NOT (src & dst);
    case 0x8: return src & dst;
    case 0x9: return NOT (src) ^ dst;
    case 0xa: return dst;
    case 0xb: return NOT (src) | dst;
    case 0xc: return src;
    case 0xd: return src | NOT (dst);
    case 0xe: return src | dst;
    case 0xf: return NOT (0);
  }
  return dst;
}

//*****************************************************************************
uint8 get_pixel(int x, int y)
{
  if (x >= 0 && x < g_width && y >= 0 && y < g_height)
    return BS[y * g_width + x];
  else
    return 0;
}

//*****************************************************************************
void set_pixel(int x, int y, uint8 pixel, int op = 0xc)
{
  if (x >= clipx && x < (clipx + clipcx) && y >= clipy && y < (clipy + clipcy))
    if (x >= 0 && x < g_width && y >= 0 && y < g_height)
      if (op == 0xc)
        BS[y * g_width + x] = pixel;
      else
        BS[y * g_width + x] = rop(op, pixel, BS[y * g_width + x]);
}

//******************************************************************************
// adjust coordinates for cliping rect
bool WarpCoords(int* x, int* y, int* cx, int* cy, int* srcx, int* srcy)
{
  int dx, dy;
  QRect InRect(*x, *y, *cx, *cy);
  QRect OutRect;
  QRect CRect(clipx, clipy, clipcx, clipcy);
  OutRect = InRect.intersect(CRect);
  if (OutRect.isEmpty())
    return false;
  dx = OutRect.x() - InRect.x();
  dy = OutRect.y() - InRect.y();
  *x = OutRect.x();
  *y = OutRect.y();
  *cx = OutRect.width();
  *cy = OutRect.height();
  if (srcx != NULL)
    *srcx = *srcx + dx;
  if (srcy != NULL)
    *srcy = *srcy + dy;
  return true;
}

//*****************************************************************************
void QMyMainWindow::paintEvent(QPaintEvent* pe)
{
  QImage* Image;
  QPainter* Painter;
  QRect Rect;
  int i, j, w, h, l, t;
  uint8* data;

  if (!testWFlags(WRepaintNoErase))
    setWFlags(WRepaintNoErase);
  if (CM != NULL)
  {
    Rect = pe->rect();
    l = Rect.left();
    t = Rect.top();
    w = Rect.width();
    h = Rect.height();
    if (w > 0 && h > 0 && CM->NumColors > 0)
    {
      data = (uint8*)xmalloc(w * h);
      for (i = 0; i < h; i++)
        for (j = 0; j < w; j++)
          data[i * w + j] = get_pixel(l + j, t + i);
      Image = new QImage(data, w, h, 8,(QRgb*)CM->RGBColors, CM->NumColors, QImage::IgnoreEndian);
      Painter = new QPainter(this);
      Painter->drawImage(l, t, *Image, 0, 0, w, h);
      xfree(data);
      delete Painter;
      delete Image;
    }
  }
}

//*****************************************************************************
void QMyMainWindow::closeEvent(QCloseEvent* e)
{
  e->accept();
}

//*****************************************************************************
void QMyMainWindow::dataReceived()
{
  if (rdp_main_loop())
    return;
  else
    SV->close();
}

//*****************************************************************************
void redraw(int x, int y, int cx, int cy)
{
  if (WarpCoords(&x, &y, &cx, &cy, NULL, NULL))
  {
    MW->update(x, y, cx, cy);
  }
}

/*
BOOL ui_init(void)
{
  QWidget* d;

  CM = NULL;
  BS = NULL;
  App = new QPEApplication(g_argc, g_argv);
  if (fullscreen)
  {
    d = QApplication::desktop();
    width = d->width();   // returns screen width
    height = d->height(); // returns screen height
  }
  return True;
}
*/

/*
void ui_deinit(void)
{
  delete App;
}
*/

/*
BOOL ui_create_window(void)
{
  MW = new QMyMainWindow();
  MW->resize(width, height);
  SV = new QMyScrollView();
  SV->resize(width + 4, height + 4);
  App->setMainWidget(SV);
  SV->addChild(MW);
  MW->setMouseTracking(true);
  SocketNotifier = new QSocketNotifier(global_sock, QSocketNotifier::Read, MW);
  MW->connect(SocketNotifier, SIGNAL(activated(int)), MW, SLOT(dataReceived()));
  if (fullscreen)
    SV->showFullScreen();
  else
    SV->showMaximized();
  BS = (uint8*)xmalloc(width * height);
  memset(BS, 0, width * height);
  clipx = 0;
  clipy = 0;
  clipcx = width;
  clipcy = height;
  CM = (QColorMap*)xmalloc(sizeof(struct QColorMap));
  memset(CM, 0, sizeof(struct QColorMap));
  SV->timer_id = SV->startTimer(1000);
  return True;
}
*/

/*
void ui_destroy_window(void)
{
  delete MW;
  delete SV;
  xfree(BS);
  xfree(CM);
}
*/

//*****************************************************************************
/* Returns 0 after user quit, 1 otherwise */
int ui_select(int rdp_socket)
{
  global_sock = rdp_socket;
  return 1;
}

//*****************************************************************************
void ui_move_pointer(int /*x*/, int /*y*/)
{
}

//*****************************************************************************
HBITMAP ui_create_bitmap(int width, int height, uint8 * data)
{
  struct bitmap* the_bitmap;
  uint8* bitmap_data;
  int i, j;

//  printf("width %d height %d times %d\n", width, height, width * height);
  bitmap_data = (uint8*)xmalloc(width * height);
  the_bitmap = (struct bitmap*)xmalloc(sizeof(struct bitmap));
  the_bitmap->w = width;
  the_bitmap->h = height;
  the_bitmap->data = bitmap_data;
  for (i = 0; i < height; i++)
    for (j = 0; j < width; j++)
      bitmap_data[i * width + j] = data[i * width + j];
  return the_bitmap;
}

//*****************************************************************************
void ui_paint_bitmap(int x, int y, int cx, int cy, int width,
                     int height, uint8 * data)
{
  int i, j;

  for (i = 0; i < cy; i++)
    for (j = 0; j < cx; j++)
      if (i < height)
        if (j < width)
          set_pixel(x + j, y + i, data[i * width + j]);
  redraw(x, y, cx, cy);
}

//*****************************************************************************
void ui_destroy_bitmap(HBITMAP bmp)
{
  struct bitmap* the_bitmap;

  the_bitmap = (struct bitmap*)bmp;
  if (the_bitmap != NULL)
  {
    if (the_bitmap->data != NULL)
      xfree(the_bitmap->data);
    xfree(the_bitmap);
  }
}

//*****************************************************************************
bool is_pixel_on(uint8* data, int x, int y, int width, int bpp)
{
  int start, shift;

  if (bpp == 1)
  {
    width = (width + 7) / 8;
    start = (y * width) + x / 8;
    shift = x % 8;
    return (data[start] & (0x80 >> shift)) != 0;
  }
  else if (bpp == 8)
    return data[y * width + x] != 0;
  else
    return false;
}

//*****************************************************************************
void set_pixel_on(uint8* data, int x, int y, int width, int bpp, uint8 pixel)
{
  if (bpp == 8)
    data[y * width + x] = pixel;
}

//*****************************************************************************
HGLYPH ui_create_glyph(int width, int height, uint8 * data)
{
  int i, j;
  uint8* glyph_data;
  struct bitmap* the_glyph;

  glyph_data = (uint8*)xmalloc(width * height);
  the_glyph = (struct bitmap*)xmalloc(sizeof(struct bitmap));
  the_glyph->w = width;
  the_glyph->h = height;
  the_glyph->data = glyph_data;
  memset(glyph_data, 0, width * height);
  for (i = 0; i < height; i++)
    for (j = 0; j < width; j++)
      if (is_pixel_on(data, j, i, width, 1))
        set_pixel_on(glyph_data, j, i, width, 8, 255);
  return the_glyph;
}

//*****************************************************************************
void ui_destroy_glyph(HGLYPH glyph)
{
  struct bitmap* the_glyph;

  the_glyph = (struct bitmap*)glyph;
  if (the_glyph != NULL)
  {
    if (the_glyph->data != NULL)
      xfree(the_glyph->data);
    xfree(the_glyph);
  }
}

//*****************************************************************************
HCURSOR ui_create_cursor(unsigned int /*x*/, unsigned int /*y*/,
                         int /*width*/, int /*height*/, uint8 * /*andmask*/,
                         uint8 * /*xormask*/)
{
  return (void*)1;
}

//*****************************************************************************
void ui_set_cursor(HCURSOR /*cursor*/)
{
}

//*****************************************************************************
void ui_destroy_cursor(HCURSOR /*cursor*/)
{
}

//*****************************************************************************
HCOLOURMAP ui_create_colourmap(COLOURMAP * colours)
{
  int i;
  int x;
  uint8 r, g, b;
  i = 0;
  while (i < colours->ncolours && i < 256)
  {
    r = colours->colours[i].red;
    g = colours->colours[i].green;
    b = colours->colours[i].blue;
    x = (r << 16) | (g << 8) | b;
    CM->RGBColors[i] = x;
    i++;
  }
  CM->NumColors = colours->ncolours;
  return CM;
}

//*****************************************************************************
void ui_destroy_colourmap(HCOLOURMAP /*map*/)
{
}

//*****************************************************************************
void ui_set_colourmap(HCOLOURMAP /*map*/)
{
}

//*****************************************************************************
void ui_begin_update(void)
{
}

//*****************************************************************************
void ui_end_update(void)
{
}

//*****************************************************************************
void ui_set_clip(int x, int y, int cx, int cy)
{
  clipx = x;
  clipy = y;
  clipcx = cx;
  clipcy = cy;
}

//*****************************************************************************
void ui_reset_clip(void)
{
  clipx = 0;
  clipy = 0;
  clipcx = g_width;
  clipcy = g_height;
}

//*****************************************************************************
void ui_bell(void)
{
  App->beep();
}

//*****************************************************************************
void ui_destblt(uint8 opcode, int x, int y, int cx, int cy)
{
  int i, j;

  for (i = 0; i < cy; i++)
    for (j = 0; j < cx; j++)
      set_pixel(x + j, y + i, get_pixel(x + j, y + i), opcode);
  redraw(x, y, cx, cy);
}

//*****************************************************************************
// does not repaint
void fill_rect(int x, int y, int cx, int cy, int colour, int opcode = 0xc)
{
  int i, j;

  for (i = 0; i < cy; i++)
    for (j = 0; j < cx; j++)
      set_pixel(x + j, y + i, colour, opcode);
}

//*****************************************************************************
void ui_rect(int x, int y, int cx, int cy, int colour)
{
  fill_rect(x, y, cx, cy, colour);
  redraw(x, y, cx, cy);
}

//*****************************************************************************
void ui_patblt(uint8 opcode, int x, int y, int cx, int cy,
               BRUSH * brush, int bgcolour, int fgcolour)
{
  int i, j;
  uint8 ipattern[8];

  switch (brush->style)
  {
    case 0:
      fill_rect(x, y, cx, cy, fgcolour, opcode);
      break;
    case 3:
      for (i = 0; i < 8; i++)
        ipattern[i] = ~brush->pattern[7 - i];
      for (i = 0; i < cy; i++)
        for (j = 0; j < cx; j++)
          if (is_pixel_on(ipattern, (x + j + brush->xorigin) % 8,
                            (y + i + brush->yorigin) % 8, 8, 1))
            set_pixel(x + j, y + i, fgcolour, opcode);
          else
            set_pixel(x + j, y + i, bgcolour, opcode);
      break;
  }
  redraw(x, y, cx, cy);
}

//*****************************************************************************
void ui_screenblt(uint8 opcode, int x, int y, int cx, int cy,
                  int srcx, int srcy)
{
  int i, j;
  uint8* temp;

  temp = (uint8*)xmalloc(cx * cy);
  for (i = 0; i < cy; i++)
    for (j = 0; j < cx; j++)
      temp[i * cx + j] = get_pixel(srcx + j, srcy + i);
  for (i = 0; i < cy; i++)
    for (j = 0; j < cx; j++)
      set_pixel(x + j, y + i, temp[i * cx + j], opcode);
  xfree(temp);
  redraw(x, y, cx, cy);
}

//*****************************************************************************
void ui_memblt(uint8 opcode, int x, int y, int cx, int cy,
               HBITMAP src, int srcx, int srcy)
{
  int i, j;
  struct bitmap* the_bitmap;

  the_bitmap = (struct bitmap*)src;
  if (the_bitmap == NULL)
    return;
  for (i = 0; i < cy; i++)
    for (j = 0; j < cx; j++)
      if ((i + srcy) < the_bitmap->h && (j + srcx) < the_bitmap->w)
        set_pixel(x + j, y + i, the_bitmap->data[(i + srcy) * the_bitmap->w + (j + srcx)], opcode);
  redraw(x, y, cx, cy);
}

//*****************************************************************************
// not used
void ui_triblt(uint8 /*opcode*/, int /*x*/, int /*y*/, int /*cx*/, int /*cy*/,
               HBITMAP /*src*/, int /*srcx*/, int /*srcy*/, BRUSH * /*brush*/,
               int /*bgcolour*/, int /*fgcolour*/)
{
}

//*****************************************************************************
// Bresenham's line drawing algorithm
void ui_line(uint8 opcode, int startx, int starty, int endx,
             int endy, PEN * pen)
{
  int dx, dy, incx, incy, dpr, dpru, p, left, top, right, bottom;

  if (startx > endx)
  {
    dx = startx - endx;
    incx = -1;
    left = endx;
    right = startx;
  }
  else
  {
    dx = endx - startx;
    incx = 1;
    left = startx;
    right = endx;
  }
  if (starty > endy)
  {
    dy = starty - endy;
    incy = -1;
    top = endy;
    bottom = starty;
  }
  else
  {
    dy = endy - starty;
    incy = 1;
    top = starty;
    bottom = endy;
  }
  if (dx >= dy)
  {
    dpr = dy << 1;
    dpru = dpr - (dx << 1);
    p = dpr - dx;
    for (; dx >= 0; dx--)
    {
      set_pixel(startx, starty, pen->colour, opcode);
      if (p > 0)
      {
        startx += incx;
        starty += incy;
        p += dpru;
      }
      else
      {
        startx += incx;
        p += dpr;
      }
    }
  }
  else
  {
    dpr = dx << 1;
    dpru = dpr - (dy << 1);
    p = dpr - dy;
    for (; dy >= 0; dy--)
    {
      set_pixel(startx, starty, pen->colour, opcode);
      if (p > 0)
      {
        startx += incx;
        starty += incy;
        p += dpru;
      }
      else
      {
        starty += incy;
        p += dpr;
      }
    }
  }
  redraw(left, top, (right - left) + 1, (bottom - top) + 1);
}

//*****************************************************************************
void draw_glyph (int x, int y, HGLYPH glyph, int fgcolour)
{
  struct bitmap* the_glyph;
  int i, j;

  the_glyph = (struct bitmap*)glyph;
  if (the_glyph == NULL)
    return;
  for (i = 0; i < the_glyph->h; i++)
    for (j = 0; j < the_glyph->w; j++)
      if (is_pixel_on(the_glyph->data, j, i, the_glyph->w, 8))
        set_pixel(x + j, y + i, fgcolour);
}

#define DO_GLYPH(ttext,idx) \
{\
  glyph = cache_get_font (font, ttext[idx]);\
  if (!(flags & TEXT2_IMPLICIT_X))\
    {\
      xyoffset = ttext[++idx];\
      if ((xyoffset & 0x80))\
	{\
	  if (flags & TEXT2_VERTICAL) \
	    y += ttext[idx+1] | (ttext[idx+2] << 8);\
	  else\
	    x += ttext[idx+1] | (ttext[idx+2] << 8);\
	  idx += 2;\
	}\
      else\
	{\
	  if (flags & TEXT2_VERTICAL) \
	    y += xyoffset;\
	  else\
	    x += xyoffset;\
	}\
    }\
  if (glyph != NULL)\
    {\
      draw_glyph (x + glyph->offset, y + glyph->baseline, glyph->pixmap, fgcolour);\
      if (flags & TEXT2_IMPLICIT_X)\
	x += glyph->width;\
    }\
}

//*****************************************************************************
void ui_draw_text(uint8 font, uint8 flags, int mixmode,
                  int x, int y, int clipx, int clipy,
                  int clipcx, int clipcy, int boxx,
                  int boxy, int boxcx, int boxcy, int bgcolour,
                  int fgcolour, uint8 * text, uint8 length)
{
  FONTGLYPH *glyph;
  int i, j, xyoffset;
  DATABLOB *entry;

  if (boxcx > 1)
    fill_rect(boxx, boxy, boxcx, boxcy, bgcolour);
  else if (mixmode == MIX_OPAQUE)
    fill_rect(clipx, clipy, clipcx, clipcy, bgcolour);

  /* Paint text, character by character */
  for (i = 0; i < length;)
  {
    switch (text[i])
    {
      case 0xff:
        if (i + 2 < length)
          cache_put_text(text[i + 1], text, text[i + 2]);
        else
        {
          error("this shouldn't be happening\n");
          exit(1);
        }
        /* this will move pointer from start to first character after FF command */
        length -= i + 3;
        text = &(text[i + 3]);
        i = 0;
        break;

      case 0xfe:
        entry = cache_get_text(text[i + 1]);
        if (entry != NULL)
        {
          if ((((uint8 *) (entry->data))[1] == 0) && (!(flags & TEXT2_IMPLICIT_X)))
          {
            if (flags & TEXT2_VERTICAL)
              y += text[i + 2];
            else
              x += text[i + 2];
          }
          for (j = 0; j < entry->size; j++)
            DO_GLYPH(((uint8 *) (entry->data)), j);
        }
        if (i + 2 < length)
          i += 3;
        else
          i += 2;
        length -= i;
        /* this will move pointer from start to first character after FE command */
        text = &(text[i]);
        i = 0;
        break;

      default:
        DO_GLYPH(text, i);
        i++;
        break;
    }
  }
  if (boxcx > 1)
    redraw(boxx, boxy, boxcx, boxcy);
  else
    redraw(clipx, clipy, clipcx, clipcy);
}

//*****************************************************************************
void ui_desktop_save(uint32 offset, int x, int y, int cx, int cy)
{
  uint8* data;
  int i, j;

  data = (uint8*)xmalloc(cx * cy);
  for (i = 0; i < cy; i++)
    for (j = 0; j < cx; j++)
      data[i * cx + j] = get_pixel(x + j, y + i);
  cache_put_desktop(offset, cx, cy, cx, 1, data);
  xfree(data);
}

//*****************************************************************************
void ui_desktop_restore(uint32 offset, int x, int y, int cx, int cy)
{
  uint8* data;
  int i, j;

  data = cache_get_desktop(offset, cx, cy, 1);
  for (i = 0; i < cy; i++)
    for (j = 0; j < cx; j++)
      set_pixel(x + j, y + i, data[i * cx + j]);
  redraw(x, y, cx, cy);
}

//*****************************************************************************
int main(int argc, char** argv)
{
  CM = NULL;
  BS = NULL;
  App = new QPEApplication(argc, argv);
  SV = new QMyScrollView();
  App->setMainWidget(SV);
  SV->showMaximized();
  SV->setCaption( QMyScrollView::tr("Remote Desktop Client (RDP)") );
  SV->timer_id = SV->startTimer(1000);
  App->exec();
  delete SV;
  delete App;
  if (CM != NULL)
    xfree(CM);
  if (BS !=NULL)
    xfree(BS);
  return 0;
}

/*
  MW = new QMyMainWindow();
  MW->resize(width, height);
  SV->resize(width + 4, height + 4);
  App->setMainWidget(SV);
  SV->addChild(MW);
  MW->setMouseTracking(true);
  SocketNotifier = new QSocketNotifier(global_sock, QSocketNotifier::Read, MW);
  MW->connect(SocketNotifier, SIGNAL(activated(int)), MW, SLOT(dataReceived()));
  if (fullscreen)
    SV->showFullScreen();
  else
    SV->showMaximized();
  BS = (uint8*)xmalloc(width * height);
  memset(BS, 0, width * height);
  clipx = 0;
  clipy = 0;
  clipcx = width;
  clipcy = height;
  CM = (QColorMap*)xmalloc(sizeof(struct QColorMap));
  memset(CM, 0, sizeof(struct QColorMap));
  SV->timer_id = SV->startTimer(1000);
  return True;





int main(int argc, char ** argv)
{
  uint32 flags;
  int i, total_params;
  char server[64] = "192.168.0.164";
  char domain[16] = "";
  char password[16] = "";
  char shell[128] = "";
  char directory[32] = "";
  char* p;
  QString param, s1, s2, s3;

  flags = RDP_LOGON_NORMAL;

  g_argc = argc;
  g_argv = argv;

  if (!ui_init())
    return 1;

  total_params = App->argc();

  for (i = 1; (i + 1) < total_params; i = i + 2)
  {
    s1 = App->argv()[i];
    s2 = App->argv()[i + 1];
    if (s1 == "-g")
    {
      width = strtol((const char*)s2, &p, 10);
      if (*p == 'x')
        height = strtol(p + 1, NULL, 10);
    }
    else if (s1 = "-u")
      strcpy(username, (const char*)s2);
  }
  if (i < total_params)
    strcpy(server, App->argv()[i]);

//  printf("server %s width %d height %d\n", server, width, height);

  if (width == 0 || height == 0)
  {
    ui_deinit();
    return 1;
  }

  if (!rdp_connect(server, flags, domain, password, shell, directory))
  {
    ui_deinit();
    return 1;
  }
  if (ui_create_window())
  {
    ui_main_loop();
    ui_destroy_window();
  }
  rdp_disconnect();
  ui_deinit();
}
*/
