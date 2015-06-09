//! Main class methods containing results.
/*!
* \file cc_main.cpp
*
* This file contains the main class methods containing results.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_06_06
*   Changes  ended  on 2015_06_06
* Added file_name_isEmbedded bool and file_name_only string for faster performance
*/

#include "cc_main.h"
#include "CUtil.h"

/*!
* Copies a results object.
*
* \param obj results to copy
*
* \return new results object
*/
results& results::operator= (const results& obj)
{
	error_code = obj.error_code;
	blank_lines = obj.blank_lines;
	comment_lines = obj.comment_lines;
	e_comm_lines = obj.e_comm_lines;
	directive_lines[0] = obj.directive_lines[0];
	directive_lines[1] = obj.directive_lines[1];
	data_lines[0] = obj.data_lines[0];
	data_lines[1] = obj.data_lines[1];
	exec_lines[0] = obj.exec_lines[0];
	exec_lines[1] = obj.exec_lines[1];
	SLOC_lines[0] = obj.SLOC_lines[0];
	SLOC_lines[1] = obj.SLOC_lines[1];
	cmplx_math_lines = obj.cmplx_math_lines;
	cmplx_trig_lines = obj.cmplx_trig_lines;
	cmplx_logarithm_lines = obj.cmplx_logarithm_lines;
	cmplx_calc_lines = obj.cmplx_calc_lines;
	cmplx_cond_lines = obj.cmplx_cond_lines;
	cmplx_logic_lines = obj.cmplx_logic_lines;
	cmplx_preproc_lines = obj.cmplx_preproc_lines;
	cmplx_assign_lines = obj.cmplx_assign_lines;
	cmplx_pointer_lines = obj.cmplx_pointer_lines;
	directive_count.assign(obj.directive_count.begin(), obj.directive_count.end());
	data_name_count.assign(obj.data_name_count.begin(), obj.data_name_count.end());
	exec_name_count.assign(obj.exec_name_count.begin(), obj.exec_name_count.end());
	math_func_count.assign(obj.math_func_count.begin(), obj.math_func_count.end());
	trig_func_count.assign(obj.trig_func_count.begin(), obj.trig_func_count.end());
	log_func_count.assign(obj.log_func_count.begin(), obj.log_func_count.end());
	cmplx_calc_count.assign(obj.cmplx_calc_count.begin(), obj.cmplx_calc_count.end());
	cmplx_cond_count.assign(obj.cmplx_cond_count.begin(), obj.cmplx_cond_count.end());
	cmplx_logic_count.assign(obj.cmplx_logic_count.begin(), obj.cmplx_logic_count.end());
	cmplx_preproc_count.assign(obj.cmplx_preproc_count.begin(), obj.cmplx_preproc_count.end());
	cmplx_assign_count.assign(obj.cmplx_assign_count.begin(), obj.cmplx_assign_count.end());
	cmplx_pointer_count.assign(obj.cmplx_pointer_count.begin(), obj.cmplx_pointer_count.end());
	cmplx_nestloop_count.assign(obj.cmplx_nestloop_count.begin(), obj.cmplx_nestloop_count.end());
	cmplx_cycfunct_count.assign(obj.cmplx_cycfunct_count.begin(), obj.cmplx_cycfunct_count.end());
	trunc_lines = obj.trunc_lines;
	total_lines = obj.total_lines;
	e_flag = obj.e_flag;
	file_name_isEmbedded = obj.file_name_isEmbedded;
	file_name = obj.file_name;
	file_name_only = obj.file_name_only;
	file_type = obj.file_type;
	class_type = obj.class_type;
	firstDuplicate = obj.firstDuplicate;
	duplicate = obj.duplicate;
	matched = obj.matched;

	return *this;
}

/*!
* Resets results to initial values.
*/
void results::reset()
{
	error_code = "";
	blank_lines = 0;
	comment_lines = 0;
	e_comm_lines = 0;
	directive_lines[0] = 0;
	directive_lines[1] = 0;
	data_lines[0] = 0;
	data_lines[1] = 0;
	exec_lines[0] = 0;
	exec_lines[1] = 0;
	SLOC_lines[0] = 0;
	SLOC_lines[1] = 0;
	cmplx_math_lines = 0;
	cmplx_trig_lines = 0;
	cmplx_logarithm_lines = 0;
	cmplx_calc_lines = 0;
	cmplx_cond_lines = 0;
	cmplx_logic_lines = 0;
	cmplx_preproc_lines = 0;
	cmplx_assign_lines = 0;
	cmplx_pointer_lines = 0;
	directive_count.clear();
	data_name_count.clear();
	exec_name_count.clear();
	math_func_count.clear();
	trig_func_count.clear();
	log_func_count.clear();
	cmplx_calc_count.clear();
	cmplx_cond_count.clear();
	cmplx_logic_count.clear();
	cmplx_preproc_count.clear();
	cmplx_assign_count.clear();
	cmplx_pointer_count.clear();
	cmplx_nestloop_count.clear();
	cmplx_cycfunct_count.clear();
	trunc_lines = 0;
	total_lines = 0;
	e_flag = false;
	file_name_isEmbedded = false;
	file_name = "";
	file_name_only = "";
	file_type = DATA;
	class_type = UNKNOWN;
	duplicate = false;
	firstDuplicate = false;
	matched = false;
}

/*!
* Clears the logical SLOC lines.
*/
void results::clearSLOC()
{
	mySLOCLines.clear();
}

/*!
* Adds a logical SLOC to the vector for differencing.
*
* \param line line to add
* \param trunc_flag indicates whether the SLOC has been truncated
*/
bool results::addSLOC(const string &line, bool &trunc_flag)
{
	string new_line = CUtil::ClearRedundantSpaces(line);
	if (new_line.length() == 0)
		return false;

	pair<srcLineVector::iterator, bool> insertResult = mySLOCLines.insert(make_pair(new_line, 1U));
	if (!insertResult.second) 
	{
		// element already exists
		(*insertResult.first).second++;	// update number of times this SLOC exists
	}
	if (trunc_flag)
	{
		trunc_lines++;
		trunc_flag = false;
	}
	return true;
}
