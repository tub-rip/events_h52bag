# events_h52bag
C++ code to convert event data from HDF5 to ROSbags. By default, data format of h5 input file is expected as in the [DSEC dataset](https://dsec.ifi.uzh.ch/) or [TUM-VIE dataset](https://vision.in.tum.de/data/datasets/visual-inertial-event-dataset). The exact data structure used can be found [here](https://github.com/uzh-rpg/DSEC#events).

**Author**: [Suman Ghosh](https://www.linkedin.com/in/suman-ghosh-a8762576/)

# Installation
* Install ROS. We need the `rosbag` and `std_msgs` packages.
* Install libhdf5: 
```
sudo apt-get update
sudo apt-get install libhdf5-dev
```

* Install the [blosc filter plugin for hdf5](https://github.com/Blosc/hdf5-blosc#installing-the-blosc-filter-plugin) for read-only access across all HDF5 applications in your system:
```
git clone https://github.com/Blosc/hdf5-blosc.git
cd hdf5-blosc
mkdir build && cd build
cmake ..
make
```
* Copy the `libH5Zblosc.so` shared library generated inside the `build` folder into your local hdf5 plugin path, which is usually `/usr/local/hdf5/lib/plugin` or `/usr/lib/x86_64-linux-gnu/hdf5/plugins` depending on your Linux distro. Installation location of hdf5 can be found using `dpkg -L libhdf5-dev`. Create the `plugins` directory if it does not exist already in your system.

Then, add the following line to your `~/.bashrc` and source it
```
export HDF5_PLUGIN_PATH=<path to plugins>
```

* Optional: For M3ED dataset, [lzf filter plugin](https://github.com/h5py/h5py/tree/master/lzf) needs to be built as a shared library. Then copy the library file `liblzf_filter.so` into the same HDF5 plugin path, as was done for the blosc filter above.
```
git clone git@github.com:h5py/h5py.git
cd h5py/lzf
h5fc -I"lzf" -O2 -fPIC -shared lzf/*.c lzf_filter.c -lhdf5 -o liblzf_filter.so
sudo cp liblzf_filter.so /usr/lib/x86_64-linux-gnu/hdf5/plugins/
```

* Add path to your installed `dvs_msgs` header in line 11 of `CMakeLists.txt`.  If not already installed, install `dvs_msgs` from https://github.com/uzh-rpg/rpg_dvs_ros in your catkin workspace.
* Clone this repository: `git clone https://github.com/tub-rip/events_h52bag.git`
* Compile this repository:
```
cd events_h52bag
mkdir build && cd build
cmake ..
make
```
# Execution
`./events_h52bag <path/to/input/h5file> <path/to/output/bagfile(without extension)> <output_topic_name> <sensor_height> <sensor_width> [optional, default=500000000]<events_read_per_iteration> [optional, default=100000]<event_packet_size> [optional, default="events"]<dataset_prefix>`

To overcome memory limitations for large data files, h5 files are read in multiple iterations (500M events per iteration max). The result is a single ROSBag.

# Working example
* Download input data from DSEC dataset: https://download.ifi.uzh.ch/rpg/DSEC/train/thun_00_a/thun_00_a_events_left.zip
* Extract `events.h5` from zip.
* Execute: `./events_h52bag events.h5 out /dvs/left/events 480 640`
* The output ROSBag `out.bag` is generated.

# Log
* 13.08.24: Changed behavior. A single ROSBag output is produced.
* 16.05.23: Now supports [M3ED dataset](https://m3ed.io/). Dataset prefix can now be passed as an argument, which is `prophesee/left` and `prophesee/right` for M3ED h5 files. 
* 14.09.21: Sensor size can now be passed an argument. Set to 0 when no `t_offset` data is present in h5 file. Tested on TUM-VIE dataset.
* 07.02.21: Optional argument `events_per_bag` can be used to generate multiple rosbags while converting from a single h5 file. Optional argument `event_packet_size` can be passed to modify ROS message rate.
