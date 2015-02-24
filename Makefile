#############################################################################
# Makefile for building: qt4urpm
# Project:  qt4urpm.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -DQT_NO_DEBUG -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED
CFLAGS        = -pipe -O2 -g -pipe -Wformat -Werror=format-security -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -fomit-frame-pointer -mtune=generic -fasynchronous-unwind-tables -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I/usr/lib/qt4/mkspecs/linux-g++ -I. -I/usr/lib/qt4/include/QtCore -I/usr/lib/qt4/include/QtGui -I/usr/lib/qt4/include -I. -I.
LINK          = g++
LFLAGS        = -Wl,--as-needed -Wl,--no-undefined -Wl,-z,relro -Wl,-O1
LIBS          = $(SUBLIBS)  -L/usr/lib -lQtGui -L/usr/lib -pthread -lpng -lfreetype -lgobject-2.0 -lSM -lICE -pthread -pthread -lXrender -lfontconfig -lXext -lX11 -lQtCore -lz -lm -pthread -lgthread-2.0 -lrt -lglib-2.0 -ldl -lpthread
AR            = ar cqs
RANLIB        = 
QMAKE         = /usr/bin/qmake
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -sf
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

ifdef LOCALEPATH
CFLAGS   := $(CFLAGS) -DLOCALEPATH=\"$(LOCALEPATH)\"
endif

CXXFLAGS := $(CFLAGS)

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = main.cpp \
		qt4urpm.cpp \
		dialog.cpp moc_qt4urpm.cpp \
		moc_dialog.cpp
OBJECTS       = main.o \
		qt4urpm.o \
		dialog.o \
		moc_qt4urpm.o \
		moc_dialog.o

QMAKE_TARGET  = qt4urpm
DESTDIR       = 
TARGET        = qt4urpm

first: all lrelease
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: $(TARGET)

$(TARGET): ui_qt4urpm.h $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

qmake:  FORCE
	@$(QMAKE) -spec /usr/lib/qt4/mkspecs/linux-g++ -unix -o Makefile qt4urpm.pro

lupdate:
	lupdate qt4urpm.pro

lrelease:
	lrelease qt4urpm.pro


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core
	-$(DEL_FILE) *.qm *.user


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


compiler_clean:
	-$(DEL_FILE) moc_*.cpp
	-$(DEL_FILE) ui_qt4urpm.h

moc_qt4urpm.cpp: dialog.h \
		qt4urpm.h
	/usr/lib/qt4/bin/moc $(DEFINES) $(INCPATH) qt4urpm.h -o moc_qt4urpm.cpp

moc_dialog.cpp: dialog.h
	/usr/lib/qt4/bin/moc $(DEFINES) $(INCPATH) dialog.h -o moc_dialog.cpp

ui_qt4urpm.h: qt4urpm.ui
	/usr/lib/qt4/bin/uic qt4urpm.ui -o ui_qt4urpm.h

####### Compile

main.o: main.cpp qt4urpm.h \
		dialog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cpp

qt4urpm.o: qt4urpm.cpp qt4urpm.h \
		dialog.h \
		ui_qt4urpm.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o qt4urpm.o qt4urpm.cpp

dialog.o: dialog.cpp dialog.h \
		qt4urpm.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o dialog.o dialog.cpp

moc_qt4urpm.o: moc_qt4urpm.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_qt4urpm.o moc_qt4urpm.cpp

moc_dialog.o: moc_dialog.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_dialog.o moc_dialog.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

