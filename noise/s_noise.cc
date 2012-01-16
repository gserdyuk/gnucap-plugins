/*$Id
 * noise analysis top
 *   
 */

//
/*
#include "u_sim_data.h"
#include "u_status.h"
#include "u_parameter.h"
#include "u_prblst.h"
#include "s__.h"
#include "s_ac.h"
*/

#include "e_node.h"     // GS - needed by setup (nodes)
#include "u_nodemap.h"
#include "l_lib.h"


#include "s_ac.cc"

/*--------------------------------------------------------------------------*/
namespace {


class NOISE : public AC {

/* how calculate gain? as STD: Vout, RELVAL:Vout/source value or RELVOL: Vout/Vin*/ 
enum NoiseRelType  {STD_NoiseType=1, RELVAL_NoiseType=2, RELVOL_NoiseType=3};  

/* how to output noise? as v^2/hz (POW) or as volt / sqrt(hz) VOLT) */
enum NoiseOutType  {POW_NoiseOutType=1, VOLT_NoiseOutType=2};         

public:
  void	do_it(CS&, CARD_LIST*);

  explicit NOISE(): AC() {}

  ~NOISE() {}
private:
  explicit NOISE(const NOISE&):AC() {unreachable(); incomplete();}
protected:
  virtual void	sweep();
  //void	first();  //GS - inheriting
  //bool	next();   //GS - inheriting
  		  void	solve();
  		  void	clear();
  virtual void	setup(CS&);
private:

  std::string _onode1_name; 		// node where to measure noise
  std::string _onode2_name;         // reference node, default =0

  std::string _source_name;	   // source to where noise has to be reduced to
  COMPLEX *noise_ac;		// noise sources of  individual elements: N_nodes X N_elements
  double inoise_total;		// like spice 
  double onoise_total; 		// like spice 
  double inoise_density;	// like spice 
  double onoise_density; 	// like spice 
  int _onode1_map;
  int _onode2_map;			// mapped (internal numbering) _onode1 and _onode2
  int _inode1_map;
  int _inode2_map;			// mapped (internul numbering) nodes of source "_source_name"
    
  // tmp
public:
  // debug and dev functions
  void outport_source();  
  void print_ac();
  void print_noise_ac();
private:
   NoiseRelType _noise_rel_t;
   NoiseOutType _noise_out_t;
};

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

/* use it
Integral a;

in loop:
	a.integrateTrapez(x,y);
	or 
	a.integrateStep(x,y);

	...
	a.value();
to re-use:
	a.reset();
*/
class Integral {
private:
	double x_;
	double y_;
	double val_;
	bool isfirst_;
public:
	Integral():x_(0),y_(0),val_(0),isfirst_(true){};
	~Integral(){};
	double integrateTrapez(double x, double y);
	double integrateStep(double x, double y);
	double value(){ return val_;};
	void reset(){isfirst_=true;}
};

double Integral::integrateTrapez(double x, double y){
    if (isfirst_){
		val_=0;
		isfirst_=false;
		}
	else
		val_+=(x-x_)*(y+y_)/2;
	x_=x;
	y_=y;
	return val_;
	}
	
double Integral::integrateStep(double x, double y){
    if (isfirst_){
		val_=0;
		isfirst_=false;
		}
	else
		val_+=(x-x_)*y;
	x_=x;
	y_=y;
	return val_;
	}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void NOISE::outport_source(){  
	//fills noise_ac at output nodes
	noise_ac[_onode1_map] = 1.;
	noise_ac[_onode2_map] =-1.;
}
/*--------------------------------------------------------------------------*/
// GS: TODO theese two need 
// OMSTREAM& operator<<(COMPLEX x)

void NOISE::print_ac(){
	int N= _sim->_total_nodes+1;
	for (int i=0; i<N; i++){
		_out<<"ac["<<i<<"]=("<<real(_sim->_ac[i])<<","<<imag(_sim->_ac[i])<<")\n";
		}
}
/*--------------------------------------------------------------------------*/
void NOISE::print_noise_ac(){
	int N= (_sim->_total_nodes+1);
	for (int i=0; i<N; i++){
		_out<<"noise_ac["<<i<<"]=("<<real(noise_ac[i])<<","<<imag(noise_ac[i])<<")\n";
		}
}
/*--------------------------------------------------------------------------*/
void NOISE::do_it(CS& Cmd, CARD_LIST* Scope)
{
  _scope = Scope;
  _sim->set_command_noise();     
  reset_timers();
  //::status.ac.reset().start();		// put proper call todo - same here
  _sim->init();				
  _sim->alloc_vectors();		        // alloc vectors for noise ; AC::alloc_vectors();
  assert(!noise_ac);
  noise_ac = new COMPLEX[(_sim->_total_nodes+1)];

  _sim->_acx.reallocate();
  _sim->_acx.set_min_pivot(OPT::pivtol);
  
  setup(Cmd);
  
  ::status.set_up.stop();
  switch (ENV::run_mode) {
  case rPRE_MAIN:	unreachable();	break;
  case rBATCH:		untested();
  case rINTERACTIVE:untested();     // todo test yet
  case rSCRIPT:		sweep();		break;
  case rPRESET:		break;          // nothing
  }
  _sim->_acx.unallocate();

  _sim->unalloc_vectors();		// unalloc vectors for NOISE; AC::unalloc_vectors();
  assert(noise_ac);
  delete [] noise_ac;
  noise_ac=NULL;

  //::status.ac.stop();		    // todo
  //::status.total.stop();      // todo

}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
void NOISE::setup(CS& Cmd)
{

  // GS: read _out__node, _out_ref_node and _source_name here  
  // format:  v(1[,3]) Vin .........
  //int node1,node2;
  
  std::string name1,name2;
  
  Cmd>>name1;								// got V
  Cmd.skip();		//  "(" 				// got (
  Cmd>>_onode1_name;					    // got node1 and (optionally) ","
  if ( Cmd.is_digit() )   					// if num follows
  	Cmd>>_onode2_name; 							// got node2
  else
  	_onode2_name="0";
  Cmd.skip();								// got ")"
  Cmd>>name2;								// got SourceName
	  
  // finally assign what did we find
  _source_name=name2;
  
  _noise_rel_t=STD_NoiseType;       // others are possible but not fully implemented
  _noise_out_t=POW_NoiseOutType;  
  
  AC::setup(Cmd);
  _out.setformat(ftos_EXP);  // _out created in AC::setup(); constant is from l__lib.h

  // now extract information regarding source (where to refer noise to)
  // A extract nodes and use them to have Vin voltage
  CARD_LIST::iterator e= CARD_LIST::card_list.find_(_source_name);

  // TODO! check if e!=0 and raise exception if e==0 (it means that no device named "_source_name")
  // TODO! check if (*e) is a pointer to valid source. if not - raise exception.
   
  assert(e != CARD_LIST::card_list.end());   
  _inode1_map=(*e)->n_(0).m_();	//_n[OUT1].m_();
  _inode2_map=(*e)->n_(1).m_();	//_n[OUT2].m_();

  assert(_inode1_map!=_inode2_map);   // otherwize K in solve() will be zero
	
  // TODO!: check if nodes _onode1 and _onode2 valid and rize exception if those nodes are not valid 
  //_onode1_map=_sim->_nm[int(_onode1)];
  //_onode2_map=_sim->_nm[int(_onode2)];
   
  // handle errors - todo yet
  _onode1_map=CARD_LIST::card_list.nodes()->operator[](_onode1_name)->m_();
  _onode2_map=CARD_LIST::card_list.nodes()->operator[](_onode2_name)->m_();
     
}
/*--------------------------------------------------------------------------*/
void NOISE::solve()
{

  _sim->_acx.zero();
  std::fill_n(_sim->_ac, _sim->_total_nodes+1, 0.);
  std::fill_n(noise_ac,  _sim->_total_nodes+1, 0.);

  //::status.load.start();				//todo  - fix call
  _sim->count_iterations(iTOTAL);
  CARD_LIST::card_list.do_ac();
  CARD_LIST::card_list.ac_load();

  outport_source();                     //  fill noise_ac
  
  // _sim->_acx.printall(); 
  // print_ac();
  // print_noise_ac();
  
  //::status.load.stop();				//todo

  ::status.lud.start();				
  _sim->_acx.lu_decomp();
  ::status.lud.stop();				
  
  //_sim->_acx.printpattern();
  //_sim->_acx.printall();
  
  
  ::status.back.start();
  _sim->_acx.fbsub(_sim->_ac);			// here are sources of AC. for noise use adjoint system
  ::status.back.stop();
  // std::cout<<"ac solved\n";  
  // print_ac();
  
  // use transposed acx and solve adjoint system
  //::status.back.start();  		//todo - fix call
  _sim->_acx.fbsubt(noise_ac);		
  //::status.back.stop();			//todo - fix call
  //std::cout<<"noise_ac solved\n";  
  //print_noise_ac();
  
  // TODO HERE  multiply noise_ac onto matrix, get initial result               !!!!
  // this will allow to check if solution is correct
  
  
  // calculate noise 
  // put formulas and reference here	//todo
  double noisepow=0;
  CARD_LIST::card_list.do_noise(noisepow, noise_ac);   // todo! - here - implement noise in different models
                        /*
                        implemented:
                        CARD        - virtual, returns zero
                        R           - resistor
                        cardlist    - set of devices
                        subckt      - SUBCKT, uses cardlist
                        dev_spice   - all spice models 
                        to update: "grep -I do_noise * "
                        */	
  double k;
  if (_noise_rel_t==STD_NoiseType){           // -- onl;y this one is fully implemneted
    // k- transfer out node to input voltage, assume input=1   
    k=abs( (_sim->_ac[_onode1_map]-_sim->_ac[_onode2_map]) / 1.  );  	
    }
  else if (_noise_rel_t==RELVOL_NoiseType){
    // k- transfer out node to input voltage   
    k=abs( (_sim->_ac[_onode1_map]-_sim->_ac[_onode2_map])/(_sim->_ac[_inode1_map]-_sim->_ac[_inode2_map])  );  	
    }
  else if (  _noise_rel_t==RELVAL_NoiseType){
    double inval =1;  // FIXIT - read input value from device
    // k- transfer out node to input voltage , need to read inval from device
    k=abs( (_sim->_ac[_onode1_map]-_sim->_ac[_onode2_map]) / inval  );  	
    }
  else {unreachable();}

  inoise_density=noisepow/k/k;
  onoise_density=noisepow;

  return;
} 

/*--------------------------------------------------------------------------*/
void NOISE::sweep()
{
  head(start(), stop(), "Freq"); // SIC! Allocates WAVE
  
  first();

  inoise_total=onoise_total=0.;
  
  CARD_LIST::card_list.ac_begin();   // need to keep that, as noise involves AC
  //  but also may need to add another method -  noise parameters - decide when will work with MOSFETs  // todo
  
  // for integration. made fast and dirty
  Integral inoiz;
  Integral onoiz;

  do {
    _sim->_jomega = COMPLEX(0., _sim->_freq * M_TWO_PI);
	solve();
    _sim->_inoise=inoise_density;
    _sim->_onoise=onoise_density;
    outdata(_sim->_freq);
    
	// integrate noise over band
	inoiz.integrateTrapez(_sim->_freq,inoise_density);   	//inoiz.integrateStep(_sim->_freq,inoise_density);
	onoiz.integrateTrapez(_sim->_freq,onoise_density);      //onoiz.integrateStep(_sim->_freq,onoise_density);

  } while (next());
  
  _sim->_inoise_tot=inoiz.value();
  _sim->_onoise_tot=onoiz.value();

   command("measure inoise_total=eval("+std::string(ftos(inoiz.value(),0,10,ftos_EXP ))+")", 
            &CARD_LIST::card_list); 
   command("measure onoise_total=eval("+std::string(ftos(onoiz.value(),0,10,ftos_EXP ))+")", 
            &CARD_LIST::card_list);
} 

/*--------------------------------------------------------------------------*/
static NOISE pn1;
static DISPATCHER<CMD>::INSTALL dn1(&command_dispatcher, "noise", &pn1);
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
