//! UCC Global values and structures
/*!
* \file UCCGlobals.cpp
*
* Changed from UCC 2013_04 release by Randy Maxwell
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

#include <string>

using namespace std;


#include	"cc_main.h"
#include	"UserIF.h"

//
//	Declare single instance of Global variables accessed elsewhere
//

//
//	These are changed from the MAIN Thread ONLY
//

string		cmdLine;						//!< Executed command line string

bool		isDiff = false;					//!< Has differencing been requested (-d)?

// Optimizations to do more processing per unit time
bool	g_process_after_read = false;
bool	g_discard_lines_after_process = false;

bool print_cmplx;								//!< Print complexity and keyword counts
bool print_csv;									//!< Print CSV report files
bool print_ascii;								//!< Print ASCII text report files
bool print_legacy;								//!< Print legacy formatted ASCII text report files
bool print_unified;								//!< Print all counting files in a single unified file

bool g_no_warnings_to_stdout = false;				//!< Suppress Warning messages to console if true

bool g_no_uncounted = false;						//!< Suppress any Uncounted files messages or reports

unsigned int  workThreadsCount = 0;				//!< Number of helper work threads to create. May be zero if not given.

size_t lsloc_truncate;							//!< # of characters allowed in LSLOC for differencing (0=no truncation)

string outDir;									//!< Output directory

StringVector duplicateFilesInA1;				//!< List of duplicate file sources in baseline A
StringVector duplicateFilesInA2;				//!< List of duplicate files in baseline A
StringVector duplicateFilesInB1;				//!< List of duplicate file sources in baseline B
StringVector duplicateFilesInB2;				//!< List of duplicate files in baseline B

SourceFileList SourceFileA;						//!< List of source files in baseline A
SourceFileList SourceFileB;						//!< List of source files in baseline B

UserIF *userIF;									//!< User interface for presenting messages/progress to user