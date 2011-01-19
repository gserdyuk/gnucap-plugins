#include <string>
#include <iostream>
#include "hdf5out.h"
#include "hdf5.h"

template <class T>
hdf5out<T>::hdf5out() : f_handle(-1), g_handle(-1), sg_handle(-1), dsp_handle(-1), dset_handle(-1)
{

}
template <class T>
hdf5out<T>::~hdf5out()
{

}

template <class T>
bool hdf5out<T>::location_management(hid_t f_handle ,char* fname, char* group)
{
    int c = 0;   // This holds the check-back flags at different time.

    //! Creates file.
    // Defines properties...
    hid_t fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_cache(fapl_id, 0, 16777216, 134217728, 1.0);    // 128mb chunks, with 16M elements.
    H5Pset_sieve_buf_size(fapl_id,1677216);                // Set sieve to match that set in Cache (16M)
    H5Pset_fapl_sec2(fapl_id);                             // Choose a driver to access file.

    // If exists opens R-RW if not, creates.
    f_handle = H5Fcreate(fname,H5F_ACC_TRUNC,fapl_id,H5P_DEFAULT);
    if (c < 0)  {return false;}

    //! Groups....
    // Creates first level group, "op, tran, ac..."
    c = H5Gopen(f_handle,group,fapl_id);
    if (c < 0)  {
        // Create group.
        H5Gcreate(f_handle,group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    }
    g_handle = H5Gopen(f_handle,group,fapl_id);
    // Creates second level group, "0, 1, 2..."
    // In order to do this, it iterates over each group until opening a group returns false (-1)
    int a=-1;
    char* b;
    while (c>-1) {
        a++;
        b = group + '/' + char(a);
        c = H5Gopen(f_handle, b, H5P_DEFAULT);
    }
    sg_handle = H5Gopen(f_handle,b, H5P_DEFAULT);
    H5Gcreate(f_handle,b,H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT);

    return true;

}

template <class T>
bool hdf5out<T>::all_create(char* header/*, float* data_array*/)
{
    uint h_size = sizeof(header);

    //! Data Management Creation
    // Create dataspace
    //*** Constrains the matrix / array
    hsize_t current_dims[2] = {h_size, 1};                // Defines the lower bound dimensions to the header_size x 1
    hsize_t max_dims_head[2] = {h_size, 1};               // Defines the upper bound dimensions to the header_size x 1
    hsize_t max_dims_data[2] = {h_size, H5S_UNLIMITED};   // Defines the upper bound data dimensions to header_size x INFTY

    hid_t head_dataspace = H5Screate_simple(2, current_dims, max_dims_head);    // Sets the head matrix-rank (2) and its axial lower/upper bounds.
    hid_t data_dataspace = H5Screate_simple(2, current_dims, max_dims_data);    // Sets the data matrix-rank (2) and its axial lower/upper bounds.

    hid_t datatype_data = H5Tcopy(H5T_NATIVE_FLOAT);    // Defines data type to Float_64
    hid_t datatype_head = H5Tcopy(H5T_C_S1);            // Defines head type to 'One-byte, null-terminated string of eight-bit characters'
    H5Tset_size(datatype_head,H5T_VARIABLE);            // Sets the header to be of variable length.
    H5Tset_order(datatype_data, H5T_ORDER_LE);          // Sets the order to Little-endian byte order

    //*** Creates the data set.
    hsize_t chunk_dims[2] ={h_size, 1};                         // Defines the
    hid_t cparms_head = H5Pcreate(H5P_DATASET_CREATE);          // Created head ***
    hid_t cparms_data = H5Pcreate(H5P_DATASET_CREATE);          //          *** and data matrices/arrays

    float fillvalue;
    H5Pset_chunk( cparms_data, 2, chunk_dims);                      // Sets cparms_data matrix to Rank(2) with {header_size} columns
    H5Pset_fill_value (cparms_data, H5T_NATIVE_FLOAT, &fillvalue ); // Sets the cparms_data with the fillvalue data.
    H5Pset_fill_time(cparms_data, H5D_FILL_TIME_NEVER  );           // Allows to run indefinitely.

    hid_t header_dataset = -1;  //  Is used in conjunction with if statement test below...
    header_dataset = H5Dcreate2(f_handle,"header",datatype_head,head_dataspace,H5P_DEFAULT, cparms_head, H5P_DEFAULT);
    H5Pclose(cparms_head);

    // ...Test
    if (header_dataset < 0)
    {
        H5Tclose(datatype_head);    H5Tclose(datatype_data);
        H5Pclose(cparms_data);
        H5Sclose(head_dataspace);   H5Sclose(data_dataspace);
        //close_file();
        return false;
    }
        dset_handle = H5Dcreate2(f_handle,"data",datatype_data,data_dataspace,H5P_DEFAULT, cparms_data, H5P_DEFAULT);
        H5Pclose(cparms_data);
    //...Check...
    if (dset_handle < 0)
    {
        H5Tclose(datatype_data);
        H5Tclose(datatype_head);
        H5Sclose(head_dataspace);
        H5Sclose(data_dataspace);
        return false;
    }
    H5Tclose(datatype_data);        H5Sclose(data_dataspace);


    //! Begin writing header to file.
    //const char* buff;
    char* ref[1];

    hsize_t t_target_size[2] = {1,1};              // 1x1 Hyperslab size *..
    hsize_t t_target_offset[2] = {0,0};            //                  ..* with no offset
    hid_t t_dataspace_from = H5Screate_simple(2, t_target_size, NULL);
    hid_t t_dataspace_to;// = H5Scopy(t_dataspace_t);

    for(uint jj = 0; jj < h_size; jj++)
    {
        ref[0] = (char*)(header[jj]);
        t_target_offset[0] = jj;
        t_dataspace_to = H5Scopy(head_dataspace);
        H5Sselect_hyperslab(t_dataspace_to, H5S_SELECT_SET, t_target_offset, NULL,t_target_size, NULL);
        H5Dwrite(header_dataset, datatype_head , t_dataspace_from, t_dataspace_to, H5P_DEFAULT, ref);
        H5Sclose(t_dataspace_to);
    }

    // Close *
    H5Tclose(datatype_head);        H5Sclose(head_dataspace);        H5Sclose(t_dataspace_from);        H5Dclose(header_dataset);

    H5Fflush(f_handle,H5F_SCOPE_GLOBAL);

    return true;
}


/*  MAY NOT NEED
void hdf5out::write_header()
{

}

void hdf5out::write_data()
{

}
*/
