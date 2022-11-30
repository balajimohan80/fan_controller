/*************************************************************************************************
*   \file      DataReaderListenerImpl.hpp
*   \author    Balaji Mohan
*   \EmailID   balajimohan80@gmail.com
*   \date      11/29/2022
*   \brief     This class creates the data reader implementation for OpenDDS. 
*              Application will register the callback function, upon every arrival of samples
*              a regsitered callback function gets called from OpenDDS reader thread.
**************************************************************************************************/

#ifndef __DATAREADER_LISTENER_IMPL_H__
#define __DATAREADER_LISTENER_IMPL_H__

#include <iostream>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/Service_Participant.h>

template<typename T>
using sample_t = std::function<void(T&)>;

template<typename T1, typename T2, typename T3, typename T4>
class DataReaderListenerImpl : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener> {	

public:
	DataReaderListenerImpl(sample_t<T4> func):call_back_func(func),seq_(), infos_() {};
	void on_requested_deadline_missed(DDS::DataReader_ptr , 
	                                  const DDS::RequestedDeadlineMissedStatus &nStat) {
		std::cerr << "on_requested_deadline_missed!!!\n";	
	}

	void on_requested_incompatible_qos(DDS::DataReader_ptr,
	                                   const DDS::RequestedIncompatibleQosStatus &nStat) {
		std::cerr << "on_requested_incompatible_qos!!!\n";
	}

	void on_liveliness_changed(DDS::DataReader_ptr,
	                           const DDS::LivelinessChangedStatus &nStat) {
		std::cerr << "on_liveliness_changed !!!\n";
	}

	void on_subscription_matched(DDS::DataReader_ptr,
	                             const DDS::SubscriptionMatchedStatus &nStat) {
		std::cerr << "on_subscription_matched !!!\n";
	}

	void on_sample_lost(DDS::DataReader_ptr,
	                    const DDS::SampleLostStatus &nStat) {
		std::cerr << "on_sample_lost !!!\n";
	}

	void on_sample_rejected(DDS::DataReader_ptr,
	                        const DDS::SampleRejectedStatus &nStat) {
		std::cerr << "on_sample_rejected !!!\n";
	}

	void on_data_available(DDS::DataReader_ptr reader) {
		T1 sample_Reader = T2::_narrow(reader);
		if (!sample_Reader) {
			std::cerr << "Failed to narrow Data Reader\n";
		} else {
			DDS::ReturnCode_t error = sample_Reader->take(seq_, infos_, DDS::LENGTH_UNLIMITED,
			                          DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE,
			                          DDS::ANY_INSTANCE_STATE);
			if (error == DDS::RETCODE_OK) {
				for (int i = 0; i < seq_.length(); i++) {
					if (infos_[i].valid_data) {
						call_back_func(seq_[i]);
					} else {
						std::cerr << "Invalid data !!!\n";
					}
				}
				sample_Reader->return_loan(seq_, infos_);	
			} else if (error == DDS::RETCODE_NO_DATA) {
				return;
			} else {
				std::cerr << "on_data_available: take_next_sample failed !!!\n";
			} 
		} 
	}
private:
	sample_t<T4> call_back_func;
	T3 seq_;
	DDS::SampleInfoSeq infos_;
};

#endif
