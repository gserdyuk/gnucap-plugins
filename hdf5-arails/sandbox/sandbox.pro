TEMPLATE = lib
TARGET = hdf5out
QT =
CONFIG -= qt
CONFIG += dll debug

INCLUDEPATH += $(GNUCAP)/src $(HDF5_INCLUDE)
LIBS += -L$(HDF5_LIB) -lhdf5

HEADERS += hdf5io.h hdf5out.h  hdf5out.h
SOURCES += hdf5io.cpp hdf5out.cpp


VERSION = 0.0.1

QMAKE_CLEAN += libhdf5out.so libhdf5out.so.0 libhdf5out.so.0.0 libhdf5out.so.0.0.1
