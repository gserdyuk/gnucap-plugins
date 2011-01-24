//#include <stdlib.h>

#include <limits>
#include <iostream>
#include <string.h>
#include <fstream>
#include "hdf5io.h"

#include <stdio.h>
#include <stdlib.h>


//using namespace std;
hdf5io::hdf5io() : _hdf5_file_handle(-1),_hdf5_data_dataset(-1),_data_offset(-1),_hdf5_group_handle(-1), _hdf5_subgroup_handle(-1), _link_handler(-1)//_hdf5_header_handle(-1),_hdf5_data_handle(-1)
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
    /* Fname hack.
       

    */
    char* gname;
    gname = new char[fname.length()+1];
    strcpy(gname, fname.c_str());
    
    fname = "file1.hdf5";
    
	// Sets the cache for the file.
	hid_t faplist_id = H5Pcreate(H5P_FILE_ACCESS);          // 128 Mb chunks cache, 16M elements
	H5Pset_cache( faplist_id, 0, 16777216, 134217728, 1.0); // sieve cache size - 16Mb
	H5Pset_sieve_buf_size(faplist_id,16777216);
	H5Pset_fapl_sec2(faplist_id);

   // Checks for existence of file.  
    file_status = H5Fis_hdf5(fname.c_str());
    std::cout << "FILE STATUS: "<<file_status << std::endl;
    if (file_status == -1)   {
        // False - File does not exist: Create new one.
        std::cout << "It is 0" << std::endl;
    	_hdf5_file_handle = H5Fcreate(fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, faplist_id);
    }   else {
        // True - File already exists: Open it.
        if (_hdf5_file_handle == -1)   {
            std::cout << "It is not 0" << std::endl;
            _hdf5_file_handle = H5Fopen(fname.c_str(), H5F_ACC_RDWR, faplist_id);       
        }
    }
	H5Pclose(faplist_id);

	if (_hdf5_file_handle < 0)
	{	return false;	}


    // Creates toplevel group.
    status = H5Eset_auto2(NULL, NULL, NULL);
    status = H5Gget_objinfo(_hdf5_file_handle, gname, 0, NULL);
    if (status == -1)    {
        H5Gclose(_hdf5_group_handle);
        // False case - Group Does not Exist
        _hdf5_group_handle = H5Gcreate2(_hdf5_file_handle, gname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        //std::cout << "MADE IT TO 1" << std::endl;     // DIAGNOSTICS
    } else  {
        // True case - Group Exists
        _hdf5_group_handle = H5Gopen2(_hdf5_file_handle, gname, H5P_DEFAULT);
        std::cout << "Group: MADE IT TO 0" << std::endl;     // DIAGNOSTICS 
    }
      
    // Creates subgroups within toplevel group.        
    unsigned int j = 0;
    char st [3];        // Maximum subdirectories is 999
    char final[9];
    status = H5Gget_objinfo(_hdf5_file_handle, gname, 0, NULL);
   
    /* 
    for (int j=0; status!=-1; j++)  {
        sprintf(st, "%i", j);
        status = H5Gget_objinfo(_hdf5_file_handle, st , 0, NULL);
        std::cout << "THE CHAR IS: "<< j << std::endl;   // DIAGNOSTICS
    }
    */
    
    status = 0;  
    while (status != -1) 
    {                   
        sprintf(st, "%d", j);
        status = H5Gget_objinfo(_hdf5_group_handle, st , 0, NULL);
        //std::cout << "THE CHAR IS: "<< status << std::endl;   // DIAGNOSTICS
        ++j;       
    }    
    _hdf5_subgroup_handle = H5Gcreate2(_hdf5_group_handle, st , H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    
    
    // Creates hard link, first creates hard link directory, link_bin  *** May not be implemented for directories.
    status = H5Gget_objinfo(_hdf5_file_handle, "link_bin", 0, NULL);
    if (status == -1)   {
        _link_handler = H5Gcreate2(_hdf5_file_handle,"link_bin", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    } else {
        _link_handler = H5Gopen2(_hdf5_file_handle, "link_bin", H5P_DEFAULT);
    }
    
    //H5Lcreate_hard(_hdf5_group_handle, st, _link_handler, "ac_0_dir", H5P_DEFAULT, H5P_DEFAULT);
    

	return true;
}

bool	hdf5io::write_head(const std::deque<std::string> &header)
{
	_header_size = header.size();

    /* Diagnostics (remove in production)
    std::cout << "HeaderSize: " << _header_size << std::endl;
	std::cout << "!!!HERE THEY ARE: " <<header[0] <<  ", " << header[1] << ", " <<  header[2] << ", " << std::endl;
    */
    
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

    /*
    // Gets the directory to work with.
    std::ifstream f;
    f.open(".sim.info", std::ios::in);    
    char* directory = "ac/0";
    f.close();
    */
    
    // **** PUT HARD LINK HERE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    hid_t header_dataset = -1;
    header_dataset = H5Dcreate2(_hdf5_subgroup_handle,"header",datatype_head,head_dataspace,H5P_DEFAULT, cparms_head, H5P_DEFAULT);
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

    // **** PUT HARD LINK HERE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    _hdf5_data_dataset = H5Dcreate2(_hdf5_subgroup_handle,"data",datatype_data,data_dataspace,H5P_DEFAULT, cparms_data, H5P_DEFAULT);
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
//    H5Fclose(_hdf5_file_handle);
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
