#include <limits>
#include <iostream>
#include "hdf5io.h"

hdf5io::hdf5io() : _hdf5_file_handle(-1),_hdf5_data_dataset(-1),_data_offset(-1)//_hdf5_header_handle(-1),_hdf5_data_handle(-1)
{
	// TODO Auto-generated constructor stub
	_header_size = 0;
}

hdf5io::~hdf5io()
{
	// TODO Auto-generated destructor stub
	close_file();

}

static std::vector<std::string> locHeader;

bool	hdf5io::close_file()
{
//	if (_hdf5_data_handle != -1)
//	{
//		if (H5Iis_valid(_hdf5_data_handle) == 1)
//			H5Gclose(_hdf5_data_handle);
//
//		_hdf5_data_handle = -1;
//	}

//	if (_hdf5_header_handle != -1)
//	{
//		if (H5Iis_valid(_hdf5_header_handle) == 1)
//			H5Gclose(_hdf5_header_handle);
//
//		_hdf5_header_handle = -1;
//	}
//

	if (_hdf5_data_dataset != -1)	{
		if (H5Iis_valid(_hdf5_data_dataset) == 1)
			H5Dclose(_hdf5_data_dataset);
		_hdf5_data_dataset = -1;
	}

	if (_hdf5_file_handle != -1)	{
		if (H5Iis_valid(_hdf5_file_handle) == 1)
			H5Fclose(_hdf5_file_handle);
		_hdf5_file_handle = -1;
	}

	return true;
}

bool	hdf5io::open_file(std::string fname)
{
	// Sets the cache for the file.
	hid_t faplist_id = H5Pcreate(H5P_FILE_ACCESS);          // 128 Mb chunks cache, 16M elements
	H5Pset_cache( faplist_id, 0, 16777216, 134217728, 1.0); // sieve cache size - 16Mb
	H5Pset_sieve_buf_size(faplist_id,16777216);
	H5Pset_fapl_sec2(faplist_id);

	_hdf5_file_handle = H5Fcreate(fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, faplist_id);

	H5Pclose(faplist_id);

	if (_hdf5_file_handle < 0)
	{	return false;	}


//	// creating groups
//	_hdf5_header_handle = H5Gcreate2(_hdf5_file_handle, "header", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//	if (_hdf5_header_handle < 0)
//	{
//		close_file();
//		return false;
//	}
//
//	_hdf5_data_handle = H5Gcreate2(_hdf5_file_handle, "data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//	if (_hdf5_data_handle < 0)
//	{
//		close_file();
//		return false;
//	}

	return true;
}

bool	hdf5io::write_head(const std::deque<std::string> &header)
{
	_header_size = header.size();

    /* Diagnostics (remove in production)
    std::cout << "HeaderSize: " << _header_size << std::endl;
	std::cout << "!!!HERE THEY ARE: " <<header[0] <<  ", " << header[1] << ", " <<  header[2] << ", " << std::endl;
    */
    
//	if (H5Iis_valid(_hdf5_data_handle) != 1)    // I think these are here to check the data handle...but why?
//	{
//		close_file();
//		return false;
//	}

    if (H5Iis_valid(_hdf5_file_handle) != 1)	{
		close_file();
		return false;
	}

    // Sets the dimensions of header/data.  
	hsize_t current_dims_head[2] = {1, _header_size};
	hsize_t max_dims_head[2] = {1, _header_size};
	hsize_t current_dims_data[2] = {_header_size,1};
	hsize_t max_dims_data[2] = {_header_size, H5S_UNLIMITED};

	hid_t head_dataspace = H5Screate_simple(2, current_dims_head, max_dims_head);
	hid_t data_dataspace = H5Screate_simple(2, current_dims_data, max_dims_data);

    // Tests created data slabs.
	if ((head_dataspace < 0) || (data_dataspace < 0))	{
		close_file();
		return false;
	}

	hid_t datatype_data = H5Tcopy(H5T_NATIVE_FLOAT);
	hid_t datatype_head = H5Tcopy(H5T_C_S1);
	H5Tset_size(datatype_head,H5T_VARIABLE);
	H5Tset_order(datatype_data, H5T_ORDER_LE);

	// Creates the dataset
	hsize_t      chunk_dims[2] ={_header_size, 1};
	hid_t cparms_head, cparms_data;
	
    cparms_head = H5Pcreate(H5P_DATASET_CREATE);
    cparms_data = H5Pcreate(H5P_DATASET_CREATE);

    float fillvalue = std::numeric_limits<float>::quiet_NaN();
    H5Pset_chunk( cparms_data, 2, chunk_dims);
    H5Pset_fill_value (cparms_data, H5T_NATIVE_FLOAT, &fillvalue );
    H5Pset_fill_time(cparms_data, H5D_FILL_TIME_NEVER  );

    hid_t header_dataset = -1;
    header_dataset = H5Dcreate2(_hdf5_file_handle,"header",datatype_head,head_dataspace,H5P_DEFAULT, cparms_head, H5P_DEFAULT);
    H5Pclose(cparms_head);

    // Tests.
	if (header_dataset < 0)
	{
		H5Tclose(datatype_head);
		H5Tclose(datatype_data);
	    H5Pclose(cparms_data);
	    H5Sclose(head_dataspace);
	    H5Sclose(data_dataspace);
	    close_file();
		return false;
	}

    _hdf5_data_dataset = H5Dcreate2(_hdf5_file_handle,"data",datatype_data,data_dataspace,H5P_DEFAULT, cparms_data, H5P_DEFAULT);
    H5Pclose(cparms_data);

	if (_hdf5_data_dataset < 0)
	{
		H5Tclose(datatype_data);
		H5Tclose(datatype_head);
	    H5Sclose(head_dataspace);
	    H5Sclose(data_dataspace);
		return false;
	}



	H5Tclose(datatype_data);
    H5Sclose(data_dataspace);

    const char * buff;
    const char * ref[1];
    // write header
    hsize_t t_target_size[2] = {1,1};              /* size of the hyperslab in the file */
    hsize_t t_target_offset[2] = {0,0};
    hid_t t_dataspace_from = H5Screate_simple(2, t_target_size, NULL);
    hid_t t_dataspace_to = H5Scopy(head_dataspace);// = H5Scopy(t_dataspace_t);

//
// All of the names in the .print statement are sent here first. we can save
// these locally in this file.
// 
    
    for(uint jj = 0; jj < header.size(); jj++)
    {
		locHeader.push_back(header.at(jj));
		// std::cout << "Header: " << header.at(jj) << std::endl; // Diagnostics.
    	buff =  header[jj].c_str(); //header.at(jj).c_str();
    	ref[0] = buff;
    	t_target_offset[1] =  jj;
    	t_dataspace_to = H5Scopy(head_dataspace);
    	H5Sselect_hyperslab(t_dataspace_to, H5S_SELECT_SET, t_target_offset, NULL,t_target_size, NULL);
    	H5Dwrite(header_dataset, datatype_head , t_dataspace_from, t_dataspace_to, H5P_DEFAULT, ref);
        H5Sclose(t_dataspace_to);
    }
      
    // Closeout the opened associations.
	H5Tclose(datatype_head);
    H5Sclose(head_dataspace);
    H5Sclose(t_dataspace_from);
    H5Dclose(header_dataset);

    H5Fflush(_hdf5_file_handle,H5F_SCOPE_GLOBAL);

	return true;
}

bool	hdf5io::write_data(const std::deque<float> &data)
{
//
// Added this here, header info was saved locally. so we do have access to
//  the header data for entire simulation run.
	
    /* Diagnostics
    //std::cout << "HeaderSize: " << _header_size << std::endl;
    for(uint jj = 0; jj < locHeader.size(); jj++)
    {
		//std::cout << "Before Data: " << locHeader.at(jj) << ",";
	}
	std::cout << std::endl;
	*/

	if (data.size() != _header_size)    {
		return false;
    }
	if (H5Iis_valid(_hdf5_data_dataset) != 1)// || (mDataspace < 0))
	{
		return false;
	}

	if (H5Iis_valid(_hdf5_file_handle) != 1)// || (mDataspace < 0))
	{
		return false;
	}

	
	// The data slab is being expanded to 1+offset.  Which likely means the dataslab ends as one row larger than dataset.
//	std::cout << " _data_offset = " << _data_offset << std::endl;   // Diagnostics
    _data_offset++; 
	bool ret = expand_data(_data_offset+1); // Expands the data slab to accomodate new row.
//	std::cerr << " expand_data returns " << ret << std::endl;       // Diagnostics
    // Creates a data carrier array (t_row) which is loaded from the deque data and is used to write out.
	float t_row[data.size()];
	for(uint ii = 0; ii < data.size(); ii++)	
	{
		t_row[ii] = data.at(ii);
	}

	hsize_t      count[2] = {data.size(),1};                /* size of the hyperslab in the file */
    hsize_t      offset[2] = {0,_data_offset};              /* hyperslab offset in the file */
	hid_t t_dataspace_t = H5Dget_space(_hdf5_data_dataset);
    hid_t t_dataspace_to = H5Scopy(t_dataspace_t);

    // Tests the validity of the dataspace to be written to.
	if (H5Iis_valid(t_dataspace_to) != 1)// || (mDataspace < 0))
	{	return false;	}

	H5Sselect_hyperslab(t_dataspace_to, H5S_SELECT_SET, offset, NULL,count, NULL);

	hid_t t_dataspace_from = H5Screate_simple(2, count, NULL);
	// Tests the validity of the dataspace.
	if (H5Iis_valid(t_dataspace_from) != 1)// || (mDataspace < 0))
	{	return false;	}

	hid_t datatype = H5Tcopy(H5T_NATIVE_FLOAT);
	H5Tset_order(datatype, H5T_ORDER_LE);
	H5Dwrite(_hdf5_data_dataset, datatype , t_dataspace_from, t_dataspace_to, H5P_DEFAULT, t_row);

	// TODO update max / min    // Max/min of what?
	if (H5Iis_valid(t_dataspace_from) == 0)
		H5Sclose(t_dataspace_from);
	if (H5Iis_valid(t_dataspace_to) == 0)
		H5Sclose(t_dataspace_to);
	if (H5Iis_valid(datatype) == 0)
		H5Tclose(datatype);

	return true;
}

bool hdf5io::expand_data(hsize_t estimated_size)
{

	if (H5Iis_valid(_hdf5_data_dataset) != 1)// || (mDataspace < 0))
	{	return false;	}

	hsize_t current_dims[2];
	//    	hsize_t max_dims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};   // Experiment to test paramater passing?

	hid_t t_dataspace = H5Dget_space(_hdf5_data_dataset);
	H5Sget_simple_extent_dims(t_dataspace,current_dims,NULL);
	//    	qDebug() << " current dims = " << current_dims[0] << " , " << current_dims[1];  // Was used in previous diagnostics?

	if (current_dims[1] < estimated_size)
	{
		current_dims[1] = estimated_size;
		H5Dset_extent(_hdf5_data_dataset,current_dims);
	}

	if (H5Iis_valid(t_dataspace) == 0)
		H5Sclose(t_dataspace);

	return true;
}
