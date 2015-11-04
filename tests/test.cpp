
#include "../src/ngxtop++.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <iostream>
#include <sstream>

int read_file_test(){
	ARGV pt;
	std::stringstream ss(std::string("{\"--no-follow\": true}"));

	boost::property_tree::read_json(ss, pt);
	auto lines = build_source("test.log", pt);
	if(lines){
		while(!lines->is_end()){
			auto l = std::move(lines->get_line());
			std::cout << l <<std::endl;
		}
	}
	return 0;
}

int file_listen() {
	ARGV argv;
	auto lines = build_source("test.log", argv);
	std::cout << "try to write something in log file:\n";
	if (lines) {
		for (int i = 0; i != 5;++i) {
			auto l = std::move(lines->get_line());
			std::cout << l << std::endl;
		}
	}
	return 0;
}

int build_pattern_test() {
	const char * log_format = "combined";
	auto pattern = build_pattern(log_format);
	std::cout << pattern.str() << "\n";
	return 0;
}

int process_log_test() {
	ARGV pt;
	std::stringstream ss(std::string("{\"--no-follow\": true, \"--pre-filter\" : \"\"}"));
	boost::property_tree::read_json(ss, pt);

	auto lines = build_source("test.log", pt);

	const char * pattern_str = 
		"$remote_addr - $remote_user [$time_local] \"$request\" $status $body_bytes_sent \"$http_referer\" \"$http_user_agent\" \"$http_x_forwarded_for\" $request_time \"$geoip_city\" \"$geoip_region_name\" \"$geoip_country_name\" \"$upstream_addr\" $upstream_response_time";
	GroupSumBy group_sum_tuple{
		std::vector<std::string>{"status", "geoip_city"}, std::vector<std::string>{"bytes_sent"}
	};
	RecordRule rule = { 
		{"summary", group_sum_tuple }
	};
	
	auto pattern = build_pattern(pattern_str);
	//std::cout << pattern.str() << "\n";
	process_log(lines, pattern, pt, rule);
	auto records = ngx_records();
	auto uri_records = records->find("summary")->second.find("count")->second;
	for (auto it = uri_records.cbegin(); it != uri_records.cend(); it++) {
		std::cout << it->first << " " << it->second << "\n";
	}
	std::cout << "\n";
	auto status_records = records->find("summary")->second.find("bytes_sent")->second;
	for (auto it = status_records.cbegin(); it != status_records.cend(); it++) {
		std::cout << it->first << " " << it->second << "\n";
	}
	return 0;
}

int main(){
	//if(read_file_test())
	//	std::cout<< "read file error\n";
	//if (file_listen())
	//	std::cout << "file listen error\n";
	//if (build_pattern_test())
		//std::cout << "build pattern error";
	if (process_log_test())
		std::cout << "build pattern error";
	return 0;
}
