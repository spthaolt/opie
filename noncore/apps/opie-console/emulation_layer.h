/* -------------------------------------------------------------------------- */
/*                                                                            */
/* [emulation.h]          Fundamental Terminal Emulation                      */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* Copyright (c) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>            */
/*                                                                            */
/* This file is part of Konsole - an X terminal for KDE                       */
/*                                                                            */
/* -------------------------------------------------------------------------- */
/*									      */
/* Ported Konsole to Qt/Embedded                                              */
/*									      */
/* Copyright (C) 2000 by John Ryland <jryland@trolltech.com>                  */
/*									      */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*									      */
/* made to a layer between io_layer and widget                                */
/*									      */
/* Copyright (C) 2002 by opie developers <opie-users@lists.sourceforge.net>                 */
/*									      */
/* -------------------------------------------------------------------------- */

#ifndef EMULATION_LAYER_H
#define EMULATION_LAYER_H

#include "widget_layer.h"
#include "screen.h"
#include <qtimer.h>
#include <stdio.h>
#include <qtextcodec.h>
#include "keytrans.h"

class EmulationLayer : public QObject
{ Q_OBJECT

public:

  EmulationLayer( WidgetLayer* gui );
  ~EmulationLayer();

public:
  virtual void setHistory(bool on);
  virtual bool history();

public slots: // signals incoming from Widget

  virtual void onImageSizeChange(int lines, int columns);
  virtual void onHistoryCursorChange(int cursor);
  virtual void onKeyPress(QKeyEvent*);

  virtual void clearSelection();
  virtual void onSelectionBegin(const int x, const int y);
  virtual void onSelectionExtend(const int x, const int y);
  virtual void setSelection(const bool preserve_line_breaks);

public slots: // signals incoming from data source

  /**
   * to be called, when new data arrives
   */
  void onRcvBlock(const QByteArray&);

signals:

  /**
   * will send data, encoded to suit emulation
   */
  void sndBlock(const QByteArray&);

  void ImageSizeChanged(int lines, int columns);

  void changeColumns(int columns);

  void changeTitle(int arg, const char* str);


public:

  /**
   * process single char (decode)
   */
  virtual void onRcvChar(int);

  virtual void setMode  (int) = 0;
  virtual void resetMode(int) = 0;

  /**
   * @deprecated use qbytearray instead
   */
  virtual void sendString(const char*) = 0;

  /**
   * sends a string to IOLayer
   * encodes to suit emulation before
   */
  virtual void sendString(const QByteArray&) = 0;

  virtual void setConnect(bool r);
  void setColumns(int columns);

  void setKeytrans(int no);
  void setKeytrans(const char * no);

protected:

  WidgetLayer* gui;
  Screen* scr;         // referes to one `screen'
  Screen* screen[2];   // 0 = primary, 1 = alternate
  void setScreen(int n); // set `scr' to `screen[n]'

  bool   connected;    // communicate with widget

  void setCodec(int c); // codec number, 0 = locale, 1=utf8

  QTextCodec* codec;
  QTextCodec* localeCodec;
  QTextDecoder* decoder;

  KeyTrans* keytrans;

// refreshing related material.
// this is localized in the class.
private slots: // triggered by timer

  void showBulk();

private:

  void bulkNewline();
  void bulkStart();
  void bulkEnd();

private:

  QTimer bulk_timer;
  int    bulk_nlcnt;   // bulk newline counter
  char*  SelectedText;
  int    bulk_incnt;   // bulk counter

};

#endif // ifndef EMULATION_H
