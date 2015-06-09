//! Code counter class methods for the VHDL language.
/*!
* \file CVHDLCounter.cpp
*
* This file contains the code counter class methods for the VHDL hardware definition language (used in FPGA programming).
*/

#include "CVHDLCounter.h"
#include <sstream>

/*!
* Constructs a CVHDLCounter object.
*/
CVHDLCounter::CVHDLCounter()
{
	classtype = VHDL;
	language_name = "VHDL";

	file_extension.push_back(".vhd");
	file_extension.push_back(".vhdl");

	LineCommentStart.push_back("--");
	QuoteStart = "\"";
	QuoteEnd = "\"";
	QuoteEscapeFront = '\"';
	
	directive.push_back("pragma");

	data_name_list.push_back("access");
	data_name_list.push_back("alias");
	data_name_list.push_back("attribute");
	data_name_list.push_back("buffer");
	data_name_list.push_back("bus");
	data_name_list.push_back("constant");
	data_name_list.push_back("file");
	data_name_list.push_back("generic");
	data_name_list.push_back("group");
	data_name_list.push_back("label");
	data_name_list.push_back("linkage");
	data_name_list.push_back("literal");
	data_name_list.push_back("new");
	data_name_list.push_back("range");
	data_name_list.push_back("record");
	data_name_list.push_back("register");
	data_name_list.push_back("shared");
	data_name_list.push_back("signal");
	data_name_list.push_back("subtype");
	data_name_list.push_back("type");
	data_name_list.push_back("units");
	data_name_list.push_back("variable");

	exec_name_list.push_back("after");
	exec_name_list.push_back("architecture");
	exec_name_list.push_back("assert");
	exec_name_list.push_back("begin");
	exec_name_list.push_back("block");
	exec_name_list.push_back("body");
	exec_name_list.push_back("case");
	exec_name_list.push_back("component");
	exec_name_list.push_back("configuration");
	exec_name_list.push_back("disconnect");
	exec_name_list.push_back("else");
	exec_name_list.push_back("elsif");
	exec_name_list.push_back("end");
	exec_name_list.push_back("entity");
	exec_name_list.push_back("exit");
	exec_name_list.push_back("for");
	exec_name_list.push_back("function");
	exec_name_list.push_back("generate");
	exec_name_list.push_back("if");
	exec_name_list.push_back("inertial");
	exec_name_list.push_back("library");
	exec_name_list.push_back("loop");
	exec_name_list.push_back("map");
	exec_name_list.push_back("next");
	exec_name_list.push_back("null");
	exec_name_list.push_back("on");
	exec_name_list.push_back("others");
	exec_name_list.push_back("package");
	exec_name_list.push_back("port");
	exec_name_list.push_back("procedure");
	exec_name_list.push_back("process");
	exec_name_list.push_back("reject");
	exec_name_list.push_back("report");
	exec_name_list.push_back("return");
	exec_name_list.push_back("select");
	exec_name_list.push_back("then");
	exec_name_list.push_back("transport");
	exec_name_list.push_back("use");
	exec_name_list.push_back("wait");
	exec_name_list.push_back("while");
	exec_name_list.push_back("with");

	cmplx_preproc_list.push_back("pragma");

	cmplx_calc_list.push_back("+");
	cmplx_calc_list.push_back("-");
	cmplx_calc_list.push_back("*");
	cmplx_calc_list.push_back("**");
	cmplx_calc_list.push_back("/");
	cmplx_calc_list.push_back("mod");
	cmplx_calc_list.push_back("abs");
	cmplx_calc_list.push_back("rem");

	cmplx_logic_list.push_back("=");
	cmplx_logic_list.push_back("<");
	cmplx_logic_list.push_back(">");
	cmplx_logic_list.push_back("/=");
	cmplx_logic_list.push_back(">=");
	cmplx_logic_list.push_back("&");
	cmplx_logic_list.push_back("not");
	cmplx_logic_list.push_back("and");
	cmplx_logic_list.push_back("or");
	cmplx_logic_list.push_back("nand");
	cmplx_logic_list.push_back("nor");
	cmplx_logic_list.push_back("xor");
	cmplx_logic_list.push_back("xnor");
	cmplx_logic_list.push_back("sll");
	cmplx_logic_list.push_back("srl");
	cmplx_logic_list.push_back("sla");
	cmplx_logic_list.push_back("sra");
	cmplx_logic_list.push_back("rol");
	cmplx_logic_list.push_back("ror");
	cmplx_logic_list.push_back("<=");

	cmplx_cond_list.push_back("case");
	cmplx_cond_list.push_back("for");
	cmplx_cond_list.push_back("if");
	cmplx_cond_list.push_back("else");
	cmplx_cond_list.push_back("elseif");
	cmplx_cond_list.push_back("loop");
	cmplx_cond_list.push_back("next");
	cmplx_cond_list.push_back("when");
	cmplx_cond_list.push_back("while");
	
	cmplx_assign_list.push_back("=>");
	cmplx_assign_list.push_back(":=");
	cmplx_assign_list.push_back("<=");
}
/*!
* Replaces quoted strings inside a string starting at idx_start with '$'.
* Handles special cases for VHDL literal strings.
*
* \param strline string to be processed
* \param idx_start index of line character to start search
* \param contd specifies the quote string is continued from the previous line
* \param CurrentQuoteEnd end quote character of the current status
*
* \return method status
*/
int CVHDLCounter::ReplaceQuote(string &strline, size_t &idx_start, bool &contd, char &CurrentQuoteEnd)
{
	size_t idx = 0;
	while (true)
	{
		idx = strline.find("\"", idx);	// replace all '"' by '$'
		if (idx != string::npos)
			strline.replace(idx, 1, 1, '$');
		else
			break;
	}
	return CCodeCounter::ReplaceQuote(strline, idx_start, contd, CurrentQuoteEnd);
}

/*!
* Counts the number of comment lines, removes comments, and
* replaces quoted strings by special chars, e.g., $
* All arguments are modified by the method.
*
* \param fmap list of processed file lines
* \param result counter results
* \param fmapBak list of original file lines (same as fmap except it contains unmodified quoted strings)
*
* \return method status
*/
int CVHDLCounter::CountCommentsSLOC(filemap* fmap, results* result, filemap *fmapBak)
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
	string curBlckCmtStart, curBlckCmtEnd;
	char CurrentQuoteEnd = 0;
	bool quote_contd = false, directiveComment =false;
	filemap::iterator itfmBak = fmapBak->begin();

	quote_idx_start = 0;

	for (filemap::iterator iter = fmap->begin(); iter != fmap->end(); iter++, itfmBak++)
	{
		directiveComment = false;
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
				if (quote_idx_start > idx_start && quote_idx_start != iter->line.length())
				{
					// comment delimiter inside quote
					idx_start = quote_idx_start;
					continue;
				}
			}
			else if (comment_start != string::npos)
			{
				// check if next word is pragma or synopsis;  this can turn out to be a declaration
				StringVector::iterator itDirective = directive.begin();
				for (; itDirective != directive.end(); ++itDirective)
				{
					if ((CUtil::FindKeyword(iter->line, *itDirective, comment_start+LineCommentStart.size(), TO_END_OF_STRING, false)) != string::npos)
					{
						directiveComment = true;
						iter->line = *itDirective;
						break;
					} 
				}
				if (directiveComment)
					break;
				
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
		if (directiveComment)
			continue;
	}
	return 1;
}

/*!
* Counts file language complexity based on specified language keywords/characters.
*
* \param fmap list of processed file lines
* \param result counter results
*
* \return method status
*/
int CVHDLCounter::CountComplexity(filemap* fmap, results* result)
{
	if (classtype == UNKNOWN || classtype == DATAFILE)
		return 0;
	filemap::iterator fit;
	filemap fitBak;
	filemap::iterator fitForw, fitBack;//used to check prior an later lines for semicolons
	unsigned int cnt;
	string line, line2;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$><=:";
	tokLocVect conditionalVector;
	tokLocVect::reverse_iterator r_tlvIter;
	StringVector::iterator strIter = this->cmplx_cond_list.begin();
	string buf;			// have a buffer string
	stringstream ss;	// insert the string into a stream
	tokenLocation tl;
	int count;
	bool whenCont;

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
		StringVector tmpList = cmplx_logic_list;//making a temporary list with the '<=' operator removed from the list; counting it on another pass;
		tmpList.pop_back();
		CUtil::CountTally(line, tmpList, cnt, 1, exclude, "", "", &result->cmplx_logic_count, casesensitive);
		result->cmplx_logic_lines += cnt;

		// preprocessor directives
		cnt = 0;
		CUtil::CountTally(line, cmplx_preproc_list, cnt, 1, exclude, "", "", &result->cmplx_preproc_count, casesensitive);
		result->cmplx_preproc_lines += cnt;

		// assignments
		cnt = 0;
		tmpList.clear();
		tmpList = cmplx_assign_list;//making a temporary list with the '<=' operator removed from the list; counting it on another pass;
		tmpList.pop_back();
		CUtil::CountTally(line, tmpList, cnt, 1, exclude, "", "", &result->cmplx_assign_count, casesensitive);
		result->cmplx_assign_lines += cnt;

		// pointers
		cnt = 0;
		CUtil::CountTally(line, cmplx_pointer_list, cnt, 1, exclude, "", "", &result->cmplx_pointer_count, casesensitive);
		result->cmplx_pointer_lines += cnt;
	}

	// do a single pass to mark and replace logical operator lessThan or equal "<="
	// these appear only in conditional statements
	// the remaining are signal assignment operators
	for (fit = fmap->begin(); fit != fmap->end(); fit++)
	{
		line = fit->line;
		line = CUtil::ToLower(line);
		
		if (CUtil::CheckBlank(line))
			continue;
		ss.clear();
		ss.str("");
		ss << line;
		count = -1;
		while (ss >> buf)
		{
			++count;
			if (!buf.compare("if"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("then"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("elsif"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("wait"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("until"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("assert"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("while"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("loop"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("next"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("when"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("exit"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("return"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (!buf.compare("case"))
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = buf;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (buf.find_last_of(";") != string::npos)
			{
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				tl.token = ";";
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
			}
			if (buf.find("<=") != string::npos)
			{
				whenCont = false;
				// iterate up the vector an look for the first conditional statement
				r_tlvIter = conditionalVector.rbegin();
				while (r_tlvIter != conditionalVector.rend())
				{
					if (!r_tlvIter->token.compare(";"))
					{
						result->cmplx_assign_count.back()++; 
						result->cmplx_assign_lines++;
						tl.token = "assign";	
						break;
					}
					else
					{
						if ((!r_tlvIter->token.compare("if") || !r_tlvIter->token.compare("elsif") || !r_tlvIter->token.compare("assert") || 
							!r_tlvIter->token.compare("while") || !r_tlvIter->token.compare("return") || !r_tlvIter->token.compare("until") ) && !whenCont)
						{
								result->cmplx_logic_count.back()++;
								result->cmplx_logic_lines++;
								tl.token = "lte";
								break;
						}
						if (!r_tlvIter->token.compare("when"))
						{
							whenCont = true;
							r_tlvIter++;
							continue;
						}
						if (!r_tlvIter->token.compare("case") || !r_tlvIter->token.compare("next") || !r_tlvIter->token.compare("exit"))
						{
							result->cmplx_assign_count.back()++; 
							result->cmplx_assign_lines++;
							tl.token = "assign";
							whenCont = false;
							break;
						}
						result->cmplx_assign_count.back()++; 
						result->cmplx_assign_lines++;
						tl.token = "assign";
						break;
					}
					r_tlvIter++;
				}
				tl.lineNumber= fit->lineNumber;
				tl.position = count;
				buf.clear();
				conditionalVector.push_back(tl);
				continue;
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
int CVHDLCounter::CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak)
{
	size_t idx;
	unsigned int cnt = 0;
	string strDirLine = "";
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$><=:";

	filemap::iterator itfmBak = fmapBak->begin();
	for (filemap::iterator iter = fmap->begin(); iter != fmap->end(); iter++, itfmBak++)
	{
		if (CUtil::CheckBlank(iter->line))
			continue;

		if (print_cmplx)
		{
			cnt = 0;
			CUtil::CountTally(" " + iter->line, directive, cnt, 1, exclude, "", "", &result->directive_count, false);
		}

		// if not a continuation of a previous directive
		for (vector<string>::iterator viter = directive.begin(); viter != directive.end(); viter++)
		{
			// merged bug fix for considering only stand-alone keywords
			// e.g. package should not be considered a directive (only 'pack' is)
			if (((idx = CUtil::FindKeyword(iter->line, *viter, 0, TO_END_OF_STRING, false)) != string::npos) && idx == 0)
			{
				iter->line = "";
				result->directive_lines[PHY]++;
				break;
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
int CVHDLCounter::LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak)
{
	bool			blank_flag		= false;
	bool			found_unit	= false;
	string			strLSLOC		= "";
	string			strLSLOCBak		= "";
	unsigned int	cnt				= 0;
	unsigned int	loopLevel		= 0;

	filemap::iterator fit, fitbak;
	string line, lineBak, tmp;
	string special = "[]()+/-*<>=,@&~!^?:%{}";
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$><=:";

	unsigned int l_paren_cnt = 0;
	bool l_foundblock, found_forifwhile, found_end, found_type, found_is, processSignatureFound, found_withSelect, found_whenConditional, foundWait, blockSignatureFound, found_record;
	l_foundblock = found_forifwhile = found_end = found_is = processSignatureFound = found_withSelect = found_whenConditional = foundWait = blockSignatureFound = found_record = false;
	vectorString currentBlock;

	for (fit = fmap->begin(), fitbak = fmapBak->begin(); fit != fmap->end(); fit++, fitbak++)
	{
		// insert blank at the beginning (for searching keywords)
		line = ' ' + fit->line;
		lineBak = ' ' + fitbak->line;

		if (CUtil::CheckBlank(line))
		{
			// the line is either blank/whole line comment/compiler directive
			blank_flag = true;
			continue;
		}
		else
			blank_flag = false;

		if (!blank_flag)
		{
			// blank line means blank_line/comment_line/directive
			// call SLOC function to detect logical SLOC and add to result
			LSLOC(result, line, lineBak, strLSLOC, strLSLOCBak, l_paren_cnt, l_foundblock,
				found_forifwhile, found_end, found_type, found_is, found_unit, loopLevel, currentBlock, processSignatureFound, found_withSelect, found_whenConditional, foundWait,
				blockSignatureFound, found_record);

			cnt = 0;
			CUtil::CountTally(line, data_name_list, cnt, 1, exclude, "", "", NULL, false);	//tie breaker in favor of physical data rather than physical exec 		
	
			// need to check also if the data line continues
			if ((cnt > 0 && currentBlock.size() == 0 ) || (currentBlock.size()>0 && (currentBlock.back().compare("record") == 0 || currentBlock.back().compare("units") == 0)))
				result->data_lines[PHY]++;
			else
				result->exec_lines[PHY]++;

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
* \param found_type found type flag
* \param found_is found is flag
* \param found_unit found accept flag
* \param trunc_flag truncate lines?
* \param currentBlock current block vector
*/
void CVHDLCounter::FoundSLOC(results* result, string &strLSLOC, string &strLSLOCBak, bool &found_block, bool &found_forifwhile,
	bool &found_end, bool &found_type, bool &found_is, bool &found_unit, bool &trunc_flag, StringVector currentBlock)
{
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:";

	// add to the list for comparison purpose
	if (result->addSLOC(CUtil::TrimString(strLSLOCBak), trunc_flag))
	{
		// determine logical type, data declaration or executable
		unsigned int cnt = 0;

		CUtil::CountTally(strLSLOC, data_name_list, cnt, 1, exclude, "", "", &result->data_name_count, false);
		if ((cnt > 0 && currentBlock.size() == 0) || (currentBlock.size()>0 && (currentBlock.back().compare("record") == 0 || currentBlock.back().compare("units") == 0))  )
			result->data_lines[LOG]++;
		else
			result->exec_lines[LOG]++;
	}

	// reset all variables whenever a new statement/logical SLOC is found
	strLSLOC = "";
	strLSLOCBak = "";
	found_block = false;
	found_forifwhile = false;
	found_end = false;
	found_type = false;
	found_is = false;
	found_unit = false;
}

/*!
* Extracts and stores logical lines of code.
* Determines and extract logical SLOC to place in the result variable
* using addSLOC function. Each time the addSLOC function is called,
* a new logical SLOC is added. This function assumes that the directive
* is handled before it is called.
*
* \param result counter results28186	
* \param line processed physical line of code
* \param lineBak original physical line of code
* \param strLSLOC processed logical string
* \param strLSLOCBak original logical string
* \param paren_cnt count of parenthesis
* \param found_block found block flag
* \param found_forifwhile found for, if, or while flag
* \param found_end found end flag
* \param found_type found type flag
* \param found_is found is flag
* \param found_unit found unit flag
* \param loopLevel nested loop level
* \param currentBlock current block vector
* \param processSignatureStartFound found process signature start flag
* \param found_withSelect found with/select flag
* \param found_whenConditional found when/conditional flag
* \param foundWait found wait flag
* \param blockSignatureStartFound found block signature start flag
* \param found_recort found record flag
*/
void CVHDLCounter::LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, unsigned int &paren_cnt,
	bool &found_block, bool &found_forifwhile, bool &found_end, bool &found_type, bool &found_is, bool &found_unit,
	unsigned int &loopLevel, vectorString &currentBlock, bool &processSignatureStartFound, bool &found_withSelect, bool &found_whenConditional, bool &foundWait,
	bool &blockSignatureStartFound, bool &found_record)
{
	size_t start = 0, forstart = 0; //starting index of the working string
	size_t i = 0, tempi, strSize;
	string templine = CUtil::TrimString(line);
	string tmp, fortmp;
	bool trunc_flag = false;
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$:";
	string keywordchars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";
	
	// there may be more than 1 logical SLOC in a line
	for (i = 0; i < line.length(); i++)
	{
		switch (line[i])
		{
		case ';':
			if (paren_cnt > 0)
				break;
			if (currentBlock.size() != 0)
			{
				if (((string) currentBlock.back()).compare("port") == 0)
				{
					// paren_cnt should be zero here
					currentBlock.pop_back();
				}
				if (((string) currentBlock.back()).compare("generic") == 0)
				{
					// paren_cnt should be zero here
					currentBlock.pop_back();	
				}
				if (found_withSelect && CUtil::FindKeyword(line.substr(start, i + 1 - start), "others", 0, TO_END_OF_STRING, false) != string::npos)
				{
					// found the end of with ... select statement
					found_withSelect = false;
					currentBlock.pop_back();
				}
				if (found_whenConditional)
				{
					//found the end of when ... conditional statement
					found_whenConditional = false;
					currentBlock.pop_back();
				}
			}
			if (!found_end)
			{
				if (foundWait) foundWait = false;

				// check for empty statement (=1 LSLOC)
				if (CUtil::TrimString(line.substr(start, i + 1 - start)) == ";" && strLSLOC.length() < 1)
				{
					strLSLOC = ";";
					strLSLOCBak = ";";
				}
				else
				{
					strSize = CUtil::TruncateLine(i - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
					if (strSize > 0)
					{
						strLSLOC += line.substr(start, strSize);
						strLSLOCBak += lineBak.substr(start, strSize);
					}
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile, 
					found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
				if (currentBlock.size() != 0 && (currentBlock.front().compare("configuration") == 0) && (currentBlock.back().compare("entity") == 0))
				{
					//pop the element in the vector matching the END that was just found
					if (currentBlock.size() != 0)
						currentBlock.pop_back();
				}
			}
			else
			{
				if (currentBlock.size() != 0 && ((string) currentBlock.back()).compare("for") == 0 && ((string) currentBlock.front()).compare("configuration") == 0)
				{
					FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile, 
						found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
				}
				found_end = false;
				found_block = false;
				found_is = false;
				found_forifwhile = false;
				found_type = false;
				found_unit = false;
				strLSLOC = "";
				strLSLOCBak = "";
				foundWait = false;
				if (found_record)
					found_record = false;
				if (found_unit)
					found_unit = false;

				//pop the element in the vector matching the END that was just found
				if (currentBlock.size() != 0)
					currentBlock.pop_back();
			}
			start = i + 1;
			break;
		case '(':
			if (found_type)
				found_type = false;
		    if (currentBlock.size() != 0)
			{
				if (((string) currentBlock.back()).compare("process") == 0 && !found_is)
					processSignatureStartFound = true;
				else if (((string) currentBlock.back()).compare("block") == 0 && !found_is)
					blockSignatureStartFound = true;
			}
			paren_cnt++;
			break;
		case ')':
			if (paren_cnt > 0)
				paren_cnt--;
			break;
		case ',':
			if (found_withSelect)
			{
				strSize = CUtil::TruncateLine(i+1-start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
					found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
				start = i + 1;
				continue;	
			}
			break;
		default:
			if (currentBlock.size() != 0)
			{
				// check for is here first
				tmp = "xxx " + CUtil::TrimString(line.substr(start, i + 1 - start));
				
				//try to locate the is at the end of the string if a block has been found
				if (found_block)
				{
					tempi = CUtil::FindKeyword(tmp, "is", 0, TO_END_OF_STRING, false);
					if (tempi != string::npos)
					{
						strSize = CUtil::TruncateLine(tempi+3-start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
						if (strSize > 0)
						{
							strLSLOC += line.substr(start, strSize);
							strLSLOCBak += lineBak.substr(start, strSize);
						}
						FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
							found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
						start = i + 1;
						found_is = true;
						if (currentBlock.size() > 0 && (currentBlock.back().compare("block") == 0 || currentBlock.back().compare("process") == 0 ))
							processSignatureStartFound = true;
						continue;
					}
					else
					{
						// now see if there is anything else other than a space or an 'i' for the start of the word is
						if (((string) currentBlock.back()).compare("process") == 0 && line[i] != ' ' && line[i] != 'i' && paren_cnt == 0 && !found_is)
						{
							strSize = CUtil::TruncateLine(i - 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
							if (strSize > 0)
							{
								strLSLOC += line.substr(start, strSize);
								strLSLOCBak += lineBak.substr(start, strSize);
							}
							FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
								found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
							start = i;
							processSignatureStartFound = false;
							continue;
						}
						else if (((string) currentBlock.back()).compare("block") == 0 && line[i] != ' ' && line[i] != 'i' && paren_cnt == 0 && !found_is)
						{
							strSize = CUtil::TruncateLine(i - 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
							if (strSize > 0)
							{
								strLSLOC += line.substr(start, strSize);
								strLSLOCBak += lineBak.substr(start, strSize);
							}
							FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
								found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
							start = i;
							blockSignatureStartFound = false;
							continue;
						}		
					}
				}
			}
		}

		// continue the following processing only if line[i] is not in a middle of a word
		if (keywordchars.find(line[i]) != string::npos && i < line.length() - 1)
			continue;

		// if it ends in xxx, then it has already been counted, so ignore it
		tmp = "xxx " + CUtil::TrimString(line.substr(start, i + 1 - start));
		fortmp = "xxx " + CUtil::TrimString(line.substr(forstart, i + 1 - start));

		if (found_block)
		{
			// try to locate the is at the end of the string if a block has been found
			tempi = CUtil::FindKeyword(tmp, "is", 0, TO_END_OF_STRING, false);
			if (tempi != string::npos)
			{
				strSize = CUtil::TruncateLine(tempi+3-start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
					found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
				start = i + 1;
				found_is = true;
				if (currentBlock.size() > 0 && (currentBlock.back().compare("block") == 0 || currentBlock.back().compare("process") == 0 ))
					processSignatureStartFound = true;
				continue;
			}
		}

		tempi = CUtil::FindKeyword(tmp, "end", 0, TO_END_OF_STRING, false);
		if (tempi != string::npos)
		{
			found_end = true;

			// record end loop for nested loop processing
			if (print_cmplx)
			{
				tmp = CUtil::TrimString(line.substr(start, i + 5 - start));
				if (CUtil::FindKeyword(tmp, "end loop", 0, TO_END_OF_STRING, false) != string::npos)
				{
					if (loopLevel > 0)
						loopLevel--;
				}
				tmp = CUtil::TrimString(line.substr(start, i + 9 - start));
				if (CUtil::FindKeyword(tmp, "end generate", 0, TO_END_OF_STRING, false) != string::npos)
				{
					if (loopLevel > 0)
						loopLevel--;
				}
			}
			start = i + 1;
		}

		if (!found_end)
		{
			// 'begin' is ignored because it's counted with procedure, function, etc. already
			// this may ignore the 'standalone' block that starts with 'declare' or only 'begin'
			if (CUtil::FindKeyword(tmp, "begin", 0, TO_END_OF_STRING, false) != string::npos)
			{
				// found a SLOC
				strLSLOC += line.substr(start, i - start + 1);
				strLSLOCBak += lineBak.substr(start, i - start + 1);
				start = i + 1;
				continue;
			}
			if (!foundWait)
			{
				if (CUtil::FindKeyword(tmp, "wait", 0, TO_END_OF_STRING, false) != string::npos)
					foundWait = true;
			}

			if (!found_forifwhile || (currentBlock.size() != 0 && ((string) currentBlock.front()).compare("configuration") == 0))
			{
				if (CUtil::FindKeyword(tmp, "for", 0 , TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "while", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "if", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "elsif", 0, TO_END_OF_STRING, false) != string::npos)
				{
					if (CUtil::FindKeyword(tmp, "if", 0, TO_END_OF_STRING, false) != string::npos)
					{
						currentBlock.push_back("if");
						found_forifwhile = true;
					}
					else if (CUtil::FindKeyword(tmp, "elsif", 0, TO_END_OF_STRING, false) != string::npos)
					{
						found_forifwhile = true;
					}
					else if (CUtil::FindKeyword(tmp, "while", 0, TO_END_OF_STRING, false) != string::npos)
					{
						currentBlock.push_back("while");
						found_forifwhile = true;
					}
					else if (CUtil::FindKeyword(tmp, "for", 0, TO_END_OF_STRING, false) != string::npos && !foundWait)
					{
						found_forifwhile = true;

						if (currentBlock.size() != 0 && ((string) currentBlock.front()).compare("configuration") == 0)
						{
							//inside a configuration block
							if (CUtil::FindKeyword(fortmp, "for", 0, TO_END_OF_STRING, false) != string::npos)
							{
								if (((string) currentBlock.back()).compare("for") == 0 )
								{
									// add sloc if this for loop is nested
									FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
										found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
									currentBlock.push_back("for");
									forstart = i + 1;
								}
								else
								{
									currentBlock.push_back("for");
									forstart = i + 1;
								}
							}								
						}
						else
						{
							// just a regular for
							currentBlock.push_back("for");
						}
					}
				}

				if (currentBlock.size() != 0 && ((string) currentBlock.front()).compare("configuration") == 0)
				{
					// inside a configuration block
					if ((CUtil::FindKeyword(fortmp, "use", 0, TO_END_OF_STRING, false) != string::npos || 
						CUtil::FindKeyword(fortmp, "group", 0, TO_END_OF_STRING, false) != string::npos ||
						CUtil::FindKeyword(fortmp, "attribute", 0, TO_END_OF_STRING, false) != string::npos) && 
						((string) currentBlock.back()).compare("for") == 0)
					{
						FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
							found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
						forstart = i + 1;
					}
				}

				// 'exception' is removed because it is not counted
				if (CUtil::FindKeyword(tmp, "loop", 0, TO_END_OF_STRING, false) != string::npos)
				{
					// found a SLOC
					strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
					if (strSize > 0)
					{
						strLSLOC += line.substr(start, strSize);
						strLSLOCBak += lineBak.substr(start, strSize);
					}
					FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
						found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
					start = i + 1;

					// record nested loop level
					if (print_cmplx)
					{
						loopLevel++;
						if ((unsigned int)result->cmplx_nestloop_count.size() < loopLevel)
							result->cmplx_nestloop_count.push_back(1);
						else
							result->cmplx_nestloop_count[loopLevel-1]++;
					}
					continue;
				}
			}
			else if (CUtil::FindKeyword(tmp, "loop", 0, TO_END_OF_STRING, false) != string::npos ||
				CUtil::FindKeyword(tmp, "then", 0, TO_END_OF_STRING, false) != string::npos ||
				CUtil::FindKeyword(tmp, "record", 0, TO_END_OF_STRING, false) != string::npos || 
				CUtil::FindKeyword(tmp, "generate", 0, TO_END_OF_STRING, false) != string::npos )	// for..use..record
			{
				// found a SLOC
				strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
					found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
				start = i + 1;

				// record nested loop level
				if (print_cmplx)
				{
					if (CUtil::FindKeyword(tmp, "loop", 0, TO_END_OF_STRING, false) != string::npos || CUtil::FindKeyword(tmp, "generate", 0, TO_END_OF_STRING, false) != string::npos)
					{
						loopLevel++;
						if ((unsigned int)result->cmplx_nestloop_count.size() < loopLevel)
							result->cmplx_nestloop_count.push_back(1);
						else
							result->cmplx_nestloop_count[loopLevel-1]++;
					}
				}
				continue;
			}

			// similarly, check for procedure, task, function - it ends with 'is' keyword
			// procedure ... is...
			// package ... is ...
			if (!found_block)
			{
				if (CUtil::FindKeyword(tmp, "procedure", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "function", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "package", 0, TO_END_OF_STRING, false) !=string::npos ||
					CUtil::FindKeyword(tmp, "component", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "case",0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "process", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "entity", 0, TO_END_OF_STRING, false) != string::npos || 
					CUtil::FindKeyword(tmp, "architecture", 0, TO_END_OF_STRING, false) != string::npos ||
					CUtil::FindKeyword(tmp, "configuration", 0, TO_END_OF_STRING, false) != string::npos || 
					CUtil::FindKeyword(tmp, "block", 0, TO_END_OF_STRING, false) != string::npos ) 
				{
					if (CUtil::FindKeyword(tmp, "entity", 0, TO_END_OF_STRING, false) != string::npos)
					{
						if (((currentBlock.size() != 0) && ! (((string) currentBlock.front()).compare("configuration") == 0)))
						{
							currentBlock.push_back("entity");
							found_block = true;
						}
						else if (currentBlock.size() == 0)
						{
							currentBlock.push_back("entity");
							found_block = true;
						} 
					}
					else if (CUtil::FindKeyword(tmp, "architecture", 0, TO_END_OF_STRING, false) != string::npos)
					{
						currentBlock.push_back("architecture");
						found_block = true;
					}
					else if (CUtil::FindKeyword(tmp, "case", 0, TO_END_OF_STRING, false) != string::npos)
					{
						currentBlock.push_back("case");
						found_block = true;
					}
					else if (CUtil::FindKeyword(tmp, "configuration", 0, TO_END_OF_STRING, false) != string::npos)
					{
						currentBlock.push_back("configuration");
						found_block = true;
					}
					else if (CUtil::FindKeyword(tmp, "block", 0, TO_END_OF_STRING, false) != string::npos)
					{
						currentBlock.push_back("block");
						blockSignatureStartFound = false;
						found_block = true;
					}
					else if (CUtil::FindKeyword(tmp, "component", 0, TO_END_OF_STRING, false) != string::npos)
					{
						currentBlock.push_back("component");
						found_block = true;
					}
					else if (CUtil::FindKeyword(tmp, "process", 0, TO_END_OF_STRING, false) != string::npos)
					{
						currentBlock.push_back("process");
						processSignatureStartFound = false;
						found_block = true;
					}	
					else
						found_block = true;
				}
			}
			
			// check for end of a when statement within a case statement
			tempi = CUtil::FindKeyword(templine, "=>", 0, TO_END_OF_STRING, false);
			if ((tempi == templine.length() - 2) && currentBlock.size() != 0 && ((string) currentBlock.back()).compare("case") == 0)
			{
				strSize = CUtil::TruncateLine(tempi + 3 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
					found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
				start = templine.length() + 1;
				continue;
			}

			if (!found_type)
			{
				if (CUtil::FindKeyword(tmp, "type", 0, TO_END_OF_STRING, false) != string::npos)
					found_type = true;
			}
			if ((currentBlock.size() != 0) && ! (((string) currentBlock.back()).compare("port") == 0))
			{
				if (CUtil::FindKeyword(tmp, "port", 0, TO_END_OF_STRING, false) != string::npos)
				{
					if (currentBlock.back().compare("component") == 0)
					{
						strSize = CUtil::TruncateLine(1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
						if (strSize > 0)
						{
							strLSLOC += line.substr(start, strSize);
							strLSLOCBak += lineBak.substr(start, strSize);
						}
						FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
							found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
					}
					currentBlock.push_back("port");
				}
			}
			if ((currentBlock.size() != 0) && !  (((string) currentBlock.back()).compare("generic") == 0))
			{
				if (CUtil::FindKeyword(tmp, "generic", 0, TO_END_OF_STRING, false) != string::npos)
				{		
					if (currentBlock.back().compare("component") == 0)
					{
						strSize = CUtil::TruncateLine(1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
						if (strSize > 0)
						{
							strLSLOC += line.substr(start, strSize);
							strLSLOCBak += lineBak.substr(start, strSize);
						}
						FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
							found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
					}
					currentBlock.push_back("generic");
				}
			}
	
			if ((currentBlock.size() != 0) && !  (((string) currentBlock.back()).compare("component") == 0))
			{
				if (CUtil::FindKeyword(tmp, "component", 0, TO_END_OF_STRING, false) != string::npos)
					currentBlock.push_back("component");
			}

			// process 'select...end select;', 'accept ... end accept;'
			// 'record ... end record;' is handled via 'type'
			// select ... end select;  --> only one word statement 'select'
			// accept id... do ... end [id]; --> SLOC starting from 'accept' to 'do'
			// find 'do' only already found 'accept'
			if (!found_withSelect)
			{
				if (CUtil::FindKeyword(tmp, "with", 0, TO_END_OF_STRING, false) != string::npos)
				{
					currentBlock.push_back("with");
					found_withSelect = true;
				}
			}
			else if (CUtil::FindKeyword(tmp, "select", 0, TO_END_OF_STRING, false) != string::npos)
			{
				// found 'select' statement, one SLOC
				strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
				if (strSize > 0)
				{
					strLSLOC += line.substr(start, strSize);
					strLSLOCBak += lineBak.substr(start, strSize);
				}
				FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
					found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
				start = i + 1;
				continue;
			}
			if (currentBlock.size() != 0)
			{
				string currentString = ((string) currentBlock.back());
				if (!(currentString.compare("case") == 0) && !(currentString.compare("with") == 0) && !(currentString.compare("when") == 0 ) && 
					CUtil::FindKeyword(tmp, "when", 0, TO_END_OF_STRING, false) != string::npos)
				{
						currentBlock.push_back("when");
						found_whenConditional = true;
				}
				else if (found_whenConditional)
				{
					if (CUtil::FindKeyword(tmp, "else", 0, TO_END_OF_STRING, false) != string::npos)
					{
						strSize = CUtil::TruncateLine(i + 1 - start, strLSLOC.length(), this->lsloc_truncate, trunc_flag);
						if (strSize > 0)
						{
							strLSLOC += line.substr(start, strSize);
							strLSLOCBak += lineBak.substr(start, strSize);
						}
						FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
							found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
						start = i + 1;
						continue;
					}
				}
			}

			if (!found_unit)
			{
				if (CUtil::FindKeyword(tmp, "units", 0, TO_END_OF_STRING, false) != string::npos)
				{
					found_unit = true;
					currentBlock.push_back("units");
					FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
						found_end, found_type, found_is, found_unit, trunc_flag, currentBlock);
					continue;
				}
			}
			if (!found_record)
			{
				if (CUtil::FindKeyword(tmp, "record", 0, TO_END_OF_STRING, false) != string::npos)
				{
					found_record = true;
					currentBlock.push_back("record");
					FoundSLOC(result, strLSLOC, strLSLOCBak, found_block, found_forifwhile,
						found_end, found_type, found_is, found_unit, trunc_flag,currentBlock);
					continue;
				}
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

		// drop continuation symbol
		if (strLSLOC[strLSLOC.length()-1] == '\\')
		{
			strLSLOC = strLSLOC.substr(0, strLSLOC.length()-1);
			strLSLOCBak = strLSLOCBak.substr(0, strLSLOCBak.length()-1);
		}
	}
	if (tmp == "") 
		found_forifwhile = false;
}
