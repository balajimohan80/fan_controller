/************************************************************************************************
*   \file      mock_Node.cpp
*   \author    Balaji Mohan
*   \EmailID   balajimohan80@gmail.com
*   \date      11/29/2022
*   \brief     This application node will compute PWM counts on each fan controller 
*              based on PWM duty cycle published by Compute NODE.
*              This application will parse the JSON configuration file to store the 
*              number of FAN controllers and its maximum PWM counts.
*************************************************************************************************/

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
#include "FanCtrlTypeSupportImpl.h"
#include "parse_fan_control_json.hpp"
#include "fan_control.hpp"


cCir_Link_List<FanController::FanController_Sys> gcList(5);


void on_Sample(FanController::FanController_Sys &msg) {
	gcList.str_Element(msg);
}


using Fan_Listener_t = DataReaderListenerImpl<FanController::FanController_SysDataReader_var,
                                              FanController::FanController_SysDataReader,
                                              FanController::FanController_SysSeq,
                                              FanController::FanController_Sys>;

int main() {	
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


	std::string topic_name = "fan_controller_node";
 	FanController::FanController_SysTypeSupport_var ts =
 	new FanController::FanController_SysTypeSupportImpl;

 	if (ts == nullptr) {
 		std::cerr << "Not able to create memory for Type Support!!!\n";
 		return -1;
 	}

 	if (ts && 0 != openDDS.mCreateTopic<FanController::FanController_SysTypeSupport_var>
	              (topic_name, ts)) {
		std::cerr << "Not able to create Topic!!!\n";
		return -1;
 	}

 	if (0 != openDDS.mCreateSubscriber()) {
		std::cerr << "Not able to create Publisher!!!\n";
		return -1;
 	}

	Fan_Listener_t* const dataReaderImpl = new Fan_Listener_t(on_Sample);
	DDS::DataReaderListener_var listener(dataReaderImpl);

	if (0 != openDDS.mCreateDataReader(listener)) {
		std::cerr << "Not able to create DataWriter!!!\n";
		return -1;
 	}
	std::cout << "Waiting for Publisher...\n";
	openDDS.mWait_For_Publisher(1);

	int32_t nPrev_Seq_no = 0;
	const int32_t nContinues_Lost_Samples =  10;
	while(1) {
		FanController::FanController_Sys *m_ptr = gcList.pop();
		if (m_ptr != nullptr)  {
			//Check the sequence are our of order or not.
			const int32_t nCurr_Delta_Seq = m_ptr->_seq_no - nPrev_Seq_no; 
			if (m_ptr->_seq_no != 0  &&  nPrev_Seq_no > m_ptr->_seq_no) {
				std::cerr << "Data's is out of order!!!\n";
				continue;
			//Find no of samples lost 
			} else if (nCurr_Delta_Seq > nContinues_Lost_Samples) {
				std::cerr << "Lost Data Sequences: " << nCurr_Delta_Seq << "\n";
			}
			nPrev_Seq_no = m_ptr->_seq_no;
			const float nPWM_Duty_Cycle = m_ptr->_common_PWM_Dutycycle;
			std::cout << "-------------"<< nPrev_Seq_no;
			std::cout << " (PWM DUTY CYCLE: " << nPWM_Duty_Cycle << ") --------------------\n";
			if (nPWM_Duty_Cycle >= 0.2f && nPWM_Duty_Cycle <= 1.0f) {
                        	cFan_Ctrl.mCompute_PWM_count(m_ptr->_common_PWM_Dutycycle);
				cFan_Ctrl.mPrint_Fan_Count();
			} else {
				std::cerr << "PWM Duty Cycle is out of range: " << nPWM_Duty_Cycle << "!!!\n"; 
			}
                        gcList.push(m_ptr);
                }
        }
			
	
	return 0;
}

