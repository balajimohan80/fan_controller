#include <iostream>

#include "openDDS_Pub_Sub.hpp" 
#include <dds/DCPS/Service_Participant.h>

#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include "ReliabilityTypeSupportImpl.h"
#include <chrono>
#include <thread>
int main(int argc, char *argv[]) {
	cOpenDDS_Pub_Sub openDDS;
	if (0 != openDDS.mCreateParticipant(argc, argv, 42)) {
		std::cerr << "Not able to create participant!!!\n";
		return -1;
	}

	std::string topic_name = "transaction_test";
	Reliability::MessageTypeSupport_var ts = new Reliability::MessageTypeSupportImpl;
	if (ts == nullptr) {
		std::cerr << "Not able to create memory for Type Support!!!\n";
		return -1;
	}

	if (ts && 0 != openDDS.mCreateTopic<Reliability::MessageTypeSupport_var>(topic_name,  ts)) {
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

	Reliability::MessageDataWriter_var msg_Writer = 
	openDDS.mNarrow_Writer<Reliability::MessageDataWriter_var, Reliability::MessageDataWriter>();
	
	if (!msg_Writer) {
		std::cerr << "narrow_Writer failed to create a memory\n";
		return -1;
	}

	openDDS.mWait_For_Subscriber(1);

	//Initialize Samples
	Reliability::Message msg;

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

	std::cout << "Publisher Exits...\n";
	return 0;			
}
