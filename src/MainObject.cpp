//! Main counting object class methods.
/*!
* \file MainObject.cpp
*
* This file contains the main counting object class methods.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_06
*   Addition of Multithreading performance enhancement 
*		Example: UCC -threads 4 (and other UCC args)
*   Refactored Duplication checking to be much faster
*   Refactoring to move code to other source files to decrease SIZE/complexity (~7,500 PSLOC)
*   Cleanup of MS Visual Studion 2010 Express edition Warning level 4 warnings
*   Added comments at the end of this file: 
*		about some high level details of the Multithreading change
*		about some next steps for better performance
*		about existing refactoring for easier changes going forward
*/
#include <sstream>
#include <time.h>

#include "MainObject.h"
#include "CUtil.h"
#include "CmpMngr.h"
#include "PrintUtil.h"

extern	UserIF *	userIF;		//!< User interface for presenting messages/progress to user

// Fill in some variables used to get timing of various steps
extern	time_t	time_end_list_built;
extern	time_t	time_end_files_read;
extern	time_t	time_end_files_analysis;
extern	time_t	time_end_find_duplicates;
extern	time_t	time_end_print_results;

/*!
* Constructs a MainObject object.
*/
MainObject::MainObject()
{
	userIF = NULL;
	counter = 0;

	isDiff = false;				// default not differencing files
	outDir = "";				// default output directory is current directory
	duplicate_threshold = 0;	// default duplicate files threshold
	lsloc_truncate = DEFAULT_TRUNCATE;	// default LSLOC maximum characters for truncation (0=no truncation)
	print_cmplx = true;			// default print complexity and keyword counts
	print_csv = true;			// default print CSV report files
	print_ascii = false;		// default don't print ASCII text report files
	print_legacy = false;		// default don't print legacy ASCII text report files
	print_unified = false;		// default don't print all counting files to a single unified file
	clearCaseFile = false;		// default don't process Clear Case file names
	followSymLinks = true;		// default follow symbolic links
	workThreadsCount = 0;		// default is no worker threads and no Experimental Optimization
	use_CommandLine = false;	// default read all files from fileList file

	int init_result = Init_CounterForEachLanguage( CounterForEachLanguage );
	if ( init_result )
	{
		// Handle error as best as possible.
		// Realistically an error here is VERY unlikely and the previous version did the above here as well.
		// Because this is called from within a class constructor there is no return value allowed.
		cout << endl << "Error: MainObject constructor unable to init CounterForEachLanguage" << endl;
		throw std::runtime_error("Error: MainObject constructor unable to init CounterForEachLanguage");
	}
	
	// initialize the count lists
	ResetCounterCounts( CounterForEachLanguage );
}

/*!
* Destroys a MainObject object.
*/
MainObject::~MainObject()
{
	// First stop and exit Worker threads in case they were updating any shared data structures
	if ( workThreadsCount > 1 )
	{
		FinishWorkerThreads();
		workThreadsCount = 0;
	}
	for (map<int, CCodeCounter*>::iterator iter = CounterForEachLanguage.begin(); iter != CounterForEachLanguage.end(); iter++)
	{
		delete iter->second;
		iter->second = 0;
	}
	if (userIF != NULL)
		delete(userIF);
}

/*!
* Must have a single point to check for User Cancelling so threads can be stopped if needed.
*/
bool MainObject::HasUserCancelled()
{
	bool	has_cancelled = false;

	if ( userIF != NULL )
	{
		has_cancelled = userIF->isCanceled();
		if ( has_cancelled )
		{
			if ( workThreadsCount > 1 )
			{
				// Perhaps use new way to stop threads and not destroy them
				FinishWorkerThreads();
				workThreadsCount = 0;
			}
		}
	}
	return has_cancelled;
}


// Helper to support reporting times of various steps
double MainObject::GetDuplicateThreshold()
{
	return duplicate_threshold;
}


/*!
* Main counting process.
*
* \param argc number of arguments
* \param argv argument list
*
* \return function status
*/
int MainObject::MainProcess(int argc, char *argv[])
{
	/*
		Counting Process:
		1. Parse options
		2. -threads 1 = Enable Optimizations (default is 0 for backwards compatability)
		   -threads 2 (or more) = Start worker Threads with Optimizations
		3. Find files to work on
		4. Read input files found into memory
		5. Run counter on baseline files
		6. Identify duplicate files
		7. Stop worker threads if started
		8. Print all results data

		Notes: Steps 3 to 6 may be implemented using worker threads
				IF 
		using multiple threads
				THEN 
		it makes more sense to go ahead and read the file contents
		after each valid input file is found.  See UCCThread.cpp
			First try: 
		Call a worker thread per file to Read
		2 extra worker threads were much slower on my 2 CPU Windows 7.1 laptop.
		More than 2 extra worker threads were even slower.
			Second try: 
		Divide the whole List of files to Read among 2 or more threads
		This was also slower than single threaded file reading.
		The code to use threads will compile/link/but run slower (see ReadAllFiles)
		and ... I have written the new thread code in a "False Sharing" way.
		Made changes to remove as much "False Sharing" as possible.
		Now 2 Threads Reads 36,337 files Linux sources about 11% faster.
		Overall speedup is 3 % on my HW/OS -- includes Duplicate checking step.
		Having more than 2 Threads on my HW does NOT speed up Reading Files.
		 
			Third try:
		Have threads work on processing after the files have been Read into memory
		Analysis and counting
			ProcessSourceList
		Duplicate detection ? Not sure how to partition this yet
		
	*/

	// create a user interface object
	if (userIF == NULL)
		userIF = new UserIF();

	// parse the command line input
	if (!ParseCommandLine(argc, argv))
		ShowUsage();		// This will EXIT

	SetCounterOptions( CounterForEachLanguage );

	// handle input file list
	vector<string> listFileNames;
	BaselineFileName1 = INPUT_FILE_LIST_NAME;
#ifdef QTGUI
	if (outDir != "")
		BaselineFileName1 = outDir + INPUT_FILE_LIST_NAME;
#endif
	if (HasUserCancelled())
		return 0;

	// Start worker threads or Enable Optimizations if wanted
	string	start_threads_result_msg;
	StartThreads( start_threads_result_msg );
	if ( start_threads_result_msg.size() )
		userIF->updateProgress( start_threads_result_msg, false );

	if ( g_process_after_read )
		userIF->updateProgress("\tOptimized processing in use ...");

	// generate user-defined language extension map
	if (userExtMapFile.length() != 0)
		ReadUserExtMapping(userExtMapFile);

	// Make a list of input source files
	userIF->updateProgress("Building list of source files...\n", false);
	if (HasUserCancelled())
		return 0;

	if (listFilesToBeSearched.size() != 0)
		CUtil::ListAllFiles(dirnameA, listFilesToBeSearched, listFileNames, followSymLinks);

	time( &time_end_list_built );

	// Read the source files, now shows % complete as it runs
	if ( g_process_after_read )
		userIF->updateProgress("Read, Analyze & Count keywords in files......", false);
	else
		userIF->updateProgress("Reading source files.........................", false);
	if (HasUserCancelled())
		return 0;

	if (!ReadAllFiles(listFileNames, BaselineFileName1))
		return 0;

	time( &time_end_files_read );

	if ( ! g_process_after_read )
	{
		// Analyze and Count keywords in files
		userIF->updateProgress("Performing files analysis and counting.......", false);
		ProcessSourceList();
		if (HasUserCancelled())
			return 0;
	}

	if (duplicate_threshold >= 0.0)
	{
		time( &time_end_files_analysis );

		// This will show % done
		userIF->updateProgress("Looking for duplicate files..................", false);

		FindDuplicateFiles(SourceFileA, duplicateFilesInA1, duplicateFilesInA2);
		if (HasUserCancelled())
			return 0;
		
		// Include this small time as part of Duplicate checks	 0 to 1 second (rounded)
		UpdateCounterCounts( CounterForEachLanguage, &SourceFileA, true, false );

		time( &time_end_find_duplicates );
	}
	else
	{
		// Include this small time as part of Analysis	0 to 1 second (rounded)
		UpdateCounterCounts( CounterForEachLanguage, &SourceFileA, true, false );

		time( &time_end_files_analysis );
	}

	// print the counting results (PSLOC & LSLOC counts and complexity counts)
	userIF->updateProgress("Generating results to files........", false);
	if (HasUserCancelled())
		return 0;
	if (print_unified)
		PrintTotalCountResults( CounterForEachLanguage, true, "", &duplicateFilesInA2);
	else
		PrintCountResults( CounterForEachLanguage, true, "", &duplicateFilesInA2);

	// Show a change to keep the User happy
	userIF->updateProgress("......", false);

	if (print_cmplx)
		PrintComplexityResults( CounterForEachLanguage );

	if (duplicate_threshold >= 0.0)
	{
		if (print_unified)
			PrintTotalCountResults( CounterForEachLanguage, true, "Duplicates-", &duplicateFilesInA2, false);
		else
			PrintCountResults( CounterForEachLanguage, true, "Duplicates-", &duplicateFilesInA2, false);
		if (print_cmplx)
			PrintComplexityResults( CounterForEachLanguage, true, "Duplicates-", true);
		PrintDuplicateSummary();
	}
#ifndef QTGUI
	userIF->updateProgress("DONE");
#endif

	time( &time_end_print_results );

	return 1;
}

/*!
* Parses the command line arguments.
*
* \param argc number of arguments
* \param argv argument list
*
* \return method status: 1 = parsed OK else 0
*/
int MainObject::ParseCommandLine(int argc, char *argv[])
{
	int i;
	string arg, cmdStr;

	// capture command line and strip executable path
	arg = argv[0];
	cmdStr = CUtil::ExtractFilename(arg);
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];
		if (arg.find(' ') != string::npos)	// quote strings containing spaces
			cmdStr += " \"" + arg + "\"";
		else
			cmdStr += " " + arg;
	}
	cmdLine = cmdStr;

	// get the output directory first for error logging
	for (i = 1; i < argc; i++)
	{
		arg = argv[i];

		if (arg == "-outdir")
		{
			// extract the output directory name
			if (i + 1 > argc)
			{
				string err = "Error: Unable to parse command line args";
				userIF->SetErrorFile("");
				userIF->AddError(err);
				return 0;
			}
			i++;
			outDir = argv[i];

			// attempt to create the specified directory (if exists, this will do nothing)
#ifdef UNIX
			// trim trailing '/' character
			if (outDir.length() > 1 && outDir[outDir.length()-1] == '/')
				outDir = outDir.substr(0, outDir.length() - 1);
#else
			// trim trailing '\' or '/' character
			if (outDir.length() > 1 && (outDir[outDir.length()-1] == '\\' || outDir[outDir.length()-1] == '/'))
				outDir = outDir.substr(0, outDir.length() - 1);
#endif
			if (CUtil::MkPath(outDir) == 0)
			{
				string err = "Error: Unable to create output directory (";
				err += outDir;
				err += ")";
				userIF->SetErrorFile("");
				userIF->AddError(err);
				return 0;
			}

			// add trailing slash and initialize the error log file path
#ifdef UNIX
			outDir.append("/");
#else
			outDir.append("\\");
#endif
			userIF->SetErrorFile(outDir);
			break;
		}
	}

	for (i = 1; i < argc; i++)
	{
		arg = argv[i];

		if (arg == "-v")
		{
			cout << "UCC version " << PRODUCT_REVISION << endl;
			exit(0);
		}
		else if (arg == "-dir")
		{
			// read the two baseline directory names and the extract the names
			// of the file extensions to be searched
			if ((isDiff && i + 3 > argc) || (!isDiff && i + 2 > argc))
			{
				string err = "Error: Unable to parse command line args";
				userIF->AddError(err);
				return 0;
			}

			// extract the directory names for BaselineA and BaselineB (if applicable)
			use_CommandLine = true;
			i++;
			dirnameA = argv[i];
			i++;
			if (isDiff)
			{
				dirnameB = argv[i];
				i++;
			}

			// extract the names of the all file extensions to be searched
			for (; i < argc; i++)
			{
				// command line arguments can be mentioned in any sequence
				// search until you do not come across the end of arguments or
				// another command line argument
				arg = argv[i];
				if (arg.find_first_of('-') == 0)	// '-' indicates a new argument
					break;
				else
					listFilesToBeSearched.push_back(arg);
			}
			if (listFilesToBeSearched.size() == 0)
			{
				// add all files as the default
				listFilesToBeSearched.push_back("*.*");
			}
		}

		// Return from the loop if all arguments have been processed.
		if (i == argc)
			return 1;

		if (arg == "-i1")
		{
			// extract the BaselineA file
			if (i + 1 > argc)
			{
				string err = "Error: Unable to parse command line args";
				userIF->AddError(err);
				return 0;
			}
			i++;

			ifstream infile;
			infile.open(argv[i], ifstream::in);
			infile.close();
			if (infile.fail())
			{
				// file did not exist
				string err = "Error: Unable to open baseline file (";
				err += argv[i];
				err += ")";
				userIF->AddError(err);
				return 0;
			}
			BaselineFileName1 = string(argv[i]);
		}
		else if (arg == "-i2" && isDiff)
		{
			// extract the BaselineB file
			if (i + 1 > argc)
			{
				string err = "Error: Unable to parse command line args";
				userIF->AddError(err);
				return 0;
			}
			i++;

			ifstream infile;
			infile.open(argv[i], ifstream::in);
			infile.close();
			if (infile.fail())
			{
				// file did not exist
				string err = "Error: Unable to open baseline file (";
				err += argv[i];
				err += ")";
				userIF->AddError(err);
				return 0;
			}
			BaselineFileName2 = string(argv[i]);
		}
		else if (arg == "-outdir")
		{
			// already processed so increment for the output directory name
			i++;
		}
		else if (arg == "-extfile")
		{
			// read the user defined language extension map
			if (i + 2 > argc)
			{
				string err = "Error: Unable to parse command line args";
				userIF->AddError(err);
				return 0;
			}
			i++;
			userExtMapFile = argv[i];
		}
		else if (arg == "-t" && isDiff)
		{
			if (i + 1 > argc)
				return 0;
			else
			{
				i++;
				double myMatch = atof(argv[i]);
				if (myMatch >= 0 && myMatch <= 100)
					match_threshold = myMatch;
				else
					return 0;
			}
		}
		else if (arg == "-tdup")
		{
			if (i + 1 > argc)
				return 0;
			else
			{
				i++;
				if (duplicate_threshold >= 0.0)
				{
					double myDup = atof(argv[i]);
					if (myDup <= 100.0)
					{
						if (myDup < 0.0)
							myDup = -1.0;
						duplicate_threshold = myDup;
					}
					else
						return 0;
				}
			}
		}
		else if (arg == "-nodup")
		{
			duplicate_threshold = -1.0;
		}
		else if (arg == "-trunc")
		{
			if (i + 1 > argc)
				return 0;
			else
			{
				i++;
				int myTrunc = atoi(argv[i]);
				if (myTrunc >= 0)
					lsloc_truncate = myTrunc;
				else
					return 0;
			}
		}
		else if (arg == "-d" && isDiff)
		{
			// ignore
		}
		else if (arg == "-cf")
		{
			// support ClearCase files
			clearCaseFile = true;
		}
		else if (arg == "-unified")
		{
			print_unified = true;
		}
		else if (arg == "-ascii")
		{
			print_csv = false;
			if (!print_legacy)
				print_ascii = true;
			else
				print_ascii = false;
		}
		else if (arg == "-legacy")
		{
			print_csv = false;
			print_ascii = false;
			print_legacy = true;
		}
		else if (arg == "-nocomplex")
		{
			print_cmplx = false;
		}
#ifdef UNIX
		else if (arg == "-nolinks")
		{
			// disable following symbolic links
			followSymLinks = false;
		}
#endif
		else if ( arg == "-threads" )
		{
			if (i + 1 > argc)
				return 0;
			else
			{
				i++;
				int myThreads = atoi(argv[i]);
				if ( myThreads < MIN_UCC_THREAD_COUNT )
				{
					if ( 1 != myThreads )	// Allow for Optimization
						myThreads = MIN_UCC_THREAD_COUNT;
				}

				if ( myThreads > MAX_UCC_THREAD_COUNT )
					myThreads = MAX_UCC_THREAD_COUNT;

				workThreadsCount = myThreads;
			}
		}
		else if ( arg == "-nowarnings" )
		{
			g_no_warnings_to_stdout = true;
		}
		else if ( arg == "-nouncounted" )
		{
			g_no_uncounted = true;
		}
		else 
			return 0;
	}
	return 1;
}

/*!
* Shows program usage and terminates application.
*
* \param option usage option (default="")
* \param do_exit exit after display (default=true)
*/
void MainObject::ShowUsage(const string &option, bool do_exit)
{
	if (option == "-v")
	{
		cout << "Usage: ucc -v" << endl << endl;
		cout << " -v: Displays the current version of UCC being executed" << endl;		
	}
	else if (option == "-d" || option == "-i1" || option == "-i2" || option == "-t")
	{
		cout << "Usage: ucc -d [-i1 <fileListA>] [-i2 <fileListB>] [-t <#>]" << endl << endl;
		cout << " -d: Enables the differencing function. If not specified, only the" << endl;
		cout << "     counting functions will execute." << endl << endl;
		cout << " Related Options:" << endl << endl;
		cout << "  -i1 <fileListA>: File containing a list of files to be used as" << endl;
		cout << "                   Baseline A for counting or comparison if -d is specified." << endl;
		cout << "                   If -i1 is not specified, the file 'fileListA.txt' will be" << endl;
		cout << "                   used as the default if -d is specified, and the file" << endl;
		cout << "                   'fileList.txt' will be used as the default without -d." << endl << endl;
		cout << "  -i2 <fileListB>: File containing a list of files to be be used as" << endl;
		cout << "                   Baseline B for comparison to Baseline A." << endl;
		cout << "                   If -i2 is not specified, the file 'fileListB.txt' will be" << endl;
		cout << "                   used as the default if -d is specified." << endl << endl;
		cout << "  -t <#>:          Specifies the percentage of common characters between two" << endl;
		cout << "                   lines of code that are being compared in order to determine" << endl;
		cout << "                   whether the line is modified or replaced. If the percentage of" << endl;
		cout << "                   common characters between the compared lines is greater than" << endl;
		cout << "                   the specified threshold, the line is considered replaced and" << endl;
		cout << "                   will be counted as one line deleted and one line added." << endl;
		cout << "                   Otherwise, it will be counted as one modified line." << endl;
		cout << "                   The valid range is 0 to 100 and defaults to 60." << endl;
	}
	else if (option == "-tdup")
	{
		cout << "Usage: ucc -tdup <#>" << endl << endl;
		cout << " -tdup <#>: Specifies the percentage of logical source lines of code (LSLOC)" << endl;
		cout << "            that have changed between two files of the same name in order to" << endl;
		cout << "            determine whether the files are duplicates. If the percentage of" << endl;
		cout << "            common LSLOC between two files is less than or equal to the" << endl;
		cout << "            specified threshold, the files are considered duplicates." << endl;
		cout << "            This method compares LSLOC similar to the differencing function" << endl;
		cout << "            and ignores formatting including blank lines and comments." << endl;
		cout << "            Note that files of different names may be checked for an exact" << endl;
		cout << "            physical match. The valid range is 0 to 100 and defaults to 0." << endl;
	}
	else if (option == "-trunc")
	{
		cout << "Usage: ucc -trunc <#>" << endl << endl;
		cout << " -trunc <#>: Specifies the maximum number of characters allowed in a logical" << endl;
		cout << "             source line of code (LSLOC). Any characters beyond the specified" << endl;
		cout << "             threshold will be truncated and ignored when compared." << endl;
		cout << "             If the truncation is disabled by setting the threshold to 0" << endl;
		cout << "             or the threshold is set too high, very long LSLOC may significantly" << endl;
		cout << "             degrade performance." << endl;
	}
	else if (option == "-cf")
	{
		cout << "Usage: ucc -cf" << endl << endl;
		cout << " -cf: Indicates that the target files were retrieved from" << endl;
		cout << "      IBM Rational ClearCase. ClearCase appends information at the end" << endl;
		cout << "      of file names beginning with '@@'. Use of this option strips" << endl;
		cout << "      all characters after the last '@@' sequence from the file name." << endl;
	}
	else if (option == "-dir")
	{
		cout << "Usage: ucc -dir <dirA> [<dirB>] <fileSpecs>" << endl << endl; 
		cout << " -dir: Specifies the directories and file types to be searched for files" << endl;
		cout << "       to be counted or compared. The directories dirA and dirB indicate" << endl;
		cout << "       the directories to be searched for each baseline. The fileSpecs indicate" << endl;
		cout << "       the file type specifications (typically containing search wildcards)." << endl;
		cout << "       The specified directories will be searched recursively." << endl << endl;
		cout << " Option Parameters:" << endl << endl;
		cout << "    <dirA>:      Specifies the directory of Baseline A to be searched for files" << endl;
		cout << "                 to be counted or compared." << endl << endl;
		cout << "    <dirB>:      If the -d option is specified, this specifies the directory" << endl;
		cout << "                 of Baseline B to be searched for files to be compared." << endl << endl;
		cout << "    <fileSpecs>: One or more specification of file types to be included" << endl;
		cout << "                 in the file search. Each file specification is separated" << endl;
		cout << "                 by whitespace and typically contains search wildcards."  << endl;
		cout << "                 For example:" << endl;
		cout << "                   *.cpp *.h *.java" << endl;
	}
	else if (option == "-outdir")
	{
		cout << "Usage: ucc -outdir <outDir>" << endl << endl;
		cout << " -outdir <outDir>: Specifies the directory where output files will be written." << endl;
		cout << "                   If this is not specified, the output files will be written" << endl;
		cout << "                   to the working directory by default. This option prevents" << endl;
		cout << "                   overwriting output files from multiple runs and allows for" << endl;
		cout << "                   batch execution and output organization." << endl;
	}
	else if (option == "-extfile")
	{
		cout << "Usage: ucc -extfile <extFile>" << endl << endl;
		cout << " -extfile <extFile>: Specifies a file containing user specified file extensions" << endl;
		cout << "                     for any of the available language counters. Any language" << endl;
		cout << "                     counter specified within this file will have its associated" << endl;
		cout << "                     extensions replaced. If a language is specified with no" << endl;
		cout << "                     extensions, the language counter will be disabled." << endl;
		cout << "                     The file format contains a single line entry for each" << endl;
		cout << "                     language. Single or multi-line comments may be included" << endl;
		cout << "                     with square brackets []. For example:" << endl << endl;
		cout << "                       C_CPP = *.cpp, *.h  [C/C++ extensions]" << endl << endl;
		cout << "                     Please see the user manual for available language counter" << endl;
		cout << "                     names." << endl;
	}
	else if (option == "-unified")
	{
		cout << "Usage: ucc -unified" << endl << endl;
		cout << " -unified: Prints language report files to a single unified report file." << endl;
		cout << "           The results are written to 'TOTAL_outfile.csv' or 'TOTAL_outfile.txt'." << endl;
		cout << "           In the absence of this option, results for each language are written" << endl;
		cout << "           to separate files." << endl;
	}
	else if (option == "-ascii")
	{
		cout << "Usage: ucc -ascii" << endl << endl;
		cout << " -ascii: Prints ASCII text (*.txt) report files instead of CSV (*.csv) files." << endl;
		cout << "         The content of the ASCII format is identical to the CSV format." << endl;
	}
	else if (option == "-legacy")
	{
		cout << "Usage: ucc -legacy" << endl << endl;
		cout << " -legacy: Prints legacy formatted ASCII text report files instead of" << endl;
		cout << "          the current format of the CSV or text files. The purpose of this" << endl;
		cout << "          option is to maintain backward compatibility with some older" << endl;
		cout << "          UCC results post-processing software systems." << endl;
	}
	else if (option == "-nodup")
	{
		cout << "Usage: ucc -nodup" << endl << endl;
		cout << " -nodup: Prevents separate processing of duplicate files. This avoids extra" << endl;
		cout << "         processing time to determine the presence of duplicate files within" << endl;
		cout << "         each baseline. With this option, all files will be counted and reported" << endl;
		cout << "         together, regardless of whether they are duplicates. Otherwise, files" << endl;
		cout << "         within a baseline will be checked for duplicates and results will" << endl;
		cout << "         be reported separately. Please see the user manual for details." << endl;
	}
	else if (option == "-nocomplex")
	{
		cout << "Usage: ucc -nocomplex" << endl << endl;
		cout << " -nocomplex: Disables printing of keyword counts and processing of complexity" << endl;
		cout << "             metrics. This can reduce processing time and limit reports." << endl;
	}
#ifdef UNIX
	else if (option == "-nolinks")
	{
		cout << "Usage: ucc -nolinks" << endl << endl;
		cout << " -nolinks: Disables following symbolic links to directories and counting" << endl;
		cout << "           of links to files on Unix systems. This can prevent duplicate file counts." << endl;
	}
#endif
	else if ( option == "-threads" )
	{
		cout << "Usage: ucc -threads" << endl << endl;
		cout << " -threads <#>       Specify the number of work threads. Minimum is 1." << endl;
		cout << "                      If not specified then no work threads are created." << endl;
		cout << "                      1 = Faster processing and no work threads created." << endl;
	}
	else if ( option == "-nowarnings" )
	{
		cout << "Usage: ucc -nowarnings" << endl << endl;
		cout << " -nowarnings        Disables warning messages on console." << endl;
		cout << "                      Warning messages will still be logged." << endl;
		cout << "                      Error messages will still show on console." << endl;
	}
	else if ( option == "-nouncounted" )
	{
		cout << "Usage: ucc -nouncounted" << endl << endl;
		cout << " -nouncounted       Disables reports or messages about uncounted files." << endl;
	}
	else if (option == "-h" || option == "-help")
	{
		cout << "Usage: ucc -help <option>" << endl << endl;
		cout << " -help <option>: Without a specified option, this displays the list of command" << endl;
		cout << "                 line options. If a command line option is specified, detailed" << endl;
		cout << "                 usage information for the specified option is displayed." << endl;
	}
	else
	{
		cout << "Usage: ucc [-v] [-d [-i1 fileListA] [-i2 <fileListB>] [-t <#>]] [-tdup <#>]" << endl;
		cout << "           [-trunc <#>] [-cf] [-dir <dirA> [<dirB>] <fileSpecs>]" << endl;
		cout << "           [-outdir <outDir>] [-extfile <extFile>] [-unified] [-ascii] [-legacy]" << endl;
#ifdef UNIX
		cout << "           [-nodup] [-nocomplex] [-nolinks] [-threads <#>] [-nowarnings]" << endl; 
		cout << "           [-nouncounted] [-help [<option>]]" << endl << endl;
#else
		cout << "           [-nodup] [-nocomplex] [-threads <#>] [-nowarnings] [-nouncounted]" << endl;
		cout << "           [-help [<option>]]" << endl << endl;
#endif
		cout << "Options:" << endl;
		cout << " -v                 Lists the current version number." << endl;
		cout << " -d                 Runs the differencing function." << endl;
		cout << "                      If not specified, runs the counting function." << endl;
		cout << " -i1 <fileListA>    Filename containing filenames in the Baseline A." << endl;
		cout << " -i2 <fileListB>    Filename containing filenames in the Baseline B." << endl;
		cout << " -t <#>             Specifies the threshold percentage for a modified line." << endl;
		cout << "                      (DEFAULTS TO 60)." << endl;
		cout << " -tdup <#>          Specifies the threshold percentage for duplicated files -" << endl;
		cout << "                      the maximum percent difference between two files of the" << endl;
		cout << "                      same name in a baseline to be considered duplicates." << endl;
		cout << "                      (DEFAULTS TO 0)" << endl;
		cout << " -trunc <#>         Specifies the maximum number of characters allowed in a" << endl;
		cout << "                      logical SLOC. Additional characters will be truncated." << endl;
		cout << "                      (DEFAULTS TO 10,000, use 0 for no truncation)" << endl;
		cout << " -cf                Indicated that target files were retrieved from ClearCase." << endl;
		cout << "                      Restored the original filename before counting." << endl;
		cout << " -dir               Specifies the following directories and file specifications: " << endl;
		cout << "      <dirA>          Name of the directory containing source files." << endl;
		cout << "                        If -d is given, dirA is the directory for Baseline A." << endl;
		cout << "      <dirB>          Name of the directory for Baseline B only if -d is given." << endl;
		cout << "      <fileSpecs>     File specifications, wildcard chars ? * are allowed." << endl;
		cout << "                        For example, *.cpp *.h" << endl;
		cout << " -outdir <outDir>   Specifies the directory to store the output files." << endl;
		cout << " -extfile <extFile> Indicates language extension mapping filename" << endl;
		cout << " -unified           Print language report files to a single unified report file." << endl;
		cout << " -ascii             Print ASCII text report files instead of CSV files." << endl;
		cout << " -legacy            Print legacy formatted ASCII text report files" << endl;
		cout << "                      instead of the current format of the CSV or text files." << endl;
		cout << " -nodup             Disables separate processing of duplicate files." << endl;
		cout << " -nocomplex         Disables printing complexity reports or keyword counts." << endl;
#ifdef UNIX
		cout << " -nolinks           Disables following symbolic links to directories and files." << endl;
#endif
		cout << " -threads <#>       Specify the number of work threads. Minimum is 1." << endl;
		cout << "                      If not specified then no work threads are created." << endl;
		cout << "                      1 = Faster processing and no work threads created." << endl;
		cout << " -nowarnings        Disables warning messages on console." << endl;
		cout << "                      Warning messages will still be logged." << endl;
		cout << "                      Error messages will still show on console." << endl;
		cout << " -nouncounted       Disables reports or messages about uncounted files." << endl;
		cout << " -help <option>     Displays this usage or usage for a specified option." << endl;
	}
	if (do_exit)
		exit(1);
}

/*!
* Read user-defined language extension map file.
*
* \param extMapFile language extension map file
*/
void MainObject::ReadUserExtMapping(const string &extMapFile)
{
	ifstream readFile;
	string line, str;
	string language;
	string extension;
	int flag;
	string token;
	size_t pos1, pos2;
	bool foundc = false;
	readFile.open(extMapFile.c_str(), ifstream::in);
	if (readFile.is_open())
	{
		while (readFile.good() || readFile.eof())
		{
			getline(readFile, line);
			if ((!readFile.good() && !readFile.eof()) || (readFile.eof() && line.empty()))
				break;
			line = CUtil::TrimString(line);
			flag = 0;

			// process embedded, whole, or multi-line comments (delimited by [])
			if (foundc)
			{
				pos2 = line.find_first_of(']');
				if (pos2 != string::npos)
				{
					if (pos2 >= line.size() - 1)
						line.clear();
					else
						line = line.substr(pos2 + 1);
					foundc = false;
				}
				else
					continue;
			}
			pos1 = line.find_first_of('[');
			while (pos1 != string::npos)
			{
				if (pos1 == 0)
					str = "";
				else
					str = line.substr(0, pos1);
				pos2 = line.find_first_of(']');
				if (pos2 != string::npos)
				{
					if (pos2 >= line.size() - 1)
						line = str;
					else
						line = str + line.substr(pos2 + 1);
				}
				else
				{
					line = str;
					foundc = true;
				}
				pos1 = line.find_first_of('[');
			}

			if (line.size() > 0)
			{
				istringstream iss(line);
				while (getline(iss, token, '='))
				{
					if (flag == 0)
					{
						language = CUtil::TrimString(CUtil::ToLower(token));
						flag = 1;
					}
					else
					{
						extension = CUtil::TrimString(CUtil::ToLower(token));
						flag = 0;
					}
				}
				LanguageExtensionMap.insert(std::pair<std::string, std::string>(language, extension));
			}
			if (!readFile.good())
				break;
		}
	}	
	readFile.close();
	CreateExtMap();
}

/*!
* Create user-defined language extension map.
*/
void MainObject::CreateExtMap()
{
	int i, j;
	string token, lang_name;
	bool found, updateWeb = false;
	CCodeCounter *langCounter;
	CWebCounter *webCounter = (CWebCounter *)CounterForEachLanguage[WEB];
	StringVector *webExten;

	for (std::map< std::string, std::string >::const_iterator iter = LanguageExtensionMap.begin();
		iter != LanguageExtensionMap.end(); ++iter)
	{
		// check for a counter for the specified language
		lang_name = iter->first;
		langCounter = NULL;
		for (i = 0; i < (int)CounterForEachLanguage.size(); i++)
		{
			if (CounterForEachLanguage[i]->classtype == WEB)
			{
				found = false;
				for (j = 0; j < (int)webCounter->web_lang_names.size(); j++)
				{
					if (lang_name.compare(CUtil::ToLower(webCounter->web_lang_names[j])) == 0)
					{
						langCounter = CounterForEachLanguage[i];
						updateWeb = true;
						found = true;
						break;
					}
				}
				if (found)
					break;
			}
			else if (lang_name.compare(CUtil::ToLower(CounterForEachLanguage[i]->language_name)) == 0)
			{
				langCounter = CounterForEachLanguage[i];
				break;
			}
		}
		if (langCounter != NULL)
		{
			if (langCounter->classtype == WEB)
			{
				if (lang_name == "asp")
					webExten = &(webCounter->file_exten_asp);
				else if (lang_name == "jsp")
					webExten = &(webCounter->file_exten_jsp);
				else if (lang_name == "php")
					webExten = &(webCounter->file_exten_php);
				else if (lang_name == "coldfusion")
					webExten = &(webCounter->file_exten_cfm);
				else if (lang_name == "xml")
					webExten = &(webCounter->file_exten_xml);
				else
					webExten = &(webCounter->file_exten_htm);

				webExten->clear();						// optional line if existing map is to be erased
				istringstream iss(iter->second);
				while (getline(iss, token, ','))
					webExten->push_back(CUtil::TrimString(token));
			}
			else
			{
				langCounter->file_extension.clear();	// optional line if existing map is to be erased
				istringstream iss(iter->second);
				while (getline(iss, token, ','))
					langCounter->file_extension.push_back(CUtil::TrimString(token));
			}
		}
		else
		{
			string err = "Error: ";
			err += iter->first;
			err += " is not a supported language as specified in the language extension file";
			userIF->AddError(err, false, 1);
		}
	}

	// update web extensions if changed
	if (updateWeb)
		webCounter->UpdateWebFileExt();
}

/*!
* Start worker Threads if wanted.
*
* \param start_result_msg	IN/OUT	Message holding results of starting threads
*
* \return nothing
*/
void MainObject::StartThreads( string & start_result_msg )
{
	start_result_msg = "";

	if ( workThreadsCount )
	{
		// Set some flags for using Optimization of processing steps
		if ( isDiff )
		{
			// Called from DiffTool
			g_process_after_read = true;

			// TODO: think about other downstream processing that can be multithreaded...

		}
		else
		{
			// Called from MainObject
			g_process_after_read = true;

			if ( g_process_after_read && ( ! (duplicate_threshold >= 0.0) ) )
				g_discard_lines_after_process = true;	// OK to discard, not doing Duplicate checking
		}
	}

	if ( workThreadsCount > 1 )
	{
		char buf[16];
		string tmp;
		string msg_result;	// This will hold details from starting threads

		int start_result = StartWorkerThreads( &(CounterForEachLanguage), userIF, workThreadsCount, msg_result );
		if ( THREADS_STARTED_OK != start_result )
		{
			if ( 0 > start_result )
			{
				// There was an Error starting threads.  Add to Error message.
				msg_result += "\nRunning without any extra threads.\n";
				workThreadsCount = 1;
			}
			else
			{
				// Number of threads started is less than wanted.
				// Continue running with these fewer threads.
				// Message already done.
				workThreadsCount = (unsigned int)start_result;
			}
		}
		else
		{
			// Message that wanted number of threads started OK.
			_itoa_s( workThreadsCount, buf, 10 );
			msg_result = buf;
			msg_result += " Threads started.\n";
		}
		start_result_msg = msg_result;
	}
}


/*!
* Reads all files into memory.  MAY also do other processing AFTER files are Read.
*
* \param inputFileVector list of files to count (may be empty if inputFileList is given)
* \param inputFileList file containing list of files to count (if any)
* \param useListA use file list A? (otherwise use list B)
*
* \return method status		1 if OK, else 0 if unrecoverable error trying to open list file 
*/
int MainObject::ReadAllFiles(StringVector &inputFileVector, string const &inputFileList, const bool useListA)
{
	int		error_count = 0;
	string	fileName;
	string	errList;

    // if the size of the vector is zero read from a given List of files
	if (inputFileVector.size() == 0 && inputFileList.size() > 0)
	{
	    ifstream fl;
		fl.open(inputFileList.c_str(), ifstream::in);

		if (!fl.is_open())
		{
			// try old input file name
			fl.open(INPUT_FILE_LIST_OLD, ifstream::in);
			if (!fl.is_open())
			{
				string err = "Error: Unable to open list file (";
				err += inputFileList;
				err += ")";
				userIF->AddError(err, false, 1);
				return 0;	// Error: Unable to open list file
			}
		}
		fl.clear();
		fl.seekg(0, ios::beg);

	// LOOP and add more File names to List of Files to process
		StringVector searchFiles;
		searchFiles.push_back("*.*");
		while (fl.good() || fl.eof())
		{
			getline(fl, fileName);
			if ((!fl.good() && !fl.eof()) || (fl.eof() && fileName.empty()))
				break;
			if (!CUtil::CheckBlank(fileName))
			{
				if (clearCaseFile)
					inputFileVector.push_back(fileName);
				else
				{
					// expand if item is a directory
					if (!CUtil::ListAllFiles(fileName, searchFiles, inputFileVector, followSymLinks))
						inputFileVector.push_back(fileName);
				}
			}
			if (!fl.good())
				break;
		}	//	END		while adding more File names

		fl.close();
	}	//	END		if adding more to List of Files to Read

	SourceFileList* mySrcFileList = (useListA) ? &SourceFileA: &SourceFileB;
	std::vector<std::string>::iterator itStart = inputFileVector.begin();
	unsigned int	num_in_list = distance( itStart, inputFileVector.end() );
	unsigned int	count_done = 0;

	// IF the User has set -threads 1 (or more)
	// THEN the global flags to tell which Optimizations to use have been set.
	if ( workThreadsCount >= MIN_UCC_THREAD_COUNT )
	{
		error_count = ReadFilesThreads( inputFileVector.size(), userIF, print_cmplx, 
										print_csv, itStart, inputFileVector.end(), 
										useListA, clearCaseFile, mySrcFileList, outDir,
										g_process_after_read,
										g_discard_lines_after_process );

		// Wait until File Reading (and possible other processing) is finished
		unsigned int	num_busy = 1;
		unsigned int	percent_done = 0;
		unsigned int	prev_percent_done = 0;

		do 
		{
			ThreadSleep( 100 );

			// Returns count of threads working 
			// and will set count_done (how many files) were processed so far
			// Don't automatically call CombineThreadResults here as UI must update first
			num_busy = NumThreadsBusy( count_done, 100, false );

			if ( num_busy )
			{
				percent_done = ( count_done * 100 ) / num_in_list;
				if ( percent_done != prev_percent_done )
				{
					// Percent changed so update UI
					userIF->updateProgress("", false, percent_done, num_in_list );
					prev_percent_done = percent_done;
				}
			}
		} while ( num_busy );
	}
	else
	{
		// Not using extra worker threads
		// May still use optimizations if -threads 1 is set
		ErrMsgStructVector			err_msgs;	// Not used but needed for arg in call
		UncountedFileStructVector	unc_files;	// Not used but needed for arg in call

		error_count = ReadFilesInList( MAX_UCC_THREAD_COUNT + 1, userIF, 
									CounterForEachLanguage, 
									print_cmplx, 
									print_csv, itStart, inputFileVector.end(), 
									useListA, clearCaseFile, mySrcFileList, 
									&SourceFileA, &SourceFileB, 
									errList,
									outDir,
									err_msgs, unc_files,
									g_process_after_read,
									g_discard_lines_after_process );
		
		// Here the error_count is for info purposes.  
		// Single thread mode mostly does the Error reporting as expected.

		count_done = mySrcFileList->size();
	}


	// Call 1 more time to show User 100% completion (this does not depend on following code).
	userIF->updateProgress("", false, 100, 1 );

#ifndef QTGUI
	if ( count_done )
		// Just erase the last 4 characters
		userIF->updateProgress("\b\b\b\b", false);
	userIF->updateProgress("DONE");	// This also adds a newline afterwards
#endif

	if ( workThreadsCount >= MIN_UCC_THREAD_COUNT )
	{
		CombineThreadResults();
		// At this point any Errors => log or UI messages, Uncounted Files, UI messages
		// were handled along with the results of Reading in the List of Files
	}

	if ( errList.size() )
		userIF->updateProgress( errList );

	return 1;
}


/*!
* Process source file list and call relevant counter for each file.
*
* \param useListA use file list A? (otherwise use list B)
*/
void MainObject::ProcessSourceList( const bool useListA )
{
	SourceFileList* mySrcFileList = (useListA) ? &SourceFileA: &SourceFileB;

	ErrMsgStructVector			err_msgs;	// Not used but needed for arg in call
	UncountedFileStructVector	unc_files;	// Not used but needed for arg in call
	
	string			errList;

	unsigned int	prev_percent = 0;
	unsigned int	num_inserted = 0;
	unsigned int	count_progress = 1;
	unsigned int	sizeo = (unsigned int)mySrcFileList->size();

	SourceFileList::iterator itEnd = mySrcFileList->end();
	for ( SourceFileList::iterator i = mySrcFileList->begin(); i != itEnd; i++ )
	{
		// Code inside ProcessSourceListFile MAY Insert entries in the List !
		// The loop check above is OK as it can handle a growing List.
		ProcessSourceListFile( MAX_UCC_THREAD_COUNT + 1, userIF, CounterForEachLanguage, 
								&counter, i, &SourceFileA, &SourceFileB, 
								num_inserted,
								err_msgs, unc_files,
								errList, useListA );

		if ( HasUserCancelled() )
			return;

		// Moving UI call here to keep the User informed has 2 benefits:
		// 1) Removes a little overhead from code path used by threads
		// 2) Fixes defect where the UI would increment the % done 
		// BEFORE any Analysis or Counting of a file.  In the case 
		// of 1 file it would show 100% and then process the file.
		// Having it here now reports % done more accurately.

		if ( i->second.file_name_isEmbedded == false )
		{
			unsigned int disp = ((count_progress * 100) / sizeo);
			if ( prev_percent != disp )
			{
				userIF->updateProgress( "", false, disp, sizeo );
				prev_percent = disp;
			}
			count_progress++;
		}
	}

#ifndef QTGUI
	if (mySrcFileList->size() > 0)
		userIF->updateProgress("\b\b\b\b", false);
	userIF->updateProgress("DONE");
#endif

	if (!errList.empty())
		userIF->updateProgress(errList);
}


/*!
* Checks whether a file has a supported extension (can be counted by UCC).
*
* \param file_name file name
*
* \return supported file extension?
*/
bool MainObject::IsSupportedFileExtension(const string &file_name) 
{
	for (map<int,CCodeCounter*>::iterator iter = CounterForEachLanguage.begin(); iter != CounterForEachLanguage.end(); iter++)
	{
		if (iter->second->IsSupportedFileExtension(file_name))
			return true;
	}
	return false;
}


/*!
* Retrieves the extensions for each counter language.
*
* \param lang_ext_map pointer to language extension map
*/
void MainObject::GetLanguageExtensionMap(multimap<const string, string> *lang_ext_map)
{
	CWebCounter *webCounter = (CWebCounter *)CounterForEachLanguage[WEB];
	StringVector *ext_list;
	string lang_name;
	
	lang_ext_map->clear();
	for (map<int,CCodeCounter*>::iterator iter = CounterForEachLanguage.begin(); iter != CounterForEachLanguage.end(); iter++)
	{
		lang_name = iter->second->language_name;
		if (lang_name != "UNDEF")
		{
			if (lang_name == "ASP")
				ext_list = &(webCounter->file_exten_asp);
			else if (lang_name == "JSP")
				ext_list = &(webCounter->file_exten_jsp);
			else if (lang_name == "PHP")
				ext_list = &(webCounter->file_exten_php);
			else if (lang_name == "ColdFusion")
				ext_list = &(webCounter->file_exten_cfm);
			else if (lang_name == "XML")
				ext_list = &(webCounter->file_exten_xml);
			else if (lang_name == "HTML")
				ext_list = &(webCounter->file_exten_htm);
			else
				ext_list = &iter->second->file_extension;

			if (ext_list->size() < 1)
				lang_ext_map->insert(pair<const string, string>(lang_name, ""));
			else
			{
				for (StringVector::iterator vit = ext_list->begin(); vit != ext_list->end(); vit++)
					lang_ext_map->insert(pair<const string, string>(lang_name, (*vit)));
			}
		}
	}
}


/*!
* Finds duplicate files in a baseline.
*
* \param fileList file list
* \param dupList1 first (source) duplicate files list
* \param dupList2 duplicate files list
* \param checkMatch check for matching files (applies only when differencing)
*/
void MainObject::FindDuplicateFiles(SourceFileList &fileList, StringVector &dupList1, StringVector &dupList2, const bool checkMatch)
{

#ifdef	_DEBUG
	int				errorRet;
	bool			call_again = true;
	bool			validate = true;
	unsigned int	start_block_count = 0;
	unsigned int	end_block_count   = 0;
	unsigned long	start_total_sizes = 0L;
	unsigned long	end_total_sizes   = 0L;

	HeapInUse( errorRet, start_block_count, start_total_sizes, validate );
	if ( ( errorRet == 1 ) || ( errorRet < 0 ) )
		call_again = false;
#endif

	// Update UI while finding Duplicates
	unsigned int	num_in_list       = 2 * fileList.size();
	unsigned int	percent_done      = 0;
	unsigned int	prev_percent_done = 0;
	unsigned int	count_done        = 0;

	SourceFileList::iterator fileList_end = fileList.end();

// This LOOP will show the first 50%
	for (SourceFileList::iterator i = fileList.begin(); i != fileList_end; i++ )
	{
		// Check Preconditions (Design by Contract) for correctness and to avoid unneeded calls
		if ( ( !(*i).second.duplicate && !(*i).second.firstDuplicate ) 
		  && ( (*i).second.file_name_isEmbedded == false ) )
		{
			SourceFileList::iterator j = i;
			j++;
			if ( j == fileList_end )
				break;		// done

			/*	COMMENTED OUT due to unexpected difference in results ! ! !
			// WARNING: Logically this code block SHOULD be a valid check to do here before calling FindDuplicateFor
			// Practically having this Precondition check done here causes some combinations of Duplicate checks 
			// to not have the same results when compared to results from unmodified 2013_4 code
			//
			// TODO: Revisit why this can not be activated to help speed up Duplicate checking step
			// 
			// Only call compare if sizes are same (faster)
			int filenameMatched = 1;	// Start with file names NOT matched 
			if ( (*i).second.file_name_only.size() == (*j).second.file_name_only.size() )
				filenameMatched = (*i).second.file_name_only.compare( (*j).second.file_name_only );

			if ( (*i).first.size() != (*j).first.size() &&
				filenameMatched != 0 )
			{
				// two files have different number of lines and different filename
				continue;
			}
			*/
			// END	Precondition checks

			FindDuplicateFor( fileList, i, dupList1, dupList2, checkMatch );
		}

		count_done++;
		percent_done = ( count_done * 100 ) / num_in_list;
		if ( percent_done != prev_percent_done )
		{
			// Percent changed so update UI
			userIF->updateProgress("", false, percent_done, num_in_list );
			prev_percent_done = percent_done;
		}

		// Check for Cancel
		if ( HasUserCancelled() )
			return;
	}

// This LOOP will show the remaining 50%

	// update duplicate embedded files
	for (SourceFileList::iterator i = fileList.begin(); i != fileList.end(); i++)
	{
		if ((*i).second.class_type == WEB && ((*i).second.duplicate || (*i).second.firstDuplicate))
		{
			SourceFileList::iterator j = i;
			for (j++; j != fileList.end(); j++)
			{
				if ( j->second.file_name_isEmbedded == true )
				{
					if ((*i).second.duplicate)
						j->second.duplicate = true;
					else
						j->second.firstDuplicate = true;
				}
				else
					break;
			}
		}

		count_done++;
		percent_done = ( count_done * 100 ) / num_in_list;
		if ( percent_done != prev_percent_done )
		{
			// Percent changed so update UI
			userIF->updateProgress("", false, percent_done, num_in_list );
			prev_percent_done = percent_done;
		}

		// Check for Cancel
		if ( HasUserCancelled() )
			return;
	}

#ifndef QTGUI
	if ( num_in_list )
		userIF->updateProgress("\b\b\b\b", false);
	userIF->updateProgress("DONE");
#endif


#ifdef	_DEBUG
	if ( call_again )
	{
		validate = true;
		HeapInUse( errorRet, end_block_count, end_total_sizes, validate );
		if ( ( end_block_count != start_block_count ) 
			|| ( end_total_sizes != start_total_sizes ) )
		{
			cout << "\nMemory LEAK (or SHRINK) likely inside FindDuplicateFiles\n";
		}
	}
#endif

}

/*!
* Searches for duplicates of a given source file.
*
* \Precondition	sfi	is NOT invalid (iterator end or 1 position from end)
*
* \param compareList list of source files
* \param sfi source file list iterator
* \param dupList1 first (source) duplicate files list
* \param dupList2 duplicate files list
* \param checkMatch check for matching files (applies only when differencing)
*/
bool MainObject::FindDuplicateFor(SourceFileList &compareList, SourceFileList::iterator &sfi,
								  StringVector &dupList1, StringVector &dupList2, 
								  const bool checkMatch)
{
	vector<pair<SourceFileElement*, SourceFileElement*> > matchedFiles;

	SourceFileElement* nullElement = NULL;

	size_t dupCnt = 0;
	bool foundDup = false, recDup = true, filesMatched;

	int filenameMatched;
	SourceFileList::iterator i = sfi;
	SourceFileList::iterator j = sfi;
	size_t sizeF1, sizeF2;
	double pctcheck, changed_lines, total_lines, pct_change;
	bool found;
	srcLineVector emptyFile;
	
	SourceFileList::iterator	cmpListEnd = compareList.end();
	//if ( j == cmpListEnd )	// NOT needed.  Caller checks this
	//	return false;

	bool	myCheckMatch = checkMatch;	// WARNING myCheckMatch gets assigned to below ? Possible logic ERROR ?

	// Set up instances on local stack of values that do NOT change in the LOOP
	// Anything related to i
	string			i_second_file_name_only      = (*i).second.file_name_only;
	unsigned int	i_second_file_name_only_size = (*i).second.file_name_only.size();
	unsigned int	i_first_size                 = (*i).first.size();
	int				i_second_file_type           = (*i).second.file_type;
	ClassType		i_second_class_type          = (*i).second.class_type;
	vector<lineElement>::iterator i_first_begin  = (*i).first.begin();
	vector<lineElement>::iterator i_first_end    = (*i).first.end();

	// Values that are set 1 time in the LOOP and used 2 or more times
	unsigned int	j_first_size = 0;

//	LOOP	Outer loop to look for Duplicates
	for ( j++; j != cmpListEnd; j++ )
	{
		// START Precondition checks (see also FindDuplicateFiles)
		if ((*j).second.duplicate || (*j).second.firstDuplicate ||
			(*j).second.file_name_isEmbedded == true )
		{
			// already been matched or embedded file
			continue;
		}

		// Only call compare if sizes are same
		filenameMatched = 1;	// Start with file names NOT matched 
		if ( i_second_file_name_only_size == (*j).second.file_name_only.size() )
			filenameMatched = i_second_file_name_only.compare( (*j).second.file_name_only );

		j_first_size = (*j).first.size();
		if ( i_first_size != j_first_size &&
			filenameMatched != 0 )
		{
			// two files have different number of lines and different filename
			continue;
		}
		// END	Precondition checks

		// if files have same name, do a diff and mark as duplicates if logical SLOC change % is below threshold
		filesMatched = false;
		if (filenameMatched == 0 && (i_second_file_type != DATA || ( i_first_size < 1 && j_first_size < 1)))
		{
			// match empty files with same name
			if ( i_first_size < 1 && j_first_size < 1)
				filesMatched = true;
			else
			{
				// each source file elements results object has a mySLOCLines object with the SLOC to be diffed
				changed_lines = total_lines = pct_change = 0.0;
				sizeF1 = 0;
				sizeF2 = 0;

				// for web languages, diff each of the embedded files
				if ( i_second_class_type == WEB )
				{
					// find all matches for i embedded files in j
					SourceFileList::iterator i1 = i;
					SourceFileList::iterator j1 = j;
					for (i1++; i1 != cmpListEnd; i1++)
					{
						if ( i1->second.file_name_isEmbedded == false )
							break;

						found = false;
						j1 = j;
						for (j1++; j1 != cmpListEnd; j1++)
						{
							if ( j1->second.file_name_isEmbedded == false )
								break;
							if (i1->second.file_name_only.compare(j1->second.file_name_only) == 0)
							{
								found = true;
								matchedFiles.push_back(make_pair(&(*i1), &(*j1)));
								sizeF1 += i1->second.mySLOCLines.size();
								sizeF2 += j1->second.mySLOCLines.size();
							}
						}
						if (!found)
						{
							sizeF1 += i1->second.mySLOCLines.size();
							matchedFiles.push_back(make_pair(&(*i1), nullElement));
						}
					}

					// find all unmatched j embedded files
					j1 = j;
					for (j1++; j1 != cmpListEnd; j1++)
					{
						if ( j1->second.file_name_isEmbedded == false )
							break;

						found = false;
						i1 = i;
						for (i1++; i1 != cmpListEnd; i1++)
						{
							if ( i1->second.file_name_isEmbedded == false )
								break;
							if (i1->second.file_name_only.compare(j1->second.file_name_only) == 0) 
							{
								found = true;
								break;
							}
						}
						if (!found)
						{
							sizeF2 += j1->second.mySLOCLines.size();
							matchedFiles.push_back(make_pair(nullElement, &(*j1)));
						}
					}
					
					if (sizeF1 > sizeF2)
						pctcheck = 100 * (double)(sizeF1 - sizeF2) / sizeF1;
					else
						pctcheck = 100 * (double)(sizeF2 - sizeF1) / sizeF2;

					// perform comparison only if the change percent (pctcheck) is not greater than threshold
					if (pctcheck <= duplicate_threshold)
					{
						vector<pair<SourceFileElement*, SourceFileElement*> >::iterator ii = matchedFiles.begin();
						while (ii != matchedFiles.end())
						{
							if (ii->first == nullElement)
							{								
								// don't need to compare the empty file to compute the information
								changed_lines += ii->second->second.mySLOCLines.size(); // all lines deleted
							} 
							else if (ii->second == nullElement)
							{
								// don't need to compare the empty file to compute the information
								changed_lines += ii->first->second.mySLOCLines.size();
								total_lines += ii->first->second.mySLOCLines.size();								
							} 
							else
								CompareForDuplicate(ii->first->second.mySLOCLines, ii->second->second.mySLOCLines, changed_lines, total_lines);

							ii++;
						}
					}
					else
						continue;
				}
				else
				{
					// only compare if the chance of duplicates is high
					sizeF1 = (*i).second.mySLOCLines.size();
					sizeF2 = (*j).second.mySLOCLines.size();
					if (sizeF1 > sizeF2)
						pctcheck = 100 * (double)(sizeF1 - sizeF2) / sizeF1;
					else
						pctcheck = 100 * (double)(sizeF2 - sizeF1) / sizeF2;

					// perform comparison only if the change percent (pctcheck) is not greater than threshold
					if (pctcheck <= duplicate_threshold)
						CompareForDuplicate((*i).second.mySLOCLines, (*j).second.mySLOCLines, changed_lines, total_lines); 
					else
						continue;
				}

				if (changed_lines > 0.0)
					pct_change = (changed_lines / total_lines) * 100.0;
				if (pct_change <= duplicate_threshold)
					filesMatched = true;
			}
		}
		else
		{
			// if filenames are different, do a line by line comparison for identical duplicate
			if ( ( i_first_size != j_first_size )
			  || ( ( i_first_size < 1 ) || ( j_first_size < 1 ) ) )
			{
				// don't match files with different line counts or empty files with different names
				continue;
			}

			// note: two files have the same number of lines
			vector<lineElement>::iterator baseLine = i_first_begin;
			vector<lineElement>::iterator compareLine = (*j).first.begin();
			while (baseLine != i_first_end && compareLine != (*j).first.end())
			{
				if ((*baseLine).line.compare((*compareLine).line) != 0)
					break;
				baseLine++;
				compareLine++;
			}
			if (baseLine == i_first_end && compareLine == (*j).first.end())
				filesMatched = true;
		}
		if (filesMatched)
		{
			// check whether a comparison match exists
			recDup = true;
			if (myCheckMatch)
			{
				if ((*i).second.matched)
					myCheckMatch = false;
				else if ((*j).second.matched)
				{
					// change previously set first duplicate (if necessary)
					if (foundDup)
					{
						(*i).second.firstDuplicate = false;
						for (size_t n = dupList1.size() - dupCnt; n < dupList1.size(); n++)
							dupList1[n] = (*j).second.file_name;
					}

					// switch first duplicate for one with a match
					recDup = false;
					myCheckMatch = false;
					(*j).second.firstDuplicate = true;
					(*i).second.duplicate = true;
					dupList1.push_back((*j).second.file_name);
					dupList2.push_back((*i).second.file_name);
					dupCnt++;
					i = j;
				}
			}

			if (recDup)
			{
				// add pair to duplicate list
				(*i).second.firstDuplicate = true;
				(*j).second.duplicate = true;
				dupList1.push_back((*i).second.file_name);
				dupList2.push_back((*j).second.file_name);
				dupCnt++;
			}
			foundDup = true;
		}
	}
	return foundDup;
}

/*!
* Compares two files and adds to the changed/total lines for duplicate processing.
*
* \param firstFile first file to compare
* \param secondFile second file to compare
* \param changedLines accumulated number of changed lines
* \param totalLines accumulated total number of lines
*/
void MainObject::CompareForDuplicate(srcLineVector &firstFile, srcLineVector &secondFile,
									 double &changedLines, double &totalLines)
{
	CmpMngr myDiffManager;
	srcLineVector ff = firstFile;
	srcLineVector sf = secondFile;

	myDiffManager.Compare(&ff, &sf, 60);
	changedLines += myDiffManager.nAddedLines + myDiffManager.nDeletedLines + myDiffManager.nChangedLines;
	totalLines += myDiffManager.nDeletedLines + myDiffManager.nChangedLines + myDiffManager.nNochangedLines;
}

#ifdef QTGUI
/*!
* Connects to the user interface (only required for GUI).
*
* \param parent user interface parent widget
*/
void MainObject::ConnectUserIF(QWidget *parent)
{
	userIF = new UserIF(parent);
}
#endif

//////////////////////////////////////////////////////////////////////////////
/*  
	Making changes to MainObject.cpp (this file)

Approach used for Multi Threading change:

Keep the differences to a minimum compared to the 2013_04 base release sources.
Except that all the Print... helpers moved to PrintUtil.cpp .h
Other refactoring done to support threads or improve performance

Follow good design and coding practices for multithreaded applications.
Move code that must be called from threads from being class member procedures
	to being as stand alone utility code.
Eliminate use of any Global values (or changeable static variables) from thread called code 
	to prevent False Sharing for threaded code.
Put references in code comments where the developer may find more information.

Previous problems with MainObject.cpp

1) Complexity and SIZE of MainObject.cpp
	As of the 2013_04 release this file is over 9,500 physical lines.

	There is one REALLY large MainObject class with 33 methods.

	Using UCC to show complexity of various sources in UCC shows MainObject.cpp 
	is the MOST complex by a large margin.

	Cyclomatic Complexity
Total    Average  Risk
-----------------------------------------------
1075     31.62    High           MainObject.cpp

This is the overwhelming winner in the UCC source code Complexity Sweepstakes !
MainObject.cpp had both the highest total AND the highest average complexity.

Moved the Print related code to PrintUtil.cpp .h  (moved ~ 7,500 PSLOC)
Moved some other code to LangUtils.cpp .h or UCCGlobals.cpp .h or UCCThread.cpp .h

Now the Physical Source Lines Of Code here is around 2,000 lines, MUCH easier to work with.

Multithreading changes Functional tasks: T at left indicates Threads working
(main thread from OS is running)
MainObject.cpp or DiffTool.cpp
	argv parsing
	start worker Threads
	Build a List of Files to Read
		Directory/File finding as in CUtil
	Pass List of Files to Read to helper in UCCThread.  Helper will:
		divide the list among the available threads
		unblock each worker thread to do a part of the file list
		returns back to calling code in MainObject - or DiffTool with small change
T	( work Threads unblock: Reading & Analyze each File from their smaller size List )
	Main thread checks for completion about 5 times a second, 
		easy to change just don't eat CPU by too much polling.
T	( work Threads finish Reading a single file into memory )
T	( work Threads Analyze and do Counts of keywords, etc. for each file as it is read )
T	( work Threads loop to read another file into memory until end of that thread's list )
	Main thread calls helper in UCCThread to combine the Thread results
	Main thread uses combined List of Files read into memory now 
		along with keyword counts to do Duplicate checking or Differencing as needed.

				Future enhancement possiblities
----------------------------------------------------------------------
7,500+ lines of C++ code that is devoted to Printing seems excessive.
Refactoring could replace multiple instances of same code blocks with calls to helper procs
Another idea is to have the object(s) themselves support printing.
	This would more easily support the more descriptive approach that
		I am playing with for Scala and other languages to add to UCC.

				Other Performance changes:

	Memory Mapped files
Boost offers a way to support cross platform code for memory mapped files.
	This may cut down the file read times.

	Existing code is very string intensive...
When I was looking for Memory LEAKS (by checking the app heap using Windows APIs)
For only doing less than 90 files there were MANY thousands of strings (as separate blocks) in the heap.
So reducing the memory thrashing would be wise.

	Parsing performance:
		The EXISTING approach used is somewhat brute force:
For EACH line in the source code:
	Check for Blank line
	Check for Comments
		But first check for Blank line again
	Check for Keywords
		Give a list of ALL keywords to check through for EACH source line
	Do Cyclomatic complexity if wanted
Loop to do another line
5 passes through the SAME line...

A NEW Parsing approach would be:
1 to 2 passes through the same line.
	The steps are interleaved more and instead of being in separate procedures
	are more in a Parsing (lex) procedure.
	The Parsing extracts tokens from whitespace and then does some sanity checks
	Tokens that may be keywords are searched for in a list of Keywords and marked when found.
	The list is sorted so search times are OK?  Needs to be profiled.
	2nd pass is Syntax analysis (not through the source text but the qualified tokens, operators, etc.)
		(Cyclomatic complexity and other viewpoints)

Overall Parse goals:
As fast (if not faster performance) than existing approach.
New approach may be easier to enhance in future for more detailed metrics.

I have a (somewhat) working model for the Scala parser.  
The Syntax analysis is a sticking point for Scala and is currently incomplete.

		Prevent Object/Class instance creation/destruction
When I was tracing code in the Debugger I would see several instances of 
a temporary string being created, assigned, used as a return value 
and then destroyed while exiting the procedure.
It will likely be more efficient to add a string reference arg 
	and avoid temporary string creation/destruction.
Ex: I pushed up the Call stack some things like 2 int arrays to improve performance

	Using multiple Threads
There is the idea of the CPU HW Cache Line Size.  Some alignment may speed up Thread processing,
See if there is a way to make Parallel the
	Matching
	Duplicate checking
	Differencing

*/