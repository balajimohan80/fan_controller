#include <iostream>

#include "circular_Linked_List.hpp"
#include "openDDS_Pub_Sub.hpp"
#include "DataReaderListenerImpl.hpp"

#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/StaticIncludes.h>
#ifdef ACE_AS_STATIC_LIBS
#  include <dds/DCPS/RTPS/RtpsDiscovery.h>
#  include <dds/DCPS/transport/rtps_udp/RtpsUdp.h>
#endif

//#include "ReliabilityTypeSupportImpl.h"
#include "TemperatureTypeSupportImpl.h"

//int gcSize = 5;
cCir_Link_List<Temperature::Temperature_Stream> gcList(5); 

#if 0
template<typename T1>
class Double_List {
public:
	Double_List *mNext;
	Double_List *mPrev;
	T1 *mptr;
}; 

Double_List<Temperature::Temperature_Stream> *clist = nullptr;
#endif

#if 0
void on_Sample(Reliability::Message& msg) {
	std::cout << "--------------------------------------\n";
	std::cout << "ID    : " << msg._id << "\n";
	std::cout << "Name  : " << msg._name << "\n";
	std::cout << "Count : " << msg._count << "\n";
}
#else
void on_Sample(Temperature::Temperature_Stream &msg) {
//	std::cout << "----------" << msg._seq_no << "-------------\n";
#if 0
	std::vector<Temperature::Temperature_Sys> &vec = msg._Vec_Temperature;
	for (Temperature::Temperature_Sys &s : vec) {
		std::cout << "System_Id : " << s._system_ID << "\n";
		std::cout << "Deg_C     : " << s._deg_C << "\n";
	}
#else
	gcList.str_Element(msg);
#endif		
}
#endif

#if 0
using Listener_t = DataReaderListenerImpl<Reliability::MessageDataReader_var, 
                                          Reliability::MessageDataReader, Reliability::MessageSeq,
                                          Reliability::Message>;
#else
using Listener_t = DataReaderListenerImpl<Temperature::Temperature_StreamDataReader_var,
                                          Temperature::Temperature_StreamDataReader,
                                          Temperature::Temperature_StreamSeq,
                                          Temperature::Temperature_Stream>;
#endif
int main(int argc, char *argv[]) {
	cOpenDDS_Pub_Sub openDDS;
        if (0 != openDDS.mCreateParticipant(argc, argv, 42)) {
                std::cerr << "Not able to create participant!!!\n";
                return -1;
        }
	
        std::string topic_name = "transaction_test";
#if 0
        Reliability::MessageTypeSupport_var ts = new Reliability::MessageTypeSupportImpl;
#else
	Temperature::Temperature_StreamTypeSupport_var ts = 
	new Temperature::Temperature_StreamTypeSupportImpl;
#endif     
   	if (ts == nullptr) {
                std::cerr << "Not able to create memory for Type Support!!!\n";
                return -1;
        }
#if 0
       if (ts && 0 != openDDS.mCreateTopic<Reliability::MessageTypeSupport_var>(topic_name,  ts)) {
#else
	if (ts && 0 != openDDS.mCreateTopic<Temperature::Temperature_StreamTypeSupport_var>
	               (topic_name, ts)) {
#endif      
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
	std::cout << "Waiting for Publisher...\n";	
	openDDS.mWait_For_Publisher(1);	
	while(1) {
		Temperature::Temperature_Stream *m_ptr = gcList.pop();
		if (m_ptr != nullptr)  {
			std::cout << "---------------Main: " << m_ptr->_seq_no << "---------\n";
			std::vector<Temperature::Temperature_Sys> &vec = m_ptr->_Vec_Temperature;
			
	for (Temperature::Temperature_Sys &s : vec) {
		std::cout << "System_Id : " << s._system_ID << "\n";
		std::cout << "Deg_C     : " << s._deg_C << "\n";
	}
			gcList.push(m_ptr);
		}
	}
	std::cout << "Exiting Subscriber\n";
	return 0;		
}
