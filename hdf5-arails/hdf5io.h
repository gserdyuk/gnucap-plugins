#ifndef HDF5IO_H_
#define HDF5IO_H_

#include <deque>
#include <vector>
#include <string>
#include <cstdio>
#include <hdf5.h>

class hdf5io
{

	hid_t 	_hdf5_file_handle;
    hid_t   _hdf5_group_handle;
    hid_t   _hdf5_subgroup_handle;
	hid_t 	_hdf5_data_dataset;

	hid_t   _link_handler;
	htri_t  file_status;
	herr_t  status;
	size_t	_header_size;
	size_t  _data_offset;


public:
	hdf5io();
	virtual ~hdf5io();

	bool	open_file(std::string fname);

	bool	write_head(const std::deque<std::string> &header);
	bool	write_data(const std::deque<float> &data);
	bool	close_file();

protected:
	bool    expand_data(hsize_t estimated_size);
};

#endif /* HDF5IO_H_ */
