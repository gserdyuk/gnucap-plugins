#include "s_ac.cc"

namespace {


class NOISE : public AC {

enum NoiseRelType  {STD=1, RELVAL=2, RELVOL=3};  /* how calculate gain? as STD: Vout, RELVAL:Vout/source value or RELVOL: Vout/Vin*/ 
enum NoiseOutType  {POW=1, VOLT=2};         /* how to output noise? as v^2/hz (POW) or as volt / sqrt(hz) VOLT) */

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

  //PARAMETER<int> _onode1; 		// node where to measure noise
  //PARAMETER<int> _onode2;       // reference node, default =0
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
  void alloc_vectors();
  void unalloc_vectors();
  void init();
public:
  // debug and dev functions
  void outport_source();  
  void print_ac();
  void print_noise_ac();
private:
   NoiseRelType _noise_rel_t;
   NoiseOutType _noise_out_t;
};

};
