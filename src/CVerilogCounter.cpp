//! Code counter class methods for the Verilog language.
/*!
* \file CVerilogCounter.cpp
*
* This file contains the code counter class methods for the Verilog hardware definition language (used in FPGA programming).
*/

#include "CVerilogCounter.h"
#include <sstream>

/*!
* Constructs a CCCounter object.
*/
CVerilogCounter::CVerilogCounter()
{
	classtype = VERILOG;
	language_name = "Verilog";

	file_extension.push_back(".v");

	LineCommentStart.push_back("//");
	BlockCommentStart.push_back("/*");
	BlockCommentEnd.push_back("*/");
	QuoteStart = "\"";
	QuoteEnd = "\"";
	QuoteEscapeFront = '\"';

	directive.push_back("`define");
	directive.push_back("`include");
	directive.push_back("`ifdef");
	directive.push_back("`else");
	directive.push_back("`endif");
	directive.push_back("`timescale");

	data_name_list.push_back("endfunction");
	data_name_list.push_back("endmodule");
	data_name_list.push_back("endtask");
	data_name_list.push_back("event");
	data_name_list.push_back("function");
	data_name_list.push_back("genvar");
	data_name_list.push_back("inout");
	data_name_list.push_back("input");
	data_name_list.push_back("integer");
	data_name_list.push_back("localparam");
	data_name_list.push_back("module");
	data_name_list.push_back("output");
	data_name_list.push_back("parameter");
	data_name_list.push_back("reg");
	data_name_list.push_back("specparam");
	data_name_list.push_back("supply0");
	data_name_list.push_back("supply1");
	data_name_list.push_back("task");
	data_name_list.push_back("time");
	data_name_list.push_back("tri");
	data_name_list.push_back("tri0");
	data_name_list.push_back("tri1");
	data_name_list.push_back("triand");
	data_name_list.push_back("trior");
	data_name_list.push_back("trireg");
	data_name_list.push_back("wand");
	data_name_list.push_back("wire");
	data_name_list.push_back("wor");

	exec_name_list.push_back("always");
	exec_name_list.push_back("assign");
	exec_name_list.push_back("begin");
	exec_name_list.push_back("case");
	exec_name_list.push_back("casex");
	exec_name_list.push_back("casez");
	exec_name_list.push_back("deassign");
	exec_name_list.push_back("defparam");
	exec_name_list.push_back("disable");
	exec_name_list.push_back("end");
	exec_name_list.push_back("endcase");
	exec_name_list.push_back("for");
	exec_name_list.push_back("forever");
	exec_name_list.push_back("fork");
	exec_name_list.push_back("generate");
	exec_name_list.push_back("if");
	exec_name_list.push_back("else if");
	exec_name_list.push_back("else");
	exec_name_list.push_back("initial");
	exec_name_list.push_back("join");
	exec_name_list.push_back("posedge");
	exec_name_list.push_back("repeat");
	exec_name_list.push_back("wait");
	exec_name_list.push_back("while");

	exec_name_list.push_back("$bitstoreal");
	exec_name_list.push_back("$display");
	exec_name_list.push_back("$dumpall");
	exec_name_list.push_back("$dumpfile");
	exec_name_list.push_back("$dumpflush");
	exec_name_list.push_back("$dumplimit");
	exec_name_list.push_back("$dumpoff");
	exec_name_list.push_back("$dumpon");
	exec_name_list.push_back("$dumpvar");
	exec_name_list.push_back("$dumpvars");
	exec_name_list.push_back("$fclose");
	exec_name_list.push_back("$fdisplay");
	exec_name_list.push_back("$finish");
	exec_name_list.push_back("$fmonitor");
	exec_name_list.push_back("$fopen");
	exec_name_list.push_back("$fstrobe");
	exec_name_list.push_back("$fwrite");
	exec_name_list.push_back("$itor");
	exec_name_list.push_back("$monitor");
	exec_name_list.push_back("$monitoroff");
	exec_name_list.push_back("$monitoron");
	exec_name_list.push_back("$printtimescale");
	exec_name_list.push_back("$random");
	exec_name_list.push_back("$readmemb");
	exec_name_list.push_back("$readmemh");
	exec_name_list.push_back("$realtime");
	exec_name_list.push_back("$realtobits");
	exec_name_list.push_back("$rtoi");
	exec_name_list.push_back("$scale");
	exec_name_list.push_back("$shm_open");
	exec_name_list.push_back("$shm_probe");
	exec_name_list.push_back("$stime");
	exec_name_list.push_back("$stop");
	exec_name_list.push_back("$strobe");
	exec_name_list.push_back("$time");
	exec_name_list.push_back("$timeformat");
	exec_name_list.push_back("$write");
	exec_name_list.push_back("@");

	exec_name_list.push_back("buf");
	exec_name_list.push_back("not");
	exec_name_list.push_back("and");
	exec_name_list.push_back("or");
	exec_name_list.push_back("nand");
	exec_name_list.push_back("nor");
	exec_name_list.push_back("xor");
	exec_name_list.push_back("xnor");
	exec_name_list.push_back("bufif0");
	exec_name_list.push_back("bufif1");
	exec_name_list.push_back("notif0");
	exec_name_list.push_back("notif1");

	cmplx_calc_list.push_back("+");
	cmplx_calc_list.push_back("-");
	cmplx_calc_list.push_back("*");
	cmplx_calc_list.push_back("/");
	cmplx_calc_list.push_back("%");

	cmplx_cond_list.push_back("?");
	cmplx_cond_list.push_back("case");
	cmplx_cond_list.push_back("if");
	cmplx_cond_list.push_back("else");
	cmplx_cond_list.push_back("else if");
	cmplx_cond_list.push_back("for");
	cmplx_cond_list.push_back("forever");
	cmplx_cond_list.push_back("repeat");
	cmplx_cond_list.push_back("while");

	cmplx_logic_list.push_back("!");
	cmplx_logic_list.push_back("~");
	cmplx_logic_list.push_back("&");
	cmplx_logic_list.push_back("|");
	cmplx_logic_list.push_back("^");
	cmplx_logic_list.push_back("~&");
	cmplx_logic_list.push_back("~|");
	cmplx_logic_list.push_back("~^");
	cmplx_logic_list.push_back("<<");
	cmplx_logic_list.push_back(">>");
	cmplx_logic_list.push_back("<");
	cmplx_logic_list.push_back(">");
	cmplx_logic_list.push_back(">=");
	cmplx_logic_list.push_back("==");
	cmplx_logic_list.push_back("!=");
	cmplx_logic_list.push_back("===");
	cmplx_logic_list.push_back("!==");
	cmplx_logic_list.push_back("^~");
	cmplx_logic_list.push_back("&&");
	cmplx_logic_list.push_back("||");
	cmplx_logic_list.push_back("<=");

	cmplx_preproc_list.push_back("`define");
	cmplx_preproc_list.push_back("`include");
	cmplx_preproc_list.push_back("`ifdef");
	cmplx_preproc_list.push_back("`else");
	cmplx_preproc_list.push_back("`endif");
	cmplx_preproc_list.push_back("`timescale");

	cmplx_assign_list.push_back("=");
	cmplx_assign_list.push_back("<=");
}

/*!
* Counts file language complexity based on specified language keywords/characters.
*
* \param fmap list of processed file lines
* \param result counter results
*
* \return method status
*/
int CVerilogCounter::CountComplexity(filemap* fmap, results* result)
{
	if (classtype == UNKNOWN || classtype == DATAFILE)
		return 0;
	filemap::iterator fit;
	filemap fitBak;
	filemap::iterator fitForw, fitBack;	// used to check prior an later lines for semicolons
	unsigned int cnt;
	string line, line2;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$><=:";
	tokLocVect conditionalVector;
	tokLocVect::reverse_iterator r_tlvIter;
	StringVector::iterator strIter = this->cmplx_cond_list.begin();
	string buf;			// have a buffer string
	stringstream ss;	// insert the string into a stream
	int count = 0;
	int condCount = 0;
	StringVector tmpLogicList = cmplx_logic_list;	// making a temporary list with the '<=' operator removed from the list;  counting it on another pass;
	tmpLogicList.pop_back();
	StringVector tmpAssignList = cmplx_assign_list;	// making a temporary list with the '<=' operator removed from the list;  counting it on another pass;
	tmpAssignList.pop_back();
	string::iterator it;

	for (fit = fmap->begin(); fit != fmap->end(); fit++)
	{
		line = fit->line;

		if (CUtil::CheckBlank(line))
			continue;

		line = " " + line;

		// mathematical functions
		cnt = 0;
		CUtil::CountTally(line, math_func_list, cnt, 1, exclude, "", "", &result->math_func_count, casesensitive);
		result->cmplx_math_lines += cnt;

		// trigonometric functions
		cnt = 0;
		CUtil::CountTally(line, trig_func_list, cnt, 1, exclude, "", "", &result->trig_func_count, casesensitive);
		result->cmplx_trig_lines += cnt;

		// logarithmic functions
		cnt = 0;
		CUtil::CountTally(line, log_func_list, cnt, 1, exclude, "", "", &result->log_func_count, casesensitive);
		result->cmplx_logarithm_lines += cnt;

		// calculations
		cnt = 0;
		CUtil::CountTally(line, cmplx_calc_list, cnt, 1, exclude, "", "", &result->cmplx_calc_count, casesensitive);
		result->cmplx_calc_lines += cnt;

		// conditionals
		cnt = 0;
		CUtil::CountTally(line, cmplx_cond_list, cnt, 1, exclude, "", "", &result->cmplx_cond_count, casesensitive);
		result->cmplx_cond_lines += cnt;

		// logical operators
		cnt = 0;
		//using tmpLogicList
		CUtil::CountTally(line, tmpLogicList, cnt, 1, exclude, "", "", &result->cmplx_logic_count, casesensitive);
		result->cmplx_logic_lines += cnt;

		// preprocessor directives
		cnt = 0;
		CUtil::CountTally(line, cmplx_preproc_list, cnt, 1, exclude, "", "", &result->cmplx_preproc_count, casesensitive);
		result->cmplx_preproc_lines += cnt;

		// assignments
		cnt = 0;
		//using tmpAssignList
		CUtil::CountTally(line, tmpAssignList, cnt, 1, exclude, "", "", &result->cmplx_assign_count, casesensitive);
		result->cmplx_assign_lines += cnt;

		// pointers
		cnt = 0;
		CUtil::CountTally(line, cmplx_pointer_list, cnt, 1, exclude, "", "", &result->cmplx_pointer_count, casesensitive);
		result->cmplx_pointer_lines += cnt;
	}

	// do another pass since we ignored every less than or equal to symbol
	// if the <= symbol appears inside parentheses count it as a less a comparison operator
	// otherwise it is a signal assignment operator
	count = 0;
	condCount = 0;
	for (fit = fmap->begin(); fit != fmap->end(); fit++)
	{
		line = fit->line;
		line = CUtil::ToLower(line);

		if (CUtil::CheckBlank(line))
			continue;
		if (line.find("(") != string::npos || line.find(")") != string::npos || line.find("<=") != string::npos || line.find("?") != string::npos || line.find(";") != string::npos)
		{
			// iterate through each character looking for parentheses or the conditional operator		
			for (it = line.begin(); it < line.end(); it++)
			{
				if (*it == '(')
					count++;
				else if (*it == ')')
					count--;
				else if (*it == '?')
					condCount++;	// found the conditional operator
				else if (condCount > 0 && *it == ';')
					condCount = 0;	// found the end of the conditional operator
				else if (*it == '=')
				{
					// looking for the <= operator
					if (it != line.begin())
					{
						if (*(it-1) == '<')
						{
							if (count == 0 && condCount == 0)
							{
								// no conditional operator and no parentheses meets conditions for this to be assignment operator
								result->cmplx_assign_count.back()++; 
								result->cmplx_assign_lines++;
							}
							else
							{
								//anything else defaults to comparison operator
								result->cmplx_logic_count.back()++; 
								result->cmplx_logic_lines++;
							}
						}
					}
				}
			}		
		}
	}			
	return 1;
}

/*!
* Counts directive lines of code.
*
* \param fmap list of processed file lines
* \param result counter results
* \param fmapBak list of original file lines (same as fmap except it contains unmodified quoted strings)
*
* \return method status
*/
int CVerilogCounter::CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak)
{
	bool contd = false, trunc_flag = false;
	size_t idx, strSize;
	unsigned int cnt = 0;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";
	string strDirLine = "";

	filemap::iterator itfmBak = fmapBak->begin();
	for (filemap::iterator iter = fmap->begin(); iter != fmap->end(); iter++, itfmBak++)
	{
		if (CUtil::CheckBlank(iter->line))
			continue;

		if (print_cmplx)
		{
			cnt = 0;
			CUtil::CountTally(" " + iter->line, directive, cnt, 1, exclude, "", "", &result->directive_count);
		}

		if (!contd)
		{
			// if not a continuation of a previous directive
			for (vector<string>::iterator viter = directive.begin(); viter != directive.end(); viter++)
			{
				// ensures the keyword stands alone, avoid, e.g., #ifabc
				if (((idx = CUtil::FindKeyword(iter->line, *viter)) != string::npos) && idx == 0)
				{
					contd = true;
					break;
				}
			}
			if (contd)
			{
				strSize = CUtil::TruncateLine(itfmBak->line.length(), 0, this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
					strDirLine = itfmBak->line.substr(0, strSize);
				result->directive_lines[PHY]++;
			}
		}
		else
		{
			// continuation of a previous directive
			strSize = CUtil::TruncateLine(itfmBak->line.length(), strDirLine.length(), this->lsloc_truncate, trunc_flag);
			if (strSize > 0)
				strDirLine += "\n" + itfmBak->line.substr(0, strSize);
			result->directive_lines[PHY]++;
		}

		if (contd)
		{
			contd = false;
			if (result->addSLOC(strDirLine, trunc_flag))
				result->directive_lines[LOG]++;
			iter->line = "";
		}
	}
	return 1;
}

/*!
* Processes physical and logical lines according to language specific rules.
* NOTE: all the blank lines +
*               whole line comments +
*               lines with compiler directives
*       should have been blanked from filemap by previous processing
*       before reaching this function
*
* \param fmap list of processed file lines
* \param result counter results
* \param fmapBak list of original file lines (same as fmap except it contains unmodified quoted strings)
*
* \return method status
*/
int CVerilogCounter::LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak)
{
	unsigned int	paren_count			= 0;
	bool			for_flag			= false;
	bool			always_flag			= false;
	bool	        case_flag	        = false;
	bool			repeat_flag         = false;
	bool			found_for			= false;
	bool			found_forifwhile	= false;
	bool			found_while			= false;
	char			prev_char			= 0;
	bool			data_continue		= false;
	string			strLSLOC			= "";
	string			strLSLOCBak			= "";

	filemap::iterator fit, fitbak;
	string line, lineBak;
	StringVector loopLevel;

	unsigned int phys_exec_lines = 0;
	unsigned int phys_data_lines = 0;
	unsigned int temp_lines = 0;
	unsigned int cnt = 0;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";

	for (fit = fmap->begin(), fitbak = fmapBak->begin(); fit != fmap->end(); fit++, fitbak++)
	{
		line = fit->line;

		// insert blank at the beginning (for searching keywords)
		line = ' ' + line;
		lineBak = ' ' + fitbak->line;

		// do not process blank lines
		// blank line means blank_line/comment_line/directive
		if (!CUtil::CheckBlank(line))
		{
			LSLOC(result, line, lineBak, strLSLOC, strLSLOCBak, paren_count, for_flag, found_forifwhile, found_while,
				prev_char, data_continue, temp_lines, phys_exec_lines, phys_data_lines, found_for,
				loopLevel, always_flag, case_flag, repeat_flag);

			if (print_cmplx)
			{
				cnt = 0;
				CUtil::CountTally(line, exec_name_list, cnt, 1, exclude, "", "", &result->exec_name_count);
			}

			result->exec_lines[PHY] += phys_exec_lines;
			phys_exec_lines = 0;

			result->data_lines[PHY] += phys_data_lines;
			phys_data_lines = 0;
		}
	}
	return 1;
}

/*!
* Extracts and stores logical lines of code.
* Determines and extract logical SLOC to place in the result variable
* using addSLOC function. Each time the addSLOC function is called,
* a new logical SLOC is added. This function assumes that the directive
* is handled before it is called.
*
* \param result counter results
* \param line processed physical line of code
* \param lineBak original physical line of code
* \param strLSLOC processed logical string
* \param strLSLOCBak original logical string
* \param paren_cnt count of parenthesis
* \param forflag found for flag
* \param found_forifwhile found for, if, or while flag
* \param found_while found while flag
* \param prev_char previous character
* \param data_continue continuation of a data declaration line
* \param temp_lines tracks physical line count
* \param phys_exec_lines number of physical executable lines
* \param phys_data_lines number of physical data lines
* \param found_for found for loop
* \param loopLevel nested loop level
* \param always_flag found always
* \param case_flag found case
* \param repeat_flag found repeat
*/
void CVerilogCounter::LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, unsigned int &paren_cnt,
	bool &forflag, bool &found_forifwhile, bool &found_while, char &prev_char, bool &data_continue,
	unsigned int &temp_lines, unsigned int &phys_exec_lines, unsigned int &phys_data_lines,
	bool &found_for, StringVector &loopLevel, bool &always_flag, bool &case_flag, bool &repeat_flag)
{
	// paren_cnt is used with 'for' statement only
	size_t start = 0, startmax = 0; // starting index of the working string
	size_t i = 0, strSize;
	bool trunc_flag = false;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:";
	string dataExclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:().";	// avoid double count of casts as data and executable lines (e.g. set { m_uiValue = (uint)value; }
	bool found_end = false, found_endcase = false;
	unsigned int cnt = 0;
	bool newline = true;

	string tmp = CUtil::TrimString(strLSLOC);
	size_t tmpi;

	// there may be more than 1 logical SLOC in this line
	while (i < line.length())
	{
		tmp = CUtil::TrimString(line.substr(start, i + 1 - start));
		if (CUtil::FindKeyword(tmp, "end") != string::npos && loopLevel.size() > 0 && loopLevel.back().compare("begin") == 0)
		{
			loopLevel.pop_back();	// pop begin
			loopLevel.pop_back();	// pop looping
			start = i + 1;
		}
		if ((tmpi = CUtil::FindKeyword(line.substr(start, i + 1 - start), "generate")) != string::npos)
		{
			strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
			if (strSize > 0)
			{
				strLSLOC += line.substr(start, strSize);
				strLSLOCBak += lineBak.substr(start, strSize);
			}
			if (result->addSLOC(strLSLOCBak, trunc_flag))
				result->exec_lines[LOG]++;
			strLSLOCBak = strLSLOC = "";
			phys_exec_lines = temp_lines;
			temp_lines = 0;
			start = start + 7 + tmpi;
			found_forifwhile = false;
			forflag = false;
			found_for = false;
			always_flag = false;
			case_flag = false;
			repeat_flag = false;
		}

		if ((tmpi = CUtil::FindKeyword(line.substr(start, i + 1 - start), "forever")) != string::npos)
		{
			if (print_cmplx)
			{
				tmp = CUtil::TrimString(line.substr(start, i + 1 - start));
				tmp = strLSLOC + " " + tmp;
				if (CUtil::FindKeyword(tmp, "begin") != string::npos && loopLevel.size() > 0 && loopLevel.back().compare("looping") == 0)
				{
					loopLevel.push_back("begin");
				}
				else if (loopLevel.size() > 0)
				{
					// didn't find begin, so pop off since no longer in a looping block
					loopLevel.pop_back();
				}
				loopLevel.push_back("looping");
				// forever doesn't have any conditions so just add it to sloc
				unsigned int loopCnt = 0;
				for (StringVector::iterator lit = loopLevel.begin(); lit < loopLevel.end(); lit++)
				{
					if ((*lit) != "begin")
						loopCnt++;
				}
				if ((unsigned int)result->cmplx_nestloop_count.size() < loopCnt)
					result->cmplx_nestloop_count.push_back(1);
				else
					result->cmplx_nestloop_count[loopCnt-1]++;
			}
			strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
			if (strSize > 0)
			{
				strLSLOC += line.substr(start, strSize);
				strLSLOCBak += lineBak.substr(start, strSize);
			}
			if (result->addSLOC(strLSLOCBak, trunc_flag))
				result->exec_lines[LOG]++;
			strLSLOCBak = strLSLOC = "";
			phys_exec_lines = temp_lines;
			temp_lines = 0;
			start = start + 7 + tmpi;
			found_forifwhile = true;
			forflag = false;
			found_for = false;
			always_flag = false;
			case_flag = false;
			repeat_flag = false;
		}

		switch (line[i])
		{
		case ';': // LSLOC terminators
			// ';' for normal executable or declaration statement
			// '{' for starting a function or 'do' stmt or a block (which is counted)
			// get the previous logical mark until i-1 index is the new LSLOC
			// except 'do' precedes '{'
			// except '}' precedes ';' ??
			// do nothing inside 'for' statement
			if (found_for == true && paren_cnt > 0 && line[i] == ';')
				break;

			// record open bracket for nested loop processing
			if (print_cmplx)
			{
				tmp = CUtil::TrimString(line.substr(start, i + 1 - start));
				tmp = strLSLOC + " " + tmp;
				
				if (CUtil::FindKeyword(tmp, "begin") != string::npos && loopLevel.size() > 0 && loopLevel.back().compare("looping") == 0)
				{
					loopLevel.push_back("begin");
				}
				else if (loopLevel.size() > 0 && loopLevel.back().compare("begin") != 0)	// check that this isn't already in a begin block...if it is leave it alone
				{
					// didn't find begin, so pop off since no longer in a looping block
					loopLevel.pop_back();
				}
			}
			// case 'while(...);', 'while(...) {', and '} while(...);'
			// this case is handled in case ')'
			if (found_while && found_forifwhile)
			{
				found_while = false;
				found_forifwhile = false;
				start = i + 1;
				break;
			}

			// check for empty statement (=1 LSLOC)
			if (CUtil::TrimString(line.substr(start, i + 1 - start)) == ";" && strLSLOC.length() < 1)
			{
				strLSLOC = ";";
				strLSLOCBak = ";";
			}
			else
			{
				strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
			}
			if (result->addSLOC(strLSLOCBak, trunc_flag))
			{
				cnt = 0;
				CUtil::CountTally(strLSLOC, data_name_list, cnt, 1, dataExclude, "", "", &result->data_name_count);

				temp_lines++;
				if (data_continue == true && line[i] == ';')
				{
					result->data_lines[LOG]++;
					phys_data_lines = temp_lines;
				}
				else
				{
					if (cnt > 0 && line[i] == ';' )
					{
						result->data_lines[LOG]++;
						if (newline)
						{
							// only add a physical line once per line, otherwise a line of code might have multiple physical data and exec lines
							phys_data_lines = temp_lines;
							newline = false;
						}
					}
					else
					{
						result->exec_lines[LOG]++;
						if (newline)
						{
							// only add a physical line once per line, otherwise a line of code might have multiple physical data and exec lines
							phys_exec_lines = temp_lines;
							newline = false;
						}
					}
				}
			}
			else if (data_continue == true && line[i] == ';')
				phys_data_lines = temp_lines;
			else
				phys_exec_lines = temp_lines;
			data_continue = false;
			temp_lines = 0;
			strLSLOC = strLSLOCBak = "";
			start = i + 1;

			// reset some flagging parameters
			forflag = false;
			paren_cnt = 0;
			found_while = false;
			found_forifwhile = false;
			found_for = false;

			break;
		case '(':
			tmp = CUtil::TrimString(line.substr(start, i));
			if (CUtil::FindKeyword(tmp, "always") != string::npos)
			{
				// found always
				paren_cnt++;
				always_flag = true;
			}
			if (CUtil::FindKeyword(tmp, "case") != string::npos || CUtil::FindKeyword(tmp, "casex") != string::npos || CUtil::FindKeyword(tmp, "casez") != string::npos)
			{
				// found case
				paren_cnt++;
				case_flag = true;
			}
			if (forflag)
				paren_cnt++;
			else
			{
				// handle 'for', 'while', 'if', 'repeat' the same way

				if (CUtil::FindKeyword(tmp, "for") != string::npos
					|| CUtil::FindKeyword(tmp, "while")!= string::npos
					|| CUtil::FindKeyword(tmp, "if") != string::npos
					|| CUtil::FindKeyword(tmp, "repeat") != string::npos)
				{
					forflag = true;
					paren_cnt++;

					if (print_cmplx)
					{
						tmp = CUtil::TrimString(line.substr(start, i + 1 - start));
						tmp = strLSLOC + " " + tmp;
						if (CUtil::FindKeyword(tmp, "begin") != string::npos && loopLevel.size() > 0 && loopLevel.back().compare("looping") == 0)
						{
							loopLevel.push_back("begin");
						}
						else if (loopLevel.size() > 0 && loopLevel.back().compare("begin") != 0)
						{
							// didn't find begin, so pop off since no longer in a looping block
							loopLevel.pop_back();
						}
					}

					if (CUtil::FindKeyword(tmp, "for") != string::npos)
					{
						if (print_cmplx)
							loopLevel.push_back("looping");
						found_for = true;
					}
					else if (CUtil::FindKeyword(tmp, "while")!= string::npos)
					{
						if (print_cmplx)
							loopLevel.push_back("looping");
						found_while = true;
					}
					else if (CUtil::FindKeyword(tmp, "repeat")!= string::npos)
					{
						if (print_cmplx)
							loopLevel.push_back("looping");
						repeat_flag = true;
					}
					

					// record nested loop level
					if (print_cmplx)
					{
						if (CUtil::FindKeyword(tmp, "if") == string::npos)
						{
							unsigned int loopCnt = 0;
							for (StringVector::iterator lit = loopLevel.begin(); lit < loopLevel.end(); lit++)
							{
								if ((*lit) != "begin")
									loopCnt++;
							}
							if ((unsigned int)result->cmplx_nestloop_count.size() < loopCnt)
								result->cmplx_nestloop_count.push_back(1);
							else
								result->cmplx_nestloop_count[loopCnt-1]++;
						}
					}
				}
			}
			break;
		case ')':
			if (always_flag || case_flag || repeat_flag || forflag)
			{
				if (paren_cnt > 0)
					paren_cnt--;
				if (paren_cnt == 0)
				{
					// handle always @
					strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
					if (strSize > 0)
					{
						strLSLOC += line.substr(start, strSize);
						strLSLOCBak += lineBak.substr(start, strSize);
					}
					if (result->addSLOC(strLSLOCBak, trunc_flag))
						result->exec_lines[LOG]++;
					strLSLOCBak = strLSLOC = "";
					phys_exec_lines = temp_lines;
					temp_lines = 0;
					start = i + 1;
					found_forifwhile = true;
					forflag = false;
					found_for = false;
					always_flag = false;
					case_flag = false;
					repeat_flag = false;
				}
			}
			break;
		}

		if (line[i] != ' ' && line[i] != '\t') 
		{
			// if ;}}} --> don't count }}} at all
			// also, if {}}} --> don't count }}} at all
			// if ( !(line[i] == '}' && (prev_char == ';' || prev_char == '{'))) // see case '}' above
			prev_char = line[i];

			// change to not found if a char appears before
			if (line[i] != ')' && found_forifwhile)
				found_forifwhile = false;
		}
		i++;
	}

	// don't save end statements to add to next sloc, they will be counted as physical sloc but not logical
	tmp = CUtil::TrimString(line.substr(start, i - start));
	if ((tmpi = CUtil::FindKeyword(tmp, "endcase")) != string::npos)
	{
		startmax = max((start + tmpi + 8), startmax);
		found_endcase = true;
	}
	if ((tmpi = CUtil::FindKeyword(tmp, "endmodule")) != string::npos)
	{
		startmax = max((start + tmpi + 10), startmax);			
	}
	if ((tmpi = CUtil::FindKeyword(tmp, "endtask")) != string::npos)
	{
		startmax = max((start + tmpi + 8), startmax);		
	}
	if ((tmpi = CUtil::FindKeyword(tmp, "endfunction")) != string::npos)
	{
		startmax = max((start + tmpi + 12), startmax);		
	}
	if ((tmpi = CUtil::FindKeyword(tmp, "end")) != string::npos)
	{
		startmax = max((start + tmpi + 4), startmax);		
		found_end = true;	// this is to catch any empty begin and end statements
	}
	if (startmax != 0) start = min(i, startmax);	// if we found and end statement update start to be the max of i and startmax

	tmp = CUtil::TrimString(line.substr(start, i - start));
	strSize = CUtil::TruncateLine(tmp.length(), strLSLOC.length(), this->lsloc_truncate, trunc_flag);
	if (strSize > 0 || (strLSLOC.size() > 0 && found_end))
	{
		strLSLOC += tmp.substr(0, strSize);
		tmp = CUtil::TrimString(lineBak.substr(start, i - start));
		strLSLOCBak += tmp.substr(0, strSize);

		if (found_end)
		{
			found_end = false;
			if (strLSLOC.compare(strLSLOCBak) != 0)
			{
				if (result->addSLOC(strLSLOCBak, trunc_flag))
					result->exec_lines[LOG]++;
				strLSLOCBak = strLSLOC = "";
				phys_exec_lines = temp_lines;
				temp_lines = 0;
			}
		}	
	}

	// make sure that we are not beginning to process a new data line
	cnt = 0;
	CUtil::CountTally(strLSLOC, data_name_list, cnt, 1, exclude, "", "", NULL);

	if (cnt > 0)
		data_continue = true;
	if (data_continue)
		temp_lines++;
	if (startmax > 0 && !found_endcase && !found_end)
		phys_data_lines = 1;
	else if (temp_lines == 0 && phys_data_lines == 0 && phys_exec_lines == 0)
		phys_exec_lines = 1;
}
