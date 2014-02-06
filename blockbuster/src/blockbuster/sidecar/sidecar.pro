######################################################################
# Automatically generated by qmake (2.01a) Mon Apr 6 18:30:44 2009
######################################################################

LIBS +=  -LINSTALLDIR_PLACEHOLDER/lib -fPIC -LINSTALLDIR_PLACEHOLDER/lib -fPIC -LINSTALLDIR_PLACEHOLDER/lib -fPIC -LINSTALLDIR_PLACEHOLDER/lib   -lsmovie -lreadline   -lsrwpng -ljpeg -llzo2 -lz -llzma -lm -lboost_regex -lboost_filesystem -lboost_system
INCLUDEPATH += libdmx  .. ../../config INSTALLDIR_PLACEHOLDER/include 
MAKEFILE = Makefile.qt.include
QT += network 
DEFINES += DEBUG NO_BOOST
CONFIG += debug 
QMAKE_PRE_LINK  = export DYLD_IMAGE_SUFFIX=_debug
CONFIG -= release
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += . ../../common /usr/X11/include

# Input
HEADERS = sidecar.h  Prefs.h RemoteControl.h \
    ../common.h \
           ../events.h \
    ../errmsg.h \
           ../Lockables.h \
           ../MovieCues.h \
           ../settings.h \
            ../../sm/smBase.h   \
           ../../common/stringutil.h \
           ../../common/timer.h
           
FORMS = ../BlockbusterControl.ui \
         blockbusterLaunchDialog.ui \
         ../InfoWindow.ui \
         ../MovieCueWidget.ui \
         sidecar.ui
SOURCES = ../common.cpp \
           ../events.cpp \
    ../errmsg.cpp \
            ../util.cpp \
           sidecar_main.cpp \       
        ../settings.cpp \
           ../MovieCues.cpp \
           Prefs.C \
           RemoteControl.cpp \
           ../../common/timer.C \
           sidecar.cpp

RESOURCES = ../images.qrc
