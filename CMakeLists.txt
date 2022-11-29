project(Robot_FanController CXX)
cmake_minimum_required(VERSION 3.5)

find_package(OpenDDS REQUIRED)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(COMMON_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/common_inc")

execute_process(COMMAND cp ${COMMON_INCLUDE}/rtps.ini ${CMAKE_CURRENT_BINARY_DIR})
execute_process(COMMAND cp ${COMMON_INCLUDE}/fan_controller.json ${CMAKE_CURRENT_BINARY_DIR})
set(IDL_HEADER ${CMAKE_CURRENT_BINARY_DIR}/opendds_generated)



# IDL TypeSupport Library
add_library(temperture_idl)
OPENDDS_TARGET_SOURCES(temperture_idl PUBLIC "idl/Temperature.idl" OPENDDS_IDL_OPTIONS "-Lc++11")
target_link_libraries(temperture_idl PUBLIC OpenDDS::Dcps)

#IDL TypeSupport Library
add_library(fanCtrl_idl)
OPENDDS_TARGET_SOURCES(fanCtrl_idl PUBLIC "idl/FanCtrl.idl" OPENDDS_IDL_OPTIONS "-Lc++11")
target_link_libraries(fanCtrl_idl PUBLIC OpenDDS::Dcps)

#FanCtrl Library
add_library(fan_control STATIC  ${CMAKE_CURRENT_SOURCE_DIR}/common_src/fan_control.cpp)
target_include_directories(fan_control PRIVATE ${COMMON_INCLUDE})

set(tempidl_libs   ${CMAKE_CURRENT_BINARY_DIR}/libtemperture_idl.a)
set(fanCtridl_libs ${CMAKE_CURRENT_BINARY_DIR}/libfanCtrl_idl.a)
set(fanCtrl_libs   ${CMAKE_CURRENT_BINARY_DIR}/libfan_control.a)


# add_custom_target(${fanCtridl_libs} depends ${EXE3})
# add_dependencies(${EXE3} ${fanCtridl_libs})

set(opendds_libs
 OpenDDS::Dcps # Core OpenDDS Library
 OpenDDS::InfoRepoDiscovery OpenDDS::Tcp
 OpenDDS::Rtps OpenDDS::Rtps_Udp 
 temperture_idl fanCtrl_idl fan_control
)

set(EXE1 temperatureSensor_Node)
set(EXE2 compute_Node)
set(EXE3 mock_Node)

add_executable(${EXE1}
   ${EXE1}/src/${EXE1}.cpp
 )
target_include_directories(${EXE1} PRIVATE ${COMMON_INCLUDE} ${DDS_ROOT} ${ACE_ROOT} ${TAO_ROOT} ${IDL_HEADER})
target_link_libraries(${EXE1} ${opendds_libs})

# Subscriber
add_executable(${EXE2}
 ${EXE2}/src/${EXE2}.cpp 
)

target_include_directories(${EXE2} PRIVATE ${COMMON_INCLUDE} ${DDS_ROOT} ${ACE_ROOT} ${TAO_ROOT} ${IDL_HEADER})

target_link_libraries(${EXE2} ${opendds_libs} ${fanCtrl_libs})
add_executable(${EXE3}
	${EXE3}/src/${EXE3}.cpp
 )

target_include_directories(${EXE3} PRIVATE ${COMMON_INCLUDE} ${DDS_ROOT} ${ACE_ROOT} ${TAO_ROOT} ${IDL_HEADER})
target_link_libraries(${EXE3} ${opendds_libs} ${fanCtrl_libs})
	


