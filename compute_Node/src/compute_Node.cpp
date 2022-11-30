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

int create_TempSensor_Subscriber(cOpenDDS_Pub_Sub &openDDS,
                                 std::string &str_Topic_Name, 
                                 Temperature::Temperature_StreamTypeSupport* ts,
                                 DDS::DataReaderListener_var listener) {
   	if (ts == nullptr) {
                std::cerr << "Type Support is null!!!\n";
                return -1;
        }

	
	if (0 != openDDS.mCreateTopic<Temperature::Temperature_StreamTypeSupport_ptr>
	               (str_Topic_Name, ts)) {
		std::cerr << "Not able to create Topic!!!\n";
                return -1;
        }
	if (0 != openDDS.mCreateSubscriber()) {
                std::cerr << "Not able to create Publisher!!!\n";
                return -1;
        }
	
        if (0 != openDDS.mCreateDataReader(listener)) {
                std::cerr << "Not able to create DataWriter!!!\n";
                return -1;
	}
	return 0;
}

int create_FanCtrl_Publisher(cOpenDDS_Pub_Sub &openDDS, 
                             std::string &str_Topic_Name,
                             FanController::FanController_SysTypeSupport* ts, 
                             FanController::FanController_SysDataWriter_var &nWriter) {
	if (ts == nullptr) {
		std::cerr << "Type Support is null!!!\n";
		return 0;
	}

	if (0 != openDDS.mCreateTopic<FanController::FanController_SysTypeSupport_ptr>
                        (str_Topic_Name, ts)) {
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

	nWriter = openDDS.mNarrow_Writer<FanController::FanController_SysDataWriter_var,
	                                 FanController::FanController_SysDataWriter>();

	if (!nWriter) {
		std::cerr << "narrow_Writer failed to create a memory\n";
		return -1;
	}

	return 0;
	
}


int main(int argc, char *argv[]) {
	std::string nJson_file_name = std::string("fan_controller.json");

 	c_Parse_Json nparse_JSON(nJson_file_name);
  	std::vector<json_data_t> nVec;
  	if (true != nparse_JSON.mGet_Json_Val(nVec)) {
		std::cerr << "Parsing JSON file failed!!!\n";
 		return -1;
 	}
	
	c_fan_Ctrl cFan_Ctrl(nVec);
	cOpenDDS_Pub_Sub cTemp_Sensor_Sub;
	cOpenDDS_Pub_Sub cFan_Ctrl_Pub;
	
	std::vector<std::string> dds_initialization=
  	{std::string("-DCPSConfigFile"), std::string("rtps.ini" )};
	char *c_Str[dds_initialization.size()] = {const_cast<char *>(dds_initialization[0].c_str()),
	                  const_cast<char *>(dds_initialization[1].c_str())};
        if (0 != cTemp_Sensor_Sub.mCreateParticipant(dds_initialization.size(), c_Str, 42)) {
        	std::cerr << "Not able to create participant!!!\n";
               	return -1;
        }
	cFan_Ctrl_Pub.mSet_Participant(cTemp_Sensor_Sub.mGet_Participant());
	std::string stemp_Snsr_topic_name = "temperature_sensor_node";
	std::string sfan_topic_name       = "fan_controller_node";
	

	Listener_t* dataReader_Impl = nullptr;
	Temperature::Temperature_StreamTypeSupport* temperature_ts = nullptr;
 	FanController::FanController_SysTypeSupport* fan_ts = nullptr;
	try {
		temperature_ts = new Temperature::Temperature_StreamTypeSupportImpl;
	} catch (std::exception &e) {
		std::cerr << "Exception Thrown: " << e.what() << "\n";
		return -1;
	}
	try {
		dataReader_Impl = new Listener_t(on_Sample);
	} catch (std::exception &e) {
		std::cerr << "Exception Thrown: " << e.what() << "\n";
		return -1;
	}

	try {
 		fan_ts = new FanController::FanController_SysTypeSupportImpl;
	} catch(std::exception &e) {
		std::cerr << "Exception Throw: " << e.what() << "\n";
		return -1;
	}

	DDS::DataReaderListener_var listener(dataReader_Impl);
	if (0 != create_TempSensor_Subscriber(cTemp_Sensor_Sub, 
	                                      stemp_Snsr_topic_name,
	                                      temperature_ts, listener)) {
		std::cerr << "Failed to create a Subscriber!!!\n";
		return -1;
	} 
	
	FanController::FanController_SysDataWriter_var nMsg_Writer;
	if (0 != create_FanCtrl_Publisher(cFan_Ctrl_Pub, sfan_topic_name, fan_ts, nMsg_Writer)) {
		std::cerr << "Not able to cretae publisher\n";
		return -1;
	}

	std::cout << "Waiting for Publisher: " << stemp_Snsr_topic_name << "...\n";	
	cTemp_Sensor_Sub.mWait_For_Publisher(1);


	std::cout << "Waiting for Subscriber: " << sfan_topic_name << "...\n";	
        cFan_Ctrl_Pub.mWait_For_Subscriber(1);
	
	FanController::FanController_Sys msg;
	int32_t nPrev_Seq_no = 0;
	const int32_t nContinues_Lost_Samples =  10;
	while(1) {
		Temperature::Temperature_Stream *m_ptr = gcList.pop();
		if (m_ptr != nullptr)  {
			std::vector<Temperature::Temperature_Sys> &vec = m_ptr->_Vec_Temperature;
			const int32_t nCurr_Delta_Seq = m_ptr->_seq_no - nPrev_Seq_no; 
			if (m_ptr->_seq_no != 0  &&  nPrev_Seq_no > m_ptr->_seq_no) {
				std::cerr << "Data's is out of order!!!\n";
				continue;
			} else if (nCurr_Delta_Seq > nContinues_Lost_Samples) {
				std::cerr << "Lost Data Sequences: " << nCurr_Delta_Seq << "\n";
			}
			nPrev_Seq_no = m_ptr->_seq_no; 
			for (Temperature::Temperature_Sys &s : vec) {
				cFan_Ctrl.mSet_temperature(s._deg_C, s._system_ID);
			}
			msg._common_PWM_Dutycycle = cFan_Ctrl.mCompute_Duty_Cycle();
			std::cout << m_ptr->_seq_no << ": Common Duty Cycle= " << msg._common_PWM_Dutycycle; 
			std::cout << " Temp: " << cFan_Ctrl.mGetMax_Temp_Sensor();
			std::cout << " Max PWM Count: " << cFan_Ctrl.mGetMax_PWM_Count() << "\n"; 	
			gcList.push(m_ptr);
			cFan_Ctrl_Pub.mSend_Sample<FanController::FanController_Sys ,
                                     FanController::FanController_SysDataWriter_var>(
                                     msg, nMsg_Writer);

		}
	}
EXIT:
	std::cout << "Exiting Compute Node:...\n";
	return 0;		

}
