//! Code counter class methods for the Python language.
/*!
* \file CPythonCounter.cpp
*
* This file contains the code counter class methods for the Python language.
*/

#include "CPythonCounter.h"

/*!
* Constructs a CPythonCounter object.
*/
CPythonCounter::CPythonCounter()
{
	classtype = PYTHON;
	language_name = "Python";

	file_extension.push_back(".py");

	BlockCommentStart.push_back("\"\"\"");
	BlockCommentEnd.push_back("\"\"\"");
	BlockCommentStart.push_back("'''");
	BlockCommentEnd.push_back("'''");
	LineCommentStart.push_back("#");
	QuoteStart = "\"\'";
	QuoteEnd = "\"\'";
	QuoteEscapeRear = '\\';

	loop_keywords.push_back("for");
	loop_keywords.push_back("while");

	directive.push_back("do");
	directive.push_back("from");
	directive.push_back("import");
	directive.push_back("no");
	directive.push_back("package");
	directive.push_back("use");
	directive.push_back("require");

	exec_name_list.push_back("and");
	exec_name_list.push_back("as");
	exec_name_list.push_back("assert");
	exec_name_list.push_back("break");
	exec_name_list.push_back("continue");
	exec_name_list.push_back("def");
	exec_name_list.push_back("del");
	exec_name_list.push_back("elif");
	exec_name_list.push_back("else");
	exec_name_list.push_back("except");
	exec_name_list.push_back("exec");
	exec_name_list.push_back("exit");
	exec_name_list.push_back("finally");
	exec_name_list.push_back("for");
	exec_name_list.push_back("global");
	exec_name_list.push_back("if");
	exec_name_list.push_back("in");
	exec_name_list.push_back("is");
	exec_name_list.push_back("lambda");
	exec_name_list.push_back("not");
	exec_name_list.push_back("or");
	exec_name_list.push_back("pass");
	exec_name_list.push_back("print");
	exec_name_list.push_back("raise");
	exec_name_list.push_back("return");
	exec_name_list.push_back("try");
	exec_name_list.push_back("while");
	exec_name_list.push_back("with");
	exec_name_list.push_back("yield");

	math_func_list.push_back("math.ceil");
	math_func_list.push_back("math.copysign");
	math_func_list.push_back("math.degrees");
	math_func_list.push_back("math.e");
	math_func_list.push_back("math.exp");
	math_func_list.push_back("math.fabs");
	math_func_list.push_back("math.factorial");
	math_func_list.push_back("math.floor");
	math_func_list.push_back("math.fmod");
	math_func_list.push_back("math.frexp");
	math_func_list.push_back("math.fsum");
	math_func_list.push_back("math.hypot");
	math_func_list.push_back("math.ldexp");
	math_func_list.push_back("math.modf");
	math_func_list.push_back("math.pi");
	math_func_list.push_back("math.pow");
	math_func_list.push_back("math.radians");
	math_func_list.push_back("math.sqrt");
	math_func_list.push_back("math.trunc");
	math_func_list.push_back("cmath.phase");
	math_func_list.push_back("cmath.polar");
	math_func_list.push_back("cmath.rect");
	
	trig_func_list.push_back("math.acos");
	trig_func_list.push_back("math.acosh");
	trig_func_list.push_back("math.asinh");
	trig_func_list.push_back("math.atanh");
	trig_func_list.push_back("math.asin");
	trig_func_list.push_back("math.atan");
	trig_func_list.push_back("math.atan2");
	trig_func_list.push_back("math.cos");
	trig_func_list.push_back("math.cosh");
	trig_func_list.push_back("math.sin");
	trig_func_list.push_back("math.sinh");
	trig_func_list.push_back("math.tan");
	trig_func_list.push_back("math.tanh");

	log_func_list.push_back("math.log");
	log_func_list.push_back("math.log10");
	log_func_list.push_back("math.log1p");

	cmplx_calc_list.push_back("+");
	cmplx_calc_list.push_back("-");
	cmplx_calc_list.push_back("*");
	cmplx_calc_list.push_back("/");
	cmplx_calc_list.push_back("%");
	cmplx_calc_list.push_back("**");

	cmplx_cond_list.push_back("elif");
	cmplx_cond_list.push_back("else");
	cmplx_cond_list.push_back("except");
	cmplx_cond_list.push_back("for");
	cmplx_cond_list.push_back("if");
	cmplx_cond_list.push_back("try");
	cmplx_cond_list.push_back("while");

	cmplx_logic_list.push_back("==");
	cmplx_logic_list.push_back("!=");
	cmplx_logic_list.push_back(">");
	cmplx_logic_list.push_back("<");
	cmplx_logic_list.push_back(">=");
	cmplx_logic_list.push_back("=<");

	cmplx_assign_list.push_back("=");
}

/*!
* Replaces up to ONE quoted string inside a string starting at idx_start.
* Uses a string instead of a character to allow processing multi-line
*  literals """ and '''.
*
* \param strline string to be processed
* \param idx_start index of line character to start search
* \param contd specifies the quote string is continued from the previous line
* \param CurrentQuoteEnd end quote string of the current status
*
* \return method status
*/
int CPythonCounter::ReplaceQuote(string &strline, size_t &idx_start, bool &contd, string &CurrentQuoteEnd)
{
	size_t idx_end, idx_quote;

	if (contd)
	{
		// python: use string instead of character to check for """ and '''
		idx_start = 0;
		if (strline.length() >= CurrentQuoteEnd.length() &&
			strline.substr(0, CurrentQuoteEnd.length()) == CurrentQuoteEnd)
		{
			idx_start = CurrentQuoteEnd.length();
			contd = false;
			return 1;
		}
		strline[0] = '$';
	}
	else
	{
		// handle two quote chars in some languages, both " and ' may be accepted
		idx_start = FindQuote(strline, QuoteStart, idx_start, QuoteEscapeFront);
		if (idx_start != string::npos)
		{
			idx_quote = QuoteStart.find_first_of(strline[idx_start]);
			CurrentQuoteEnd = QuoteEnd[idx_quote];
			// python: check for """ or '''
			if (strline.length() >= idx_start + 3)
			{
				if (CurrentQuoteEnd == "\"")
				{
					if (strline.substr(idx_start, 3) == "\"\"\"")
						CurrentQuoteEnd = "\"\"\"";
				}
				else if (CurrentQuoteEnd == "'")
				{
					if (strline.substr(idx_start, 3) == "'''")
						CurrentQuoteEnd = "'''";
				}
			}
		}
		else
		{
			idx_start = strline.length();
			return 0;
		}
	}

	// python: handle """ and '''
	if (CurrentQuoteEnd.length() == 3)
	{
		if (idx_start + 3 >= strline.length())
			idx_end = string::npos;
		else
		{
			idx_end = strline.find(CurrentQuoteEnd, idx_start + 3);
			if (idx_end != string::npos)
				idx_end += 2;	// shift to last quote character
		}
	}
	else
		idx_end = CUtil::FindCharAvoidEscape(strline, CurrentQuoteEnd[0], idx_start + 1, QuoteEscapeFront);
	if (idx_end == string::npos)
	{
		idx_end = strline.length() - 1;
		strline.replace(idx_start + 1, idx_end - idx_start, idx_end - idx_start, '$');
		contd = true;
		idx_start = idx_end + 1;
	}
	else
	{
		if (CurrentQuoteEnd.length() != 3 && (QuoteEscapeRear) && (strline.length() > idx_end + 1) && (strline[idx_end+1] == QuoteEscapeRear))
		{
			strline[idx_end] = '$';
			strline[idx_end+1] = '$';
		}
		else
		{
			contd = false;
			strline.replace(idx_start + 1, idx_end - idx_start - 1, idx_end - idx_start - 1, '$');
			idx_start = idx_end + 1;
		}
	}
	return 1;
}

/*!
* Counts the number of comment lines, removes comments, and
* replaces quoted strings by special chars, e.g., $
* All arguments are modified by the method.
* Special processing for """ and ''' which can be multi-line literal
*  or a multi-line comment if it stands alone.
*
* \param fmap list of processed file lines
* \param result counter results
* \param fmapBak list of original file lines (same as fmap except it contains unmodified quoted strings)
*
* \return method status
*/
int CPythonCounter::CountCommentsSLOC(filemap* fmap, results* result, filemap *fmapBak)
{
	if (BlockCommentStart.empty() && LineCommentStart.empty())
		return 0;
	if (classtype == UNKNOWN || classtype == DATAFILE)
		return 0;

	bool contd = false;
	bool contd_nextline;
	int comment_type = 0;
	/*
	comment_type:
	0 : not a comment
	1 : line comment, whole line
	2 : line comment, embedded
	3 : block comment, undecided
	4 : block comment, embedded
	*/

	size_t idx_start, idx_end, comment_start;
	size_t quote_idx_start;
	string curBlckCmtStart, curBlckCmtEnd, tmp;
	string CurrentQuoteEnd = "";
	bool quote_contd = false;
	filemap::iterator itfmBak = fmapBak->begin();

	quote_idx_start = 0;

	for (filemap::iterator iter = fmap->begin(); iter != fmap->end(); iter++, itfmBak++)
	{
		contd_nextline = false;

		quote_idx_start = 0;
		idx_start = 0;

		if (CUtil::CheckBlank(iter->line))
			continue;
		if (quote_contd)
		{
			// Replace quote until next character
			ReplaceQuote(iter->line, quote_idx_start, quote_contd, CurrentQuoteEnd);
			if (quote_contd)
				continue;
		}

		if (contd)
			comment_type = 3;

		while (!contd_nextline && idx_start < iter->line.length())
		{
			// need to handle multiple quote chars in some languages, both " and ' may be accepted
			quote_idx_start = FindQuote(iter->line, QuoteStart, quote_idx_start, QuoteEscapeFront);
			comment_start = idx_start;
			if (!contd)
			{
				FindCommentStart(iter->line, comment_start, comment_type, curBlckCmtStart, curBlckCmtEnd);
				if (comment_start != string::npos && comment_type > 2)
				{
					// python: check whether this is a multi-line literal or a block comment
					tmp = CUtil::TrimString(iter->line, -1);
					if (iter->line.length() - tmp.length() != comment_start)
					{
						quote_idx_start = comment_start;
						comment_start = string::npos;
					}
				}
			}

			if (comment_start == string::npos && quote_idx_start == string::npos)
				break;

			if (comment_start != string::npos)
				idx_start = comment_start;

			// if found quote before comment, e.g., "this is quote");//comment
			if (quote_idx_start != string::npos && (comment_start == string::npos || quote_idx_start < comment_start))
			{
				ReplaceQuote(iter->line, quote_idx_start, quote_contd, CurrentQuoteEnd);
				if (quote_idx_start > idx_start && quote_idx_start != iter->line.length())
				{
					// comment delimiter inside quote
					idx_start = quote_idx_start;
					continue;
				}
			}
			else if (comment_start != string::npos)
			{
				// comment delimiter starts first
				switch (comment_type)
				{
				case 1:	// line comment, definitely whole line
					iter->line = "";
					itfmBak->line = "";
					result->comment_lines++;
					contd_nextline = true;
					break;
				case 2:	// line comment, possibly embedded
					iter->line = iter->line.substr(0, idx_start);
					itfmBak->line = itfmBak->line.substr(0, idx_start);
					// trim trailing space
					iter->line = CUtil::TrimString(iter->line, 1);
					itfmBak->line = CUtil::TrimString(itfmBak->line, 1);
					if (iter->line.empty())
						result->comment_lines++;	// whole line
					else
						result->e_comm_lines++;		// embedded
					contd_nextline = true;
					break;
				case 3:	// block comment
				case 4:
					if (contd)
						idx_end = iter->line.find(curBlckCmtEnd);
					else
						idx_end = iter->line.find(curBlckCmtEnd, idx_start + curBlckCmtStart.length());

					if (idx_end == string::npos)
					{
						if (comment_type == 3)
						{
							iter->line = "";
							itfmBak->line = "";
							result->comment_lines++;
						}
						else if (comment_type == 4)
						{
							iter->line = iter->line.substr(0, idx_start);
							itfmBak->line = itfmBak->line.substr(0, idx_start);
							// trim trailing space
							iter->line = CUtil::TrimString(iter->line, 1);
							itfmBak->line = CUtil::TrimString(itfmBak->line, 1);
							if (iter->line.empty())
								result->comment_lines++;	// whole line
							else
								result->e_comm_lines++;		// embedded
						}
						contd = true;
						contd_nextline = true;
						break;
					}
					else
					{
						contd = false;
						iter->line.erase(idx_start, idx_end - idx_start + curBlckCmtEnd.length());
						itfmBak->line.erase(idx_start, idx_end - idx_start + curBlckCmtEnd.length());
						if (iter->line.empty())
							result->comment_lines++;
						else
						{
							// trim trailing space
							iter->line = CUtil::TrimString(iter->line, 1);
							itfmBak->line = CUtil::TrimString(itfmBak->line, 1);
							if (iter->line.empty())
								result->comment_lines++;	// whole line
							else
								result->e_comm_lines++;		// embedded
						}

						// quote chars found may be erased as it is inside comment
						quote_idx_start = idx_start;
					}
					break;
				default:
					cout << "Error in CountCommentsSLOC()" << endl;
					break;
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
int CPythonCounter::CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak)
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
				if ((idx = iter->line.find((*viter), 0)) != string::npos && idx == 0)
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
			if (strDirLine[strDirLine.length()-1] == '\\')
				strDirLine = strDirLine.substr(0, strDirLine.length()-1);

			// if a directive or continuation of a directive (no continuation symbol found)
			if (iter->line[iter->line.length()-1] != '_')
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
int CPythonCounter::LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak)
{
	unsigned int	paren_count			= 0;
	string			strLSLOC			= "";
	string			strLSLOCBak			= "";

	filemap::iterator fit, fitbak;
	string line, lineBak;
	UIntVector loopWhiteSpace;

	unsigned int cnt = 0;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";

	for (fit = fmap->begin(), fitbak = fmapBak->begin(); fit != fmap->end(); fit++, fitbak++)
	{
		line = fit->line;
		lineBak = fitbak->line;

		// do not process blank lines
		// blank line means blank_line/comment_line/directive
		if (!CUtil::CheckBlank(line))
		{
			// does the ReplaceQuote get the continuation character \ replaced?
			LSLOC(result, line, lineBak, strLSLOC, strLSLOCBak, paren_count, loopWhiteSpace);

			if (print_cmplx)
			{
				cnt = 0;
				CUtil::CountTally(line, exec_name_list, cnt, 1, exclude, "", "", &result->exec_name_count);
			}
		}
	}
	return 1;
}

// Logical Counting Consideration
/*
Not counted line:
else:
\ (continuation char)
in ()
in []
in {}
end of line preceding by operator characters + - * / = < > | & is in % ^ \ ~ not , : 
compound statement, with : and ;
esp. compound statement with : in middle line, not in () [] {} or in else:
*/

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
* \param loopWhiteSpace count of white space to determine loop ends
*/
void CPythonCounter::LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak,
						   unsigned int &paren_cnt, UIntVector &loopWhiteSpace)
{
#define CONT_STR_LENGTH 18
	string continuation_str[] = {"is", "in", "not", "+", "-", "*", "/", "=", "<", ">", "|", "&", "%", "^", "\\", "~", ",", "$"};
	
	size_t start = 0;	// starting index of the working string
	size_t i = 0, idx, strSize;
	int n;
	bool trunc_flag = false;
	unsigned int cnt = 0, numWS;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";

	string tmp;

	// process:
	// paren_cnt is used with {} [] ()
	// 1. check if the  current char is in one of the parentheses
	// 2.   if no, check if the line has : or ; (statement separators), except else:
	// 3.      if yes, count and put the statement in the result
	// 4. if the line does not ends with a continuation string or a statement separator (handled)
	//      and the line is not in one of the parentheses
	//      then count and put the statement in the result
	// 5. physical count considers all lines executables (or directives, no declarations)

	// check for loop ends, new loops, and record white space in order to determine ends
	if (print_cmplx)
	{
		// check white space for loop ends
		if (loopWhiteSpace.size() > 0)
		{
			// get white space
			tmp = line;
			tmp = CUtil::TrimString(tmp, -1);
			numWS = (unsigned)(line.length() - tmp.length());

			// check for loop ends
			for (n = (int)loopWhiteSpace.size() - 1; n >= 0; n--)
			{
				if (loopWhiteSpace.at(n) != numWS)
					break;
				else
					loopWhiteSpace.pop_back();
			}
		}

		// check for loop keywords (for, while)
		cnt = 0;
		CUtil::CountTally(line, loop_keywords, cnt, 1, exclude, "", "", NULL);
		if (cnt > 0)
		{
			if (loopWhiteSpace.size() < 1)
			{
				// get white space
				tmp = line;
				tmp = CUtil::TrimString(tmp, -1);
				numWS = (unsigned)(line.length() - tmp.length());
			}

			// add nested loop white space and record nested loop level
			for (i = 0; i < cnt; i++)
			{
				loopWhiteSpace.push_back(numWS);

				if ((unsigned int)result->cmplx_nestloop_count.size() < loopWhiteSpace.size())
					result->cmplx_nestloop_count.push_back(1);
				else
					result->cmplx_nestloop_count[loopWhiteSpace.size()-1]++;
			}
		}
	}

	line = CUtil::TrimString(line);
	lineBak = CUtil::TrimString(lineBak);
	size_t line_length = line.length();
	bool lineContinued = false;

	while (i < line_length)
	{
		switch (line[i])
		{
		case '{': case '[': case '(': // parentheses opener
			paren_cnt++;
			break;
		case '}': case ']': case ')': // parentheses closer
			if (paren_cnt > 0)
				paren_cnt--;
			break;
		}

		// 2. if no parentheses enclosing, and if the char is a statement separator
		if (paren_cnt == 0 && (line[i] == ';' || line[i] == ':'))
		{
			tmp = CUtil::ClearRedundantSpaces(line);
			// if line[..i] is else: then exit the outer if
			if (tmp.rfind("else:") != tmp.length() - 5)
			{
				// 3.
				strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, i);
					strLSLOCBak += lineBak.substr(start, i);
				}
				if (result->addSLOC(strLSLOCBak, trunc_flag))
				{
					// increase logical SLOC here
					result->exec_lines[LOG]++;
				}
				strLSLOC = strLSLOCBak = "";
				start = i + 1;
			}
			else
				lineContinued = true;
		}
		i++;
	}

	if (paren_cnt == 0)
	{
		// add logical SLOC if the line does not end with a continuation string/char
		if (!lineContinued)
		{
			for (i = 0; i < CONT_STR_LENGTH; i++)
			{
				if (continuation_str[i].length() == 1)
				{
					if (line[line_length - 1] == continuation_str[i][0])
					{
						lineContinued = true;
						break;
					}
				}
				else
				{
					idx = CUtil::FindKeyword(line, continuation_str[i]);
					if (idx != string::npos && idx == line_length - continuation_str[i].length() - 1)
					{
						lineContinued = true;
						break;
					}
				}
			}
		}

		if (!lineContinued)
		{
			strSize = CUtil::TruncateLine(line_length - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
			if (strSize > 0)
			{
				strLSLOC += line.substr(start, line_length);
				strLSLOCBak += lineBak.substr(start, line_length);
			}
			if (result->addSLOC(strLSLOCBak, trunc_flag)) 
			{
				// increase logical SLOC here
				result->exec_lines[LOG]++;
			}
			strLSLOC = strLSLOCBak = "";
		}
		else
		{
			tmp = CUtil::TrimString(line.substr(start, line_length - start));
			strSize = CUtil::TruncateLine(tmp.length(), strLSLOC.length(), this->lsloc_truncate, trunc_flag);
			if (strSize > 0)
			{
				strLSLOC += tmp.substr(0, strSize);
				tmp = CUtil::TrimString(lineBak.substr(start, line_length - start));
				strLSLOCBak += tmp.substr(0, strSize);
			}
		}
	}
	result->exec_lines[PHY]++;
}
