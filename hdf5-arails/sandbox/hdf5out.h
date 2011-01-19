#ifndef HDF5OUT_H
#define HDF5OUT_H

#include <string>
#include <hdf5.h>

template <class T>
class hdf5out   {
    // Variables
    char* fname;
    char* group;
    char* subgroup;

    hid_t   f_handle;
    hid_t   g_handle, sg_handle;
    hid_t   dsp_handle, dset_handle;

    hsize_t RANK, dims;

public:
    hdf5out();
    virtual ~hdf5out();

    bool    check_info(hid_t, char*, char* group);       // After this step, the file (if not existent) will be created and the necessary directories will be esablished.
    bool    write_out();
    static  void write_header();
    static  void write_data();

    /*  Templates for creation...
    bool	open_file(std::string fname);
    bool        make_groups();
    bool        write_header();

    bool	write_data(const std::deque<float> &data);
    bool	close_file();

protected:
    bool    expand_data(hsize_t estimated_size);
    */
};


int calc_rank() {
    int rank = 0;
    return rank;
}

 int *calc_dims()    {
    int *foo;
    return foo;
}
