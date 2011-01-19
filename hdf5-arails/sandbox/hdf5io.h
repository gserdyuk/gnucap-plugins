#ifndef HDF5IO_H
#define HDF5IO_H

#include <string>
#include <hdf5.h>
using namespace std;

class hdf5io
{
    // Variables
    hid_t   file_handle;
    //
    string group_name;
    hid_t   group_handle;
    //
    string subgroup_name;
    hid_t subgroup_handle;
    //
    size_t  head_size;
    size_t  data_offset;

public:
    hdf5io();
    virtual ~hdf5io();

    bool	open_file(std::string fname);
    bool        make_groups();
    bool        write_header();

    bool	write_data();
    bool	close_file();

protected:
    bool    expand_data(hsize_t estimated_size);

};

#endif // HDF5IO_H
