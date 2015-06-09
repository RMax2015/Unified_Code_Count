//! The main UCC function.
/*!
* \file main.cpp
*
* This file contains the main UCC function.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_06
*	Much more detailed Exception handlers to help find defects
*	Support to get and display Time used of various steps and show files processed / second
*	DEBUG build to easily capture parts of the output as wanted and do Heap checks (Windows)
*/

#include <string>
#include <time.h>
#include <conio.h>

#include "DiffTool.h"

using namespace std;

// Declare a Global type and string to hold any Exception info.  
// Logging later perhaps?
//
int		g_thread_exception_type = EXCEPTION_DID_NOT_HAPPEN;
int		g_thread_exception_idx  = MAX_UCC_THREAD_COUNT + 1;		// Index of thread with Exception
string	g_thread_exception_msg;

//
// Declare some variables used by other code to get Timing of various steps
// The overhead of Timing steps is VERY MINIMAL and so is left in the rest of the code
//
	// These are in order of when Time is set for them
time_t	time_end_list_built = 0;
time_t	time_end_files_read = 0;
time_t	time_end_files_readB = 0;		// Differencing step only
time_t	time_end_files_analysis = 0;
time_t	time_end_files_analysisB = 0;	// Differencing step only
time_t	time_end_match_baselines = 0;	// Differencing step only
time_t	time_end_find_duplicates = 0;
time_t	time_end_print_results = 0;

	// Differencing steps only
time_t	time_end_find_duplicatesB = 0;
time_t	time_end_print_resultsB = 0;
time_t	time_end_match_baselines_web = 0;
time_t	time_end_process_pairs = 0;
time_t	time_end_print_resultsLast = 0;


// Helper to format a message string
// If time is < 0 it will not show
#define		STR_BUF_SIZE	256
void	TimeMsg( char *buf, string & time_str, const char * format, int time )
{
	strcpy_s( buf, STR_BUF_SIZE * sizeof(char), "" );
	if ( time >= 0 )
		sprintf_s( buf, STR_BUF_SIZE * sizeof(char), format, time );
	else
		sprintf_s( buf, STR_BUF_SIZE * sizeof(char), format );
	time_str += buf;
}

// Show Times to perform various steps
// Don't call this if you do not want any Times shown
void	ShowTiming( string & time_str, 
					const time_t timeStart, const time_t timeEnd, 
					const bool show_total_only, 
					const bool doDiff, const bool doDups )
{
	int		diffSeconds;
	char	buf[STR_BUF_SIZE];

	if ( false == show_total_only )
	{
		TimeMsg( buf, time_str, "\n     Processing  Step     :  seconds\n", -1 );
		TimeMsg( buf, time_str,   "------------------------- : --------\n", -1 );

		diffSeconds = (int)( difftime( time_end_list_built, timeStart ) + 0.5 );	// round to nearest second
		TimeMsg( buf, time_str, " Build file list          : %5d\n", diffSeconds );

		// Show the Read and Analyze times
		if ( ! g_process_after_read )
		{
			diffSeconds = (int)( difftime( time_end_files_read, time_end_list_built ) + 0.5 );
			if ( doDiff )
			{
				TimeMsg( buf, time_str, "  Reading A files         : %5d\n", diffSeconds );

				diffSeconds = (int)( difftime( time_end_files_readB, time_end_files_read ) + 0.5 );
				TimeMsg( buf, time_str, "  Reading B files         : %5d\n", diffSeconds );

				diffSeconds = (int)( difftime( time_end_files_analysis, time_end_files_readB ) + 0.5 );
				TimeMsg( buf, time_str, " Analyze A files          : %5d\n", diffSeconds );

				diffSeconds = (int)( difftime( time_end_files_analysisB, time_end_files_analysis ) + 0.5 );
				TimeMsg( buf, time_str, " Analyze B files          : %5d\n", diffSeconds );
			}
			else
			{
				TimeMsg( buf, time_str, "  Reading files           : %5d\n", diffSeconds );

				diffSeconds = (int)( difftime( time_end_files_analysis, time_end_files_read ) + 0.5 );
				TimeMsg( buf, time_str, "  Analyze files           : %5d\n", diffSeconds );
			}
		}
		else
		{	// Reading with Analysis following immediately on per file basis
			// instead of Reading going through list then start Analyze through list.
			diffSeconds = (int)( difftime( time_end_files_analysis, time_end_list_built ) + 0.5 );
			if ( doDiff )
			{
				TimeMsg( buf, time_str, "Read, Analyze & Count A   : %5d\n", diffSeconds );

				diffSeconds = (int)( difftime( time_end_files_analysisB, time_end_files_analysis ) + 0.5 );
				TimeMsg( buf, time_str, "Read, Analyze & Count B   : %5d\n", diffSeconds );
			}
			else
				TimeMsg( buf, time_str, "Read, Analyze & Count     : %5d\n", diffSeconds );
		}

		if ( doDiff )
		{
			diffSeconds = (int)( difftime( time_end_match_baselines, time_end_files_analysisB ) + 0.5 );
			TimeMsg( buf, time_str, " Match baselines          : %5d\n", diffSeconds );
		}
		else
			time_end_match_baselines = time_end_files_analysis;

		if ( doDups )
		{
			diffSeconds = (int)( difftime( time_end_find_duplicates, time_end_match_baselines ) + 0.5 );
			if ( doDiff )
				TimeMsg( buf, time_str, "   Find A duplicates      : %5d\n", diffSeconds );
			else
				TimeMsg( buf, time_str, " Find duplicates          : %5d\n", diffSeconds );
		}
		else
			time_end_find_duplicates = time_end_match_baselines;

		diffSeconds = (int)( difftime( time_end_print_results, time_end_find_duplicates ) + 0.5 );
		if ( ! doDiff )
			TimeMsg( buf, time_str, "  Generate results files  : %5d\n", diffSeconds );

		if ( doDiff )
		{
			// Differencing steps only
			TimeMsg( buf, time_str, " Generate results A files : %5d\n", diffSeconds );

			if ( doDups )
			{
				diffSeconds = (int)( difftime( time_end_find_duplicatesB, time_end_print_results ) + 0.5 );
				TimeMsg( buf, time_str, "   Find B duplicates      : %5d\n", diffSeconds );
			}
			else
				time_end_find_duplicatesB = time_end_print_results;

			diffSeconds = (int)( difftime( time_end_print_resultsB, time_end_find_duplicatesB ) + 0.5 );
			TimeMsg( buf, time_str, " Generate results B files : %5d\n", diffSeconds );

			diffSeconds = (int)( difftime( time_end_match_baselines_web, time_end_print_resultsB ) + 0.5 );
			TimeMsg( buf, time_str, " Match web baselines      : %5d\n", diffSeconds );

			diffSeconds = (int)( difftime( time_end_process_pairs, time_end_match_baselines_web ) + 0.5 );
			TimeMsg( buf, time_str, " Files comparison         : %5d\n", diffSeconds );

			diffSeconds = (int)( difftime( time_end_print_resultsLast, time_end_process_pairs ) + 0.5 );
			TimeMsg( buf, time_str, " Generate final results   : %5d\n", diffSeconds );
		}	//	END  if  doDiff

	}	//	END		! show_total_only

	// Show the elapsed time
	diffSeconds = (int)( difftime( timeEnd, timeStart ) + 0.5 );
	if ( diffSeconds > 60 )
	{
		if ( diffSeconds >= 120 )
		{
			// Variable minutes
		TimeMsg( buf, time_str, "               Total time : %d minutes",
					(diffSeconds / 60) );
			// seconds
			TimeMsg( buf, time_str, " %d seconds\n", (diffSeconds % 60) );
		}
		else
		{
		TimeMsg( buf, time_str, "               Total time : 1 minute %d seconds\n",
					(diffSeconds % 60) );
		}
	}
	else
		TimeMsg( buf, time_str, "               Total time : %5d seconds\n", diffSeconds );

}


/*!
* Main function.
*
* \param argc number of arguments
* \param argv argument list
*
* \return function status
*/
int main(int argc, char *argv[])
{
	int				retVal = EXCEPTION_DID_NOT_HAPPEN;	// 0 for backwards compatability
	string			exception_msg;
	bool			doDiff = false;
	bool			doDups = false;
	double			duplicate_threshold_used = 0.0;
	unsigned int	files_A_count = 0;
	unsigned int	files_B_count = 0;

	CUtil::InitToLower();

#ifdef _DEBUG
	// Wait for User input 
	// to allow Debugger to be attached to this running process
	//
	// Suggest checking for Memory Leaks running Debug build...
	// This is also a good place to note the amount of memory allocated here.
	char keyBuffer[256];

	printf( "Here is where you can attach a Debugger." );
	printf( "\nHit Enter key to continue running in UCC main()\n" );

	char * pChar = gets( keyBuffer );
#endif

	// Now that UCC is running without any User input 
	// capture the start of the total running time
	time_t	timeStart;
	time_t	timeEnd;

	// Change these as you wish
	bool	show_any_times = true;
	bool	show_total_only = false;

	time( &timeStart );
	timeEnd = timeStart;

	try
	{
		// determine whether differencing functionality is needed
		string myArg;
		for (int i = 0; i < argc; i++)
		{
			myArg = argv[i];
			if (myArg == "-d")
			{
				doDiff = true;
				break;
			}
			else if (myArg == "?" || myArg == "-help" || myArg == "-h" || myArg == "/?")
			{
				// check whether a help option has been specified
				if (argc > i + 1)
				{
					// show usage option help and Exit
					MainObject::ShowUsage(argv[i+1]);
				}		
				else
				{
					// show general usage help and Exit
					MainObject::ShowUsage();
				}
			}
		}

		if ( doDiff )
		{
			// run the DiffTool to include differencing
			DiffTool diffTool;
			diffTool.diffToolProcess(argc, argv);
			duplicate_threshold_used = diffTool.GetDuplicateThreshold();
			files_A_count = SourceFileA.size();
			files_B_count = SourceFileB.size();
		}
		else
		{
			// if no differencing functionality required, just call the UCC
			MainObject mainObject;
			mainObject.MainProcess(argc, argv);
			duplicate_threshold_used = mainObject.GetDuplicateThreshold();
			files_A_count = SourceFileA.size();
		}
	}
	// Handle any Exceptions to get as much context info as possible
	// (in a cross platform way).
	// On Windows for example there is an another entire set 
	// (a LOT more than those listed below)
	// of Windows OS related Exceptions that may be thrown.
	// Perhaps the system_error handler below will cover them OK.
	//
	// Idea for this chain of Exception handlers was from cppreference.com docs.
	// More Exception types were found in <stdexcept> module.
	// If you find any that dont compile please just comment out those.
	catch(const std::overflow_error& e) 
	{
		// this executes if above throws std::overflow_error (same type rule)
		retVal = EXCEPTION_OVERFLOW_ERROR;
		exception_msg = "Error: there was an Overflow error exception.  Please try again.\n";
		exception_msg += e.what();
	}
	catch(const std::underflow_error& e)
	{
		// this executes if above throws std::underflow_error (base class rule)
		retVal = EXCEPTION_UNDERFLOW_ERROR;
		exception_msg = "Error: there was an Underflow error exception.  Please try again.\n";
		exception_msg += e.what();
	}
	catch(const std::range_error& e)
	{
		// this executes if above throws std::range_error
		retVal = EXCEPTION_RANGE_ERROR;
		exception_msg = "Error: there was a Range error exception.  Please try again.\n";
		exception_msg += e.what();
	}
	catch(const std::system_error& e)
	{
		// this executes if above throws std::system_error
		retVal = EXCEPTION_SYSTEM_ERROR;
		exception_msg = "Error: there was a System error exception.  Please try again.\n";
		exception_msg += e.what();
	}
	catch(const std::runtime_error& e) 
	{
		// this executes if above throws std::runtime_error (base class rule)
		retVal = EXCEPTION_RUNTIME_ERROR;
		exception_msg = "Error: there was a Runtime error exception.  Please try again.\n";
		exception_msg += e.what();
	} 
	catch(const std::domain_error& e) 
	{
		// this executes if above throws std::domain_error (base class rule)
		retVal = EXCEPTION_DOMAIN_ERROR;
		exception_msg = "Error: there was a Domain error exception.  Please try again.\n";
		exception_msg += e.what();
	} 
	catch(const std::length_error& e) 
	{
		// this executes if above throws std::length_error (base class rule)
		retVal = EXCEPTION_LENGTH_ERROR;
		exception_msg = "Error: there was a Length error exception.  Please try again.\n";
		exception_msg += e.what();
	} 	
	catch(const std::invalid_argument& e) 
	{
		// this executes if above throws std::invalid_argument (base class rule)
		retVal = EXCEPTION_INVALID_ARG_ERROR;
		exception_msg = "Error: there was an Invalid Argument error exception.  Please try again.\n";
		exception_msg += e.what();
	} 		
	catch(const std::out_of_range& e) 
	{
		// this executes if above throws std::out_of_range (base class rule)
		retVal = EXCEPTION_OUT_OF_RANGE_ERROR;
		exception_msg = "Error: there was a Out of Range error exception.  Please try again.\n";
		exception_msg += e.what();
	} 
	catch(const std::logic_error& e) 
	{
		// this executes if above throws std::logic_error (base class rule)
		retVal = EXCEPTION_LOGIC_ERROR;
		exception_msg = "Error: there was a Logic error exception.  Please try again.\n";
		exception_msg += e.what();
	} 
	catch(const std::exception& e) 
	{
		// this executes if above throws std::exception (base class rule)
		retVal = EXCEPTION_STD_EXCEPTION;
		exception_msg = "Error: there was a std namespace error exception.  Please try again.\n";
		exception_msg += e.what();
	} 
	catch(...) 
	{
		// this executes if above throws std::string or int or any other unrelated type
		retVal = EXCEPTION_NOT_SPECIFIC;		// Some kind of severe error that prevented normal finish
		exception_msg = "Error: a general exception occurred. Please try again.";
	}

	if ( retVal )	// Non Zero if an Exception was caught
	{
		printf( "\n%s\n", exception_msg );
		if ( workThreadsCount > 1 )
		{
			printf( "Because %d extra threads were also running, it is possible that\n", workThreadsCount );
			printf( "this Exception happened due to a side effect from running extra threads.\n" );
			printf( "As a workaround you should try using fewer threads or UCC -threads 1\n" );
		}
	}

	if ( g_thread_exception_type )
	{
		// If any Thread Exception info gets to here it means that the thread related code
		// did not gracefully recover from the Exception.  Let the User know about it.
		printf( "\nWarning: there was an Exception that happened in 1 or more threads.\n" );
		printf( "It is likely that the thread(s) did not fully finish the task.\n" );
		printf( "So results may be missing files or be incomplete in other ways.\n" );
		printf( "As a workaround you could try using fewer threads or UCC without extra threads.\n" );
		printf( "The Exception type code is: %d (see UCCGlobals.h and UCCThreads.cpp).\n", g_thread_exception_type );
		printf( "%s\n", g_thread_exception_msg );
	}

	// Get the Time to show
	time( &timeEnd );
	double	total_seconds = difftime( timeEnd, timeStart ) + 0.5;

	string	time_str;
	char buf[STR_BUF_SIZE];
	strcpy_s( buf, STR_BUF_SIZE * sizeof(char), "" );

	if ( doDiff )
	{
		TimeMsg( buf, time_str, " A Files: %d  ", files_A_count );
		TimeMsg( buf, time_str, " B Files: %d", files_B_count );
	}
	else
		TimeMsg( buf, time_str, " %d files", files_A_count );
	
	if ( 1.0 > total_seconds )
		total_seconds = 1.0;		// AVOID divide by Zero

	strcpy_s( buf, STR_BUF_SIZE * sizeof(char), "" );
	sprintf_s( buf, STR_BUF_SIZE * sizeof(char), " for %.1f files processed per second\n", 
					( (double)( files_A_count + files_B_count )/total_seconds ) + .05 );
	time_str += buf;

	if ( duplicate_threshold_used >= 0.0 )
		doDups = true;

	// show the Times of various steps and total elapsed Time if wanted
	if ( true == show_any_times )
		ShowTiming( time_str, timeStart, timeEnd, show_total_only, doDiff, doDups );

	if ( time_str.size() )
	{
		cout << time_str;
		// Here would be a place to also write the Times to a file
	}

#ifdef	_DEBUG
// Show some internal profiling that could be followed up
//	extern unsigned int calls_FindModifiedLines;

//	printf( "\nFindModifiedLines was called %d times.\n", calls_FindModifiedLines );
#endif

#ifdef	_DEBUG
	// Wait for User input
	// This prevents the console window (running on Windows) from closing 
	// and therefore not allowing the User to know the total running time
	//
	// Suggest checking for Memory Leaks running Debug build...
	// This is also a good place to compare the amount of memory allocated
	// on startup with what remains allocated here. See calls to HeapInUse
	printf( "\nFinished.  Hit Enter key and this will exit.\n" );

	char keyBuffer2[256];

	pChar = gets( keyBuffer2 );
#endif

	return retVal;
}
