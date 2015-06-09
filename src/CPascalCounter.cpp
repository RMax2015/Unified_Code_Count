//! Code counter class methods for the Pascal language.
/*!
* \file CCJavaCsCounter.cpp
*
* This file contains the code counter class methods for the Pascal language.
*/

#include "CPascalCounter.h"

/*!
* Constructs a CPascalCounter object.
*/
CPascalCounter::CPascalCounter()
{
	classtype = PASCAL;
	language_name = "Pascal";
	casesensitive = false;

	QuoteStart = "'";
	QuoteEnd = "'";
	QuoteEscapeFront = '\'';
	BlockCommentStart.push_back("(*");
	BlockCommentEnd.push_back("*)");
	BlockCommentStart.push_back("{");
	BlockCommentEnd.push_back("}");
	LineCommentStart.push_back("//");

	file_extension.push_back(".pas");
	file_extension.push_back(".p");
	file_extension.push_back(".pp");
	file_extension.push_back(".pa3");
	file_extension.push_back(".pa4");
	file_extension.push_back(".pa5");

	data_name_list.push_back("ansistring");
	data_name_list.push_back("array");
	data_name_list.push_back("boolean");
	data_name_list.push_back("byte");
	data_name_list.push_back("bytebool");
	data_name_list.push_back("cardinal");
	data_name_list.push_back("char");
	data_name_list.push_back("class");
	data_name_list.push_back("comp");
	data_name_list.push_back("complex");
	data_name_list.push_back("const");
	data_name_list.push_back("double");
	data_name_list.push_back("extended");
	data_name_list.push_back("file");
	data_name_list.push_back("integer");
	data_name_list.push_back("interface");
	data_name_list.push_back("int64");
	data_name_list.push_back("longbool");
	data_name_list.push_back("longint");
	data_name_list.push_back("longword");
	data_name_list.push_back("object");
	data_name_list.push_back("pchar");
	data_name_list.push_back("qword");
	data_name_list.push_back("real");
	data_name_list.push_back("record");
	data_name_list.push_back("set");
	data_name_list.push_back("shortint");
	data_name_list.push_back("shortstring");
	data_name_list.push_back("single");
	data_name_list.push_back("smallint");
	data_name_list.push_back("string");
	data_name_list.push_back("type");
	data_name_list.push_back("widestring");
	data_name_list.push_back("word");
	data_name_list.push_back("wordbool");

	exec_name_list.push_back("absolute");
	exec_name_list.push_back("assembler");
	exec_name_list.push_back("case");
	exec_name_list.push_back("const");
	exec_name_list.push_back("constructor");
	exec_name_list.push_back("destructor");
	exec_name_list.push_back("dispose");
	exec_name_list.push_back("downto");
	exec_name_list.push_back("else");
	exec_name_list.push_back("exit");
	exec_name_list.push_back("far");
	exec_name_list.push_back("for");
	exec_name_list.push_back("forward");
	exec_name_list.push_back("freemem");
	exec_name_list.push_back("function");
	exec_name_list.push_back("getmem");
	exec_name_list.push_back("goto");
	exec_name_list.push_back("if");
	exec_name_list.push_back("implementation");
	exec_name_list.push_back("inline");
	exec_name_list.push_back("interrupt");
	exec_name_list.push_back("label");
	exec_name_list.push_back("mark");
	exec_name_list.push_back("near");
	exec_name_list.push_back("new");
	exec_name_list.push_back("nil");
	exec_name_list.push_back("packed");
	exec_name_list.push_back("private");
	exec_name_list.push_back("procedure");
	exec_name_list.push_back("program");
	exec_name_list.push_back("protected");
	exec_name_list.push_back("public");
	exec_name_list.push_back("repeat");
	exec_name_list.push_back("unit");
	exec_name_list.push_back("uses");
	exec_name_list.push_back("var");
	exec_name_list.push_back("virtual");
	exec_name_list.push_back("while");
	exec_name_list.push_back("with");

	math_func_list.push_back("abs");
	math_func_list.push_back("arg");
	math_func_list.push_back("cmplx");
	math_func_list.push_back("dec");
	math_func_list.push_back("exp");
	math_func_list.push_back("im");
	math_func_list.push_back("inc");
	math_func_list.push_back("min");
	math_func_list.push_back("max");
	math_func_list.push_back("polar");
	math_func_list.push_back("pow");
	math_func_list.push_back("re");
	math_func_list.push_back("round");
	math_func_list.push_back("sqr");
	math_func_list.push_back("sqrt");

	trig_func_list.push_back("cos");
	trig_func_list.push_back("sin");
	trig_func_list.push_back("arccos");
	trig_func_list.push_back("arcsin");
	trig_func_list.push_back("arctan");

	log_func_list.push_back("ln");

	cmplx_calc_list.push_back("+");
	cmplx_calc_list.push_back("-");
	cmplx_calc_list.push_back("*");
	cmplx_calc_list.push_back("/");
	cmplx_calc_list.push_back("**");
	cmplx_calc_list.push_back("div");
	cmplx_calc_list.push_back("mod");

	cmplx_cond_list.push_back("case");
	cmplx_cond_list.push_back("else");
	cmplx_cond_list.push_back("for");
	cmplx_cond_list.push_back("if");
	cmplx_cond_list.push_back("repeat");
	cmplx_cond_list.push_back("while");
	cmplx_cond_list.push_back("with");

	cmplx_logic_list.push_back("=");
	cmplx_logic_list.push_back("<>"); 
	cmplx_logic_list.push_back(">");
	cmplx_logic_list.push_back("<");
	cmplx_logic_list.push_back(">=");
	cmplx_logic_list.push_back("=<");
	cmplx_logic_list.push_back("not");
	cmplx_logic_list.push_back("and");
	cmplx_logic_list.push_back("or");
	cmplx_logic_list.push_back("xor");
	cmplx_logic_list.push_back("shl");
	cmplx_logic_list.push_back("shr");

	cmplx_assign_list.push_back(":=");

	cmplx_pointer_list.push_back("^");
}

/*!
* Counts the number of comment lines, removes comments, and
* replaces quoted strings by special chars, e.g., $
* All arguments are modified by the method.
* Since Pascal compiler directives are block comments starting with '$'
* this method also captures directive SLOC.
*
* \param fmap list of processed file lines
* \param result counter results
* \param fmapBak list of original file lines (same as fmap except it contains unmodified quoted strings)
*
* \return method status
*/
int CPascalCounter::CountCommentsSLOC(filemap* fmap, results* result, filemap *fmapBak)
{
	if (BlockCommentStart.empty() && LineCommentStart.empty())
		return 0;
	if (classtype == DATAFILE)
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
	string curBlckCmtStart, curBlckCmtEnd;
	char CurrentQuoteEnd = 0;
	bool quote_contd = false;
	filemap::iterator itfmBak = fmapBak->begin();
	
	string strDirLine;
	size_t strSize;
	bool isDirective = false, trunc_flag = false;

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
				FindCommentStart(iter->line, comment_start, comment_type, curBlckCmtStart, curBlckCmtEnd);

			if (comment_start == string::npos && quote_idx_start == string::npos)
				break;

			if (comment_start != string::npos)
				idx_start = comment_start;

			// if found quote before comment, e.g., "this is quote");//comment
			if (quote_idx_start != string::npos && (comment_start == string::npos || quote_idx_start < comment_start))
			{
				ReplaceQuote(iter->line, quote_idx_start, quote_contd, CurrentQuoteEnd);
				if (quote_idx_start > idx_start)
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
					{
						idx_end = iter->line.find(curBlckCmtEnd, idx_start + curBlckCmtStart.length());

						// check whether comment is a directive (starts with '$')
						isDirective = false;
						for (size_t i = 0; i < BlockCommentStart.size(); i++)
						{
							if (iter->line.substr(idx_start, BlockCommentStart[i].length() + 1) == BlockCommentStart[i] + "$")
							{
								strDirLine = "";
								isDirective = true;
								break;
							}
						}
					}

					if (idx_end == string::npos)
					{
						if (comment_type == 3)
						{
							if (isDirective)
							{
								strSize = CUtil::TruncateLine(itfmBak->line.length(), strDirLine.length(), this->lsloc_truncate, trunc_flag);
								if (strSize > 0)
								{
									if (contd)
										strDirLine += "\n" + itfmBak->line.substr(0, strSize);
									else
										strDirLine = itfmBak->line.substr(0, strSize);
								}
								result->directive_lines[PHY]++;
							}
							else
								result->comment_lines++;
							iter->line = "";
							itfmBak->line = "";
						}
						else if (comment_type == 4)
						{
							if (isDirective)
							{
								strSize = CUtil::TruncateLine(itfmBak->line.length() - idx_start, 0, this->lsloc_truncate, trunc_flag);
								if (strSize > 0)
									strDirLine = itfmBak->line.substr(idx_start, strSize);
								result->directive_lines[PHY]++;
							}
							iter->line = iter->line.substr(0, idx_start);
							itfmBak->line = itfmBak->line.substr(0, idx_start);
							// trim trailing space
							iter->line = CUtil::TrimString(iter->line, 1);
							itfmBak->line = CUtil::TrimString(itfmBak->line, 1);
							if (!isDirective)
							{
								if (iter->line.empty())
									result->comment_lines++;	// whole line
								else
									result->e_comm_lines++;		// embedded
							}
						}
						contd = true;
						contd_nextline = true;
						break;
					}
					else
					{
						if (isDirective)
						{
							strSize = CUtil::TruncateLine(idx_end - idx_start + curBlckCmtEnd.length(), strDirLine.length(), this->lsloc_truncate, trunc_flag);
							if (strSize > 0)
							{
								if (contd)
									strDirLine += "\n" + itfmBak->line.substr(idx_start, strSize);
								else
									strDirLine = itfmBak->line.substr(idx_start, strSize);
							}
							result->directive_lines[PHY]++;
						}
						contd = false;
						iter->line.erase(idx_start, idx_end - idx_start + curBlckCmtEnd.length());
						itfmBak->line.erase(idx_start, idx_end - idx_start + curBlckCmtEnd.length());
						if (isDirective)
						{
							if (result->addSLOC(strDirLine, trunc_flag))
								result->directive_lines[LOG]++;
							strDirLine = "";
						}
						else
						{
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
int CPascalCounter::LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak)
{
	bool		found_block			= false;
	bool		found_forifwhile	= false;
	bool		found_end			= false;
	bool		found_loop			= false;
	string		strLSLOC			= "";
	string		strLSLOCBak			= "";

	filemap::iterator fit, fitbak;
	string line, lineBak;
	StringVector loopLevel;
	unsigned int cnt = 0;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";

	for (fit = fmap->begin(), fitbak = fmapBak->begin(); fit != fmap->end(); fit++, fitbak++)
	{
		// insert blank at the beginning (for searching keywords)
		line = ' ' + fit->line;
		lineBak = ' ' + fitbak->line;
	
		// do not process blank lines
		// blank line means blank_line/comment_line/directive
		if (!CUtil::CheckBlank(line))
		{
			// blank line means blank_line/comment_line/directive
			// call SLOC function to detect logical SLOC and add to result
			LSLOC(result, line, lineBak, strLSLOC, strLSLOCBak, found_block,
				found_forifwhile, found_end, found_loop, loopLevel);

			cnt = 0;
			CUtil::CountTally(line, data_name_list, cnt, 1, exclude, "", "", NULL, false);

			// need to check also if the data line continues
			if (cnt > 0)
				result->data_lines[PHY] += 1;
			else
				result->exec_lines[PHY] += 1;

			if (print_cmplx)
			{
				cnt = 0;
				CUtil::CountTally(line, exec_name_list, cnt, 1, exclude, "", "", &result->exec_name_count, false);
			}
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
* \param found_block found block flag
* \param found_forifwhile found for, if, or while flag
* \param found_end found end flag
* \param found_loop found loop flag
* \param loopLevel nested loop level
*/
void CPascalCounter::LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, bool &found_block,
						   bool &found_forifwhile, bool &found_end, bool &found_loop, StringVector &loopLevel)
{
	size_t start = 0; // starting index of the working string
	size_t i, tempi, strSize;
	string templine = CUtil::TrimString(line);
	string tmp;
	bool trunc_flag = false;
	unsigned int loopCnt;
	StringVector::iterator lit;
	string keywordchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";

	// there may be more than 1 logical SLOC in a line
	found_end = false;
	for (i = 0; i < line.length(); i++)
	{
		if (line[i] == ';')
		{
			if (!found_end)
			{
				strSize = CUtil::TruncateLine(i - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				tmp = strLSLOC;
				FoundSLOC(result, strLSLOC, strLSLOCBak,
					found_block, found_forifwhile, found_end, trunc_flag);

				// record end loop for nested loop processing
				if (print_cmplx)
				{
					if (found_loop)
					{
						found_loop = false;
						loopLevel.push_back("do");

						loopCnt = 0;
						for (lit = loopLevel.begin(); lit < loopLevel.end(); lit++)
						{
							if ((*lit) != "")
								loopCnt++;
						}
						if ((unsigned int)result->cmplx_nestloop_count.size() < loopCnt)
							result->cmplx_nestloop_count.push_back(1);
						else
							result->cmplx_nestloop_count[loopCnt-1]++;

						loopLevel.pop_back();
					}
					else if (CUtil::FindKeyword(tmp, "end", 0, TO_END_OF_STRING, false) != string::npos ||
						CUtil::FindKeyword(tmp, "until", 0, TO_END_OF_STRING, false) != string::npos)
					{
						if (loopLevel.size() > 0)
							loopLevel.pop_back();
					}
				}
			}
			else
			{
				if (strLSLOC.size() > 0)
				{
					FoundSLOC(result, strLSLOC, strLSLOCBak,
						found_block, found_forifwhile, found_end, trunc_flag);
				}
				found_end = false;	// end xxx
				found_block = false;
				found_forifwhile = false;
				strLSLOC = "";
				strLSLOCBak = "";
			}
			found_loop = false;
			start = i + 1;
		}

		// if it ends in xxx, then it has already been counted, so ignore it
		tmp = "xxx " + CUtil::TrimString(line.substr(start, i + 1 - start));
		if (CUtil::FindKeyword(tmp, "end", 0, TO_END_OF_STRING, false) != string::npos)
		{
			// check for 'end,' and skip
			if ((line.length() > i + 1 && line[i + 1] == ',') || line[i] == ',')
				continue;

			found_end = true;
			found_block = false;
			found_loop = false;

			// capture SLOC
			strSize = CUtil::TruncateLine(i - 2 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
			if (strSize > 0)
			{
				strLSLOC += line.substr(start, strSize);
				strLSLOCBak += lineBak.substr(start, strSize);
			}
			FoundSLOC(result, strLSLOC, strLSLOCBak,
				found_block, found_forifwhile, found_end, trunc_flag);

			// if end is followed by a period 'end.' record SLOC if any
			if (line.length() > i + 1 && line[i + 1] == '.')
			{
				// record end loop for nested loop processing
				if (print_cmplx)
				{
					while (loopLevel.size() > 0)
						loopLevel.pop_back();
				}
				start = i + 2;
				continue;
			}
			else
			{
				// record end loop for nested loop processing
				if (print_cmplx)
				{
					if (loopLevel.size() > 0)
						loopLevel.pop_back();
				}
				start = i + 1;
			}
		}

		// continue the following processing only if line[i] is not in a middle of a word
		if (keywordchars.find(line[i]) != string::npos && i < line.length() - 1)
			continue;

		if (!found_end)
		{
			if (!found_forifwhile)
			{
				if (CUtil::FindKeyword(tmp, "for", 0 , TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "while", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "repeat", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "with", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "if", 0, TO_END_OF_STRING, false) != string::npos)
				{
					found_forifwhile = true;
				}

				if (CUtil::FindKeyword(tmp, "do", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "repeat", 0, TO_END_OF_STRING, false) != string::npos)
				{
					if (CUtil::FindKeyword(tmp, "do", 0, TO_END_OF_STRING, false) != string::npos)
					{
						// found a SLOC
						strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
						if (strSize > 0)
						{
							strLSLOC += line.substr(start, strSize);
							strLSLOCBak += lineBak.substr(start, strSize);
						}
						FoundSLOC(result, strLSLOC, strLSLOCBak,
							found_block, found_forifwhile, found_end, trunc_flag);
						start = i + 1;

						found_loop = true;
					}
					else
					{
						// record nested loop level
						if (print_cmplx)
						{
							loopLevel.push_back("repeat");

							loopCnt = 0;
							for (lit = loopLevel.begin(); lit < loopLevel.end(); lit++)
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
					continue;
				}
			}
			else if (CUtil::FindKeyword(tmp, "do", 0, TO_END_OF_STRING, false) != string::npos ||
				CUtil::FindKeyword(tmp, "then", 0, TO_END_OF_STRING, false) != string::npos)
			{
				// found a SLOC
				strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak,
					found_block, found_forifwhile, found_end, trunc_flag);
				start = i + 1;

				if (CUtil::FindKeyword(tmp, "do", 0, TO_END_OF_STRING, false) != string::npos)
					found_loop = true;

				continue;
			}

			// process else since no ';' is allowed before else
			if (CUtil::FindKeyword(tmp, "else", 0, TO_END_OF_STRING, false) != string::npos)
			{
				strSize = CUtil::TruncateLine(i - 4 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak,
					found_block, found_forifwhile, found_end, trunc_flag);
				strLSLOC = strLSLOCBak = "else ";
				start = i + 1;
				continue;
			}

			// process until since ';' is optional before else
			if (CUtil::FindKeyword(tmp, "until", 0, TO_END_OF_STRING, false) != string::npos)
			{
				strSize = CUtil::TruncateLine(i - 5 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak,
					found_block, found_forifwhile, found_end, trunc_flag);
				strLSLOC = strLSLOCBak = "until ";
				start = i + 1;
				continue;
			}

			if (!found_block)
			{
				if (CUtil::FindKeyword(tmp, "begin", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "asm", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "case",0, TO_END_OF_STRING, false) != string::npos)
				{
					found_block = true;

					// record nested loop level
					if (print_cmplx)
					{
						if (found_loop)
						{
							found_loop = false;
							loopLevel.push_back("do");

							loopCnt = 0;
							for (lit = loopLevel.begin(); lit < loopLevel.end(); lit++)
							{
								if ((*lit) != "")
									loopCnt++;
							}
							if ((unsigned int)result->cmplx_nestloop_count.size() < loopCnt)
								result->cmplx_nestloop_count.push_back(1);
							else
								result->cmplx_nestloop_count[loopCnt-1]++;
						}
						else
							loopLevel.push_back("");
					}
				}
			}
			else
			{
				// only add new SLOC if 'of' is at the end of line and follows 'case', etc.
				tempi = CUtil::FindKeyword(templine, "of", 0, TO_END_OF_STRING, false);
				if (tempi == templine.length() - 2)
				{
					strSize = CUtil::TruncateLine(line.length() - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
					if (strSize > 0)
					{
						strLSLOC += line.substr(start, strSize);
						strLSLOCBak += lineBak.substr(start, strSize);
					}
					FoundSLOC(result, strLSLOC, strLSLOCBak,
						found_block, found_forifwhile, found_end, trunc_flag);
					start = line.length();
					found_block = false;
					continue;
				}
			}

			// check for '= record'
			if (CUtil::FindKeyword(tmp, "= array", 0, TO_END_OF_STRING, false) != string::npos ||
				CUtil::FindKeyword(tmp, "= record", 0, TO_END_OF_STRING, false) != string::npos)
			{
				strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak,
					found_block, found_forifwhile, found_end, trunc_flag);
				start = i + 1;

				if (print_cmplx)
					loopLevel.push_back("");

				continue;
			}
		}
	}

	tmp = CUtil::TrimString(line.substr(start, i - start));
	strSize = CUtil::TruncateLine(tmp.length(), strLSLOC.length(), this->lsloc_truncate, trunc_flag);
	if (strSize > 0)
	{
		strLSLOC += tmp.substr(0, strSize);
		tmp = CUtil::TrimString(lineBak.substr(start, i - start));
		strLSLOCBak += tmp.substr(0, strSize);
	}
	if (tmp == "")
	{
		found_forifwhile = false;
	}
}

/*!
* Processes a logical line of code.
* This method is called after a logical SLOC is determined.
* The method adds LSLOC to the result, increases counts, and resets variables.
*
* \param result counter results
* \param strLSLOC processed logical string
* \param strLSLOCBak original logical string
* \param found_block found block flag
* \param found_forifwhile found for, if, or while flag
* \param found_end found end flag
* \param trunc_flag truncate lines?
*/
void CPascalCounter::FoundSLOC(results* result, string &strLSLOC, string &strLSLOCBak, bool &found_block,
							   bool &found_forifwhile, bool &found_end, bool &trunc_flag)
{
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";

	// add to the list for comparison purpose
	if (result->addSLOC(strLSLOCBak, trunc_flag))
	{
		// determine logical type, data declaration or executable
		unsigned int cnt = 0;
		CUtil::CountTally(strLSLOC, data_name_list, cnt, 1, exclude, "", "", &result->data_name_count, false);
		if (cnt > 0)
			result->data_lines[LOG] += 1;
		else
			result->exec_lines[LOG] += 1;

		// reset all variables whenever a new statement/logical SLOC is found
		strLSLOC = "";
		strLSLOCBak = "";
		found_block = false;
		found_forifwhile = false;
		found_end = false;
	}
}
