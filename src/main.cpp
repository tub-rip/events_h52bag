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


void readH5Datasets(std::string fname, std::string dataset, std::vector<uint64_t> &data, hsize_t hslab_offset, hsize_t hslab_count){

    H5File file( fname.c_str(), H5F_ACC_RDONLY );
    cout<<"Reading dataset "<<dataset<<endl;
    if(!H5Lexists(file.getId(), dataset.c_str(), H5P_DEFAULT)){
        cout<<"Dataset does not exist.. skipping!"<<endl;
        return;
    }

    DataSet dset= file.openDataSet(dataset.c_str());

    //Get dataspace of the dataset.
    DataSpace dataspace = dset.getSpace();
    // Get the number of dimensions in the dataspace.
    int rank = dataspace.getSimpleExtentNdims();

    if (rank == 0){ // for single value datasets
        // create a vector the same size as the dataset
        data.resize(1);
        cout<<"Vectsize: "<<data.size()<<endl;
        dset.read(data.data(), PredType::NATIVE_UINT64, H5S_ALL, H5S_ALL);
    }
    else { // for array datasets
        // Get the dimension size of each dimension in the dataspace and display them.
        hsize_t dims_out[1];
        int ndims = dataspace.getSimpleExtentDims( dims_out, NULL);
        cout << "rank " << rank << ", dimensions " <<
                (unsigned long)(dims_out[0]) << endl;
        cout<<"ndims :"<<ndims<<endl;

        hsize_t offset[1];
        hsize_t count[1];
        offset[0] = hslab_offset;
        count[0] = std::min(hslab_count, dims_out[0] - hslab_offset);
        cout<<"count: "<< count[0]<<" offset: "<<offset[0]<<endl;
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

        // Define the memory dataspace
        DataSpace memspace (1, count);

        // create a vector the same size as the dataset
        data.resize(count[0], 0);
        cout<<"Vectsize: "<<data.size()<<endl;

        // pass pointer to the array (or vector) to read function, along with the data type and space.
        dset.read(data.data(), PredType::NATIVE_UINT64, memspace, dataspace);

    }

    // close the HDF5 file
    file.close();
}

int main(int argc, char *argv[]){
    int numev_per_iter = 500000000; //500M
    int packet_size = 100000;
    std::string dataset_prefix = "events";
    if (argc<6){
        cout<<"Too few arguments.. Exiting!\n";
        return 0;
    }
    std::string fname = argv[1];
    std::string bagname = argv[2];
    std::string topic = argv[3];
    int height = std::stoi(argv[4]);
    int width = std::stoi(argv[5]);

    if (argc == 7){
        numev_per_iter = std::stoi(argv[6]);
    }
    if (argc == 8){
        packet_size = std::stoi(argv[7]);
    }
    if (argc == 9){
        dataset_prefix = argv[8];
    }

    /*
        std::string dataset_prefix = "prophesee/right";
        * Try block to detect exceptions raised by any of the calls inside it
        */
    try
    {
        hsize_t hslab_offset = 0;
        int i=0;
        rosbag::Bag bag;
        bag.open(bagname+".bag", rosbag::bagmode::Write);
        while(true){
            std::vector<uint64_t> data;
            readH5Datasets(fname, dataset_prefix + "/t", data, hslab_offset, numev_per_iter);
            std::vector<uint64_t> t(data.begin(), data.end());
            readH5Datasets(fname, dataset_prefix + "/x", data, hslab_offset, numev_per_iter);
            std::vector<uint16_t> x(data.begin(), data.end());
            readH5Datasets(fname, dataset_prefix + "/y", data, hslab_offset, numev_per_iter);
            std::vector<uint16_t> y(data.begin(), data.end());
            readH5Datasets(fname, dataset_prefix + "/p", data, hslab_offset, numev_per_iter);
            std::vector<uint8_t> p(data.begin(), data.end());
            readH5Datasets(fname, "t_offset", data, hslab_offset, numev_per_iter);
            std::vector<uint64_t> offset_data(data.begin(), data.end());
            std::vector<uint64_t>().swap(data); //clearing memory

            uint64_t t_offset;
            if (offset_data.size()>0)
                t_offset = offset_data[0];
            else
                t_offset = 0;

            cout<<"Writing to rosbag #iteration"<<i<<" at offset "<<hslab_offset<<" containing "<<t.size()<<" events"<<endl;
            // Write to rosbag

            dvs_msgs::EventArray evQueue;
            for (int j=0; j<t.size(); j++){
                dvs_msgs::Event ev;
                ev.ts.fromSec(double(t[j]+t_offset)/1e6);
                ev.x = x[j];
                ev.y = y[j];
                ev.polarity = (int)p[j];
                evQueue.events.push_back(ev);
                if(j % packet_size == 0){
                    //                    cout<<"writing msg "<<i*100./t.size()<<"%"<<endl;
                    evQueue.header.stamp = ev.ts;
                    evQueue.header.frame_id = j;
                    evQueue.height = height;
                    evQueue.width = width;
                    bag.write(topic, ev.ts, evQueue);
                    evQueue.events.clear();
                }
            }
            if(t.size()!=numev_per_iter){
                cout<<"Final bag file has been written."<<endl;
                break;
            }
            hslab_offset+=numev_per_iter;
            i++;
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
        return -1;
    }

    return 0;  // successfully terminated
}
