//Notes:
//CJH - 01/28/2011

#ifndef hdf5out_H_
#define hdf5out_H_

#include <string>
#include <deque>
#include <pthread.h>

#include "m_wave.h"
#include "u_prblst.h"

#include "hdf5io.h"
#include "tools.h"


namespace
{

template<class T>
class hdf5out : public T
{
    hdf5io _hdf5io;
    std::string file_name;
    bool  require_print;
    bool  need_output;


public:
    hdf5out();
    virtual ~hdf5out();
    void do_it(CS&, CARD_LIST*);


protected:
    virtual void head(double start, double stop, const std::string& col1);
    virtual void print_results(double x);


};

template<class T>
hdf5out<T>::hdf5out()   : T(), need_output(false)
{   }
template<class T>
hdf5out<T>::~hdf5out()
{	}

template<class T>
void hdf5out<T>::head(double start, double stop, const std::string& col1)
{

        std::string t_col1 = tools::my_trim(col1, "\t\n\r");
        if (T::_sim->_waves)	{ delete [] T::_sim->_waves; }

        // Create new wave.
        T::_sim->_waves = new WAVE [T::storelist().size()];
        if (!need_output)	{ return; } // LOCATE THIS FUNCTION

       if (!_hdf5io.open_file(file_name) == true)
            throw Exception_File_Open("hdf5out file could not be opened.");


        std::deque<std::string> t_labels;	// Define holder for headers.

        if (t_col1.empty())
                {	t_labels.push_back("#");	}
          else
                {	t_labels.push_back(t_col1);	}


        for (PROBELIST::const_iterator p = T::printlist().begin(); p!=T::printlist().end(); ++p)
        {
                t_labels.push_back((p->label()));
//              std::cout << t_labels[j] << std::endl;
        }
        // Writes the headers
         _hdf5io.write_head(t_labels);
         t_labels.clear();
}


template<class T>
void hdf5out<T>::print_results(double x)
{ 
    std::deque<float>t_data;

    t_data.push_back(x);

    for (PROBELIST::const_iterator p=T::printlist().begin();
            p!=T::printlist().end(); ++p)
    {
        t_data.push_back(p->value());
    }
    _hdf5io.write_data(t_data);
    t_data.clear();
}

template<class T>
void hdf5out<T>::do_it(CS& cmd, CARD_LIST* cl)
{    
    std::vector<std::string> res = tools::split_string(tools::my_trim(cmd.fullstring()," \t\r\n"),' ',true);
         // Manage the gnucap input strings.
        if ( (res.size() > 2) && (res.at(res.size()-2)==">") )
        {
                file_name = res.at(res.size() - 1);	// Strip out binary end char.

                // Special conditions...
                if (file_name != "/dev/null")
                        {	need_output = true;	}
                 else
                        {	need_output = false;		}

                std::string t_string;	t_string+= res.at(0);
                for (unsigned int e = 1; e < res.size() - 2; e++)	{
                        t_string += " "; //std::deque<float> data_set;
                        t_string += res.at(e);
                }
        }
        T::do_it(cmd, cl);
        std::cout << "ITS GETTING HERE before close file" << std::endl;
        _hdf5io.close_file();
        std::cout << "ITS GETTING HERE after close file" << std::endl;
}

};




#endif
