project(opendds_devguide_temperture CXX)
cmake_minimum_required(VERSION 3.3)

find_package(OpenDDS REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(COMMON_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/common_inc")
message("COMMON_INCLUDE: ${COMMON_INCLUDE}")
# set(IDL_INCLUDE "${CAMKE_BINARY_DIR}")

# IDL TypeSupport Library
add_library(temperture_idl)
OPENDDS_TARGET_SOURCES(temperture_idl PUBLIC "idl/Temperature.idl" OPENDDS_IDL_OPTIONS "-Lc++11")
target_link_libraries(temperture_idl PUBLIC OpenDDS::Dcps)


set(opendds_libs
 OpenDDS::Dcps # Core OpenDDS Library
 OpenDDS::InfoRepoDiscovery OpenDDS::Tcp # For run_test.pl
 OpenDDS::Rtps OpenDDS::Rtps_Udp # For run_test.pl --rtps
 temperture_idl
)


# Subscriber
add_executable(subscriber
   sub/subscriber.cpp 
)

target_include_directories(subscriber PRIVATE ${COMMON_INCLUDE} ${DDS_ROOT} ${ACE_ROOT} ${TAO_ROOT})
target_link_libraries(subscriber ${opendds_libs})

add_executable(publisher
  pub/publisher.cpp 
 )

target_include_directories(publisher PRIVATE ${COMMON_INCLUDE} ${DDS_ROOT} ${ACE_ROOT} ${TAO_ROOT})
target_link_libraries(publisher ${opendds_libs})
