/*************************************************************************************************
*   \file      fan_control.cpp
*   \author    Balaji Mohan
*   \EmailID   balajimohan80@gmail.com
*   \date      11/29/2022
*   \brief     This class act as an interface/map between Temperature subsystem and Fan Controller.
*              Each temp sensor is assigned with unique ID. This Unique ID is mapped with
*              Fan Controller. Compute function will sort and identify the highest temperature
*              subsytem. Based on the highest temperature will compute PWM duty cycle based on
*              linear interpolation. Based on the common PWM duty cycle, it will find PWM
*              counts on each FAN controller.  
**************************************************************************************************/
#include<iostream>
#include<algorithm>

#include "fan_control.hpp"
#include "fan_ctrl_types.hpp"

c_fan_Ctrl::c_fan_Ctrl(std::vector<json_data_t>& nfan_max_PWM_ct):
	mMax_Sub_System(nfan_max_PWM_ct.size()),
	mFan_Controllers(nfan_max_PWM_ct.size()), 
	mSorting_Index(nfan_max_PWM_ct.size()), 
	mPWM_Duty_Percent(20) {

	for(int i = 0; i < mMax_Sub_System; i++) {
		std::get<0>(mFan_Controllers[i]) = nfan_max_PWM_ct[i].first;
		std::get<1>(mFan_Controllers[i]) = nfan_max_PWM_ct[i].second;
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
	const float32_t y2_y1_div_by_x2_x1 = y2_y1 / x2_x1;
	const float32_t x_x1               = nTemp_DegC - mC_Pt_1.first;
	float32_t ntemp                    = y2_y1_div_by_x2_x1 * x_x1;
	ntemp                             += mC_Pt_1.second;
	return ntemp;  
}

void c_fan_Ctrl::mCompute_PWM_count(float32_t nPWM_Duty_Cycle) {
	for (fan_ctrl_t &fan : mFan_Controllers) {
		double fan_PWM_Count = static_cast<double>(nPWM_Duty_Cycle);
		fan_PWM_Count *= static_cast<double>(std::get<1>(fan));
		std::get<2>(fan) = static_cast<uint32_t>(fan_PWM_Count);  
	}
}

float32_t c_fan_Ctrl::mCompute_Duty_Cycle() {
	std::sort(mSorting_Index.begin(), mSorting_Index.end(), 
			[&, this] (const int a, const int b)->bool {
			return std::get<0>(mFan_Controllers[a]) > std::get<0>(mFan_Controllers[b]);
	});
	mPWM_Duty_Percent = mCompute_Interpolation(std::get<0>(mFan_Controllers[mSorting_Index[0]]));
	//mCompute_PWM_count(mPWM_Duty_Percent);
	return mPWM_Duty_Percent;
}

float32_t c_fan_Ctrl::mGetMax_Temp_Sensor() {
	return std::get<0>(mFan_Controllers[mSorting_Index[0]]);
}

uint32_t c_fan_Ctrl::mGetMax_PWM_Count() {
	return std::get<1>(mFan_Controllers[mSorting_Index[0]]);
}

int c_fan_Ctrl::mGet_PWM_count(int index, uint32_t &val) {
	if (index >= mMax_Sub_System) return -1;
	val = std::get<2>(mFan_Controllers[index]);
	return 0;
}

void c_fan_Ctrl::mPrint_Fan_Count() {
	int ct = 0;
	for (int index : mSorting_Index) {
		fan_ctrl_t& fan = mFan_Controllers[index];
		std::cout << " FAN Controller "   << ct+1; 
		std::cout << ": Max_PWM_Count = " << std::get<1>(fan);
		std::cout << " HW REG = "         << std::get<2>(fan) << "\n";
		ct++;
	}	
}

 
