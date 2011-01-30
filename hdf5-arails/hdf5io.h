
//Notes:
//CJH - 01/28/2011    The IO class is responsible for handling functions on the HDF5 end.  All data out is performed here.
//        Header writes and data writes are separated into two unique functions because this is how they are done in gnucap.
//        Most of the persistent information is initialized in the open_file; and all persistence is closed in close_file()
//        unless it would have other impact on the programs or efficiency.
//        Functions in this class are called from a template class in hdf5out.h.  Names are closely related between the classes.


#ifndef hdf5io_H_
#define hdf5io_H_

#include <cstdio>
#include <string>
#include <deque>
#include <vector>

#include <hdf5.h>


class hdf5io
{

	// Defines necessary handles
    hid_t 	faplist_id;
    hid_t	file_handle, group_handle, subgroup_handle;
    size_t  head_size;

	// Header Information
	hid_t 	header_handle;
    hid_t 	datatype_head, memspaceid_head, xferplistid_head, filespaceid_head;

	hssize_t head_space;
	size_t header_offset;


    hsize_t current_head_dimensions, maximum_head_dimensions;
	hid_t head_dataspace;
    hid_t cparms_head;

	
	// Dataset Information
	hid_t 	data_handle;
    hid_t 	datatype_data, memspaceid_data, xferplistid_data, filespaceid_data;

	hssize_t data_space;
	size_t 	data_offset;	
    size_t maxsize;

	hsize_t current_data_dimensions, maximum_data_dimensions;
	hid_t data_dataspace;	
    hid_t cparms_data;


	// Assorted Handlers
	hid_t	link_handle;

	htri_t	file_status;
	herr_t	status_univ;	// This status holder is used almost universally



	// File information -- Local Information.
//	char* file_name = "";
//	char* directory = "";
//	char* test_type = "";

	

public:
    hdf5io();
    virtual ~hdf5io();
    bool	open_file(std::string file_name);
    void	write_head(const std::deque<std::string> &header);
    void	write_data(const std::deque<float> &data);
    bool	close_file();

protected:
    void	data_expansion(hsize_t new_size);

};






#endif
