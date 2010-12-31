TEMPLATE = lib
TARGET = hdf5out
QT = 
CONFIG -= qt
CONFIG += dll debug

INCLUDEPATH += $(GNUCAP)/src $(HDF5_INCLUDE) 
LIBS += -L$(HDF5_LIB) -lhdf5

HEADERS += hdf5io.h hdf5out.h tools.h
SOURCES += hdf5io.cpp hdf5mod.cc
	
VERSION = 0.0.1
	
QMAKE_CLEAN += libhdf5out.so libhdf5out.so.0 libhdf5out.so.0.0 libhdf5out.so.0.0.1
