//! Code counter class methods for the Matlab language.
/*!
* \file CMatlabCounter.cpp
*
* This file contains the code counter class methods for the Matlab language.
*/

#include "CMatlabCounter.h"

/*!
* Constructs a CMatlab object.
*/
CMatlabCounter::CMatlabCounter()
{
	classtype = MATLAB;
	language_name = "MATLAB";

	file_extension.push_back(".m");

	QuoteStart = "'";
	QuoteEnd = "'";
	QuoteEscapeRear = '\'';
	ContinueLine = "...";

	BlockCommentStart.push_back("%{");
	BlockCommentEnd.push_back("%}");
	LineCommentStart.push_back("%");

	directive.push_back("import");

	exec_name_list.push_back("all");
	exec_name_list.push_back("break");
	exec_name_list.push_back("case");
	exec_name_list.push_back("catch");
	exec_name_list.push_back("continue");
	exec_name_list.push_back("for");
	exec_name_list.push_back("if");
	exec_name_list.push_back("else");
	exec_name_list.push_back("elseif");
	exec_name_list.push_back("end");
	exec_name_list.push_back("otherwise");
	exec_name_list.push_back("parfor");
	exec_name_list.push_back("return");
	exec_name_list.push_back("switch");
	exec_name_list.push_back("try");
	exec_name_list.push_back("while");

	math_func_list.push_back("ceil");
	math_func_list.push_back("eps");
	math_func_list.push_back("exp");
	math_func_list.push_back("factor");
	math_func_list.push_back("factorial");
	math_func_list.push_back("fix");
	math_func_list.push_back("floor");
	math_func_list.push_back("idivide");
	math_func_list.push_back("Inf");
	math_func_list.push_back("intmax");
	math_func_list.push_back("intmin");
	math_func_list.push_back("max");
	math_func_list.push_back("mod");
	math_func_list.push_back("NaN");
	math_func_list.push_back("pi");
	math_func_list.push_back("pow2");
	math_func_list.push_back("power");
	math_func_list.push_back("realmax");
	math_func_list.push_back("realmin");
	math_func_list.push_back("rem");
	math_func_list.push_back("round");
	math_func_list.push_back("sqrt");

	trig_func_list.push_back("acos");
	trig_func_list.push_back("acot");
	trig_func_list.push_back("acsc");
	trig_func_list.push_back("asec");
	trig_func_list.push_back("asin");
	trig_func_list.push_back("atan");
	trig_func_list.push_back("atan2");
	trig_func_list.push_back("cos");
	trig_func_list.push_back("cot");
	trig_func_list.push_back("csc");
	trig_func_list.push_back("sec");
	trig_func_list.push_back("sin");
	trig_func_list.push_back("tan");

	log_func_list.push_back("log");
	log_func_list.push_back("log10");
	log_func_list.push_back("log1p");
	log_func_list.push_back("log2");

	cmplx_calc_list.push_back("+");
	cmplx_calc_list.push_back("-");
	cmplx_calc_list.push_back("*");
	cmplx_calc_list.push_back("^");
	cmplx_calc_list.push_back("\\");
	cmplx_calc_list.push_back("/");
	cmplx_calc_list.push_back("\'");
	cmplx_calc_list.push_back(".'");
	cmplx_calc_list.push_back(".*");
	cmplx_calc_list.push_back(".^");
	cmplx_calc_list.push_back(".\\");
	cmplx_calc_list.push_back("./");

	cmplx_cond_list.push_back("case");
	cmplx_cond_list.push_back("else");
	cmplx_cond_list.push_back("elseif");
	cmplx_cond_list.push_back("for");
	cmplx_cond_list.push_back("if");
	cmplx_cond_list.push_back("switch");
	cmplx_cond_list.push_back("while");

	cmplx_logic_list.push_back("<");
	cmplx_logic_list.push_back(">");
	cmplx_logic_list.push_back(">=");
	cmplx_logic_list.push_back("<=");
	cmplx_logic_list.push_back("==");
	cmplx_logic_list.push_back("~=");

	cmplx_preproc_list.push_back("import");

	cmplx_assign_list.push_back("=");
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
int CMatlabCounter::CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak)
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
			// drop continuation symbol
			if (strDirLine.length() > 3 && strDirLine.substr(strDirLine.length()-3, 3) == "...")
				strDirLine = strDirLine.substr(0, strDirLine.length()-3);

			// if a directive or continuation of a directive (no continuation symbol found)
			if (iter->line.length() < 3 || iter->line.substr(iter->line.length()-4, 3) != "...")
			{
				contd = false;
				if (result->addSLOC(strDirLine, trunc_flag))
					result->directive_lines[LOG]++;
			}
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
int CMatlabCounter::LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak)
{
	bool cont_str = false;
	unsigned int openBrackets = 0;
	string strLSLOC = "";
	string strLSLOCBak = "";
	filemap::iterator fit, fitbak;
	string line, lineBak;
	StringVector loopLevel;
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
			LSLOC(result, line, lineBak, strLSLOC, strLSLOCBak, cont_str, openBrackets, loopLevel);

			if (print_cmplx)
			{
				cnt = 0;
				CUtil::CountTally(line, exec_name_list, cnt, 1, exclude, "", "", &result->exec_name_count);
			}
			result->exec_lines[PHY]++;
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
* \param cont_str continue string
* \param openBrackets number of open brackets (no matching close bracket)
* \param loopLevel nested loop level
*/
void CMatlabCounter::LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak,
						   bool &cont_str, unsigned int &openBrackets, StringVector &loopLevel)
{
	size_t start = 0, len;
	size_t i = 0, strSize;
	bool trunc_flag = false;
	string tmp, tmpBak, str;

	// check exclusions/continuation
	tmp = CUtil::TrimString(line);
	tmpBak = CUtil::TrimString(lineBak);
	if (CUtil::FindKeyword(tmp, "end") == 0)
	{
		if (loopLevel.size() > 0)
			loopLevel.pop_back();
		return;
	}
	else if (CUtil::FindKeyword(tmp, "case") == 0 || CUtil::FindKeyword(tmp, "else") == 0 || CUtil::FindKeyword(tmp, "otherwise") == 0)
	{
		strLSLOC += tmp + " ";
		strLSLOCBak += tmpBak + " ";
		return;
	}

	// process nested loops
	if (print_cmplx)
	{
		if (CUtil::FindKeyword(tmp, "for") != string::npos ||
			CUtil::FindKeyword(tmp, "while") != string::npos ||
			CUtil::FindKeyword(tmp, "parfor")!= string::npos)
		{
			loopLevel.push_back("loop");

			// record nested loop level
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
		else if (CUtil::FindKeyword(tmp, "if") != string::npos ||
			CUtil::FindKeyword(tmp, "switch") != string::npos ||
			CUtil::FindKeyword(tmp, "try") != string::npos)
			loopLevel.push_back("");
	}

	// there may be more than 1 logical SLOC in this line
	while (i < line.length())
	{
		switch (line[i])
		{
		case ';': case ',': // LSLOC terminators

			if (openBrackets > 0)
			{
				i++;
				continue;
			}

			tmp = CUtil::TrimString(line.substr(start, i - start + 1));
			tmpBak = CUtil::TrimString(lineBak.substr(start, i - start + 1));

			if (cont_str && strLSLOC.length() > 0)
			{
				// check for string continuation
				if (tmp[0] == '\'')
				{
					tmp = tmp.substr(1, tmp.length() - 1);
					tmpBak = tmpBak.substr(1, tmpBak.length() - 1);
				}
			}
			strSize = CUtil::TruncateLine(tmp.length(), strLSLOC.length(), this->lsloc_truncate, trunc_flag);
			if (strSize > 0)
			{
				strLSLOC += tmp.substr(0, strSize);
				strLSLOCBak += tmpBak.substr(0, strSize);
			}
			result->addSLOC(strLSLOCBak, trunc_flag);
			result->exec_lines[LOG]++;
			strLSLOC = strLSLOCBak = "";
			cont_str = false;
			start = i + 1;

			break;
		case '[': case '(': case '{':
			openBrackets++;
			break;
		case ']': case ')': case '}':
			openBrackets--;
			break;
		}
		i++;
	}

	// check for line continuation
	tmp = CUtil::TrimString(line.substr(start, i - start));
	tmpBak = CUtil::TrimString(lineBak.substr(start, i - start));
	if (tmp.length() > 3 && tmp.substr(tmp.length()-3, 3) == "...")
	{
		// strip off trailing (...)
		tmp = tmp.substr(0, tmp.length()-3);
		tmpBak = tmpBak.substr(0, tmpBak.length()-3);

		// strip off trailing (') to continue string
		str = CUtil::TrimString(tmp, 1);
		if (str[str.length()-1] == '\'')
		{
			len = str.length() - 1;
			cont_str = true;
		}
		else
			len = tmp.length();

		strSize = CUtil::TruncateLine(len, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
		if (strSize > 0)
		{
			if (cont_str)
			{
				strLSLOC += CUtil::TrimString(tmp.substr(0, strSize), -1);
				strLSLOCBak += CUtil::TrimString(tmpBak.substr(0, strSize), -1);
			}
			else
			{
				strLSLOC += CUtil::TrimString(tmp.substr(0, strSize)) + " ";
				strLSLOCBak += CUtil::TrimString(tmpBak.substr(0, strSize)) + " ";
			}
		}
	}
	else
	{
		// save LSLOC
		if (cont_str && strLSLOC.length() > 0)
		{
			// check for string continuation
			if (tmp[0] == '\'')
			{
				tmp = tmp.substr(1, tmp.length() - 1);
				tmpBak = tmpBak.substr(1, tmpBak.length() - 1);
			}
		}

		strSize = CUtil::TruncateLine(tmp.length(), strLSLOC.length(), this->lsloc_truncate, trunc_flag);
		if (strSize > 0)
		{
			strLSLOC += tmp.substr(0, strSize);
			strLSLOCBak += tmpBak.substr(0, strSize);

			result->addSLOC(strLSLOCBak, trunc_flag);
			result->exec_lines[LOG]++;
			strLSLOC = strLSLOCBak = "";
			cont_str = false;
		}
	}
}
