#include <Python.h>
#include <vector>
#include <string>
#include <tuple>
#include <sstream>
#include <iostream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>

#include "ngxtop++.h"

static PyObject *
run(PyObject *self, PyObject *args)
{
	char *access_log=NULL, *arguments_str=NULL, *pattern_str=NULL;
	PyObject* group_rule;
	
	if (!PyArg_ParseTuple(args, "sssO", &access_log, &arguments_str, &pattern_str, &group_rule))
		return NULL;
	// ======== data init ====================
	ARGV pt;
	const std::string arguments_string(arguments_str);
	std::stringstream ss(arguments_string);
	boost::property_tree::read_json(ss, pt);
	RecordRule record_rule;

	auto p_key_list = PyDict_Keys(group_rule);
	for (auto i = 0;i != PyList_Size(p_key_list); ++i) {
		auto p_key = PyList_GetItem(p_key_list, i);
		auto p_value = PyDict_GetItem(group_rule, p_key);
		auto p_groupby_list = PyTuple_GetItem(p_value, 0);
		auto p_sumby_list = PyTuple_GetItem(p_value, 1);
		std::vector<std::string> groupby_vec;
		std::vector<std::string> sumby_vec;
		for (auto j = 0; j != PyList_Size(p_groupby_list); j++) {
			auto p_groupby_val = PyList_GetItem(p_groupby_list, j);
			char* s = PyString_AsString(p_groupby_val);
			groupby_vec.push_back(s);
		}
		for (auto j = 0; j != PyList_Size(p_sumby_list); j++) {
			auto p_sumby_val = PyList_GetItem(p_sumby_list, j);
			char* s = PyString_AsString(p_sumby_val);
			sumby_vec.push_back(s);
		}
		char *s_key = PyString_AsString(p_key);
		GroupSumBy gsby{groupby_vec, sumby_vec };
		record_rule.emplace(s_key, gsby);
	}
	// ============== function call ==============
	auto lines = build_source(access_log, pt);
	auto pattern = build_pattern(pattern_str);
	process_log(lines, pattern, pt, record_rule);

	return Py_BuildValue("");
}

static PyObject *
get_records(PyObject *self, PyObject *args)
{
	auto _records = ngx_records();
	auto p_records_dict = PyDict_New();
	for (auto table : *_records) {
		auto p_table_dict = PyDict_New();
		for (auto group : table.second) {
			auto p_group_dict = PyDict_New();
			for (auto key_value : group.second) {
				std::vector<std::string> strs;
				boost::split(strs, key_value.first, boost::is_any_of(","));
				// group key is a string with type xxx,xxx,xxx 
				// i want to convert to tuple (xxx,xxx,xxx)
				auto p_key_tuple = PyTuple_New(strs.size());
				for (auto i = 0; i != PyTuple_Size(p_key_tuple); ++i) {
					PyTuple_SetItem(p_key_tuple, i, Py_BuildValue("s", strs[i].c_str()));
				}
				PyDict_SetItem(p_group_dict, p_key_tuple, Py_BuildValue("i", key_value.second));
			}
			PyDict_SetItem(p_table_dict, Py_BuildValue("s", group.first.c_str()), p_group_dict);
		}
		PyDict_SetItem(p_records_dict, Py_BuildValue("s", table.first.c_str()), p_table_dict);
	}
	return p_records_dict;
}

#ifdef __cplusplus
extern "C" {
#endif

	/* C API functions */
#define PySpam_System_NUM 0
#define PySpam_System_RETURN int
#define PySpam_System_PROTO (const char *command)

static PyMethodDef ngxtoppp_Methods[] = {
	{ "run", run, METH_VARARGS, "run the lines parser" },
	{ "get_records", get_records, METH_VARARGS, "get results with lines parser" },
	{ NULL,NULL,0,NULL }
};

PyMODINIT_FUNC
initngxtop_cpp(void)
{
	PyObject *m;

	m = Py_InitModule("ngxtop_cpp", ngxtoppp_Methods);
	if (m == NULL)
		return;
}

#ifdef __cplusplus
}
#endif