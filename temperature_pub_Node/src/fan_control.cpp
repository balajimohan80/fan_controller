#include<iostream>
#include<algorithm>

#include "fan_control.hpp"

c_fan_Ctrl::c_fan_Ctrl(std::vector<uint32_t>& nfan_max_PWM_ct):mMax_Sub_System(nfan_max_PWM_ct.size()),
	mFan_Controllers(nfan_max_PWM_ct.size()), 
	mSorting_Index(nfan_max_PWM_ct.size()), 
	mPWM_Duty_Percent(20) {

	for(int i = 0; i < mMax_Sub_System; i++) {
		std::get<1>(mFan_Controllers[i]) = nfan_max_PWM_ct[i];
		mSorting_Index[i]                = i;
	}
}

c_fan_Ctrl::~c_fan_Ctrl(){}

int c_fan_Ctrl::mSet_temperature(float32_t ndeg_C, int index) {
	if(index >= mMax_Sub_System) {
		return -1;
	}

	std::get<0>(mFan_Controllers[index])  = ndeg_C;
	return 0;
}



float32_t c_fan_Ctrl::mCompute_Interpolation(float32_t nTemp_DegC) {
	if (nTemp_DegC <= mC_Pt_1.first)  return mC_Pt_1.second;
	if (nTemp_DegC >= mC_Pt_2.first)  return mC_Pt_2.second;
	const float32_t y2_y1              = mC_Pt_2.second - mC_Pt_1.second;
	const float32_t x2_x1              = mC_Pt_2.first - mC_Pt_1.first;
	const float32_t y2_y1_div_by_x2_x1 = static_cast<float32_t>(y2_y1) / x2_x1;
	const float32_t x_x1               = nTemp_DegC - mC_Pt_1.first;
	const float32_t x_x1_div_by_x2_x1  = x_x1 / x2_x1; 	 
	float32_t ntemp    = y2_y1_div_by_x2_x1 * x_x1_div_by_x2_x1;
	ntemp             += mC_Pt_2.second;
	return ntemp;  
}

void c_fan_Ctrl::mCompute_PWM_count(float32_t nPWM_Duty_Cycle) {
	for (fan_ctrl_t &fan : mFan_Controllers) {
		const float32_t fan_PWM_Count = static_cast<float32_t>(std::get<1>(fan)) * nPWM_Duty_Cycle;
		std::get<2>(fan) = static_cast<uint32_t>(fan_PWM_Count);  
	}	
}

float32_t c_fan_Ctrl::mCompute_Duty_Cycle() {
	std::sort(mSorting_Index.begin(), mSorting_Index.end(), 
			[&, this] (const int a, const int b)->bool {
			return std::get<0>(mFan_Controllers[a]) > std::get<0>(mFan_Controllers[b]);
		});
	mPWM_Duty_Percent = mCompute_Interpolation(std::get<0>(mFan_Controllers[mSorting_Index[0]]));
	mCompute_PWM_count(mPWM_Duty_Percent);
	return mPWM_Duty_Percent;
}

int c_fan_Ctrl::mGet_PWM_count(int index, uint32_t &val) {
	if (index >= mMax_Sub_System) return -1;
	val = std::get<2>(mFan_Controllers[index]);
	return 0;
}


void c_fan_Ctrl::mPrint_Fan_Count() {
	int ct = 0;
	for (const fan_ctrl_t& fan: mFan_Controllers) {
		std::cout << ct++ << ": " << std::get<2>(fan) << "\n";
	}	
}

 
