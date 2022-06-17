cmake_minimum_required(VERSION 3.16)
project(mygst)

set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)

# 不同的.pc文件存放位置需要用不同的pkg_search名称（eg. OPENCV GSTAPP GST）
pkg_search_module(OPENCV REQUIRED opencv4)
pkg_search_module(GSTAPP REQUIRED gstreamer-app-1.0)
pkg_search_module(GST REQUIRED glib-2.0)
pkg_search_module(GST REQUIRED gobject-2.0)
pkg_search_module(GST REQUIRED gstreamer-1.0)

# pkg_search到的库（用GST为例）头文件用GST_INCLUDE_DIRS，库文件用GST_LIBRARIES
include_directories(${GST_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} ${GSTAPP_INCLUDE_DIRS})
LINK_DIRECTORIES(/usr/local/opencv4/lib/)
add_executable(mygst mygst.cpp)
# add_library(mygst SHARED data_plugin VideoCap.h VideoCap.cpp)

target_link_libraries(mygst ${GST_LIBRARIES} ${OPENCV_LIBRARIES} ${GSTAPP_LIBRARIES})

target_include_directories(mygst PRIVATE ${GST_INCLUDE_DIRS} ${OPENCV_INCLUDE_DIRS} ${GSTAPP_INCLUDE_DIRS})

message(STATUS "GST_LIBS ${OPENCV_LIBRARIES}")