#ifndef __OPEN_DDS_PUB_SUB_HPP__
#define __OPEN_DDS_PUB_SUB_HPP__

#include <iostream>

#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/WaitSet.h>

class cOpenDDS_Pub_Sub {
public:
	int mCreateParticipant(int argc, char *argv[], int nDomain_ID) {
		//Handling Command line args to initialize Domain Participant Factory
		std::cout << "mCreateParticipant: " << argv[1] << " " << argv[2] << "\n";	
		mDpf = TheParticipantFactoryWithArgs(argc, argv);

		//Create Domain Participant
		mParticipant = mDpf->create_participant(nDomain_ID, //domain ID
		                                        PARTICIPANT_QOS_DEFAULT,
		                                        0,  // no listener
		                                        OpenDDS::DCPS::DEFAULT_STATUS_MASK); 
		if (!mParticipant) {
			std::cerr << "Not able to create Participant \n";
			return -1;
		}
		return 0;				
	} 

	template<typename T1>
	int mCreateTopic(std::string& str_Topic_Name, T1 nType_Support) {
		if (nType_Support == nullptr) return -1;
		if (nType_Support->register_type(mParticipant, "") != DDS::RETCODE_OK) {
			std::cerr << "Failed to register type support with participant\n";
			return -1;
		}

		CORBA::String_var type_name = nType_Support->get_type_name();
		//Creating Topic with DataType and register with Domain Particpant
		mTopic = mParticipant->create_topic(str_Topic_Name.c_str(), type_name,
		                                    TOPIC_QOS_DEFAULT, 0,
		                                    OpenDDS::DCPS::DEFAULT_STATUS_MASK);  	
				
		if (!mTopic) {
			std::cerr << "Failed to create Topic\n";
			return -1;
		}
		return 0;
	}

	int mCreatePublisher() {
		//Create Publisher and register with Domain participant
		mPub = mParticipant->create_publisher(PUBLISHER_QOS_DEFAULT, 0,
		                                      OpenDDS::DCPS::DEFAULT_STATUS_MASK);
		if (!mPub) {
			std::cerr << "Failed to create Publisher\n";
			return -1;
		}
		return 0;
	}

	
	int mCreateSubscriber() {
		mSub = mParticipant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, 0,
		                                       OpenDDS::DCPS::DEFAULT_STATUS_MASK);
		if (!mSub) {
			std::cerr << "Failed to create Subsriber\n";
			return -1;
		}
		return 0;
	}

	int mCreateDataWriter() {
		mPub->get_default_datawriter_qos(mDataWriter_qos);
		// RELIABLE/KEEP_ALL/10/10 works
		mDataWriter_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
		mDataWriter_qos.reliability.max_blocking_time.sec = 1;
		mDataWriter_qos.reliability.max_blocking_time.nanosec = 0;
		mDataWriter_qos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
		mDataWriter_qos.resource_limits.max_samples = 1000;
		mDataWriter_qos.resource_limits.max_samples_per_instance = 1000;
		mDataWriter_qos.liveliness.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
		mDataWriter_qos.liveliness.lease_duration.sec = 1;
		mDataWriter_qos.liveliness.lease_duration.nanosec = 0;

		// Create DataWriter
		mDataWriter = mPub->create_datawriter(mTopic, mDataWriter_qos, 0, 
		                                      OpenDDS::DCPS::DEFAULT_STATUS_MASK);
		if (!mDataWriter) {
			std::cerr << "Failed to create data writer\n";
			return -1;
		}
		return 0;
	}

	int mCreateDataReader(DDS::DataReaderListener_var listener) {
		// RELIABLE/KEEP_LAST/10 works
		mSub->get_default_datareader_qos(mDataReader_qos);
		mDataReader_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
		mDataReader_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
		mDataReader_qos.history.depth = 10;

		// Create DataReader
		mDataReader = mSub->create_datareader(mTopic, mDataReader_qos, listener, 
		                                      OpenDDS::DCPS::DEFAULT_STATUS_MASK);
		if (!mDataReader) {
			std::cerr << "Failed to create DataReader\n";
		}
		return 0;
	}

	template<typename T1, typename T2>
	T1 mNarrow_Writer() {
		T1 nWriter = T2::_narrow(mDataWriter);
		if (!nWriter) {
			std::cerr << "Failed to narrow DataWriter!!!\n";
			return nullptr;	
		}
		return nWriter;
	}

	void mWait_For_Subscriber(int no_of_Subscribers) {
		DDS::StatusCondition_var condition = mDataWriter->get_statuscondition();
		condition->set_enabled_statuses(DDS::PUBLICATION_MATCHED_STATUS);

		DDS::WaitSet_var ws = new DDS::WaitSet;
		ws->attach_condition(condition);

		while (true) {
			DDS::PublicationMatchedStatus matches;
			if (mDataWriter->get_publication_matched_status(matches) != ::DDS::RETCODE_OK) {
				std::cerr << "Get_publication_matched_status failed!!!\n";
			}
			if (matches.current_count >= no_of_Subscribers) break;

			DDS::ConditionSeq conditions;
			DDS::Duration_t timeout = { 60, 0 };
			if (ws->wait(conditions, timeout) != DDS::RETCODE_OK) {
				std::cerr << "Wait failed!!!\n";
			} 
		}
	} 

	template<typename T1, typename T2>
	int mSend_Sample(T1& nMsg, T2 nWriter) {
		DDS::ReturnCode_t nError = DDS::RETCODE_TIMEOUT;
		while (nError == DDS::RETCODE_TIMEOUT) {
			std::cerr << "Pub: Trying to Send...\n";
			nError = nWriter->write( nMsg, DDS::HANDLE_NIL);
			if (nError == DDS::RETCODE_TIMEOUT) {
				std::cerr << "Timeout, resending !!!\n";
			} else if (nError != DDS::RETCODE_OK) {
				std::cerr << "Write returned " << nError << "\n";
			}
		}
		return 0;
	}

	void mcleanup() {
		mParticipant->delete_contained_entities();
		mDpf->delete_participant(mParticipant);
		TheServiceParticipant->shutdown();
		std::cout << "OpenDDS cleanup done\n";
	}
	~cOpenDDS_Pub_Sub() {
		mcleanup();
	}	

private:
	DDS::DomainParticipantFactory_var mDpf;
	DDS::DomainParticipant_var mParticipant;
	DDS::Topic_var mTopic;
	DDS::Publisher_var mPub; 
	DDS::Subscriber_var mSub;
	DDS::DataWriter_var mDataWriter;
	DDS::DataWriterQos mDataWriter_qos;
	DDS::DataReader_var mDataReader;
	DDS::DataReaderQos mDataReader_qos;
};
#endif
