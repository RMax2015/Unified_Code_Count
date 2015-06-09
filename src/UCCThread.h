//! UCC Thread support
/*!
* \file UCCThread.h
*
* This file encapsulates thread implementation details and dependencies
* so that the rest of UCC is relatively unchanged.
*
* ADDED to UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_06
*   Addition of Multithreading performance enhancement feature
*		Example: UCC -threads 4 (to have 4 worker threads)
*/

#ifndef UCC_THREAD_PUBLIC_H
#define UCC_THREAD_PUBLIC_H

#include "UCCGlobals.h"

#include "cc_main.h"

// It is really bad practice to let threads access the UI
// In this case the thread will be the main thread
#include "UserIF.h"

//
//		These have been relocated here to support thread interfaces
//

// Convert an int to a std::string
void	IntToStr( const int val, string & result );

#ifdef	_DEBUG
	// This does a quick? survey of the Heap giving number of blocks and total committed sizes
	// Implemented for Windows
	void	HeapInUse( int & errorRet, unsigned int & block_count, unsigned long & total_sizes, 
					bool & validate, const bool in_use = true );
#endif

//
//		These are the Public interfaces for UCC threads.
//

	#define MIN_UCC_THREAD_COUNT	2
	#define MAX_UCC_THREAD_COUNT	(MIN_UCC_THREAD_COUNT+78)

	#define	THREADS_STARTED_OK	1

	// Minimum thread sleep time in milliseconds
	// Amount of actual time spent asleep is OS/HW and workload dependant.
	// Below is about 1/100 of a second
	#define		MIN_SLEEP_TIME		10
	#define		ONE_SECOND		1000

// Give up the rest of the time slice this thread has and
// Put the currently executing thread to sleep for approximately the time wanted
void ThreadSleep( const unsigned int milliSeconds = MIN_SLEEP_TIME );

/*!
*			StartWorkerThreads
* Set up and Start a number of worker threads.
* Any threads started will be waiting for either work to do or to exit thread if wanted.
*
* \param num_to_start	IN/OUT		ref to number of worker threads wanted to start
* \param message		IN/OUT		ref to message giving details for number actually started
*
* \returns							THREADS_STARTED_OK if number of threads wanted was started OK
*							or		positive number >= 2 giving number of threads actually started
*										message has INFO details
*							or		negative number giving Severe unrecoverable Error code
*										message has ERROR details
*
*	It is the calling code's responsibility to check for Errors.
*
* Recommended actions for Severe unrecoverable Errors: 
*		Log the ERROR with all given details
*		Continue running in single thread mode without any worker threads
*
* Recommended actions for the number of threads actually started being lower than wanted:
*		Log an INFO message about wanted and actual worker thread count
*		Go ahead and use the availble (smaller number) of worker threads
*/
int StartWorkerThreads( CounterForEachLangType * pCounterForEachLanguage, 
						UserIF			*		userIF,
						const unsigned int num_to_start, string & message );

/*!
*			FinishWorkerThreads
* Finish any running worker threads.
* This may block for a few seconds while worker threads exit if they were busy.
*/	
void FinishWorkerThreads();


// Propagate back the results from recently finished worker threads
void CombineThreadResults();


// Return the number of threads working
unsigned int NumThreadsBusy( unsigned int & count_done, 
						const unsigned int	sleep_milliseconds = 100, 
						const bool			combine_results_from_threads = true );

// ReadFilesInList
// returns number of Errors found
int ReadFilesInList( const unsigned int					threadIdx,
					UserIF					*			userIF,
					CounterForEachLangType		&		CounterForEachLanguage,
					const		bool					print_cmplx,
					const		bool					print_csv,
					std::vector<std::string>::iterator itStart, 
					std::vector<std::string>::iterator itEnd,
					const		bool					useListA,
					const		bool					clearCaseFile,
					SourceFileList			*			mySrcFileList,
					SourceFileList			*			mySourceFileA,
					SourceFileList			*			mySourceFileB,
					string						&		errList,
					string								outDir,
					ErrMsgStructVector			&		err_msgs,
					UncountedFileStructVector	&		unc_files,
					const		bool					process_after_read = false,
					const		bool					discard_lines_after_process = false );

// Analyze and do Counts for a single Source File
void ProcessSourceListFile( const unsigned int threadIdx, UserIF * userIF, 
							CounterForEachLangType		&	CounterForEachLanguage,
							CCodeCounter				**	ppCounter,
							SourceFileList::iterator		i,
							SourceFileList				*	mySourceFileA,
							SourceFileList				*	mySourceFileB,
							unsigned int				&	num_inserted,
							ErrMsgStructVector			&	err_msgs,
							UncountedFileStructVector	&	unc_files,
							string						&	errList,
							const bool						useListA = true );

// Divide a List of Files among multiple Threads and start them working
int ReadFilesThreads( const unsigned int				num_files,	// number of Files in the list
					UserIF					*			userIF,
					const		bool					print_cmplx,
					const		bool					print_csv,
					std::vector<std::string>::iterator itStart,		// Start of entire list to divide
					std::vector<std::string>::iterator itEnd,		//  End  of entire list to divide
					const		bool					useListA,
					const		bool					clearCaseFile,
					SourceFileList			*			mySrcFileList,
					string								outDir,
					const		bool					process_after_read = false,
					const		bool					discard_lines_after_process = false );

/*!
*			ThreadAddFileToRead		COMMENTED OUT
* Request a thread to read in the Physical lines of a given named file.
* This will return usually before the thread finishes reading.  It is NOT synchronous.
*
* This is not efficient for doing many files but there may be a case later
* on other tasks in UCC not currently multithreaded that could use this.
*
*
* Call with an empty file name means the caller will wait until 
* ALL file reading THREADS HAVE FINISHED reading all requested files.
*
* \param fileName			IN		Name of a file to request reading.  If empty name, this waits as above.
* \param trim_line			IN		True to trim whitespace from a line else false
* \param pSrcFileElement	IN/OUT	Pointer to a SourceFileElement structure to change
* \param error_count		IN/OUT	Reference to int to tell about prior errors
*
* \returns	number of Requests that are pending (includes Requests already assigned to threads)
*			The actual number of pending requests MAY be higher than the return value
*/
/*
	unsigned int ThreadAddFileToRead( const string fileName, const bool trim_line, 
									SourceFileElement * pSrcFileElement,
									unsigned int & error_count );
*/

#endif	//	END		#ifndef	UCC_THREAD_PUBLIC_H
