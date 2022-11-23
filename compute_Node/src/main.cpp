#include<iostream>
#include<vector>
#include<string>
#include<random>
#include<ctime>
#include<cstdlib>

#include "fan_control.hpp"
#include "parse_fan_control_json.hpp" 

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
	if (argc < 1) {
		std::cout << "1. Pass JSON Filename and its path\n";
	}
	std::string nJson_file_name = std::string(std::string(argv[1]));

	c_Parse_Json nparse_JSON(nJson_file_name);
	std::vector<json_data_t> nVec;
	if (true != nparse_JSON.mGet_Json_Val(nVec)) {
		std::cerr << "Parsing JSON file faied!!!\n";
	}
		
	c_fan_Ctrl cFan_Ctrl(nVec);
	
	while (1) {
		std::cout << "Common Duty Cycle: " << cFan_Ctrl.mCompute_Duty_Cycle() << "\n";
		cFan_Ctrl.mPrint_Fan_Count();
		int index = 0;
		std::cout << "Enter Temp Index to modify Temperature: ";
		std::cin >> index; 	
		float32_t f_temp_val = get_temperature(index);
		if (0 != cFan_Ctrl.mSet_temperature(f_temp_val, index))
			std::cout << "Not able to set temp\n";	
		
	}
	return 0;
} 
