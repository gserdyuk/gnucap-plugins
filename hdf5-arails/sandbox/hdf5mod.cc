#include "s_dc.cc"
#include "s_ac.cc"
#include "s_tr.h"
#include "s_fo.cc"
//#include "m_wave.h"
#include "hdf5out.h"

namespace {

class OPHDF5 : public hdf5out<OP> {
public:
  explicit OPHDF5(): hdf5out<OP>() {}
  ~OPHDF5() {}
private:
  explicit OPHDF5(const OPHDF5&): hdf5out<OP>() {unreachable(); incomplete();}

};

class DCHDF5 : public hdf5out<DC> {
public:
  explicit DCHDF5(): hdf5out<DC>() {}
  ~DCHDF5() {}
private:
  explicit DCHDF5(const DCHDF5&): hdf5out<DC>() {unreachable(); incomplete();}

};

class ACHDF5 : public hdf5out<AC> {
public:
  explicit ACHDF5(): hdf5out<AC>() {}
  ~ACHDF5() {}
private:
  explicit ACHDF5(const ACHDF5&): hdf5out<AC>() {unreachable(); incomplete();}

};

class TRHDF5 : public hdf5out<TRANSIENT> {
public:
  explicit TRHDF5(): hdf5out<TRANSIENT>() {}
  ~TRHDF5() {}
private:
  explicit TRHDF5(const TRHDF5&): hdf5out<TRANSIENT>() {unreachable(); incomplete();}

};


class FOHDF5 : public hdf5out<FOURIER> {
public:
  explicit FOHDF5(): hdf5out<FOURIER>() {}
  ~FOHDF5() {}
private:
  explicit FOHDF5(const FOHDF5&): hdf5out<FOURIER>() {unreachable(); incomplete();}

};

static OPHDF5 ophdf5;
static DISPATCHER<CMD>::INSTALL hdf5op(&command_dispatcher, "op", &ophdf5);

static DCHDF5 dchdf5;
static DISPATCHER<CMD>::INSTALL hdf5dc(&command_dispatcher, "dc", &dchdf5);

static ACHDF5 achdf5;
static DISPATCHER<CMD>::INSTALL hdf5ac(&command_dispatcher, "ac", &achdf5);

static TRHDF5 trhdf5;
static DISPATCHER<CMD>::INSTALL hdf5tr(&command_dispatcher, "transient", &trhdf5);

static FOHDF5 fohdf5;
static DISPATCHER<CMD>::INSTALL hdf5fo(&command_dispatcher, "fourier", &fohdf5);

}

