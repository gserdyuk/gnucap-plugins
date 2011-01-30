// Copyright 2006-2011 by Get2Spec Corporation - all rights reserved
//Notes:
//CJH - 01/28/2011

#include <iostream> // DELETE
#include <limits>
#include <string.h>
#include "hdf5io.h"


hdf5io::hdf5io() :
	faplist_id(-1),		file_handle(-1),
    group_handle(-1),	subgroup_handle(-1),
	header_handle(-1),	data_handle(-1),
    link_handle(-1)
{
    // Set up access protocols.
        faplist_id = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_cache(faplist_id, 0, 1048576, 4194304, 0.0);
        H5Pset_sieve_buf_size(faplist_id, 2097152);
        H5Pset_fapl_sec2(faplist_id);

        head_size = 0;
}

hdf5io::~hdf5io()
{	close_file();	}


static std::vector<std::string>locHeader;
//static std::vector<float>locData;

bool hdf5io::open_file(std::string fname)
{
//    std::cout << "Initialized" << std::endl;


//    // Hacked information
//    char* gname = "pizza";	//strcpy(gname, file_name.c_str());
//    std::string file_name = /*gname +  */"temp.hdf5";


    file_handle = H5Fcreate(fname.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, faplist_id);



//    // Checks file status: If exists then Open, if not then creates.
//    file_status = H5Fis_hdf5(file_name.c_str());
//    if (file_status == -1)	{
//            // Does not exist.
//            file_handle = H5Fcreate (file_name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, faplist_id);
//    } else {
//            file_handle = H5Fopen(file_name.c_str(), H5F_ACC_RDWR, faplist_id);
//    }

//    H5Pclose(faplist_id);


//  //  if (file_handle < 0)	{ return false; }
//    std::cout << "Create top level group" << std::endl;
//    // Creates the top level group.
//    status_univ = H5Gget_objinfo(file_handle, gname, 0, NULL);
//    if (status_univ == -1)	{
//            H5Gclose(group_handle);	// It CAN possibly have a hanging group handle, very unlikely (Impossible).
//            group_handle = H5Gcreate2(file_handle, gname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
//    } else {
//            group_handle = H5Gopen2(file_handle, gname, H5P_DEFAULT);
//    }

//    // Creates relevant subgroup within the toplevel group.
//    unsigned int j = 0;		status_univ = 0;
//    char sub_d[3];		// Maximum sub-directories is 999
//    while (status_univ != -1)
//    {
//            sprintf(sub_d, "%d", j);
//            status_univ = H5Gget_objinfo(group_handle, sub_d, 0, NULL);
//            ++j;
//    }
//    // Creates the subgroup just determined.1
//    subgroup_handle = H5Gcreate2(group_handle, sub_d, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);


    return true;
}

void hdf5io::write_head(const std::deque<std::string> &header)
{

    head_size = header.size();

    // Create header data space
    hsize_t current_head_dimensions[2] = {1, head_size};
    hsize_t maximum_head_dimensions[2] = {1, head_size};
    head_dataspace = H5Screate_simple(2, current_head_dimensions, maximum_head_dimensions);
    head_space = H5Screate_simple(2, current_head_dimensions, maximum_head_dimensions);


    // Create header data type
    datatype_head   =   H5Tcopy(H5T_C_S1);
    memspaceid_head =   H5Screate_simple(2,current_head_dimensions, maximum_head_dimensions);
    //xferplistid_head=   H5
    filespaceid_head = H5Pcreate(H5P_DATASET_CREATE);

    H5Tset_size(datatype_head,H5T_VARIABLE);
    cparms_head = H5Pcreate(H5P_DATASET_CREATE);

    header_handle = H5Dcreate2(file_handle, "header", datatype_head, memspaceid_head, H5P_DEFAULT, filespaceid_head, H5P_DEFAULT);
    H5Pclose(cparms_head);


    // Write header
    const char * buff;
    const char * ref[1];

    hsize_t t_target_size[2] = {1,1};
    hsize_t t_target_offset[2] = {0,0};
    hid_t t_dataspace_from = H5Screate_simple(2, t_target_size, NULL);
    hid_t t_dataspace_to = H5Scopy(head_space);

    for(uint jj = 0; jj < header.size(); jj++)
      {
        locHeader.push_back(header.at(jj));
        buff =  header.at(jj).c_str();
        ref[0] = buff;
        t_target_offset[1] =  jj;
        t_dataspace_to = H5Scopy(head_dataspace);
        H5Sselect_hyperslab(t_dataspace_to, H5S_SELECT_SET, t_target_offset, NULL,t_target_size, NULL);
        H5Dwrite(header_handle, datatype_head , t_dataspace_from, t_dataspace_to, H5P_DEFAULT, ref);
      }
    locHeader.clear();
   // delete[] ref;   delete buff;


    //*** Define Data Datspace information
    hsize_t current_data_dimensions[2] = {head_size, 1};
    hsize_t maximum_data_dimensions[2] = {head_size, H5S_UNLIMITED};

    data_dataspace = H5Screate_simple(2, current_data_dimensions, maximum_data_dimensions);

    datatype_data = H5Tcopy(H5T_NATIVE_FLOAT);
    H5Tset_order(datatype_data, H5T_ORDER_LE);

    hsize_t chunk_dims[2] = {head_size, 1};

    hid_t cparms_data;
    cparms_data = H5Pcreate(H5P_DATASET_CREATE);

    float fillvalue = std::numeric_limits<float>::quiet_NaN();

    H5Pset_chunk(cparms_data, 2, chunk_dims);
    H5Pset_fill_value(cparms_data, H5T_NATIVE_FLOAT, &fillvalue);
    H5Pset_fill_time(cparms_data, H5D_FILL_TIME_NEVER);

    data_handle = H5Dcreate2(file_handle,"data",datatype_data, data_dataspace, H5P_DEFAULT, cparms_data, H5P_DEFAULT);

    H5Pclose(cparms_data);
    H5Tclose(datatype_head);
    H5Sclose(t_dataspace_from);


    H5Fflush(file_handle,H5F_SCOPE_GLOBAL);
}

void hdf5io::write_data(const std::deque<float> &data)
{

     // Expands persistent offset.

    if ((data_offset % 1000 == 0))   {
        data_expansion(data_offset + 1000 );
        maxsize = maxsize + 1000;
    }


    // Creates new data carrier.
    float* t_row;
    t_row = new float[data.size()];
    for (unsigned int i = 0; i < data.size(); i++)  {
        t_row[i] = data.at(i);
    }

    hsize_t                 count[2] = {data.size(), 1};                         // size of the hyperslab in the file
    hsize_t                 offset[2] = {0, data_offset};                        // hyperslab offset in the file
    hid_t t_dataspace_t  =  H5Dget_space(data_handle);
    hid_t t_dataspace_to =  H5Scopy(t_dataspace_t);

    H5Sselect_hyperslab(t_dataspace_to, H5S_SELECT_SET, offset, NULL, count, NULL);

    hid_t t_dataspace_from = H5Screate_simple(2, count, NULL);
    datatype_data = H5Tcopy(H5T_NATIVE_FLOAT);
    H5Tset_order(datatype_data, H5T_ORDER_LE);

    H5Dwrite(data_handle, datatype_data, t_dataspace_from, t_dataspace_to, H5P_DEFAULT, t_row);

    H5Sclose(t_dataspace_from);
    H5Sclose(t_dataspace_to);
    H5Tclose(datatype_data);

    delete[] t_row; //delete data;

    data_offset++;
}

void hdf5io::data_expansion(hsize_t new_size)
{
    hsize_t current_dims[2];

    hid_t t_dataspace = H5Dget_space(data_handle);
    H5Sget_simple_extent_dims(t_dataspace,current_dims,NULL);

    if (current_dims[1] < new_size)	{
        current_dims[1] = new_size;
        H5Dset_extent(data_handle, current_dims);
        //std::cout << "TRULY EXPANDING";
    }

    if (H5Iis_valid(t_dataspace) == 0)  {
        H5Sclose(t_dataspace);
    }
    H5Fflush(file_handle, H5F_SCOPE_GLOBAL);
}

bool hdf5io::close_file()
{

    if (data_handle != -1)	{
        if (H5Iis_valid(data_handle) == 1)
        {
            // Tightens up the excess dataspace created.
            hsize_t current_dims[2];
            hid_t t_dataspace = H5Dget_space(data_handle);
            H5Sget_simple_extent_dims(t_dataspace,current_dims,NULL);
            current_dims[1] = data_offset;
            H5Dset_extent(data_handle, current_dims);

            // Resets for next run of class.
            maxsize = 0;
            data_offset = 0;
            H5Dclose(data_handle);
            H5Dclose(header_handle);

            data_handle = -1;
        }

    if (file_handle != -1)	{
        if (H5Iis_valid(file_handle) == 1)
            H5Fclose(file_handle);
        file_handle = -1;
        }
    }
    return true;
}
