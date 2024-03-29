/*************************************************************************************************
*   \file      circular_Linked_List.hpp
*   \author    Balaji Mohan
*   \EmailID   balajimohan80@gmail.com
*   \date      11/29/2022
*   \brief     This ring buffer is designed based on doubly linked list.
*              This list can traverse reverse and front of the list.
*              It has known limitation that, it can work with single producer and consumer thread.
*              Producer thread can do deep copy the data into the one of the readily 
*              available node from the list.
*              To process the data by the consumer thread, it will first detach only one node
*              from the list and process it and then attach the same node to the list.
**************************************************************************************************/
#ifndef __CIRCULAR_DOUBLE_LIST_HPP__
#define __CIRCULAR_DOUBLE_LIST_HPP__
#include<iostream>
#include<vector>
#include<cstdlib>
#include<cstring>
#include<atomic>

using up_counter_t = uint32_t;

template<typename T1>
class cDouble_Link_List {
public:
	cDouble_Link_List<T1> *mNext;
	cDouble_Link_List<T1> *mPrev;
	T1 *mData;
	cDouble_Link_List<T1>():mNext(nullptr), mPrev(nullptr), mData(nullptr){};
	cDouble_Link_List(T1 *data_ptr):mNext(nullptr), mPrev(nullptr), mData(data_ptr) {};
	~cDouble_Link_List<T1>();
};

template<typename T>
class cCir_Link_List {
private:
	cDouble_Link_List<T> *mList;
	std::vector<T> mData_Container;
	std::atomic<cDouble_Link_List<T>*> mHead;
	std::atomic<cDouble_Link_List<T>*> mCurr_Pop_List;
	cDouble_Link_List<T> *mLast_Popped_List;
	const uint32_t max_Buff_Size;


public:
	void mprint_addr() {
		cDouble_Link_List<T> *nTemp = mHead.load();
		cDouble_Link_List<T> *nEnd  = nTemp;
		int i = 0;
		for (; i < max_Buff_Size && nTemp->mNext != nEnd; i++) {
			printf("%d: %p %p %p\n",i, nTemp->mPrev, nTemp, nTemp->mNext);
			nTemp=nTemp->mNext; 
		}
		
		 printf("%d: %p %p %p\n",i, nTemp->mPrev, nTemp, nTemp->mNext);
	}

	
	cCir_Link_List<T>(size_t nCount):
	mList(static_cast<cDouble_Link_List<T>*>(std::calloc(nCount,sizeof(cDouble_Link_List<T>)))),
	mData_Container(nCount),max_Buff_Size(nCount) {
		cDouble_Link_List<T> *n_nxt  = &mList[1];
		cDouble_Link_List<T> *n_prev = &mList[nCount-1];
		cDouble_Link_List<T> *n_forward = mList;
		mHead.store(mList);
		T* n_Vec_ptr = mData_Container.data();	
		for (int i = 0; i < nCount; i++) {		 
			n_forward->mNext = n_nxt;
			n_forward->mPrev = n_prev;
			n_forward->mData = n_Vec_ptr++;
			n_prev           = n_forward;
			n_forward        = n_nxt;
	  	n_nxt            = &n_nxt[1]; 	 
		}
		cDouble_Link_List<T> *n_End  = &mList[nCount-1];
		n_End->mNext = mList;
			
		mCurr_Pop_List.store(mHead.load());
		mLast_Popped_List = nullptr;
	}  
	
	~cCir_Link_List<T>() {
		std::free(mList);
	}



	void str_Element(T& msg) {
		cDouble_Link_List<T> *n_Head = mHead.load();
		T* data_ptr = n_Head->mData;
		*data_ptr = msg;
		n_Head = n_Head->mNext;
		cDouble_Link_List<T> *n_Curr_Pop_List = mCurr_Pop_List.load();
		if (n_Curr_Pop_List == n_Head) {
			n_Curr_Pop_List = n_Curr_Pop_List->mNext;
			mCurr_Pop_List.exchange(n_Curr_Pop_List);
		}
		mHead.exchange(n_Head);
	}	
	
	
	T* pop() {
		T* ret_Ptr = nullptr;
	 	cDouble_Link_List<T> *nCurr_Pop_List = mCurr_Pop_List.load();
		if (nCurr_Pop_List == mHead) return  ret_Ptr;
		
		cDouble_Link_List<T> *nPrev = nCurr_Pop_List->mPrev;
		nPrev->mNext                = nCurr_Pop_List->mNext;
		cDouble_Link_List<T> *nNext = nCurr_Pop_List->mNext;
		nNext->mPrev                = nPrev;
		ret_Ptr                     = nCurr_Pop_List->mData;
		mLast_Popped_List           = nCurr_Pop_List;
		nCurr_Pop_List              = nCurr_Pop_List->mNext;
		mCurr_Pop_List.exchange(nCurr_Pop_List);
		return ret_Ptr; 	
	}

	void push(T *take) {
		if (mLast_Popped_List == nullptr || mLast_Popped_List->mData != take) {
			std::cerr << "Push fails, please check Pop!!!\n";
			return;
		} 
		cDouble_Link_List<T> *nHead = mHead.load();
		cDouble_Link_List<T> *nNext = nHead->mNext;
		nHead->mNext                       = mLast_Popped_List;
		mLast_Popped_List->mNext           = nNext;
		mLast_Popped_List->mPrev           = nHead;
		nNext->mPrev                       = mLast_Popped_List;
		mHead.exchange(nHead);
		mLast_Popped_List        = nullptr;
	}
}; 
#endif
