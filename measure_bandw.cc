/*$Id$
 * Copyright (C) 2010 Albert Davis
 * Author: Albert Davis <aldavis@gnu.org>
 *
 * This file is part of "Gnucap", the Gnu Circuit Analysis Package
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *------------------------------------------------------------------
 */
#include "u_parameter.h"
#include "m_wave.h"
#include "u_function.h"
/*--------------------------------------------------------------------------*/
namespace {
/*--------------------------------------------------------------------------*/
/*
usage: 
measure b=bandw("vdb(out)",3)

will return bandwidth (f max - f min) where value (vdb(out)) is larger than max_value - 3

Also accepts parameters:

"probe"             - probe name
"before", "end"     - end of interval to analyze
"after", "begin"    - start of interval to analyze
"arg"               - accepted but ignored
"last"              - report last bandwidth among few available intervals (not implemented yet)
"first"             - report first bandwidth amond few available intervals (not impolemented yet)
"diff"              - gain difference toi calculate bandwidth

*/
class MEASURE : public FUNCTION {
public:
  std::string eval(CS& Cmd, const CARD_LIST* Scope)const
  {
    std::string probe_name;
    PARAMETER<double> before(BIGBIG);
    PARAMETER<double> after(-BIGBIG);
    PARAMETER<double> gain_diff(0);
    bool last = false;
    bool arg = false;

    unsigned here = Cmd.cursor();
    Cmd >> probe_name;
    Cmd >> gain_diff;
    WAVE* w = find_wave(probe_name);

    if (!w) {
      Cmd.reset(here);
    }else{
    }

    here = Cmd.cursor();
    do {
      ONE_OF
	|| Get(Cmd, "probe",  &probe_name)
	|| Get(Cmd, "before", &before)
	|| Get(Cmd, "after",  &after)
	|| Get(Cmd, "end",    &before)
	|| Get(Cmd, "begin",  &after)
	|| Set(Cmd, "arg",    &arg, true)
	|| Set(Cmd, "last",   &last, true)
	|| Set(Cmd, "first",  &last, false)
    || Get(Cmd, "diff",   &gain_diff)
	;
    }while (Cmd.more() && !Cmd.stuck(&here));

    if (!w) {
      w = find_wave(probe_name);
    }else{
    }
    
    if (w) {
      before.e_val(BIGBIG, Scope);
      after.e_val(-BIGBIG, Scope);
      gain_diff.e_val(0,Scope);
      // find max
      double time = (last) ? -BIGBIG : BIGBIG;
      double m = -BIGBIG;
      WAVE::const_iterator begin = lower_bound(w->begin(), w->end(), DPAIR(after, -BIGBIG));
      WAVE::const_iterator end   = upper_bound(w->begin(), w->end(), DPAIR(before, BIGBIG));
      for (WAVE::const_iterator i = begin; i < end; ++i) {
	    double val = i->second;
        if (val > m) {
	    time = i->first;
	    m = val;        
        } 
        else{
        }
      }  

      // find first bandwidth which satisfies  gain_max-gain_diff
      double tmin=-BIGBIG;
      double tmax=-BIGBIG;
      bool is_passband=false;       // initially is in stopband
      double minval=m-gain_diff;    // minval allowed = m(ax) - gain_diff

      WAVE::const_iterator j = begin;
      tmin=tmax=j->first;
      
      for (WAVE::const_iterator i = begin; i < end; ++i) {    // sccan range and find passband
        double val=i->second;
        if (val>minval){                     // entering passband, advance only second argument
            tmax=i->first;
            is_passband=true;
            }  
        else if (val<minval && is_passband) // returning  from passband to stopband, exit loop
            break;
        else                                // initially in stopband, both arguments equal
            tmin=tmax=i->first;
      }
      double d_time=tmax-tmin;
      return to_string(d_time);
    }
    else{
      throw Exception_No_Match(probe_name);
    }
  }
} p1;
DISPATCHER<FUNCTION>::INSTALL d1(&measure_dispatcher, "bandwidth", &p1);
/*--------------------------------------------------------------------------*/
}
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
