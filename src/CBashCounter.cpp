//! Code counter class methods for the Bash shell script language.
/*!
* \file CBashCounter.cpp
*
* This file contains the code counter class methods for the Bash shell script language.
* This also includes the Korn shell language.
*/

#include "CBashCounter.h"

/*!
* Constructs a CBashCounter object.
*/
CBashCounter::CBashCounter()
{
	classtype = BASH;
	language_name = "Bash";

	file_extension.push_back(".sh");
	file_extension.push_back(".ksh");

	QuoteStart = "\"'";
	QuoteEnd = "\"'";
	QuoteEscapeFront = '\\';
	ContinueLine = "\\";
	LineCommentStart.push_back("#");

	exclude_keywords.push_back("done");
	exclude_keywords.push_back("esac");
	exclude_keywords.push_back("fi");

	continue_keywords.push_back("do");
	continue_keywords.push_back("else");
	continue_keywords.push_back("then");

	data_name_list.push_back("declare");
	data_name_list.push_back("local");
	data_name_list.push_back("type");
	data_name_list.push_back("typeset");

	exec_name_list.push_back("alias");
	exec_name_list.push_back("awk");
	exec_name_list.push_back("bind");
	exec_name_list.push_back("break");
	exec_name_list.push_back("builtin");
	exec_name_list.push_back("caller");
	exec_name_list.push_back("case");
	exec_name_list.push_back("cd");
	exec_name_list.push_back("command");
	exec_name_list.push_back("continue");
	exec_name_list.push_back("coproc");
	exec_name_list.push_back("dirs");
	exec_name_list.push_back("echo");
	exec_name_list.push_back("elif");
	exec_name_list.push_back("enable");
	exec_name_list.push_back("eval");
	exec_name_list.push_back("exec");
	exec_name_list.push_back("exit");
	exec_name_list.push_back("export");
	exec_name_list.push_back("for");
	exec_name_list.push_back("function");
	exec_name_list.push_back("getopts");
	exec_name_list.push_back("hash");
	exec_name_list.push_back("if");
	exec_name_list.push_back("let");
	exec_name_list.push_back("mapfile");
	exec_name_list.push_back("popd");
	exec_name_list.push_back("printf");
	exec_name_list.push_back("pushd");
	exec_name_list.push_back("pwd");
	exec_name_list.push_back("read");
	exec_name_list.push_back("readarray");
	exec_name_list.push_back("readonly");
	exec_name_list.push_back("return");
	exec_name_list.push_back("select");
	exec_name_list.push_back("set");
	exec_name_list.push_back("shift");
	exec_name_list.push_back("source");
	exec_name_list.push_back("test");
	exec_name_list.push_back("time");
	exec_name_list.push_back("times");
	exec_name_list.push_back("trap");
	exec_name_list.push_back("ulimit");
	exec_name_list.push_back("umask");
	exec_name_list.push_back("unalias");
	exec_name_list.push_back("unset");
	exec_name_list.push_back("until");
	exec_name_list.push_back("while");

	cmplx_calc_list.push_back("%");
	cmplx_calc_list.push_back("^");
	cmplx_calc_list.push_back("++");
	cmplx_calc_list.push_back("+");
	cmplx_calc_list.push_back("--");
	cmplx_calc_list.push_back("-");
	cmplx_calc_list.push_back("*");
	cmplx_calc_list.push_back("/");

	cmplx_cond_list.push_back("case");
	cmplx_cond_list.push_back("elif");
	cmplx_cond_list.push_back("if");
	cmplx_cond_list.push_back("for");
	cmplx_cond_list.push_back("select");
	cmplx_cond_list.push_back("until");
	cmplx_cond_list.push_back("while");

	cmplx_logic_list.push_back("&&");
	cmplx_logic_list.push_back("||");
	cmplx_logic_list.push_back("==");
	cmplx_logic_list.push_back("!");
	cmplx_logic_list.push_back("~");
	cmplx_logic_list.push_back(">");
	cmplx_logic_list.push_back("<");
	cmplx_logic_list.push_back(">=");
	cmplx_logic_list.push_back("=<");
	cmplx_logic_list.push_back("-lt");
	cmplx_logic_list.push_back("-gt");
	cmplx_logic_list.push_back("-ge");
	cmplx_logic_list.push_back("-le");
	cmplx_logic_list.push_back("-eq");
	cmplx_logic_list.push_back("-ne");

	cmplx_assign_list.push_back("=");
}

/*!
* Perform preprocessing of file lines before counting.
*
* \param fmap list of file lines
*
* \return method status
*/
int CBashCounter::PreCountProcess(filemap* fmap)
{
	filemap::iterator fit;
	for (fit = fmap->begin(); fit != fmap->end(); fit++) 
	{
		if (fit->line.empty())
			continue;
		for (size_t i = fit->line.length() - 1; i > 0; i--)
		{
			// replace $# and ${# with $ to avoid determination of a comment
			if (fit->line[i] == '#' && (fit->line[i-1] == '$' || fit->line[i-1] == '{'))
				fit->line[i] = '$';
		}
	}
	return 0;
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
int CBashCounter::LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak)
{
	filemap::iterator fit, fitbak;
	string line, lineBak;

	bool data_continue = false;
	string strLSLOC = "";
	string strLSLOCBak = "";
	string str;
	unsigned int phys_exec_lines = 0;
	unsigned int phys_data_lines = 0;
	unsigned int temp_lines = 0;
	unsigned int cnt = 0;
	StringVector loopLevel;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";

	for (fit = fmap->begin(), fitbak = fmapBak->begin(); fit != fmap->end(); fit++, fitbak++)
	{
		line = fit->line;
		lineBak = fitbak->line;

		// do not process blank lines (blank_line/comment_line/directive)
		if (!CUtil::CheckBlank(line))
		{
			// process logical SLOC
			LSLOC(result, line, lineBak, strLSLOC, strLSLOCBak, data_continue,
				temp_lines, phys_exec_lines, phys_data_lines, loopLevel);

			if (print_cmplx)
			{
				cnt = 0;
				CUtil::CountTally(line, exec_name_list, cnt, 1, exclude, "", "", &result->exec_name_count);
			}

			// update physical SLOC lines
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
* \param data_continue continuation of a data declaration line
* \param temp_lines tracks physical line count
* \param phys_exec_lines number of physical executable lines
* \param phys_data_lines number of physical data lines
* \param loopLevel nested loop level
*/
void CBashCounter::LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak,
						 bool &data_continue, unsigned int &temp_lines, unsigned int &phys_exec_lines,
						 unsigned int &phys_data_lines, StringVector &loopLevel)
{
	size_t start, end;
	size_t i = 0, m, strSize;
	bool trunc_flag = false, found;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";
	string str, spc;
	unsigned int cnt = 0;

	string tmp    = CUtil::TrimString(line);
	string tmpBak = CUtil::TrimString(lineBak);
	start = 0;

	// skip whole line '{' or '}'
	if (tmp == "{" || tmp == "}")
	{
		strLSLOC = strLSLOCBak = "";
		phys_exec_lines++;
		temp_lines = 0;
		return;
	}

	// trim trailing '{'
	if (tmp[tmp.length() - 1] == '{')
	{
		tmp = CUtil::TrimString(tmp.substr(0, tmp.length() - 1));
		tmpBak = CUtil::TrimString(tmpBak.substr(0, tmpBak.length() - 1));
	}

	// there may be more than 1 logical SLOC in this line
	while (start < tmp.length())
	{
		// check for semicolon to denote end of SLOC
		end = tmp.find(";", start);
		if (end != string::npos)
		{
			// handle empty statement
			if (CUtil::TrimString(tmp.substr(start, end - start + 1)) == ";")
			{
				start = end + 1;
				strLSLOC = strLSLOCBak = "";
				temp_lines = 0;
				if (tmp == ";")
					phys_exec_lines++;
				continue;
			}

			// handle for (( ; ; ))
			i = CUtil::FindKeyword(tmp, "for", start, end); 
			if (i != string::npos)
			{
				i += 3;
				i = tmp.find("((", i);
				if (i != string::npos && i < end)
				{
					i += 2;
					i = tmp.find("))", i);
					if (i != string::npos)
					{
						i += 2;
						end = tmp.find(";", i);
						if (end == string::npos)
							end = tmp.length() - 1;
					}
					else
						end = tmp.length() - 1;
				}
			}

			// handle case ';;' or ';&' or ';;&'
			if (end < tmp.length() - 1)
			{
				if (tmp[end + 1] == ';' || tmp[end + 1] == '&')
					end++;
				if (end < tmp.length() - 2 && tmp[end + 2] == '&')
					end++;
			}
		}
		else
			end = tmp.length() - 1;

		// process nested loops
		if (print_cmplx)
		{
			str = CUtil::TrimString(tmp.substr(start, end - start + 1));
			if (CUtil::FindKeyword(str, "for") != string::npos
				|| CUtil::FindKeyword(str, "while") != string::npos
				|| CUtil::FindKeyword(str, "until")!= string::npos
				|| CUtil::FindKeyword(str, "select")!= string::npos)
			{
				if (CUtil::FindKeyword(str, "for") != string::npos)
					loopLevel.push_back("for");
				else if (CUtil::FindKeyword(str, "while")!= string::npos)
					loopLevel.push_back("while");
				else if (CUtil::FindKeyword(str, "until") != string::npos)
					loopLevel.push_back("until");
				else if (CUtil::FindKeyword(str, "select") != string::npos)
					loopLevel.push_back("");

				// record nested loop level
				if (CUtil::FindKeyword(str, "select") == string::npos)
				{
					unsigned int loopCnt = 0;
					for (StringVector::iterator lit = loopLevel.begin(); lit < loopLevel.end(); lit++)
					{
						if ((*lit) != "")
							loopCnt++;
					}
					if ((unsigned int)result->cmplx_nestloop_count.size() < loopCnt)
						result->cmplx_nestloop_count.push_back(1);
					else
						result->cmplx_nestloop_count[loopCnt-1]++;
				}
			}
			if (CUtil::FindKeyword(str, "done") != string::npos && loopLevel.size() > 0)
				loopLevel.pop_back();
		}

		// check for line containing excluded keywords
		for (StringVector::iterator it = exclude_keywords.begin(); it != exclude_keywords.end(); it++)
		{
			i = CUtil::FindKeyword(tmp, (*it), start, end);
			if (i != string::npos)
			{
				// strip specified keyword and skip if empty
				start = i + (*it).length();
				if (CUtil::CheckBlank(CUtil::TrimString(tmp.substr(start, end - start))))
					start = end + 1;
				break;
			}
		}
		if (start > end)
		{
			if (temp_lines == 0 && phys_data_lines == 0 && phys_exec_lines == 0)
				phys_exec_lines = 1;
			continue;
		}

		// check for continuation words
		found = false;
		if (tmp[end] == ';')
			str = CUtil::TrimString(tmp.substr(start, end - start));
		else
			str = CUtil::TrimString(tmp.substr(start, end - start + 1));
		for (StringVector::iterator it = continue_keywords.begin(); it != continue_keywords.end(); it++)
		{
			if (str == (*it))
			{
				found = true;
				strLSLOC += str + " ";
				strLSLOCBak += str + " ";
				start = end + 1;
				if (temp_lines == 0 && phys_data_lines == 0 && phys_exec_lines == 0)
					phys_exec_lines = 1;
				temp_lines = 0;
			}
		}
		if (found)
			continue;

		// check for line continuation
		if (tmp[end] == '\\')
		{
			// strip off trailing (\)
			strSize = CUtil::TruncateLine(end - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
			if (strSize > 0)
			{
				spc = "";
				str = tmp.substr(start, strSize);
				for (m = str.length() - 1; m > 0; m--)
				{
					if (str[m] == ' ')
						spc += " ";
					else
						break;
				}
				if (m == 0)
				{
					if (str[0] == ' ')
						spc += " ";
				}
				strLSLOC += CUtil::TrimString(tmp.substr(start, strSize)) + spc;
				strLSLOCBak += CUtil::TrimString(tmpBak.substr(start, strSize)) + spc;
			}
			start = end + 1;

			// make sure that we are not beginning to process a new data line
			cnt = 0;
			CUtil::CountTally(strLSLOC, data_name_list, cnt, 1, exclude, "", "", NULL);

			if (cnt > 0)
				data_continue = true;
			if (data_continue == true)
				temp_lines++;
			if (temp_lines == 0 && phys_data_lines == 0 && phys_exec_lines == 0)
				phys_exec_lines = 1;
		}
		else
		{
			// save LSLOC
			if (tmp[end] == ';')
			{
				// don't trim if ';;'
				if (tmp.length() > 1 && tmp[end - 1] == ';')
					strSize = CUtil::TruncateLine(end - start + 1, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				else
					strSize = CUtil::TruncateLine(end - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
			}
			else
				strSize = CUtil::TruncateLine(end - start + 1, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
			if (strSize > 0)
			{
				strLSLOC += CUtil::TrimString(tmp.substr(start, strSize));
				strLSLOCBak += CUtil::TrimString(tmpBak.substr(start, strSize));
			}
			start = end + 1;
			if (strLSLOCBak.length() > 0)
			{
				if (result->addSLOC(strLSLOCBak, trunc_flag))
				{
					// add a logical SLOC
					cnt = 0;
					CUtil::CountTally(strLSLOC, data_name_list, cnt, 1, exclude, "", "", &result->data_name_count);

					temp_lines++;
					if (data_continue == true || cnt > 0)
					{
						result->data_lines[LOG]++;
						phys_data_lines = temp_lines;
					}
					else
					{
						result->exec_lines[LOG]++;
						phys_exec_lines = temp_lines;
					}
				}
				else if (data_continue == true)
					phys_data_lines = temp_lines;
				else
					phys_exec_lines = temp_lines;
			}
			data_continue = false;
			temp_lines = 0;
			strLSLOC = strLSLOCBak = "";
		}
	}
}
