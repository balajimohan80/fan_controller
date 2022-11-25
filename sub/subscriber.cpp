#include <iostream>

#include "openDDS_Pub_Sub.hpp"
#include "DataReaderListenerImpl.hpp"

#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/StaticIncludes.h>
#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

#include "ReliabilityTypeSupportImpl.h"


void on_Sample(Reliability::Message& msg) {
	std::cout << "--------------------------------------\n";
	std::cout << "ID    : " << msg._id << "\n";
	std::cout << "Name  : " << msg._name << "\n";
	std::cout << "Count : " << msg._count << "\n";
}

#if 1
using Listener_t = DataReaderListenerImpl<Reliability::MessageDataReader_var, 
                                          Reliability::MessageDataReader, Reliability::MessageSeq,
                                          Reliability::Message>;
#endif

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

	
	openDDS.mWait_For_Publisher();	
	std::cout << "Exiting Subscriber\n";
	return 0;		
}
