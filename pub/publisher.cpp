#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>

#include "openDDS_Pub_Sub.hpp" 
#include <dds/DCPS/Service_Participant.h>

#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

//#include "ReliabilityTypeSupportImpl.h"
#include "TemperatureTypeSupportImpl.h"

float generate_number(float lower_bound, float upper_bound) {
        std::random_device rd;
        std::default_random_engine generator(rd());
        std::uniform_real_distribution<float> distribution(lower_bound, upper_bound);
        return distribution(generator);
}


int main(int argc, char *argv[]) {
	cOpenDDS_Pub_Sub openDDS;
	if (0 != openDDS.mCreateParticipant(argc, argv, 42)) {
		std::cerr << "Not able to create participant!!!\n";
		return -1;
	}

	std::string topic_name = "transaction_test";
//	Reliability::MessageTypeSupport_var ts = new Reliability::MessageTypeSupportImpl;
	Temperature::Temperature_StreamTypeSupport_var ts = 
	new Temperature::Temperature_StreamTypeSupportImpl;
	
	if (ts == nullptr) {
		std::cerr << "Not able to create memory for Type Support!!!\n";
		return -1;
	}

//	if (ts && 0 != openDDS.mCreateTopic<Reliability::MessageTypeSupport_var>
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

//	Reliability::MessageDataWriter_var msg_Writer = 
//	openDDS.mNarrow_Writer<Reliability::MessageDataWriter_var, Reliability::MessageDataWriter>();
	Temperature::Temperature_StreamDataWriter_var msg_Writer =
	openDDS.mNarrow_Writer<Temperature::Temperature_StreamDataWriter_var,
	                       Temperature::Temperature_StreamDataWriter>();    
		
	if (!msg_Writer) {
		std::cerr << "narrow_Writer failed to create a memory\n";
		return -1;
	}

	openDDS.mWait_For_Subscriber(1);

#if 0
	//Initialize Samples
	//Reliability::Message msg;
	std::this_thread::sleep_for(std::chrono::seconds(1));				
	int max_loop_count = 2000;
	for (int i = 0; i < max_loop_count; i++) {
		msg._id     = std::string("foo") + std::to_string(i);
		msg._name   = "foo";
		msg._count  = static_cast<long>(i);
		msg._expected = 	static_cast<long>(max_loop_count);

		openDDS.mSend_Sample<Reliability::Message, Reliability::MessageDataWriter_var>
		                    (msg, msg_Writer);	
	}
#else
	
	Temperature::Temperature_Stream msg;
	int max_loop_count = 10000;
	for (int ct = 0 ; ct < max_loop_count; ct++) {
		std::vector<Temperature::Temperature_Sys> temp(10, 
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
		std::this_thread::sleep_for(std::chrono::milliseconds(1)); 
	}
#endif
	std::cout << "Publisher Exits...\n";
	return 0;			
}
