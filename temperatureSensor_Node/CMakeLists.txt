project(Robot_FanController CXX)
cmake_minimum_required(VERSION 3.5)

set(EXE1 publisher)
find_package(OpenDDS REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(COMMON_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../common_inc")
set(IDL_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../idl")
set(IDL_HEADER ${CMAKE_CURRENT_BINARY_DIR}/opendds_generated)


# IDL TypeSupport Library
add_library(temperture_idl)
OPENDDS_TARGET_SOURCES(temperture_idl PUBLIC "${IDL_SRC}/Temperature.idl" OPENDDS_IDL_OPTIONS "-Lc++11")
target_link_libraries(temperture_idl PUBLIC OpenDDS::Dcps)


# set(tempidl_libs   ${CMAKE_CURRENT_BINARY_DIR}/libtemperture_idl.a)


set(opendds_libs
 OpenDDS::Dcps # Core OpenDDS Library
 OpenDDS::InfoRepoDiscovery OpenDDS::Tcp
 OpenDDS::Rtps OpenDDS::Rtps_Udp
 temperture_idl
)

add_executable(${EXE1}
   src/temperatureSensor_Node.cpp 
 )
target_include_directories(${EXE1} PRIVATE ${COMMON_INCLUDE} ${DDS_ROOT} ${ACE_ROOT} ${TAO_ROOT} ${IDL_HEADER})
target_link_libraries(${EXE1} ${opendds_libs})

	


