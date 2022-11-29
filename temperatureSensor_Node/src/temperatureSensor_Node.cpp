#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>

#include "openDDS_Pub_Sub.hpp" 
#include "parse_fan_control_json.hpp"

#include <dds/DCPS/Service_Participant.h>

#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include "TemperatureTypeSupportImpl.h"

float generate_number(float lower_bound, float upper_bound) {
        std::random_device rd;
        std::default_random_engine generator(rd());
        std::uniform_real_distribution<float> distribution(lower_bound, upper_bound);
        return distribution(generator);
}



int main(int argc, char *argv[]) {
	std::chrono::milliseconds ms_to_sleep(1);
	if (argc > 1) {
		if (std::string(argv[1]) == std::string("--sleep")) 
			ms_to_sleep = static_cast<std::chrono::milliseconds>(std::stoi(std::string(argv[2])));
	}
	
	std::string nJson_file_name = std::string("fan_controller.json");
	c_Parse_Json nparse_JSON(nJson_file_name);
	const int no_of_Subsystems = nparse_JSON.mGet_total_sensors();
	if (0 > no_of_Subsystems) {
		std::cout << "Not able to parse Json file\n";
		return -1;
	} 
	cOpenDDS_Pub_Sub openDDS;
	std::vector<std::string> dds_initialization= 
	{std::string("-DCPSConfigFile"), 
	 std::string("rtps.ini" )};
	char *c_Str[2] = {const_cast<char *>(dds_initialization[0].c_str()),
	                  const_cast<char *>(dds_initialization[1].c_str())}; 
	if (0 != openDDS.mCreateParticipant(2, c_Str, 42)) {
		std::cerr << "Not able to create participant!!!\n";
		return -1;
	}

	std::string topic_name = "temperature_sensor_node";
	Temperature::Temperature_StreamTypeSupport_var ts; 
	try {
		ts = new Temperature::Temperature_StreamTypeSupportImpl;
	} catch(std::exception &e) {
		std::cerr << "Exception Thrown: " << e.what() << "\n";
		return -1;
	}
	if (ts == nullptr) {
		std::cerr << "Not able to create memory for Type Support!!!\n";
		return -1;
	}

	if (ts && 0 != openDDS.mCreateTopic<Temperature::Temperature_StreamTypeSupport_var>
	               (topic_name,  ts)) {
		std::cerr << "Not able to create Topic!!!\n";
		return -1;
	}
	
	if (0 != openDDS.mCreatePublisher()) {
		std::cerr << "Not able to create Publisher!!!\n";
		return -1;		
	}

	if (0 != openDDS.mCreateDataWriter()) {
		std::cerr << "Not able to create DataWriter!!!\n";
		return -1;
	}

	Temperature::Temperature_StreamDataWriter_var msg_Writer =
	openDDS.mNarrow_Writer<Temperature::Temperature_StreamDataWriter_var,
	                       Temperature::Temperature_StreamDataWriter>();    
		
	if (!msg_Writer) {
		std::cerr << "narrow_Writer failed to create a memory\n";
		return -1;
	}

	openDDS.mWait_For_Subscriber(1);

	
	Temperature::Temperature_Stream msg;
	int max_loop_count = 10000;
	for (int ct = 0 ; ct < max_loop_count; ct++) {
		std::vector<Temperature::Temperature_Sys> temp(no_of_Subsystems, 
		                                          Temperature::Temperature_Sys());
		for (int i = 0 ; i < temp.size(); i++) {
			temp[i]._system_ID  = i;
			temp[i]._deg_C      = generate_number(25.0f, 75.0f);
		}

		msg._Vec_Temperature = temp;
		msg._seq_no          = ct;

		openDDS.mSend_Sample<Temperature::Temperature_Stream , 
		                     Temperature::Temperature_StreamDataWriter_var>(
		                     msg, msg_Writer);
		std::this_thread::sleep_for(std::chrono::milliseconds(ms_to_sleep)); 
	}
	std::cout << "Publisher Exits...\n";
	return 0;			
}
