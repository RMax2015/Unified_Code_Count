//! UCC Global values, defines and structures
/*!
* \file UCCGlobals.h
*
* Added to UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_06
*
* This file helps to partition the UCC application to gracefully support
* multiple concurrent worker threads (not really so much here)
* looser coupling of functionality (in some ways)
*	violated class encapsulation to move some of these here
*		BUT
*	these declarations are needed by utility code that really does not
*	have to be embedded in a class (PrintUtil and MainObject for example)
*/

#ifndef _UCC_GLOBALS_H
#define _UCC_GLOBALS_H

#include	<string>
#include	<vector>

using namespace std;

#include	"cc_main.h"

// Define various Exceptions types.
// Use Defines instead of Enum so can be assigned to int type.
// See main.cpp for how these are used as overall UCC exit value.
// See UCCThread.cpp for exceptions in threads.
//
#define	EXCEPTION_DID_NOT_HAPPEN		0		// 0 for backwards compatability
#define	EXCEPTION_NOT_SPECIFIC			1		// 1 for backwards compatability
#define	EXCEPTION_UNDERFLOW_ERROR		(EXCEPTION_NOT_SPECIFIC+1)
#define EXCEPTION_OVERFLOW_ERROR		(EXCEPTION_NOT_SPECIFIC+2)
#define	EXCEPTION_RANGE_ERROR			(EXCEPTION_NOT_SPECIFIC+3)
#define	EXCEPTION_SYSTEM_ERROR			(EXCEPTION_NOT_SPECIFIC+4)
#define	EXCEPTION_RUNTIME_ERROR			(EXCEPTION_NOT_SPECIFIC+5)
#define EXCEPTION_DOMAIN_ERROR			(EXCEPTION_NOT_SPECIFIC+6)
#define	EXCEPTION_LENGTH_ERROR			(EXCEPTION_NOT_SPECIFIC+7)
#define	EXCEPTION_INVALID_ARG_ERROR		(EXCEPTION_NOT_SPECIFIC+8)
#define	EXCEPTION_OUT_OF_RANGE_ERROR	(EXCEPTION_NOT_SPECIFIC+9)
#define	EXCEPTION_LOGIC_ERROR			(EXCEPTION_NOT_SPECIFIC+10)
#define	EXCEPTION_STD_EXCEPTION			(EXCEPTION_NOT_SPECIFIC+11)

// Declare a global type and string to hold any Exception info.  Logging later perhaps?
extern	int		g_thread_exception_type;
extern	int		g_thread_exception_idx;		// Index of thread with Exception
extern	string	g_thread_exception_msg;

#define DEF_LANG_NAME		  "UNDEF"

#define UNCOUNTED_FILES		"outfile_uncounted_files.txt"
#define UNCOUNTED_FILES_CSV "outfile_uncounted_files.csv"


//! Structure to contain an Error message and what to do with it.
/*!
* \struct errMsgStruct
*
* Allow Error handling/presentation to be decoupled from Thread execution.
*/
struct errMsgStruct
{
	errMsgStruct(const string &Err, bool LogOnly = false, unsigned int PreNL = 0, unsigned int PostNL = 1, unsigned int ThreadIdx = 0)
	{
		threadIdx = ThreadIdx;
		err       = Err;
		logOnly   = LogOnly;
		preNL     = PreNL;
		postNL    = PostNL;
	}
	errMsgStruct()
	{
		// Set defaults almost as if called
		threadIdx = 0;
		err       = "";
		logOnly   = false;
		preNL     = 0;
		postNL    = 1;
	}
	unsigned int	threadIdx;		//!< Zero based index of the thread that found the error
									//!< 0 for main thread and also for 1st created worker thread
	string			err;
	bool			logOnly;
	unsigned int	preNL;			//!< Number of NewLines to insert Before the Error
	unsigned int	postNL;			//!< Number of NewLines to insert After  the Error
};

//! Vector containing Error messages and what to do with them.
/*!
* \typedef ErrMsgStructVector
*
* Defines Error messages that are processed later 
* from the main thread and not the thread that found the Error.
*/
typedef vector<errMsgStruct> ErrMsgStructVector;

//! Structure to contain info about an Uncounted File and what to do with it.
/*!
* \struct uncountedFileStruct
*
* Allow Uncounted File handling/presentation to be decoupled from Thread execution.
*/
struct uncountedFileStruct
{
	uncountedFileStruct(const string &Msg, const string &UncFile, const string &OutDir, bool UseListA = true, bool CsvOutput = false, unsigned int ThreadIdx = 0)
	{
		threadIdx = ThreadIdx;
		msg       = Msg;
		uncFile   = UncFile;
		useListA  = UseListA;
		csvOutput = CsvOutput;
		outDir    = OutDir;
	}
	uncountedFileStruct()
	{
		// Set defaults almost as if called
		threadIdx = 0;
		msg       = "";
		uncFile   = "";
		useListA  = true;
		csvOutput = false;
		outDir    = "";
	}
	unsigned int	threadIdx;		//!< Zero based index of the thread that found the Uncounted File
									//!< 0 for main thread and also for 1st created worker thread
	string			msg;
	string			uncFile;
	bool			useListA;
	bool			csvOutput;		//!< 
	string			outDir;
};

//! Vector containing information on Uncounted Files and what to do with them.
/*!
* \typedef UncountedFileStructVector
*
* Defines Uncounted Files info structs that are processed later 
* from the main thread and not the thread that found them.
*/
typedef vector<uncountedFileStruct> UncountedFileStructVector;


//
//	PUBLIC declarations of Global values which are changed from the MAIN Thread ONLY
//
extern	string		cmdLine;							//!< Executed command line string

extern	bool		isDiff;								//!< Has differencing been requested (-d)?

// Optimizations to do more processing per unit time
extern	bool	g_process_after_read;
extern	bool	g_discard_lines_after_process;

extern	bool print_cmplx;								//!< Print complexity and keyword counts
extern	bool print_csv;									//!< Print CSV report files
extern	bool print_ascii;								//!< Print ASCII text report files
extern	bool print_legacy;								//!< Print legacy formatted ASCII text report files
extern	bool print_unified;								//!< Print all counting files in a single unified file

extern	bool g_no_warnings_to_stdout;					//!< Suppress Warning messages to if true

extern	bool g_no_uncounted;							//!< Suppress any Uncounted files messages or reports

extern	unsigned int  workThreadsCount;					//!< Number of helper work threads to create. May be zero if not given.

extern	size_t lsloc_truncate;							//!< # of characters allowed in LSLOC for differencing (0=no truncation)

extern	string outDir;									//!< Output directory

extern	StringVector duplicateFilesInA1;				//!< List of duplicate file sources in baseline A
extern	StringVector duplicateFilesInA2;				//!< List of duplicate files in baseline A
extern	StringVector duplicateFilesInB1;				//!< List of duplicate file sources in baseline B
extern	StringVector duplicateFilesInB2;				//!< List of duplicate files in baseline B

extern	SourceFileList SourceFileA;						//!< List of source files in baseline A
extern	SourceFileList SourceFileB;						//!< List of source files in baseline B


#endif	// #ifndef _UCC_GLOBALS_H
