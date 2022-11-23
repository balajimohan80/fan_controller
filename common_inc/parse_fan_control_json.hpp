#ifndef __PARSE_JSON_HPP__
#define __PARSE_JSON_HPP__

#include<iostream>
#include<string>
#include<vector>
#include<fstream>

#include "picojson.h"
#include "fan_ctrl_types.hpp"

class c_Parse_Json {
	public:
		c_Parse_Json(const std::string& json_f_name):mjson_file_name(json_f_name) {
		}; 

		bool mGet_Json_Val(std::vector<json_data_t> &nums) {
			if (0 != mreadFile_Into_String(mjson_file_name, mjson_str)) {
				return false;
			}

			double no_of_fans = 0.0;
			if (0 != mget_no_Of_fans(mjson_str, no_of_fans)) {
				return false;	
			}	

			std::vector<json_data_t> temp_data(static_cast<int>(no_of_fans));	
			if (true != mget_fan_info(temp_data, mjson_str)) {
				return false;
			}

			nums = std::move(temp_data);
			return true;
		}

		int mGet_total_fans() {
			int ret = -1;
			
			if (0 != mreadFile_Into_String(mjson_file_name, mjson_str)) {
				return -1;
			}

			double no_of_fans = 0.0;
			if (0 != mget_no_Of_fans(mjson_str, no_of_fans)) { 
				return -1;
			}

			ret = static_cast<int>(no_of_fans);
			return ret;	
		}
			
	private:
		const std::string mjson_file_name;		
		std::string mjson_str;	
		
			
	int mreadFile_Into_String(const std::string& nsfile_Name_And_Path, std::string &nsfile_str) {
		std::ifstream njson_File(nsfile_Name_And_Path);
		if (!njson_File.is_open()) {
			std::cerr << "Not able to open JSON file: " << nsfile_Name_And_Path << "\n";
			return -1; 
		}

		nsfile_str = std::string(std::istreambuf_iterator<char>(njson_File),
	                         std::istreambuf_iterator<char>());
		return 0; 
	} 
	

	int mget_no_Of_fans(std::string& json_data, double& no_of_fans) {
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

	int mget_fan_info(std::vector<json_data_t> &nums, std::string& json_data) {
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
			json_data_t temp = {static_cast<float>(arr[0].get<double>()), 
		               static_cast<uint32_t>(arr[1].get<double>())};
			nums[count-1] = temp;	
		}	

		return 0;
	}
	

};
#endif
