#include <string>
#include <hdf5/serial/hdf5.h>
#include <hdf5/serial/H5Cpp.h>
#include <vector>
#include <rosbag/bag.h>
#include <std_msgs/Int32.h>
#include <std_msgs/String.h>
#include <dvs_msgs/Event.h>
#include <dvs_msgs/EventArray.h>

#ifdef OLD_HEADER_FILENAME
#else
#include <iostream>
#endif

using std::cout;
using std::endl;
using namespace H5;


void readH5Datasets(std::string fname, std::string dataset, std::vector<double> &data){

  H5File file( fname.c_str(), H5F_ACC_RDONLY );
  DataSet dset = file.openDataSet(dataset.c_str());
  cout<<"Reading dataset "<<dataset<<endl;

  // get the dataset type class
  H5T_class_t type_class = dset.getTypeClass();
  //Get dataspace of the dataset.
  DataSpace dataspace = dset.getSpace();
  // Get the number of dimensions in the dataspace.
  int rank = dataspace.getSimpleExtentNdims();
  // Get the dimension size of each dimension in the dataspace and display them.
  hsize_t dims_out[1];
  int ndims = dataspace.getSimpleExtentDims( dims_out, NULL);
  cout << "rank " << rank << ", dimensions " <<
          (unsigned long)(dims_out[0]) << endl;
  cout<<"ndims :"<<ndims<<endl;

  if (rank == 0){
      // create a vector the same size as the dataset
      data.resize(1);
      cout<<"Vectsize: "<<data.size()<<endl;
      dset.read(data.data(), PredType::NATIVE_DOUBLE, H5S_ALL, H5S_ALL);
    }
  else {
      // Define the memory dataspace
      hsize_t dimsm[1];
      dimsm[0] = dims_out[0];
      DataSpace memspace (1,dimsm);

      // create a vector the same size as the dataset
      data.resize(dims_out[0]);
      cout<<"Vectsize: "<<data.size()<<endl;

      // pass pointer to the array (or vector) to read function, along with the data type and space.
      dset.read(data.data(), PredType::NATIVE_DOUBLE, memspace, dataspace);
    }

  cout<<data.size()<<endl;

  // close the HDF5 file
  file.close();
}

int main(){

  /*
        * Try block to detect exceptions raised by any of the calls inside it
        */
  try
  {
    /*
           * Turn off the auto-printing when failure occurs so that we can
           * handle the errors appropriately
           */
    std::string fname = "/home/suman/data/rpg/DSEC/zurich_city_04_a/zurich_city_04_a_events_right/events.h5";
    std::vector<double> data;
    readH5Datasets(fname, "events/t", data);
    std::vector<double> t(data);
    data.resize(0);
    readH5Datasets(fname, "events/x", data);
    std::vector<uint16_t> x(data.begin(), data.end());
    data.resize(0);
    readH5Datasets(fname, "events/y", data);
    std::vector<uint16_t> y(data.begin(), data.end());
    data.resize(0);
    readH5Datasets(fname, "events/p", data);
    std::vector<uint8_t> p(data.begin(), data.end());
    data.resize(0);
    readH5Datasets(fname, "t_offset", data);
    std::vector<double> offset_data(data.begin(), data.end());
    data.resize(0);

    float t_offset = offset_data[0];

    cout<<"Writing to rosbag"<<endl;

    // Write to rosbag
    rosbag::Bag bag;
    std::string topic = "/dvs/right/events";
    std::string bagname = "/home/suman/data/right_events.bag";
    bag.open(bagname, rosbag::bagmode::Write);
    dvs_msgs::EventArray evQueue;
    for (int i=0; i<t.size(); i++){
        dvs_msgs::Event ev;
        ev.ts.fromSec((t[i]+t_offset)/1e6);
        ev.x = x[i];
        ev.y = y[i];
        ev.polarity = (int)p[i];
        evQueue.events.push_back(ev);
        if(i % 100000 == 0){
            cout<<"writing msg "<<i*100./t.size()<<"%"<<endl;
            evQueue.header.stamp = ev.ts;
            evQueue.header.frame_id = i;
            evQueue.height = 480;
            evQueue.width = 640;
            bag.write(topic, ev.ts, evQueue);
            evQueue.events.clear();
          }
      }
    bag.close();

  }  // end of try block
  // catch failure caused by the H5File operations
  catch( FileIException error )
  {
    error.printErrorStack();
    return -1;
  }
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
    error.printErrorStack();
    return -1;
  }
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
    error.printErrorStack();
    return -1;
  }
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
    error.printErrorStack();
    return -1;
  }
  catch(rosbag::BagException error){
    cout<<error.what()<<endl;
  }

  return 0;  // successfully terminated
}
