#ifndef NGXTOP_PLUS_PLUS_H_
#define NGXTOP_PLUS_PLUS_H_
#include <vector>
#include <string>
#include <cstdio>
#include <map>
#include <memory>
#include <tuple>
#include <cstdarg>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>


#include "lines.h"

//typedef std::shared_ptr<Lines> SafeLines;
typedef std::shared_ptr<Lines> SafeLines;
typedef boost::property_tree::ptree ARGV;
typedef void Processor;

//class RecordTuple {
//public:
//	RecordTuple(const std::vector<std::string> &strs): strs(strs) {};
//	RecordTuple(std::vector<std::string> &&strs) : strs(strs) {};
//	inline bool operator>(const RecordTuple& lhf, const RecordTuple& rhf) const;
//private:
//	std::vector<std::string> strs;
//};
typedef std::map<std::string, std::map<std::string, int> > RecordTable;
typedef std::map<std::string, RecordTable> RecordType;
//                  group by list             sum by list
typedef std::tuple<std::vector<std::string>, std::vector<std::string> >GroupSumBy;
typedef std::map<std::string, GroupSumBy> RecordRule;


SafeLines build_source(std::string &access_log, const ARGV &argv);
SafeLines build_source(std::string &&access_log, const ARGV &argv);
SafeLines build_source(const char *access_log, const ARGV &argv);

boost::regex build_pattern(const char* log_format);

Processor build_processor(ARGV &argv);

void process_log(SafeLines & safe_lines, const boost::regex &pattern, const ARGV &argv, const RecordRule& rule);
void parse_log(SafeLines & safe_lines, const boost::regex &pattern,const RecordRule& rule);
const RecordType* ngx_records();
 


#endif
