//! Common code counter class for sub-classing individual languages.
/*!
* \file CCodeCounter.h
*
* This file contains the common code counter class for sub-classing individual languages.
*/

#ifndef CCodeCounter_h
#define CCodeCounter_h

#include "cc_main.h"
#include "CUtil.h"

//! Common code counter class.
/*!
* \class CCodeCounter
*
* Defines the common code counter class.
*/
class CCodeCounter
{
public:
	CCodeCounter();
	virtual ~CCodeCounter();
	virtual void InitializeCounts();
	virtual int CountSLOC(filemap* fmap, results* result);
	bool IsSupportedFileExtension(const string &file_name);
	virtual ofstream* GetOutputStream(const string &outputFileNamePrePend = "",
		const string &cmd = "", bool csvOutput = false, bool legacyOutput = false);
	virtual void CloseOutputStream();

	StringVector  directive;				//!< Directive statement keywords
	StringVector  data_name_list;			//!< Data statement keywords
	StringVector  exec_name_list;			//!< Executable statement keywords
	StringVector  file_extension;			//!< File extension

	StringVector  math_func_list;			//!< Math functions
	StringVector  trig_func_list;			//!< Trigonometric functions
	StringVector  log_func_list;			//!< Logarithmic functions
	StringVector  cmplx_calc_list;			//!< Calculations (complexity)
	StringVector  cmplx_cond_list;			//!< Conditionals (complexity)
	StringVector  cmplx_logic_list;			//!< Logicals (complexity)
	StringVector  cmplx_preproc_list;		//!< Preprocessor directives (complexity)
	StringVector  cmplx_assign_list;		//!< Assignments (complexity)
	StringVector  cmplx_pointer_list;		//!< Pointers (complexity)
	StringVector  cmplx_cyclomatic_list;	//!< Cyclomatic complexity decision keywords (complexity)
	StringVector  ignore_cmplx_cyclomatic_list;	//!< Cyclomatic complexity decision keywords to ignore (for example End If)
	StringVector  skip_cmplx_cyclomatic_file_extension_list;	//!< Cyclomatic complexity file extensions to skip

	UIntPairVector directive_count;			//!< Count of each directive statement keyword
	UIntPairVector data_name_count;			//!< Count of each data statement keyword
	UIntPairVector exec_name_count;			//!< Count of each executable statement keyword

	UIntPairVector math_func_count;			//!< Count of math functions
	UIntPairVector trig_func_count;			//!< Count of trigonometric functions
	UIntPairVector log_func_count;			//!< Count of logarithmic functions
	UIntPairVector cmplx_calc_count;		//!< Count of calculations
	UIntPairVector cmplx_cond_count;		//!< Count of conditionals
	UIntPairVector cmplx_logic_count;		//!< Count of logicals
	UIntPairVector cmplx_preproc_count;		//!< Count of preprocessor directives
	UIntPairVector cmplx_assign_count;		//!< Count of assignments
	UIntPairVector cmplx_pointer_count;		//!< Count of pointers

	bool          print_cmplx;				//!< Print complexity and keyword counts
	size_t        lsloc_truncate;			//!< # of characters allowed in LSLOC for differencing (0=no truncation)
	string        language_name;			//!< Counter language name
	ClassType	  classtype;				//!< Language class type
	unsigned int  counted_files;			//!< Number of files counted
	unsigned int  counted_dupFiles;			//!< Number of duplicate files counted
	unsigned int  total_filesA;				//!< Total number of files in baseline A
	unsigned int  total_filesB;				//!< Total number of duplicate files in baseline B
	unsigned int  total_dupFilesA;			//!< Total number of files in baseline A
	unsigned int  total_dupFilesB;			//!< Total number of duplicate files in baseline B

protected:
	virtual void InitializeResultsCounts(results* result);
	static size_t FindQuote(string const &strline, string const &QuoteStart, size_t idx_start, char QuoteEscapeFront);
	virtual int ReplaceQuote(string &strline, size_t &idx_start, bool &contd, char &CurrentQuoteEnd);
	virtual int PreCountProcess(filemap* /*fmap*/) { return 0; }
	int CountBlankSLOC(filemap* fmap, results* result);
	virtual int CountCommentsSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	int FindCommentStart(string strline, size_t &idx_start, int &comment_type,
		string &curBlckCmtStart, string &curBlckCmtEnd);
	virtual int CountComplexity(filemap* fmap, results* result);
	virtual int CountDirectiveSLOC(filemap* /*fmap*/, results* /*result*/, filemap* /*fmapBak = NULL*/) { return 0; }
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int ParseFunctionName(const string & /*line*/, string & /*lastline*/,
		filemap & /*functionStack*/, string & /*functionName*/, unsigned int & /*functionCount*/) { return 0; }

	StringVector  exclude_keywords;			//!< List of keywords to exclude from counts

	// if language supports multiple quote marks such as javascript, you can put all of them here, ex. "\"'"
	string QuoteStart;						//!< Starting quotation mark(s)
	string QuoteEnd;						//!< Ending quotation mark(s)

	char QuoteEscapeFront;					//!< Escape character for front quote (ex. '\' in C++)
	char QuoteEscapeRear;					//!< Escape character for rear quote
	string ContinueLine;					//!< Line continuation character(s) (ex. \\ in C++)
	StringVector BlockCommentStart;			//!< Block comment start character(s) (ex. /* in C++)
	StringVector BlockCommentEnd;			//!< Block comment end character(s) (ex. */ in C++)
	StringVector LineCommentStart;			//!< Single line or embedded comment character(s)

	bool casesensitive;						//!< Is language is case sensitive?

	ofstream output_file;					//!< Output file stream
	ofstream output_file_csv;				//!< Output CSV file stream
};

#endif
