#include<iostream>
#include<vector>
#include<string>
#include<random>
#include<ctime>
#include<cstdlib>

#include "fan_control.hpp"

void fill_random_Val(std::vector<uint32_t> &nums, int low_bound, int upper_bound) {
	std::random_device rd;
	std::uniform_int_distribution<> distribute(low_bound, upper_bound);
	
	for (int i = 0; i < nums.size(); i++) {
		nums[i] = distribute(rd);
	}    

}

void print_1d_Vec(std::vector<uint32_t> &nums) {
	std::cout << "Max PWM Counts: \n";
	for (int i = 0; i < nums.size(); i++) {
		std::cout << i << ": " << nums[i] << "\n";
	}
}

float32_t get_temperature(int i) {
	float32_t val;
	std::cout << i << ": Enter Temperature in DEG C: ";
	std::cin  >> val;
	return val;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		std::cout << "1. Total Number of Subsystems\n";
		std::cout << "2. No Of Iteration\n";
	}
	int no_Of_Subsystem = std::stoi(std::string(argv[1]));
	int no_Of_Iteration = std::stoi(std::string(argv[2]));
	
	std::vector<uint32_t> fan_Pwm_Count(no_Of_Subsystem);
	fill_random_Val(fan_Pwm_Count, 70, 32000);
	print_1d_Vec(fan_Pwm_Count);

	c_fan_Ctrl cFan_Ctrl(fan_Pwm_Count);
	for (int i = 0 ; i < no_Of_Subsystem; i++) {
		cFan_Ctrl.mSet_temperature(get_temperature(i), i);
	}

	std::cout << "Common Duty Cycle: " << cFan_Ctrl.mCompute_Duty_Cycle() << "\n";
	cFan_Ctrl.mPrint_Fan_Count();
	return 0;
} 
