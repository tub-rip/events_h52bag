# events_h52bag
C++ code to convert event data from HDF5 to ROSbags. Data format of h5 input file should be as in the (DSEC dataset)[https://dsec.ifi.uzh.ch/]. 

# Installation
* Install ROS. We need the `rosbag` and `std_msgs` packages.
* Install libhdf5: 
```
sudo apt-get update
sudo apt-get install libhdf5-dev
```

* Install the blosc filter plugin for hdf5: https://github.com/Blosc/hdf5-blosc#installing-the-blosc-filter-plugin
* Include path to your installed `dvs_msgs` header in CMakeLists.txt.  If not already installed, install `dvs_msgs` from https://github.com/uzh-rpg/rpg_dvs_ros in your catkin workspace.
* Clone this repository: `git clone https://github.com/tub-rip/events_h52bag.git`
* Check the custom paths to the `hdf5` include and library files in your `CMakeLists.txt`. Modify if needed. (TODO: Automate this step)
* Set the path of input and output files in the source code `src/main.cpp`. (TODO: Pass as runtime argument)
* Compile this repository:
```
cd events_h52bag
mkdir build && cd build
cmake ..
make
```
* Run: `./events_h52bag`
