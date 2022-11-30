/*************************************************************************************************
*   \file      fan_control.hpp
*   \author    Balaji Mohan
*   \EmailID   balajimohan80@gmail.com
*   \date      11/29/2022
*   \brief     This is header file for Fan_Controller Class.
**************************************************************************************************/

#ifndef __FAN_CONTROL_H__
#define __FAN_CONTROL_H__

#include<vector>
#include<tuple>

#include "fan_ctrl_types.hpp"


class c_fan_Ctrl {
private:
	using fan_ctrl_t = std::tuple<temp_DegC_t, max_PWM_Count_t, fanctrl_HW_Reg>;
	//Setting Lowerbound Temperature 25 degC and 20% PWM duty cycle
	const linera_InterPol_t mC_Pt_1 = {25.0f, 0.2f};
	//Setting Upperbound Temperature 75 degc and 100% PWM duty Cycle
	const linera_InterPol_t mC_Pt_2 = {75.0f, 1.0f};
	const int mMax_Sub_System; 	
	std::vector<fan_ctrl_t> mFan_Controllers;
	std::vector<int> mSorting_Index;
	float32_t mPWM_Duty_Percent;
	
	float32_t mCompute_Interpolation(float32_t);
	
public:
	int mSet_temperature(float32_t, int);	
	float32_t mCompute_Duty_Cycle();
	int mGet_PWM_count(int, uint32_t &);
	void mPrint_Fan_Count();
	void mCompute_PWM_count(float32_t);
	float32_t mGetMax_Temp_Sensor();
	uint32_t mGetMax_PWM_Count();
	
	c_fan_Ctrl(std::vector<json_data_t>&);
	c_fan_Ctrl(const c_fan_Ctrl&)  = delete;
	c_fan_Ctrl(c_fan_Ctrl &&) = delete;
	c_fan_Ctrl& operator=(const c_fan_Ctrl &) = delete;
	c_fan_Ctrl& operator=(c_fan_Ctrl &&) = delete;
	~c_fan_Ctrl();
};

#endif
