CONFIG      += qt warn_on  quick-app

HEADERS      = buttonsettings.h \
               buttonutils.h \
               remapdlg.h \
               adddlg.h

SOURCES      = main.cpp \
               buttonsettings.cpp \
               buttonutils.cpp \
               remapdlg.cpp \
               adddlg.cpp

INTERFACES   = remapdlgbase.ui

INCLUDEPATH += $(OPIEDIR)/include
DEPENDPATH  += $(OPIEDIR)/include
LIBS        += -lqpe -lopiecore2
TARGET	     = buttonsettings

include( $(OPIEDIR)/include.pro )
