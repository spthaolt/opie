TEMPLATE	= app
#CONFIG		= qt warn_on debug
CONFIG		= qt warn_on release
DESTDIR         = $(OPIEDIR)/bin
HEADERS		= gsmtool.h
SOURCES		= main.cpp gsmtool.cpp
CXXFLAGS  += -fexceptions
INCLUDEPATH	+= $(OPIEDIR)/include
INCLUDEPATH	+= $(GSMLIBDIR)
DEPENDPATH	+= $(OPIEDIR)/include
LIBS            += -lqpe -L$(GSMLIBDIR)/gsmlib/.libs -lgsmme
INTERFACES	= gsmtoolbase.ui
TARGET		= gsmtool
