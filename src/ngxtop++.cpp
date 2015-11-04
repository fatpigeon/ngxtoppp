#include <iostream>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include "ngxtop++.h"

const char* REGEX_SPECIAL_CHARS = "([\\.\\*\\+\\?\\|\\(\\)\\{\\}\\[\\]])";
const char* REGEX_LOG_FORMAT_VARIABLE = "\\$([a-zA-Z0-9\\_]+)";
const char* LOG_FORMAT_COMBINED = "$remote_addr - $remote_user [$time_local] \"$request\" $status $body_bytes_sent \"$http_referer\" \"$http_user_agent\"";
const char* LOG_FORMAT_COMMON = "$remote_addr - $remote_user [$time_local] \"$request\" $status $body_bytes_sent \"$http_x_forwarded_for\"";
RecordType records;

//======   RecordTuple =========

//bool RecordTuple::operator>=(const RecordTuple& lhf, const RecordTuple& rhf) const {
//	for (auto rit = rhf.strs.begin(), lit = lhf.strs.begin(); lit != lhf.strs.end(); lit++, rit++) {
//		if (*rhf > *lhf)
//			return true;
//		else if (*lhf > rhf)
//			return false;
//	}
//	return true;
//}

//======   function ==========

SafeLines build_source(std::string &access_log,const ARGV &argv){
	return build_source(access_log.c_str(), argv);
}
SafeLines build_source(std::string &&access_log, const ARGV &argv){
	return build_source(access_log.c_str(), argv);
}

SafeLines build_source(const char *access_log, const ARGV &argv){
	Lines *l = NULL;
	if(std::strcmp(access_log, "stdin") == 0){
		l = new StdinRead;
	}
	else if(argv.find("--no-follow") != argv.not_found()){
		l = new FileStream(access_log);
	}
	else{
		l = new FileListener(access_log);
	}
	return SafeLines(l, std::default_delete<Lines>());
}

boost::regex build_pattern(const char* log_format) {
	if (std::strcmp(log_format, "combined") == 0) {
		log_format = LOG_FORMAT_COMBINED;
	}
	else if (std::strcmp(log_format, "common") == 0) {
		log_format = LOG_FORMAT_COMMON;
	}
	boost::regex expr{ log_format };
	auto pattern = boost::regex_replace(std::string{ log_format }, boost::regex{ REGEX_SPECIAL_CHARS }, std::string{ "\\\\\\1" });
	std::cout << pattern << "\n";
	auto pattern2 = boost::regex_replace(pattern, boost::regex{ REGEX_LOG_FORMAT_VARIABLE }, std::string{ "(?<\\1>.*)" });
	std::cout << pattern2 << "\n";
	return boost::regex{pattern2};
}

Processor build_processor(ARGV &argv) {
	auto fields = argv.find("<var>")->second;
	std::string label;
	std::string selections;
	if (argv.find("print") != argv.not_found()) {
		std::vector<std::string> list;
		for (auto it = fields.begin(); it != fields.end(); it++)
			list.push_back(it->second.get_value<std::string>());
		selections = label = std::move(boost::algorithm::join(list, ", "));
		label += ":";
	}
	
}

void process_log(SafeLines & safe_lines, const boost::regex &regex, const ARGV &argv, const RecordRule& rule) {
	auto pre_filter= argv.find("--pre-filter")->second.get_value<std::string>();
	if (pre_filter != "") {
		
	}
	parse_log(safe_lines, regex, rule);

}

class MatchDict : public std::map<std::string, std::string> {
public:

	MatchDict(const boost::smatch &smatch) :what(smatch) {

	}

	mapped_type get(const key_type& k) {
		auto it = this->find(k);
		if (it == this->end()) {
			this->insert(std::make_pair(k, std::string(what[k])));
			return this->at(k);
		}
		return it->second;
	}
	mapped_type& operator[](const key_type& k); //don't use
private:
	boost::smatch what;
};

void parse_log(SafeLines & safe_lines, const boost::regex &pattern, const RecordRule& rule) {
	while (!safe_lines->is_end()) {
		std::string l = std::move(safe_lines->get_line());
		boost::smatch what;
		if (boost::regex_search(l, what, pattern)) {

			MatchDict match_dict(what);

			match_dict.emplace("bytes_sent", what["body_bytes_sent"]);
			std::string uri;
			if (what["request_uri"].matched) {
				uri = what["request_uri"];
			}
			else if (what["request"].matched) {
				std::vector<std::string> strs;
				auto what_request = std::string(what["request"]);
				boost::split(strs, what_request, boost::is_any_of(" "));

				uri = std::move(boost::algorithm::join(std::vector<std::string>(strs.begin() + 1, strs.end() - 1), " "));
				// url parse todo
			}
			match_dict.emplace("uri", uri);
			for (auto e : rule) {
				auto table_name = e.first;
				auto group_list = std::get<0>(e.second);
				auto sum_list = std::get<1>(e.second);
				std::vector<std::string> group_name_list;
				for (auto group_key : group_list)
					group_name_list.push_back(match_dict.get(group_key) );
				auto group_name = std::move(boost::algorithm::join(group_name_list, ","));
				// sum by
				//records[table_name]["sum"]["count"] += 1;
				//auto records_table_name = records[table_name];
				for (auto sum_key : sum_list) {
					records[table_name][sum_key][group_name] += std::stoi(match_dict.get(sum_key) );
				}
				// group by
				records[table_name]["count"][group_name] += 1;
			}
			
		}
	}
	
}

const RecordType* ngx_records() {
	return &records;
}