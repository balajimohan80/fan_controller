#ifndef __CIRCULAR_DOUBLE_LIST_HPP__
#define __CIRCULAR_DOUBLE_LIST_HPP__
#include<iostream>
#include<vector>
#include<cstdlib>
#include<cstring>
#include<atomic>

template<typename T>
class cDouble_Link_List {
public:
	cDouble_Link_List *mNext;
	cDouble_Link_List *mPrev;
	T                 *mData;
	cDouble_Link_List():mNext(nullptr), mPrev(nullptr), mData(nullptr) {};
	cDouble_Link_List(T *data_ptr):mNext(nullptr), mPrev(nullptr), mData(data_ptr) {};
	~cDouble_Link_List();
};

template<typename T>
class cCir_Link_List {
private:
	using up_counter_t = uint32_t;
	cDouble_Link_List<T> *mList;
	std::vector<T> mData_Container;
	std::atomic<cDouble_Link_List<T>*> mHead;
	cDouble_Link_List<T> *mCurr_Pop_List;
	cDouble_Link_List<T> *mLast_Popped_List;
	std::atomic<up_counter_t> mWrite_Counter;	
	std::atomic<up_counter_t> mRead_Counter;
	const uint32_t max_Buff_Size;

	void mAdjust_Read_Counter_Offset() {
		up_counter_t nWrite_count = mWrite_Counter.load()-1; 
		up_counter_t nRead_count  = mRead_Counter.load();
		up_counter_t nDelta_count = nWrite_count - nRead_count;
		std::cout << "Delta: " << nDelta_count << ", " << nWrite_count << ", " << nRead_count << "\n";
		if (nDelta_count > max_Buff_Size) {
			up_counter_t nMod = nDelta_count % max_Buff_Size;
			nRead_count += nMod;
			while(nMod--) {
				mCurr_Pop_List=mCurr_Pop_List->mNext;
			}
		 	up_counter_t nQuotient = nDelta_count / max_Buff_Size;
			nQuotient = (nQuotient > 0) ? --nQuotient : nQuotient;
			nRead_count += (nQuotient * max_Buff_Size);
			std::cout << "Changes RdCount: " << nRead_count << "\n";
			mRead_Counter.store(nRead_count);
		}  
	}

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

	
	cCir_Link_List(size_t nCount):
	mList(static_cast<cDouble_Link_List<T>*>(std::calloc(nCount,sizeof(cDouble_Link_List<T>)))),
	mData_Container(nCount),max_Buff_Size(nCount) {
		std::cout << "Sze: " << mData_Container.size() << "\n";
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
			
		mCurr_Pop_List = mHead;
		mLast_Popped_List = nullptr;
		mWrite_Counter.store(0);
		mRead_Counter  = 0;
		mprint_addr();
	}  
	
	~cCir_Link_List() {
		std::free(mList);
	}



	void str_Element(T& msg) {
		cDouble_Link_List<T> *n_Head = mHead.load();
		T* data_ptr = n_Head->mData;
		*data_ptr = msg;
		n_Head = n_Head->mNext;
		mHead.exchange(n_Head);
		mWrite_Counter.fetch_add(1);    
	}	
	
	T* pop() {
		T* ret_Ptr = nullptr;
		if (mRead_Counter.load() ==  mWrite_Counter.load()) return  ret_Ptr;
#if 0
		cDouble_Link_List<T> *nTemp = mHead->mPrev;
		mHead->mPrev                = nTemp->mPrev;
		cDouble_Link_List<T> *nPrev = nTemp->mPrev;
		nPrev->mNext                = nTemp->mNext;
		mCurr_Pop_List                = nTemp;
		return mCurr_Pop_List->mData; 	
#else
		mAdjust_Read_Counter_Offset();
		cDouble_Link_List<T> *nPrev = mCurr_Pop_List->mPrev;
		nPrev->mNext                = mCurr_Pop_List->mNext;
		cDouble_Link_List<T> *nNext = mCurr_Pop_List->mNext;
		nNext->mPrev                = nPrev;
		ret_Ptr                     = mCurr_Pop_List->mData;
		mLast_Popped_List           = mCurr_Pop_List;
		mCurr_Pop_List              = mCurr_Pop_List->mNext;
		mRead_Counter.fetch_add(1);
//		mprint_addr();
		return ret_Ptr; 	
#endif	
	}

	void push(T *take) {
		if (mLast_Popped_List == nullptr || mLast_Popped_List->mData != take) {
			std::cerr << "Push fails, please check Pop!!!\n";
			return;
		} 
		cDouble_Link_List<T> *nTemp = mHead.load()->mNext;
		mHead.load()->mNext = mLast_Popped_List;
		mLast_Popped_List->mPrev = mHead.load();
		mLast_Popped_List->mNext = nTemp;
		nTemp->mPrev             = mLast_Popped_List;
		mLast_Popped_List        = nullptr;
	}
}; 
#endif
