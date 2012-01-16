#############################################################################
# Makefile for building: libnoise.so
# Project:  gnucap-a noise
#############################################################################

GNUCAP=		../../gnucap-a/gnucap/

TARGET  = libnoise_1_0.so

####### Compiler, tools and options

CC       = gcc
CXX      = g++44
CFLAGS   = -pipe  -W -g -fPIC -O0 
CXXFLAGS = -pipe  -W -g -fPIC -O0 

INCPATH  = -I. -I$(GNUCAP)/src 

LINK     = g++
LFLAGS   = -shared -Wl,-soname,$(TARGET)
LIBS     = $(SUBLIBS) 
AR       = ar cqs
RANLIB   = 
TAR      = tar -cf
GZIP     = gzip -9f
COPY     = cp -f
COPY_FILE= $(COPY)
COPY_DIR = $(COPY) -r
INSTALL_FILE= $(COPY_FILE)
INSTALL_DIR = $(COPY_DIR)
DEL_FILE = rm -f
SYMLINK  = ln -sf
DEL_DIR  = rmdir
MOVE     = mv -f
CHK_DIR_EXISTS= test -d
MKDIR    = mkdir -p

####### Output directory

OBJECTS_DIR = ./

####### Files

HEADERS = s_noise.h
SOURCES = s_noise.cc 
OBJECTS = s_noise.o 



####### Implicit rules

.SUFFIXES: .o .cpp .cc .cxx

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

####### Build rules

all: $(TARGET)

$(TARGET):  $(OBJECTS)   
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) 

clean:
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) libnoise*.so 


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET)

install:  

uninstall:  

