TEMPLATE    = lib
CONFIG      += qt warn_on 
DESTDIR     = $(OPIEDIR)/lib
HEADERS     = osoundsystem.h oimagezoomer.h oimagescrollview.h opieexif.h
SOURCES     = osoundsystem.cpp oimagezoomer.cpp oimagescrollview.cpp opieexif.cpp
INTERFACES  =
TARGET      = opiemm2
VERSION     = 1.9.0
INCLUDEPATH += $(OPIEDIR)/include
DEPENDPATH  += $(OPIEDIR)/include

!contains( platform, x11 ) {
  include ( $(OPIEDIR)/include.pro )
}

contains( platform, x11 ) {
  LIBS        = -L$(OPIEDIR)/lib -Wl,-rpath,$(OPIEDIR)/lib 
}
