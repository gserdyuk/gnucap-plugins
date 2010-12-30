/*$Id$ 
 * Copyright (C) 2010 Gennadiy Serdyuk
 * Author: Gennadiy Serdyuk <gserdyuk@gserdyuk.com>
 *
 * This file is plugin to "Gnucap", the Gnu Circuit Analysis Package
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

#include "c_comand.h"
#include "d_subckt.h"

//#include "u_status.h"
//#include "d_dot.h"
//#include "d_coment.h"
//#include "u_lang.h"

/*--------------------------------------------------------------------------*/
class CMD_ECHO : public CMD {
        
public:
  void do_it(CS& cmd, CARD_LIST* Scope)
  {
    IO::mstdout<<".echo ";
    std::string msg;
    while (cmd >> msg)
          IO::mstdout<<msg<<" ";
  IO::mstdout<<"\n";
  }
} p1001;
DISPATCHER<CMD>::INSTALL d1001(&command_dispatcher, ".echo", &p1001);
/*--------------------------------------------------------------------------*/
class CMD_PRINTPARAM : public CMD {
        
public:
  void do_it(CS& cmd, CARD_LIST* Scope)
  {
    PARAMETER<double> param;
    std::string value;
    
    IO::mstdout<<".printparam ";
    while (! cmd.is_end()) {
        if (cmd.is_float()) {		// simple unnamed value
            cmd >> value;
        }
        else {	// another value
            cmd >> value; // strips off the quotes
            value = '{' + value + '}'; // put them back
        }
        IO::mstdout<<value<<"= ";
        param=value;
        IO::mstdout<<param.e_val(NOT_INPUT, Scope)<<" \n";
    }
    IO::mstdout<<"";
  }
} p1002;
DISPATCHER<CMD>::INSTALL d1002(&command_dispatcher, ".printparam", &p1002);
