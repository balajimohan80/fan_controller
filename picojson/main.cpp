#include<iostream>
#include<fstream>
#include<string>

#include "picojson.h"


int readFile_Into_String(const std::string& nsfile_Name_And_Path, std::string &nsfile_str) {
	std::ifstream njson_File(nsfile_Name_And_Path);
	if (!njson_File.is_open()) {
		std::cerr << "Not able to open JSON file: " << nsfile_Name_And_Path << "\n";
		return -1; 
	}

	nsfile_str = std::string(std::istreambuf_iterator<char>(njson_File),
	                         std::istreambuf_iterator<char>());
	return 0; 
} 

using data_t = std::pair<float, uint32_t>;

int get_no_Of_fans(std::string& json_data, double& no_of_fans) {
	picojson::value json_val;

	std::string err = picojson::parse(json_val, json_data);
	if (!err.empty()) {
		std::cerr << "Not able to parse JSON data: " << err << "\n";
		return -1;
	}

	if (true != json_val.get("Total_Number_Of_Fans").is<double>()) {
		std::cerr << "Total_Number_Of_Fans key is missing!!!\n";
		return -1;
	}

	no_of_fans = json_val.get("Total_Number_Of_Fans").get<double>();
	return 0;
}

int get_fan_info(std::vector<data_t> &nums, std::string& json_data) {
	picojson::value json_val;

	std::string err = picojson::parse(json_val, json_data);
	if (!err.empty()) {
		std::cerr << "Not able to parse JSON data: " << err << "\n";
		return -1;
	}

	if (true != json_val.get("Fan_Info").is<picojson::object>() || 
	    nums.size() != json_val.get("Fan_Info").get<picojson::object>().size()) {
		std::cerr << "Fan_Info not matched, please check JSON file!!!\n";
		return -1;
	}
	
	picojson::object &obj = json_val.get("Fan_Info").get<picojson::object>();
	int count = 1;
	for (picojson::object::iterator it = obj.begin(); it != obj.end(); it++, count++) {
		if (it->first != std::to_string(count))  {
			std::cerr << "Count value not matched on JSON file (ct : " << count << ")\n";
			return -1;
		}
		if (true != it->second.is<picojson::array>()) {
			std::cerr << "Array not matched on JSON!!!\n";
			return -1;
		}
		picojson::array &arr = it->second.get<picojson::array>();
		if (arr.size() != 2) {
			std::cerr << "Inside arrary element size not matched on JSON!!!\n";
		}
		
		if (true != arr[0].is<double>() && 
		    true != arr[1].is<double>()) {
			std::cerr << "Element Data type not matched on JSON!!!\n";
			return -1;
		}
		data_t temp = {static_cast<float>(arr[0].get<double>()), 
		               static_cast<uint32_t>(arr[1].get<double>())};
		nums[count-1] = temp;	
	}	

	return 0;
}

void print_vals(std::vector<data_t>& nums) {
	for (data_t &d : nums) {
		std::cout << d.first << ", " << d.second << "\n";
	}
}

int main() {
	std::string json_Str;
	const std::string json_f_name = "fan_controller.json";
	if (0 != readFile_Into_String(json_f_name, json_Str)) {
		std::cerr << "Not able to read JSON file: " << json_f_name << "\n";
		return -1;
	}
	
	double no_of_fans = 0.0;
	if (0 != get_no_Of_fans(json_Str, no_of_fans)) {
		std::cout << "Not able to parse it!!!\n";
		return -1;
	}
	std::vector<data_t> nums(static_cast<int>(no_of_fans));
	std::cout << "Size: " << nums.size() << "\n";
	if (0 != get_fan_info(nums, json_Str))
		std::cout << "Not able to parse JSON!!!\n";
	print_vals(nums);
	return 0;
}
