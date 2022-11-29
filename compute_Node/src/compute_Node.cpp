#include <iostream>
#include <chrono>
#include <thread>

#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/StaticIncludes.h>
#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include "circular_Linked_List.hpp"
#include "openDDS_Pub_Sub.hpp"
#include "DataReaderListenerImpl.hpp"
#include "TemperatureTypeSupportImpl.h"
#include "FanCtrlTypeSupportImpl.h"
#include "parse_fan_control_json.hpp"
#include "fan_control.hpp"


cCir_Link_List<Temperature::Temperature_Stream> gcList(5); 


void on_Sample(Temperature::Temperature_Stream &msg) {
	gcList.str_Element(msg);
}

using Listener_t = DataReaderListenerImpl<Temperature::Temperature_StreamDataReader_var,
                                          Temperature::Temperature_StreamDataReader,
                                          Temperature::Temperature_StreamSeq,
                                          Temperature::Temperature_Stream>;
int main(int argc, char *argv[]) {
	std::string nJson_file_name = std::string("fan_controller.json");

 	c_Parse_Json nparse_JSON(nJson_file_name);
  	std::vector<json_data_t> nVec;
  	if (true != nparse_JSON.mGet_Json_Val(nVec)) {
		std::cerr << "Parsing JSON file failed!!!\n";
 		return -1;
 	}

	c_fan_Ctrl cFan_Ctrl(nVec);
	
	cOpenDDS_Pub_Sub openDDS;
	std::vector<std::string> dds_initialization=
  	{std::string("-DCPSConfigFile"), std::string("rtps.ini" )};
	char *c_Str[2] = {const_cast<char *>(dds_initialization[0].c_str()),
	                  const_cast<char *>(dds_initialization[1].c_str())};
        if (0 != openDDS.mCreateParticipant(2, c_Str, 42)) {
                std::cerr << "Not able to create participant!!!\n";
                return -1;
        }
	
        std::string topic_name = "temperature_sensor_node";
	Temperature::Temperature_StreamTypeSupport_var ts = 
	new Temperature::Temperature_StreamTypeSupportImpl;

   	if (ts == nullptr) {
                std::cerr << "Not able to create memory for Type Support!!!\n";
                return -1;
        }

	if (ts && 0 != openDDS.mCreateTopic<Temperature::Temperature_StreamTypeSupport_var>
	               (topic_name, ts)) {
		std::cerr << "Not able to create Topic!!!\n";
                return -1;
        }
	if (0 != openDDS.mCreateSubscriber()) {
                std::cerr << "Not able to create Publisher!!!\n";
                return -1;
        }
	
	Listener_t* const dataReaderImpl = new Listener_t(on_Sample);
	DDS::DataReaderListener_var listener(dataReaderImpl);

        if (0 != openDDS.mCreateDataReader(listener)) {
                std::cerr << "Not able to create DataWriter!!!\n";
                return -1;
        }
	std::cout << "Waiting for Publisher: " << topic_name << "...\n";	
	openDDS.mWait_For_Publisher(1);

	cOpenDDS_Pub_Sub openDDS_Fan;
	openDDS_Fan.mSet_Participant(openDDS.mGet_Participant()); 
	std::string fan_topic_name = "fan_controller_node";

 	FanController::FanController_SysTypeSupport_var nFan_ts =
 	new FanController::FanController_SysTypeSupportImpl;

 	if (nFan_ts == nullptr) {
 		std::cerr << "Not able to create memory for Type Support!!!\n";
 		return -1;
 	}
	
 	if (nFan_ts && 0 != openDDS_Fan.mCreateTopic<FanController::FanController_SysTypeSupport_var>
	              (fan_topic_name, nFan_ts)) {
		std::cerr << "Not able to create Topic!!!\n";
		return -1;
 	}

 	if (0 != openDDS_Fan.mCreatePublisher()) {
		std::cerr << "Not able to create Publisher!!!\n";
		return -1;
	}

	if (0 != openDDS_Fan.mCreateDataWriter()) {
		std::cerr << "Not able to create DataWriter!!!\n";
		return -1;
        }

        FanController::FanController_SysDataWriter_var msg_Writer =
        openDDS_Fan.mNarrow_Writer<FanController::FanController_SysDataWriter_var,
                               FanController::FanController_SysDataWriter>();

        if (!msg_Writer) {
                std::cerr << "narrow_Writer failed to create a memory\n";
                return -1;
        }

	std::cout << "Waiting for Subscriber: " << fan_topic_name << "...\n";	
        openDDS_Fan.mWait_For_Subscriber(1);
	FanController::FanController_Sys msg;
	while(1) {
		Temperature::Temperature_Stream *m_ptr = gcList.pop();
		if (m_ptr != nullptr)  {
			std::vector<Temperature::Temperature_Sys> &vec = m_ptr->_Vec_Temperature;
			for (Temperature::Temperature_Sys &s : vec) {
				cFan_Ctrl.mSet_temperature(s._deg_C, s._system_ID);
			}
			msg._common_PWM_Dutycycle = cFan_Ctrl.mCompute_Duty_Cycle();
			std::cout << "Common Duty Cycle: " << msg._common_PWM_Dutycycle; 
			std::cout << " Temp: " << cFan_Ctrl.mGetMax_Temp_Sensor();
			std::cout << " Max PWM Count: " << cFan_Ctrl.mGetMax_PWM_Count() << "\n"; 	
			gcList.push(m_ptr);
			openDDS_Fan.mSend_Sample<FanController::FanController_Sys ,
                                     FanController::FanController_SysDataWriter_var>(
                                     msg, msg_Writer);

		}
	}
	std::cout << "Exiting Subscriber\n";
	return 0;		
}
