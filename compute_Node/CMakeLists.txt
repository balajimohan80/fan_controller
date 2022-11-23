project(compute_Node CXX)
cmake_minimum_required(VERSION 3.3)

# find_package(OpenDDS REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
# IDL TypeSupport Library

# add_library(messenger_idl)
# OPENDDS_TARGET_SOURCES(messenger_idl PUBLIC "Idl/Messenger.idl" OPENDDS_IDL_OPTIONS "-Lc++11")
# target_link_libraries(messenger_idl PUBLIC OpenDDS::Dcps)

# set(opendds_libs
# OpenDDS::Dcps # Core OpenDDS Library
# OpenDDS::InfoRepoDiscovery OpenDDS::Tcp # For run_test.pl
# OpenDDS::Rtps OpenDDS::Rtps_Udp # For run_test.pl --rtps
# messenger_idl
# )

# Subscriber
# include_directories(${DDS_ROOT} ${ACE_ROOT} ${TAO_ROOT} ${idl_generated_source})



set(COMMON_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/../common_inc")
message("COMMON_INCLUDE: ${COMMON_INCLUDE}")

add_executable(compute_Node
  ${CMAKE_CURRENT_SOURCE_DIR}/src/fan_control.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp 
)
target_include_directories(compute_Node PRIVATE ${COMMON_INCLUDE})
target_include_directories(compute_Node PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc)
# target_link_libraries(subscriber ${opendds_libs})


