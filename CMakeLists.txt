set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_minimum_required(VERSION 2.8.3)
project(events_h52bag LANGUAGES CXX)
find_package(rosbag REQUIRED
    std_msgs REQUIRED
    )
include_directories(
    ${rosbag_INCLUDE_DIRS}
    ${std_msgs_INCLUDE_DIRS}
    /usr/include/hdf5/serial/
    /home/suman/catkin_ws/devel/include #path to dvs_msgs headers
)
add_executable(events_h52bag src/main.cpp)
#message(STATUS "HDF5_LIBRARIES: ${HDF5_LIBRARIES}")
target_link_libraries(events_h52bag
    /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5.so
    /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5_cpp.so
    /usr/lib/x86_64-linux-gnu/hdf5/serial/libhdf5_hl.so
    ${rosbag_LIBRARIES}
    ${std_msgs_LIBRARIES})

