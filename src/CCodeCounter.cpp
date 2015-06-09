//! Common code counter methods for sub-classing individual languages.
/*!
* \file CCodeCounter.h
*
* This contains the base class code counter methods for inherited classes individual languages.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_06_06
*   Changes  ended  on 2015_06_06
*/

#include <time.h>

#include "CCodeCounter.h"
#include "UCCGlobals.h"

/*!
* Constructs a CCodeCounter object.
*/
CCodeCounter::CCodeCounter()
{
	print_cmplx = false;
	lsloc_truncate = DEFAULT_TRUNCATE;
	QuoteStart = "";
	QuoteEnd = "";
	QuoteEscapeFront = 0;
	QuoteEscapeRear = 0;
	ContinueLine = "";
	classtype = UNKNOWN;
	language_name = DEF_LANG_NAME;
	casesensitive = true;
	total_filesA = 0;
	total_filesB = 0;
	total_dupFilesA = 0;
	total_dupFilesB = 0;
}

/*!
* Destroys a CCodeCounter object.
*/
CCodeCounter::~CCodeCounter()
{
}

/*!
* Initializes the count vectors.
* This method removes the existing content of the vectors and assigns them all zeros
*/
void CCodeCounter::InitializeCounts()
{
	unsigned int i = 0;
	counted_files = 0;
	counted_dupFiles = 0;

	directive_count.assign(directive.size(), make_pair(i, i));
	data_name_count.assign(data_name_list.size(), make_pair(i, i));
	exec_name_count.assign(exec_name_list.size(), make_pair(i, i));
	math_func_count.assign(math_func_list.size(), make_pair(i, i));
	trig_func_count.assign(trig_func_list.size(), make_pair(i, i));
	log_func_count.assign(log_func_list.size(), make_pair(i, i));

	cmplx_calc_count.assign(cmplx_calc_list.size(), make_pair(i, i));
	cmplx_cond_count.assign(cmplx_cond_list.size(), make_pair(i, i));
	cmplx_logic_count.assign(cmplx_logic_list.size(), make_pair(i, i));
	cmplx_preproc_count.assign(cmplx_preproc_list.size(), make_pair(i, i));
	cmplx_assign_count.assign(cmplx_assign_list.size(), make_pair(i, i));
	cmplx_pointer_count.assign(cmplx_pointer_list.size(), make_pair(i, i));
}

/*!
* Initializes the count vectors for a result.
* This method removes the existing content of the vectors and assigns them all zeros
*/
void CCodeCounter::InitializeResultsCounts(results* result)
{
	result->directive_count.assign(directive.size(), 0);
	result->data_name_count.assign(data_name_list.size(), 0);
	result->exec_name_count.assign(exec_name_list.size(), 0);
	result->math_func_count.assign(math_func_list.size(), 0);
	result->trig_func_count.assign(trig_func_list.size(), 0);
	result->log_func_count.assign(log_func_list.size(), 0);

	result->cmplx_calc_count.assign(cmplx_calc_list.size(), 0);
	result->cmplx_cond_count.assign(cmplx_cond_list.size(), 0);
	result->cmplx_logic_count.assign(cmplx_logic_list.size(), 0);
	result->cmplx_preproc_count.assign(cmplx_preproc_list.size(), 0);
	result->cmplx_assign_count.assign(cmplx_assign_list.size(), 0);
	result->cmplx_pointer_count.assign(cmplx_pointer_list.size(), 0);
}

/*!
* Processes and counts the source file.
*
* \param fmap list of file lines
* \param result counter results
*
* \return method status
*/
int CCodeCounter::CountSLOC(filemap* fmap, results* result)
{
	// backup file content before modifying it (comments and directive lines are cleared)
	// fmapBak is same as fmap except that it stores unmodified quoted strings
	// fmap has quoted strings replaced with '$'
	filemap fmapMod = *fmap;
	filemap fmapModBak = *fmap;
	
	InitializeResultsCounts(result);

	PreCountProcess(&fmapMod);

	CountBlankSLOC(&fmapMod, result);

	CountCommentsSLOC(&fmapMod, result, &fmapModBak);

	if (print_cmplx)
		CountComplexity(&fmapMod, result);

	CountDirectiveSLOC(&fmapMod, result, &fmapModBak);

	LanguageSpecificProcess(&fmapMod, result, &fmapModBak);

	return 0;
}

/*!
* Checks whether the file extension is supported by the language counter.
*
* \param file_name file name
*
* \return whether file extension is supported
*/
bool CCodeCounter::IsSupportedFileExtension(const string &file_name)
{
	// if Makefile, check whether name equals MAKEFILE since no extension exists
	if (classtype == MAKEFILE && file_name.size() >= 8)
	{
		if (CUtil::ToLower(file_name.substr(file_name.size() - 8)) == "makefile")
			return true;
	}
	size_t idx = file_name.find_last_of(".");
	if (idx == string::npos)
		return false;
	string file_ext = file_name.substr(idx);
	file_ext = CUtil::ToLower(file_ext);
	if (find(file_extension.begin(), file_extension.end(), file_ext) != file_extension.end())
	{
		// if X-Midas/NeXtMidas, parse file to check for startmacro or endmacro (needed since Midas can use .txt or .mm)
		if (classtype == XMIDAS || classtype == NEXTMIDAS)
		{
			string oneline;
			ifstream fr(file_name.c_str(), ios::in);
			if (!fr.is_open())
				return false;

			// search for "startmacro" (optional) or "endmacro" (required)
			while (fr.good() || fr.eof())
			{
				getline(fr, oneline);
				if ((!fr.good() && !fr.eof()) || (fr.eof() && oneline.empty()))
					break;
				oneline = CUtil::ToLower(CUtil::TrimString(oneline));
				if (oneline.compare(0, 10, "startmacro") == 0 || oneline.compare(0, 8, "endmacro") == 0)
				{
					fr.clear();
					fr.close();
					return true;
				}
				if (!fr.good())
					break;
			}
			fr.clear();
			fr.close();
		}
		else
			return true;
	}
	return false;
}

/*!
* Retrieves the language output file stream.
* Opens a new stream if it has not been opened already.
*
* \param outputFileNamePrePend name to prepend to the output file
* \param cmd current command line string
* \param csvOutput CSV file stream? (otherwise ASCII text file)
* \param legacyOutput legacy format file stream? (otherwise standard text file)
*
* \return output file stream
*/
ofstream* CCodeCounter::GetOutputStream(const string &outputFileNamePrePend, const string &cmd, bool csvOutput, bool legacyOutput)
{
	if (csvOutput)
	{
		if (!output_file_csv.is_open())
		{
			string fname = outputFileNamePrePend + language_name + OUTPUT_FILE_NAME_CSV;
			output_file_csv.open(fname.c_str(), ofstream::out);

			if (!output_file_csv.is_open()) return NULL;

			CUtil::PrintFileHeader(output_file_csv, "SLOC COUNT RESULTS", cmd);

			CUtil::PrintFileHeaderLine(output_file_csv, "RESULTS FOR " + language_name + " FILES");
			output_file_csv << endl;
			output_file_csv << "Total,Blank,Comments,,Compiler,Data,Exec.,Logical,Physical,File,Module" << endl;
			output_file_csv << "Lines,Lines,Whole,Embedded,Direct.,Decl.,Instr.,SLOC,SLOC,Type,Name" << endl;
		}
		return &output_file_csv;
	}
	else
	{
		if (!output_file.is_open())
		{
			string fname = outputFileNamePrePend + language_name + OUTPUT_FILE_NAME;
			output_file.open(fname.c_str(), ofstream::out);

			if (!output_file.is_open()) return NULL;

			CUtil::PrintFileHeader(output_file, "SLOC COUNT RESULTS", cmd);

			CUtil::PrintFileHeaderLine(output_file, "RESULTS FOR " + language_name + " FILES");
			output_file << endl;
			if (legacyOutput)
			{
				output_file << "   Total   Blank |      Comments    | Compiler  Data   Exec.  | Logical | File  Module" << endl;
				output_file << "   Lines   Lines |   Whole Embedded | Direct.   Decl.  Instr. |   SLOC  | Type  Name" << endl;
				output_file << "-----------------+------------------+-------------------------+---------+---------------------------" << endl;
			}
			else
			{
				output_file << "   Total   Blank |      Comments    | Compiler  Data   Exec.  | Logical Physical | File  Module" << endl;
				output_file << "   Lines   Lines |   Whole Embedded | Direct.   Decl.  Instr. |   SLOC    SLOC   | Type  Name" << endl;
				output_file << "-----------------+------------------+-------------------------+------------------+---------------------------" << endl;
			}
		}
		return &output_file;
	}
}

/*!
* Closes the language output file stream.
*/
void CCodeCounter::CloseOutputStream()
{
	if (output_file.is_open())
		output_file.close();
	if (output_file_csv.is_open())
		output_file_csv.close();
}

/*!
* Finds the first index of one of the characters of strQuote in strline.
*
* \param strline string line
* \param strQuote string of character(s) to find in strline
* \param idx_start index of line character to start search
* \param QuoteEscapeFront quote escape character
*
* \return index of strQuote character in strline
*/
size_t CCodeCounter::FindQuote(string const &strline, string const &strQuote, size_t idx_start, char QuoteEscapeFront)
{
	size_t min_idx, idx;
	min_idx = strline.length();
	for (size_t i = 0; i < strQuote.length(); i++)
	{
		idx = CUtil::FindCharAvoidEscape(strline, strQuote[i], idx_start, QuoteEscapeFront);
		if (idx != string::npos && idx < min_idx)
			min_idx = idx;
	}
	if (min_idx < strline.length())
		return min_idx;
	return string::npos;
}

/*!
* Replaces up to ONE quoted string inside a string starting at idx_start.
*
* \param strline string to be processed
* \param idx_start index of line character to start search
* \param contd specifies the quote string is continued from the previous line
* \param CurrentQuoteEnd end quote character of the current status
*
* \return method status
*/
int CCodeCounter::ReplaceQuote(string &strline, size_t &idx_start, bool &contd, char &CurrentQuoteEnd)
{
	size_t idx_end, idx_quote;

	if (contd)
	{
		idx_start = 0;
		if (strline[0] == CurrentQuoteEnd)
		{
			idx_start = 1;
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
		}
		else
		{
			idx_start = strline.length();
			return 0;
		}
	}

	idx_end = CUtil::FindCharAvoidEscape(strline, CurrentQuoteEnd, idx_start + 1, QuoteEscapeFront);
	if (idx_end == string::npos)
	{
		idx_end = strline.length() - 1;
		strline.replace(idx_start + 1, idx_end - idx_start, idx_end - idx_start, '$');
		contd = true;
		idx_start = idx_end + 1;
	}
	else
	{
		if ((QuoteEscapeRear) && (strline.length() > idx_end + 1) && (strline[idx_end+1] == QuoteEscapeRear))
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
* Counts blank lines in a file.
*
* \param fmap list of file lines
* \param result counter results
*
* \return method status
*/
int CCodeCounter::CountBlankSLOC(filemap* fmap, results* result)
{
	for (filemap::iterator i = fmap->begin(); i != fmap->end(); i++)
	{
		if (CUtil::CheckBlank(i->line))
			result->blank_lines++;
	}
	return 1;
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
int CCodeCounter::CountCommentsSLOC(filemap* fmap, results* result, filemap *fmapBak)
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
* Finds a starting position of a comment in a string starting at idx_start.
*
* \param strline string to be processed
* \param idx_start index of line character to start search
* \param comment_type comment type (0=not a comment, 1=whole line, 2=embedded line, 3=whole line block, 4=embedded block)
* \param curBlckCmtStart current block comment start string
* \param curBlckCmtEnd current block comment end string
*
* \return method status
*/
int CCodeCounter::FindCommentStart(string strline, size_t &idx_start, int &comment_type,
								   string &curBlckCmtStart, string &curBlckCmtEnd) 
{
	size_t idx_line, idx_tmp, idx_block;
	string line = strline;
	comment_type = 0;

	if (!casesensitive)
		line = CUtil::ToLower(line);

	// searching for starting of line comment
	idx_line = string::npos;
	for (StringVector::iterator i = LineCommentStart.begin(); i != LineCommentStart.end(); i++)
	{
		idx_tmp = line.find((casesensitive ? (*i) : CUtil::ToLower(*i)), idx_start);
		if (idx_tmp < idx_line) idx_line = idx_tmp;
	}

	// searching for starting of block comment
	idx_block = string::npos;
	for (StringVector::iterator i = BlockCommentStart.begin(); i != BlockCommentStart.end(); i++)
	{
		idx_tmp = strline.find(*i, idx_start);
		if (idx_tmp < idx_block)
		{
			idx_block = idx_tmp;
			curBlckCmtStart = *i;
			curBlckCmtEnd = *(BlockCommentEnd.begin() + (i - BlockCommentStart.begin()));
		}
	}

	// see what kind of comment appears first
	if (idx_line == string::npos && idx_block == string::npos)
	{
		comment_type = 0;
		idx_start = idx_line;
	}
	else if (idx_block > idx_line)
	{
		idx_start = idx_line;
		comment_type = idx_start == 0 ? 1 : 2;
	}
	else
	{
		idx_start = idx_block;
		comment_type = idx_start == 0 ? 3 : 4;
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
int CCodeCounter::CountComplexity(filemap* fmap, results* result)
{
	if (classtype == UNKNOWN || classtype == DATAFILE)
		return 0;
	filemap::iterator fit;
	size_t idx;
	unsigned int cnt, ret, cyclomatic_cnt = 0, ignore_cyclomatic_cnt = 0, main_cyclomatic_cnt = 0, function_count = 0;
	string line, lastline, file_ext, function_name = "";
	string exclude = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_$";
	filemap function_stack;
	stack<unsigned int> cyclomatic_stack;
	map<unsigned int, lineElement> function_map;
	bool process_cyclomatic_complexity = false;

	// check whether to process cyclomatic complexity
	if (cmplx_cyclomatic_list.size() > 0)
	{
		process_cyclomatic_complexity = true;
		if (skip_cmplx_cyclomatic_file_extension_list.size() > 0)
		{
			idx = result->file_name.find_last_of(".");
			if (idx != string::npos)
			{
				file_ext = result->file_name.substr(idx);
				file_ext = CUtil::ToLower(file_ext);
				if (find(skip_cmplx_cyclomatic_file_extension_list.begin(), skip_cmplx_cyclomatic_file_extension_list.end(), file_ext) != skip_cmplx_cyclomatic_file_extension_list.end())
					process_cyclomatic_complexity = false;
			}
		}
	}

	// process each line
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
		CUtil::CountTally(line, cmplx_logic_list, cnt, 1, exclude, "", "", &result->cmplx_logic_count, casesensitive);
		result->cmplx_logic_lines += cnt;

		// preprocessor directives
		cnt = 0;
		CUtil::CountTally(line, cmplx_preproc_list, cnt, 1, exclude, "", "", &result->cmplx_preproc_count, casesensitive);
		result->cmplx_preproc_lines += cnt;

		// assignments
		cnt = 0;
		CUtil::CountTally(line, cmplx_assign_list, cnt, 1, exclude, "", "", &result->cmplx_assign_count, casesensitive);
		result->cmplx_assign_lines += cnt;

		// pointers
		cnt = 0;
		CUtil::CountTally(line, cmplx_pointer_list, cnt, 1, exclude, "", "", &result->cmplx_pointer_count, casesensitive);
		result->cmplx_pointer_lines += cnt;

		// cyclomatic complexity
		if (process_cyclomatic_complexity)
		{
			// search for cyclomatic complexity keywords
			CUtil::CountTally(line, cmplx_cyclomatic_list, cyclomatic_cnt, 1, exclude, "", "", 0, casesensitive);

			// search for keywords to exclude
			if (ignore_cmplx_cyclomatic_list.size() > 0)
				CUtil::CountTally(line, ignore_cmplx_cyclomatic_list, ignore_cyclomatic_cnt, 1, exclude, "", "", 0, casesensitive);

			// parse function name if found
			ret = ParseFunctionName(line, lastline, function_stack, function_name, function_count);
			if (ret != 1 && !cyclomatic_stack.empty() && cyclomatic_stack.size() == function_stack.size())
			{
				// remove count stack entry for non-function names
				cyclomatic_cnt += cyclomatic_stack.top();
				ignore_cyclomatic_cnt = 0;
				cyclomatic_stack.pop();
			}
			if (ret == 1)
			{
				// capture count at end of function
				lineElement element(cyclomatic_cnt - ignore_cyclomatic_cnt + 1, function_name);
				function_map[function_count] = element;

				if (!function_stack.empty())
				{
					// grab previous function from stack to continue
					if (!cyclomatic_stack.empty())
					{
						cyclomatic_cnt = cyclomatic_stack.top();
						cyclomatic_stack.pop();
					}
				}
				else
					cyclomatic_cnt = 0;
				function_name = "";
				ignore_cyclomatic_cnt = 0;
			}
			else if (ret == 2)
			{
				// some code doesn't belong to any function
				main_cyclomatic_cnt += cyclomatic_cnt - ignore_cyclomatic_cnt;
				if (main_cyclomatic_cnt < 1)
					main_cyclomatic_cnt = 1;	// add 1 for main function here in case no other decision points are found in main
				cyclomatic_cnt = ignore_cyclomatic_cnt = 0;
			}
			else if (!function_stack.empty() && (function_stack.size() > cyclomatic_stack.size() + 1 || (cyclomatic_stack.empty() && function_stack.size() > 1)))
			{
				// capture previous complexity count from open function
				cyclomatic_stack.push(cyclomatic_cnt - ignore_cyclomatic_cnt);
				cyclomatic_cnt = ignore_cyclomatic_cnt = 0;
			}
		}
	}

	// done with a file
	if (main_cyclomatic_cnt > 0)
	{
		// add "main" code
		lineElement element(main_cyclomatic_cnt, "main");
		function_map[0] = element;
	}
	else
	{
		// finish the first function if not closed
		while (!function_stack.empty())
		{
			function_name = function_stack.back().line;
			function_count = function_stack.back().lineNumber;
			function_stack.pop_back();

			if (!function_stack.empty())
			{
				// grab previous function from stack to continue
				if (!cyclomatic_stack.empty())
				{
					cyclomatic_cnt = cyclomatic_stack.top();
					cyclomatic_stack.pop();
				}
			}
			else
			{
				// capture count at end of function
				lineElement element(cyclomatic_cnt + 1, function_name);
				function_map[0] = element;
			}
		}
	}

	// process ordered functions
	for (map<unsigned int, lineElement>::iterator it = function_map.begin(); it != function_map.end(); ++it)
		result->cmplx_cycfunct_count.push_back(it->second);

	return 1;
}

/*!
* Processes physical and logical lines.
* This method is typically implemented in the specific language sub-class.
*
* \param fmap list of processed file lines
* \param result counter results
* \param fmapBak list of original file lines (same as fmap except it contains unmodified quoted strings)
*
* \return method status
*/
int CCodeCounter::LanguageSpecificProcess(filemap* fmap, results* result, filemap* /*fmapBak*/)
{
	for (filemap::iterator iter = fmap->begin(); iter != fmap->end(); iter++)
	{
		if (!CUtil::CheckBlank(iter->line))
			result->exec_lines[PHY]++;
	}
	return 1;
}
