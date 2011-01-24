#ifndef HDF5OUT_H_
#define HDF5OUT_H_
#include <string>
#include <deque>
#include "m_wave.h"
#include "u_prblst.h"
#include "tools.h"
#include "hdf5io.h"


namespace
{

template<class T>
class hdf5out : public T
{
	hdf5io _hdf5io;
	std::string _file_name;
	bool		_need_output;

public:
	hdf5out();
	virtual ~hdf5out();
	void  do_it(CS&, CARD_LIST*);

protected:
  virtual void head(double start, double stop, const std::string& col1);
  virtual void print_results(double x);

//private:
//  void	setup(CS& in);
};

template<class T>
hdf5out<T>::hdf5out() : T(),_need_output(false)
{
	
}

template<class T>
hdf5out<T>::~hdf5out()
{
	
}

//template<class T>
//void hdf5out<T>::setup(CS& in)
//{
////	T::setup(in);
//}

template<class T>
void hdf5out<T>::head(double start, double stop, const std::string& col1)
{
	std::string t_col1 = tools::my_trim(col1," \t\n\r");

	if (T::_sim->_waves)
		delete [] T::_sim->_waves;

	T::_sim->_waves = new WAVE [T::storelist().size()];

	if (!_need_output)
		return;

	/// output
	if (!_hdf5io.open_file(_file_name))
		throw Exception_File_Open("hdf5out : " + _file_name + " - open failed ");

	std::deque<std::string> t_labels;

	if (t_col1.empty())
	{
		t_labels.push_back("#");
	} else {
		t_labels.push_back(col1);
	}

	for (PROBELIST::const_iterator p = T::printlist().begin();  p!=T::printlist().end();  ++p)
	{

		t_labels.push_back((p->label()));
	}

	if (!_hdf5io.write_head(t_labels))
		throw Exception_File_Open("hdf5out : " + _file_name + " - write failed ");

//	T::head(start,stop,col1);
}

template<class T>
void hdf5out<T>::print_results(double x)
{
	if (!_need_output)
		return;

	std::cout << " x = " << x << std::endl;
	std::deque<float> t_values;

	t_values.push_back(x);

	for (PROBELIST::const_iterator
			p=T::printlist().begin();  p!=T::printlist().end();  ++p) {

		t_values.push_back(p->value());
	}

        if (!_hdf5io.write_data(t_values))
        {
        //id hdf5out<T>::setup(CS& in);
//{
////	T::setup(in);
//}
//		std::cout << " write_data returns error" << std::endl;
		throw Exception_File_Open("hdf5out : " + _file_name + " - write failed ");
	} else {
//		std::cout << " write_data returns ok" << std::endl;
	}

//	T::print_results(x);
}

template<class T>
void hdf5out<T>::do_it(CS& cmd, CARD_LIST* cl)
{
	std::cerr << " command : " << cmd.fullstring() << std::endl;

	std::vector<std::string> res = tools::split_string(tools::my_trim(cmd.fullstring()," \t\r\n"),' ',true);
    
    
//    for (int i=0; i<res.size(); i++)
//        std::cerr << " res=" <<i<<" "<<res[i] <<std::endl;
    
// GS, 16-Jan-2010: Need to add handling of ">>" here too 
	if ((res.size() > 2) && (res.at(res.size()-2) == ">"))
	{
//		std::cerr << " we will need to handle '>'" << std::endl;
		std::cerr << " file name : " << res.at(res.size() - 1) << std::endl;

		_file_name = res.at(res.size() - 1);

		if (_file_name != "/dev/null")
			_need_output = true;
		else
			_need_output = false;

		std::cerr << " need output ? : " << _need_output << std::endl;

		std::string t_str;
		t_str += res.at(0);
		for(uint eee = 1; eee < res.size() - 2; eee++)
		{
			t_str += " ";
			t_str += res.at(eee);
		}
//		std::cerr << " new command : " << t_str << std::endl;

//		cmd = t_str;
	}


	T::do_it(cmd,cl);
	_hdf5io.close_file();
}


}
#endif /* HDF5OUT_H_ */
