//! UCC multiple work Threads support implementation
/*!
* \file UCCThread.cpp
*
* This file encapsulates thread implementation details and dependencies
* so that the rest of UCC is relatively unchanged.
*
* Added to UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_06
*   Addition of Multithreading performance enhancement 
*
*		Example: UCC -threads 4 (and other UCC args)
*
* The cross platform thread feature and some related interfaces are from Boost C++ library (Thread)
* See the file BOOST_LICENSE_1_0.txt in this directory or at
* http://www.boost.org/LICENSE_1_0.txt
*
* Treat the contents of this file as PRIVATE and do not casually move to other files.
* All items with prv_ prefix should stay as PRIVATE unless you have a very strong reason.
*
* See comments at the end of this file for some Design and Analysis details.
* See UCC_Multithreading_Notes.doc for some older details of how the code got this way.
*/

// Uncomment this to get 2 or more worker threads supported.  Requires Boost libarary.
// #define		ENABLE_THREADS

#ifdef	_DEBUG
	// Debug utilities to support checking the Heap.  Implemented for Windows
	#ifdef	WIN32
		#define		WIN32_LEAN_AND_MEAN
		#include	<Windows.h>
		#undef		WIN32_LEAN_AND_MEAN

		// Windows defines PASCAL as a legacy call interface 
		// so undefine it here so we can use PASCAL as a Language name
		#ifdef	PASCAL
			#undef	PASCAL
		#endif
	#else
		// Add support for another OS here if wanted

	#endif
#endif	// _DEBUG

#include <iostream>
#include <sstream>
using namespace std;

#ifdef	ENABLE_THREADS
	#include <boost/thread/thread.hpp>
	#include <boost/thread/mutex.hpp>

	// Enable this to try out cross platform Memory Mapped files
	// Initial testing showed that Memory Mapped files were SLOWER
	// This is left available if you want to experiment...
	// #define		USE_MEMORY_MAPPED_FILES

	#ifdef		USE_MEMORY_MAPPED_FILES

		//#include <boost/interprocess/file_mapping.hpp>
		//#include <boost/interprocess/mapped_region.hpp>
		#include <boost/iostreams/stream.hpp>
		#include <boost/iostreams/device/mapped_file.hpp>

		// Tried using boost/interprocess for Semaphore but had side effect 
		// on Windows OS where expected blocking of work thread 
		// still had polling in the Interprocess library code going on.
		// After reading the Boost code the polling behavior is clear (and unwanted in this case).
		//
		// TODO: Provide feedback to boost Threads and Interprocess library maintainers:
		// 1) It would be nice if there was a constructor arg or define 
		// to allow use of native semaphores (especially on Windows)
		//
		// 2) IF implementing 1) is problematic perhaps some approach like that used by
		// the cross platform Semaphore library
		// 
		//
		// #include <boost/interprocess/sync/interprocess_semaphore.hpp>

		// using namespace boost::interprocess;
		using namespace boost::iostreams;
	#endif

	using namespace boost;
	using namespace boost::this_thread;

#endif


#include "CUtil.h"
#include "LangUtils.h"

// Get Public Thread interfaces
#include "UCCThread.h"

#ifdef		ENABLE_THREADS
	//		Get cross platform Semaphore support
	// Uncomment this define if your compiler has C++ 2011 support AND
	// you want to make some changes to how semaphores are used.
	//
	// #define	_CPP_2011_AVAILABLE_
	#include "sema.h"
#endif

// USE_LESS_MEM_ON_COPY  Enabled as default.
// Disable this IF you are sure UCC threads have enough Memory 
// to do somewhat Faster but use MUCH more temporary Memory while copying.
//
// Tradeoffs
// Enabled, several MB to GB or more of temporary memory is not needed (depends on Data size).
// Disabled, you reduce runtime by a some seconds (depends on Data size) at a greater risk of running out of memory.
//
// Disabled USE_LESS_MEM_ON_COPY example: Single thread (original or -threads 1) needs 4 GB to build the file list.
// Using 2 threads:  Each thread needs 2 GB.  When ready to copy back result for first thread,
// there would be a need for an extra 2 GB or 6 GB total at one time for success to advance to next step.
#define		USE_LESS_MEM_ON_COPY


////////////////////////////////////////////////////////////////////////
//
//		P R I V A T E		D A T A		a n d		P R O C E D U R E S
//
///////////////////////////////////////////////////////////////////////

// Keep what is defined here from polluting other namespaces
//namespace _prv_UCC_THREAD_NAMESPACE_
//{

#ifdef		ENABLE_THREADS

	/*!
	* \enum ThreadState
	*
	* Enumeration of the states of a single worker thread.
	* Only a worker thread may WRITE these states.
	*/
	enum ThreadState {
		THREAD_WAITING_FOR_WORK = 1,	// Initial state and also after completing some work
		THREAD_WORKING,					// Actively doing some wanted work
		THREAD_HAS_EXITED,				// LAST value written just before a thread exits
	};

	/*!
	* \enum ThreadActionsWanted
	*
	* Enumeration of the actions of a worker thread wanted by the main thread.
	* Only the main thread may WRITE these actions that are wanted.
	*/
	enum ThreadActionsWanted {
		NO_WORK_YET = 10,		// No work for the thread yet
		GET_TO_WORK,			// Worker thread has enough data to do some work
		STOP_WORKING,			// Stop working even if not finished with work (User Cancelled?)
		EXIT_NOW,				// Ask the worker thread to exit
	};

	//! Define a Structure to encapsulate the status of a worker thread
	/*!
	* \struct _prv_threadStatus
	*
	* This also allows very controlled access by both the main thread and the owning worker thread
	*/
	struct _prv_threadStatus
	{
		// There is 1 access rule: You can not change what I WRITE (you just have READ ONLY access).
		// A value has only 1 WRITER associated with it (either main thread or worker thread side).
		// Ownership (who can WRITE) is part of the entity name (as a prefix) main_ or work_ 

		volatile	ThreadState				work_ThreadState;	//!< State of this worker thread
		volatile	ThreadActionsWanted		main_WantsAction;	//!< Action that main thread wants
					unsigned int			thread_idx;			//!< Index in array for this worker
					boost::thread::id		threadId;			//!< thread ID found after thread created

		Semaphore					*		pSem;				//!< Semaphore used to prevent polling

		CounterForEachLangType		*		pCounterForEachLanguage;	//!< 

		// Info flag about a thread having started
		volatile	bool					thread_has_started;

		// Info about specific work for the thread to do
		//
		volatile	bool					thread_work_done;	//!< true when just completed some work
		volatile	bool					thread_had_error;	//!< true if there was an error

		// Possible Optimizations to use
					bool					process_after_read;	//!< After Reading a File into memory, process it
					bool					discard_lines_after_process;	//!< After Processing, del PSLOC

					unsigned int			thread_work_count_done;
		// WORK: Read a List of Files (variable names same as rest of code)
					UserIF			*		userIF;
		std::vector<std::string>::iterator	itStart;
		std::vector<std::string>::iterator	itEnd;
					bool					print_cmplx;
					bool					print_csv;
					bool					useListA;
					bool					clearCaseFile;
					SourceFileList	*		mySrcFileList;
					SourceFileList			SourceFileA;		//!< List of source files in baseline A
					SourceFileList			SourceFileB;		//!< List of source files in baseline B
					string					errList;
					string					outDir;

		// Cache for Error messages, Uncounted File info, etc. so multiple threads don't pound on UI
		ErrMsgStructVector					thread_err_msgs;	//!< OUT Used if needed for any from thread_idx >= 1
		UncountedFileStructVector			thread_unc_files;	//!< OUT Used if needed for any from thread_idx >= 1

		// WORK: Read Physical lines of a single file
					bool					main_trim_line;		//!< true is default, false for Fortran and Python
	};

	void	_prv_Init_threadStatus( _prv_threadStatus & rThStatus )
	{
		// Initialize values so when a worker thread takes ownership
		// the worker thread is in a predictable state.
		rThStatus.work_ThreadState	= THREAD_WAITING_FOR_WORK;
		rThStatus.main_WantsAction	= NO_WORK_YET;
		rThStatus.thread_idx		= MAX_UCC_THREAD_COUNT + 1;		//!< Start with invalid index

		// No good way to initialize threadId here
		rThStatus.pSem = NULL;						//!< Semaphore will be created later with new
		rThStatus.pCounterForEachLanguage = NULL;

		rThStatus.thread_has_started = false;

		rThStatus.thread_work_done = false;
		rThStatus.thread_had_error = false;

		// Possible Optimizations to use
		rThStatus.process_after_read          = false;
		rThStatus.discard_lines_after_process = false;
		rThStatus.thread_work_count_done = 0;
		rThStatus.userIF           = NULL;
		// No good way to initialize itStart or itEnd here
		rThStatus.print_cmplx      = false;
		rThStatus.print_csv        = false;
		rThStatus.useListA         = true;
		rThStatus.clearCaseFile    = false;
		rThStatus.mySrcFileList    = NULL;
		rThStatus.thread_work_count_done = 0;
		rThStatus.SourceFileA.clear();
		rThStatus.SourceFileB.clear();
		rThStatus.outDir           = "";
		rThStatus.thread_err_msgs.clear();
		rThStatus.thread_unc_files.clear();
		rThStatus.main_trim_line   = true;
	}


////////////////////////////////////////////////////////////////////////////////
//
// Declare some global values (inside this namespace) that have a lifetime
// Longer than any given public procedure.
// Could use a C++ singleton class to do this maybe...
//
	// Number of threads assigned to work
	int		g_assigned_count = 0;

	// Used to prevent (hopefully very rare) race conditions between these threads.
	boost::mutex g_exclusive_Mutex;

// Create empty arrays of needed long lived values
	_prv_threadStatus	g_threads_status[ MAX_UCC_THREAD_COUNT ];
	unsigned int		g_threads_status_in_use = 0;

	CounterForEachLangType * g_thread0_CounterForEachLanguage = NULL;

	UserIF		*		g_userIF = NULL;

	// May not be needed ?
	string				g_threads_file_names[ MAX_UCC_THREAD_COUNT ];

	// May not be needed ?
	SourceFileElement * g_threads_pSrcFileElement[ MAX_UCC_THREAD_COUNT ];

	// Thread ID of main thread that calls into public procedures
	boost::thread::id 	g_main_thread_id;

//================================================================================
//
//			Thread	Functions
//
//================================================================================

void prv_WorkThread_Function( const unsigned int thr_array_idx )
{
	if ( thr_array_idx >= MAX_UCC_THREAD_COUNT )
	{
		// SEVERE ERROR of thread interface
		cout << endl << "Error: prv_WorkThread_Function called with invalid thr_array_idx" << endl;
		//throw std::runtime_error("Error: prv_WorkThread_Function called with invalid thr_array_idx");
		return;
	}

	// Exceptions within this thread
	int		thread_exception_type = EXCEPTION_DID_NOT_HAPPEN;
	string	thread_exception_msg;

	// Make a pointer to a thread status struct 
	// so when values in the struct change later this code will see them.
	_prv_threadStatus * pStatus = &(g_threads_status[ thr_array_idx ]);

	bool	need_to_wait		= true;
	bool	have_done_work		= false;
	bool	done_with_thread	= false;

	// Variables for Reading Physical lines
	string			srcFileName;
	string			oneline;
	// lineElement	element();	// do NOT use by itself, causes MEMORY LEAKS see below for better way

	CounterForEachLangType		myCounterForEachLanguage;	// Declare here so can be Release later

// Protect main thread and other threads from any exceptions
try 
{
	//		Very Important:
	//
	// Minimize use of outside variables from inside a thread.  
	// This helps prevent "False Sharing" which can defeat the whole purpose of using threads.
	//
	// So create variables here on the Thread's stack where possible

	SourceFileList				mySourceFileA;
	SourceFileList				mySourceFileB;
	SourceFileList			*	mySrcFileList = &mySourceFileA;
	SourceFileList			*	pDestFileList = NULL;

	ErrMsgStructVector			err_msgs;
	UncountedFileStructVector	unc_files;

/*		CRITICAL NOTE for THREAD SAFETY:
While debugging on Win 7.1 using Visual Studio C++ 2010 Express 
(Expect this is common with other Windows OS and VS versions, perhaps other OS/compilers as well.)

Looking at the code in Init_CounterForEachLanguage there is no explicit 
(or implicit that I can tell) use of Global variables = 
which is where a lot of threaded code has problems.

The thread initialization code (2 threads) deadlocked trying to acquire a Mutex.  ? ! ?
Implemented several layers down from the new operator in Microsoft code.
Debugger shows where one Thread blocked on a new for HTML class 
and another Thread blocked at at a different new for a different Language class.

Remember that all of the Language classes inherit from the CCodeCounter class.
So that even if the new is being done for apparently 2 different classes; underneath
there will also be new calls to create an instance of CCodeCounter class for each.

So the underlying Microsoft code detected that Init_CounterForEachLanguage was being
called by one thread when the OS thread scheduler switched to the other thread 
thereby allowing the race condition to the underlying Microsoft code Mutex in question. 

By extension, any time the same classes are created dynamically by 2 or more threads
	-- EVEN IF the memory (stack of each thread) is different 
		from where the Class instances will be created --
there is a potential for extra excitement.

TO help reduce the excitement, try to minimize the creation of classes at runtime. 
For example a LOT of std::string classes are return values in UCC code.
	Each time a temporary string is created and then destroyed.
	Consider adding an arg that is a reference to an already existing string 
	for the called function to change as a way to return instead.
	Semantically the 2 approaches are nearly identical but using a reference 
	(or Pointer) to an existing instance of a class 
	is more efficient of both dynamic memory and CPU resources.
This also helps performance.

Careful reading of the code that is run by threads here will show the above approach.
Of couse, I may have missed a few spots...
*/

// Acquire a Mutex to prevent race condition that may deadlock on underlying C++ library Mutex
	g_exclusive_Mutex.lock();

	Init_CounterForEachLanguage( myCounterForEachLanguage );
	ResetCounterCounts( myCounterForEachLanguage );
	SetCounterOptions( myCounterForEachLanguage );

	pStatus->pCounterForEachLanguage = &( myCounterForEachLanguage );

	g_exclusive_Mutex.unlock();

	pStatus->thread_has_started = true;

	// Should have just been created with no work to do
	do
	{
		need_to_wait = false;

		if ( EXIT_NOW == pStatus->main_WantsAction )
		{
			done_with_thread = true;
			break;
		}

		if ( ( NO_WORK_YET  == pStatus->main_WantsAction )
		  || ( STOP_WORKING == pStatus->main_WantsAction ) )
		{
			pStatus->work_ThreadState = THREAD_WAITING_FOR_WORK;
			if ( STOP_WORKING == pStatus->main_WantsAction )
				have_done_work = false;
			
			need_to_wait = true;
		}

		if ( ( GET_TO_WORK == pStatus->main_WantsAction )
		  && ( ( 0 == srcFileName.size() ) && ( NULL == pStatus->userIF ) ) )
		{
			need_to_wait = true;
		}

		if ( true == need_to_wait )
		{
			// Wait on a semaphore until there is some change to wanted action
			pStatus->pSem->wait();

			continue;
		}

		srcFileName = g_threads_file_names[ thr_array_idx ];

		if ( ( GET_TO_WORK == pStatus->main_WantsAction )
		  && ( ( srcFileName.size() ) || ( NULL != pStatus->userIF ) ) )
		{
			// WORK ... hooray !
			//
			// Keep going here until work is done or 
			// STOP_WORKING or EXIT_NOW is wanted  TODO: Add checks for these 2

			pStatus->work_ThreadState = THREAD_WORKING;
			pStatus->thread_work_count_done = 0;
			pStatus->thread_work_done = false;
			pStatus->thread_had_error = false;
			
			SourceFileElement * pSrcFile = g_threads_pSrcFileElement[ thr_array_idx ];
			srcFileName = g_threads_file_names[ thr_array_idx ];

			while ( false == pStatus->thread_work_done )
			{
				if ( NULL != pStatus->userIF )
				{
					// Call helper to Read a List of Files
					// Optimizations to do other processing after Reading are likely

					// Must use internal Lists (on this thread's stack!)
					mySrcFileList = (pStatus->useListA) ? &(mySourceFileA): &(mySourceFileB);
					mySrcFileList->clear();
					err_msgs.clear();
					unc_files.clear();
					pStatus->errList.clear();

					// Set up to Copy back results
					pDestFileList = (pStatus->useListA) ? &(pStatus->SourceFileA): &(pStatus->SourceFileB);
					pDestFileList->clear();		// Empty destination in struct

					int error_count = ReadFilesInList( thr_array_idx, pStatus->userIF, 
													myCounterForEachLanguage, 
													pStatus->print_cmplx, pStatus->print_csv,
													pStatus->itStart, pStatus->itEnd, pStatus->useListA, 
													pStatus->clearCaseFile, 
													mySrcFileList,		// Destination of Files read into memory
													&mySourceFileA,		// Destination if using A
													&mySourceFileB,		// Destination if using B
													pStatus->errList,
													pStatus->outDir,
													err_msgs, unc_files,
													pStatus->process_after_read,
													pStatus->discard_lines_after_process );

					if ( ( thr_array_idx ) 
					  && ( error_count > (int)( pStatus->thread_err_msgs.size() ) ) )
					{
						// More Error counts got returned here than got Saved... strange
						
					}

					// Avoid extra copying of data around, just point to it
					pStatus->mySrcFileList = mySrcFileList;
/*
					// Copy back results to struct
					// 
				#ifdef	USE_LESS_MEM_ON_COPY
					// Minimize use of temporary memory.  
					// This can save several MB or GB or more depending on size of File list and Files
					// Which can make the difference between success with Threads or not.
					// So this is the default approach.
					unsigned int num_to_copy = mySrcFileList->size();
					for ( unsigned int j = 0; j < num_to_copy; j++ )
					{
						pDestFileList->push_back( *(mySrcFileList->begin()) );	// Append element to end of list
						mySrcFileList->pop_front();		// Get rid of element just copied
					}
				#else
					// Use Faster but temporarily uses much MORE memory approach
					pDestFileList->insert( pDestFileList->begin(), mySrcFileList->begin(), mySrcFileList->end() );
				#endif
					mySrcFileList->clear();	// Release memory in thread stack
*/
					// These are typically MUCH smaller than the File List structs, so use Faster approach.
					// Change this as above if you want to be even more conservative of temporary Memory use.
					pStatus->thread_err_msgs = err_msgs;
					err_msgs.clear();
					pStatus->thread_unc_files = unc_files;
					unc_files.clear();

					pStatus->thread_work_done = true;
					have_done_work = true;
					g_threads_file_names[ thr_array_idx ] = "";
					pStatus->userIF = NULL;
					pStatus->work_ThreadState = THREAD_WAITING_FOR_WORK;
					break;
				}

			/////////////////////////////////////////////////////////////////////////////
				//pStatus = &(g_threads_status[ g_array_idx ]);
				if ( srcFileName.size() )
				{
					// WORK: Read the Physical lines from a source file
					//
					// Open the file
					ifstream fr( srcFileName.c_str(), ios::in );
					if (!fr.is_open())
					{
						pStatus->thread_had_error = true;
						pSrcFile->second.e_flag = true;
						pSrcFile->second.error_code = "Unable to open file";
						pStatus->thread_work_done = true;
						have_done_work = true;
						g_threads_file_names[ thr_array_idx ] = "";
						pStatus->work_ThreadState = THREAD_WAITING_FOR_WORK;
						continue;
					}

					if ( ( EXIT_NOW     == pStatus->main_WantsAction )
					  || ( STOP_WORKING == pStatus->main_WantsAction ) )
					{
						pStatus->thread_work_done = true;
						have_done_work = false;		// to use longer sleep time
						g_threads_file_names[ thr_array_idx ] = "";
						fr.clear();
						fr.close();
						pStatus->work_ThreadState = THREAD_WAITING_FOR_WORK;
						continue;
					}

					// Read the Physical lines
					// File opened OK, read each Physical line into memory
					unsigned int lineNum = 0;
					bool lineTooLong = false;

					while (fr.good() || fr.eof())
					{
						getline(fr, oneline);
						if ((!fr.good() && !fr.eof()) || (fr.eof() && oneline.empty()))
							break;

						if (oneline.length() > MAX_LINE_LENGTH)
						{
							lineTooLong = true;
							break;
						}
						lineNum++;
						if ( pStatus->main_trim_line )
							oneline = CUtil::TrimString( oneline );
						if ( oneline.size() )
							oneline = CUtil::ReplaceSmartQuotes( oneline );

						// Use lineElement inside the call to push_back to prevent memory leaks
						// This will show up for a Debug build but may not show for a Release build
						// as (I am guessing) the code Optimization will move the call inside as below.
						pSrcFile->first.push_back( lineElement( lineNum, oneline ) );

						if (!fr.good())
							break;

						// Check after each line if main thread wants worker to stop
						if ( ( EXIT_NOW     == pStatus->main_WantsAction )
						  || ( STOP_WORKING == pStatus->main_WantsAction ) )
						{
							pStatus->thread_work_done = true;
							have_done_work = false;		// to use longer sleep time
							g_threads_file_names[ thr_array_idx ] = "";
							// fr.clear(); done below
							// fr.close(); done below
							// rStatus.work_ThreadState = THREAD_WAITING_FOR_WORK;
							break;
						}
					}
					fr.clear();
					fr.close();

					if (lineTooLong)
					{
						pStatus->thread_had_error = true;
						pSrcFile->second.e_flag = true;
						pSrcFile->second.error_code = "Line too long";

						// Fall through
					}

					pStatus->thread_work_done = true;
					g_threads_file_names[ thr_array_idx ] = "";
					pStatus->work_ThreadState = THREAD_WAITING_FOR_WORK;
					break;
				}	//	END		Read Physical lines from source file
				else
				{
					// No name given
					pStatus->thread_work_done = true;
					pStatus->main_WantsAction = NO_WORK_YET;
					pStatus->work_ThreadState = THREAD_WAITING_FOR_WORK;
				}

				//
				// Check for another kind of work
				// 

			}
		}	//	END of WORK for thread
	
	}
	while ( false == done_with_thread );
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
	// If you find any that do not compile please just comment out those.
catch(const std::overflow_error& e) 
{
	// this executes if above throws std::overflow_error (same type rule)
	thread_exception_type = EXCEPTION_OVERFLOW_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was an Overflow error exception.\n";
	thread_exception_msg += e.what();
}
catch(const std::underflow_error& e)
{
	// this executes if above throws std::underflow_error (base class rule)
	thread_exception_type = EXCEPTION_UNDERFLOW_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was an Underflow error exception.\n";
	thread_exception_msg += e.what();
}
catch(const std::range_error& e)
{
	// this executes if above throws std::range_error
	thread_exception_type = EXCEPTION_RANGE_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was a Range error exception.\n";
	thread_exception_msg += e.what();
}
catch(const std::system_error& e)
{
	// this executes if above throws std::system_error
	thread_exception_type = EXCEPTION_SYSTEM_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was a System error exception.\n";
	thread_exception_msg += e.what();
}
catch(const std::runtime_error& e) 
{
	// this executes if above throws std::runtime_error (base class rule)
	thread_exception_type = EXCEPTION_RUNTIME_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was a Runtime error exception.\n";
	thread_exception_msg += e.what();
} 
catch(const std::domain_error& e) 
{
	// this executes if above throws std::domain_error (base class rule)
	thread_exception_type = EXCEPTION_DOMAIN_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was a Domain error exception.\n";
	thread_exception_msg += e.what();
} 
catch(const std::length_error& e) 
{
	// this executes if above throws std::length_error (base class rule)
	thread_exception_type = EXCEPTION_LENGTH_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was a Length error exception.\n";
	thread_exception_msg += e.what();
} 	
catch(const std::invalid_argument& e) 
{
	// this executes if above throws std::invalid_argument (base class rule)
	thread_exception_type = EXCEPTION_INVALID_ARG_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += "Error: there was an Invalid Argument error exception.\n";
	thread_exception_msg += e.what();
} 		
catch(const std::out_of_range& e) 
{
	// this executes if above throws std::out_of_range (base class rule)
	thread_exception_type = EXCEPTION_OUT_OF_RANGE_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was a Out of Range error exception.\n";
	thread_exception_msg += e.what();
} 
catch(const std::logic_error& e) 
{
	// this executes if above throws std::logic_error (base class rule)
	thread_exception_type = EXCEPTION_LOGIC_ERROR;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was a Logic error exception.\n";
	thread_exception_msg += e.what();
} 
catch(const std::exception& e) 
{
	// this executes if above throws std::exception (base class rule)
	thread_exception_type = EXCEPTION_STD_EXCEPTION;
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: there was a std namespace error exception.\n";
	thread_exception_msg += e.what();
} 
catch(...) 
{
	// this executes if above throws std::string or int or any other unrelated type
	thread_exception_type = EXCEPTION_NOT_SPECIFIC;		// Some kind of severe error that prevented normal finish
	IntToStr( thr_array_idx, thread_exception_msg );
	thread_exception_msg = "Thread " + thread_exception_msg;
	thread_exception_msg += " Error: a general exception occurred.";
}

if ( EXCEPTION_DID_NOT_HAPPEN != thread_exception_type )
{
	// TODO: See if there are graceful ways to recover from Exceptions in a thread.
	// Because the error context does not give the call stack (for example) this could get messy in a hurry.
	//
	// For now set the Global values so the thread Exception is known.
	// Note that this is NOT a thread safe way but at this point that is the least of our worries.
	g_thread_exception_type = thread_exception_type;
	g_thread_exception_idx  = thr_array_idx;
	g_thread_exception_msg  = thread_exception_msg;

	g_exclusive_Mutex.unlock();		// OK to call even if not locked
}

// Release any resources held by this Thread
//
	for (map<int, CCodeCounter*>::iterator iter = myCounterForEachLanguage.begin(); iter != myCounterForEachLanguage.end(); iter++)
	{
		delete iter->second;
		iter->second = 0;
	}

	pStatus->work_ThreadState = THREAD_HAS_EXITED;
}


// Increment work done counter to support UI updating
// This should be called as FEW times as POSSIBLE to minimize effects of "False sharing" here.
//
// Return the wanted Action from the main thread
int _prv_IncrementWorkDone( const unsigned int threadIdx, const unsigned int amount )
{
	int		retVal = 0;

	if ( threadIdx < MAX_UCC_THREAD_COUNT )
	{
		g_threads_status[ threadIdx ].thread_work_count_done += amount;
		
		// See if main thread has a different action wanted
		retVal = g_threads_status[ threadIdx ].main_WantsAction;
	}

	return retVal;
}


#endif		//	#ifdef		ENABLE_THREADS


// For worker threads this Saves the Uncounted File info
// Main thread this handles it without saving as struct
void _prv_SaveOrWriteUncountedFile( const unsigned int threadIdx, UserIF * userIF,
									UncountedFileStructVector & my_unc_files,
									string msg, string fileName, 
									const bool useListA, const bool print_csv,
									string outDir )
{
	if ( true == g_no_uncounted )
		return;

	if ( threadIdx < MAX_UCC_THREAD_COUNT )
	{
		// Save in a struct for later output
		uncountedFileStruct		unc_file;
		unc_file.threadIdx = threadIdx;
		unc_file.msg       = msg;
		unc_file.uncFile   = fileName;
		unc_file.useListA  = useListA;
		unc_file.csvOutput = print_csv;
		unc_file.outDir    = outDir;
		my_unc_files.push_back( unc_file );
	}
	else
	{
		string writeErr = CUtil::WriteUncountedFileUtil( msg, fileName, useListA, print_csv, outDir );
		if ( ! writeErr.empty() )
			userIF->AddError( writeErr );
	}
}

// For worker threads this Saves the Error info
// Main thread this handles Error without saving as struct
// void AddError(const string &err, bool logOnly = false, int preNL = 0, int postNL = 1);
void _prv_SaveOrAddError( const unsigned int threadIdx, UserIF * userIF,
						ErrMsgStructVector & my_err_msgs,
						const string & err, const bool logOnly = false, 
						const int preNL = 0, const int postNL = 1 )
{
	bool	myLogOnly = logOnly;
	if ( myLogOnly == false )
	{
		if ( g_no_warnings_to_stdout )
		{
			if ( err.find( "Warning:" ) != string::npos )
			{
				// Found a Warning.  Are there also Error messages ?
				if ( ( err.find( "ERROR:" ) == string::npos )
				  && ( err.find( "Error:" ) == string::npos ) )
				{
					// No.  So set logging only
					myLogOnly = true;
				}
				else
				{
					// At least 1 Error message

				}
			}
		}
	}
	if ( threadIdx < MAX_UCC_THREAD_COUNT )
	{
		// Save in a struct for later output
		errMsgStruct		err_msg;
		err_msg.threadIdx = threadIdx;
		err_msg.err       = err;
		err_msg.logOnly   = myLogOnly;
		err_msg.preNL     = preNL;
		err_msg.postNL    = postNL;
		my_err_msgs.push_back( err_msg );
	}
	else
	{
		userIF->AddError( err, myLogOnly, preNL, postNL );
	}
}


//}	//	END		namespace _prv_UCC_THREAD_NAMESPACE_


////////////////////////////////////////////////////////////////////////
//
//			P U B L I C		P R O C E D U R E S
//
///////////////////////////////////////////////////////////////////////

//using namespace _prv_UCC_THREAD_NAMESPACE_;

// Convert an int to a std::string
void	IntToStr( const int val, string & result )
{
	char buf[32];

	_itoa_s( val, buf, 10 );
	result = buf;
}


#ifdef	_DEBUG
// This does a quick? survey of the Heap(s) giving number of blocks and total committed sizes
// Implemented for Windows
void	HeapInUse( int & errorRet, unsigned int & block_count, unsigned long & total_sizes, 
					bool & validate, const bool in_use )
{
#ifdef	WIN32
	block_count = 0;
	total_sizes = 0L;

	bool				is_valid = true;
	unsigned int		region_count = 0;
	unsigned int		uncommitted_count = 0;
	PROCESS_HEAP_ENTRY	heapEntry;

	DWORD		num_of_heaps  = GetProcessHeaps( 0, NULL );
	HANDLE *	pHeapsHandles = (HANDLE *)malloc( sizeof( HANDLE ) * num_of_heaps );
	if ( NULL == pHeapsHandles )
	{
		cout << "\nERROR: HeapInUse unable to Allocate array of Handles\n";
		if ( validate )
			validate = false;
		errorRet = -1;
		return;
	}

	DWORD	num_of_heaps2 = GetProcessHeaps( num_of_heaps, pHeapsHandles );
	if ( num_of_heaps2 != num_of_heaps )
	{
		// The below approach will NOT work, get out
		cout << "\nERROR: HeapInUse logical ERROR.\n";
		free( pHeapsHandles );
		if ( validate )
			validate = false;
		errorRet = -2;
		return;
	}

	BOOL	walk_result;
	HANDLE	hHeap;
	for ( DWORD j = 0; j < num_of_heaps; j++ )
	{
		memset( &heapEntry, 0, sizeof( heapEntry ) );
		hHeap = pHeapsHandles[ j ];
		BOOL lock_result = HeapLock( hHeap );
		if ( FALSE == lock_result )
		{
			cout << "\nERROR: HeapInUse unexpected ERROR: unable to Lock a Heap, set INVALID and skipping\n";
			is_valid = false;
			continue;
		}

		if ( validate )
		{
			if ( HeapValidate( hHeap, 0, 0 ) == 0 )
			{
				cout << "\nERROR: HeapInUse unexpected result.  Heap is INVALID.  Corruption ?\n";
				is_valid = false;

				// For now fall through and try to get other info
			}
		}

		do		// Get Heap details
		{
			walk_result = HeapWalk( hHeap, &heapEntry );
			if ( TRUE == walk_result )
			{
				if ( heapEntry.wFlags & PROCESS_HEAP_ENTRY_BUSY )
				{
					block_count++;
					if ( in_use )
						total_sizes += (unsigned long)( heapEntry.cbData );
				}
				else if ( heapEntry.wFlags & PROCESS_HEAP_REGION )
				{
					region_count++;
					block_count++;
					// No need to go through the Region linked list
					total_sizes += (unsigned long)( heapEntry.Region.dwCommittedSize );
				}
				else if ( heapEntry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE ) 
				{
					// Uncommitted range
					uncommitted_count++;
				}
				else 
				{
					// Block
					block_count++;
				}

				if ( false == in_use )
					total_sizes += (unsigned long)( heapEntry.cbData );
			}
		} while ( TRUE == walk_result );

		HeapUnlock( hHeap );
	}
		
	free( pHeapsHandles );

	if ( validate )
		validate = is_valid;

	errorRet = 0;

#else	// Add support for another OS here if wanted

	errorRet = 1;		// NOT IMPLEMENTED

	// Assign defaults but not even realistic.
	block_count = 1;
	total_sizes = 2;
	validate = false;

#endif

	return;
}
#endif	//	ifdef	_DEBUG


// Give up the rest of the time slice this thread has and
// Put the currently executing thread to sleep for approximately the time wanted
void ThreadSleep( const unsigned int milliSeconds )
{
#ifdef	ENABLE_THREADS
	//boost::this_thread::yield();

	unsigned int sleep_amount = milliSeconds;

	if ( sleep_amount < MIN_SLEEP_TIME )
		sleep_amount = MIN_SLEEP_TIME;

	// Sleep for a given amount.  Actual time could be somewhat longer.
	sleep( boost::posix_time::milliseconds( sleep_amount ) );

	// boost::this_thread::yield() could be called instead (just not from here)
	// to give up rest of current time slice of the running thread
#else
	milliSeconds;
#endif
}

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
					string						&		myErrList,
					string								outDir,
					ErrMsgStructVector			&		err_msgs,
					UncountedFileStructVector	&		unc_files,
					const		bool					process_after_read,
					const		bool					discard_lines_after_process )
{
	int			error_count = 0;
	CCodeCounter * pCounter = NULL;

	filemap		fmap;
	results		r;

	bool			OK_to_process = false;
	bool			lineTooLong;
	bool			trim_line = true;
	bool			isErr = false;
	unsigned int	lineNum = 0;
	string			oneline;

	ClassType	fileclass;
	CWebCounter *webCounter;
	WebType		webType;

	string		fileName, clearCaseCroppedFile;

#ifdef	USE_MEMORY_MAPPED_FILES
	// Set up to Read using a Memory Mapped file.
	// Should be faster as it could avoid 1 or more copy operations by OS.
	// However testing this implementation on Windows 7.1 showed it was SLOWER by a fair amount.  
	// Experiment with it if you want.

	// Create a read only Memory Mapped file interface with stream semantics
	// The way this library works is that we only need to create 1 instance here.
	// Cleanup happens when this procedure returns and the class object instance goes out of scope.
	boost::iostreams::stream<boost::iostreams::mapped_file_source> fr;
#endif

// LOOP through the given file list
			
	unsigned int	percent_done = 0;
	unsigned int	prev_percent_done = 0;
	unsigned int	count_done = 0;

	unsigned int	num_in_loop = distance( itStart, itEnd );
	unsigned int	num_inserted = 0;

	// Only need to update when percent changes so will do twice as often
	unsigned int	inc_amount = num_in_loop / 200;
	if ( 0 == inc_amount )
		inc_amount = 1;
	unsigned int	count_down = inc_amount;

	std::vector<std::string>::iterator itVectorData = itStart;
	for ( ; itVectorData != itEnd;  itVectorData++ )
	{
		OK_to_process = false;
		fileName = *(itVectorData);
		fileName = CUtil::TrimString( fileName );
		if (clearCaseFile)
		{
			// handle the @@ from ClearCase
			clearCaseCroppedFile = CUtil::ConvertClearCaseFile(fileName);
			fileclass = DecideLanguage( CounterForEachLanguage, &(pCounter), print_cmplx, clearCaseCroppedFile, false );
		}
		else
			fileclass = DecideLanguage( CounterForEachLanguage, &(pCounter), print_cmplx, fileName, false );
		
		if (fileclass == UNKNOWN)
		{
			_prv_SaveOrWriteUncountedFile( threadIdx, userIF, unc_files, "Unknown File Extension", fileName, useListA, print_csv, outDir );
			continue;
		}
		
		// Save so if Errors later file is properly handled
		r.class_type = fileclass;
		r.file_type = (pCounter->classtype == UNKNOWN || pCounter->classtype == DATAFILE) ? DATA : CODE;
		fmap.clear();

		// Set up decision variable later used when processing each Physical line.
		trim_line = true;
		if ( ( fileclass == FORTRAN ) 
			|| ( fileclass == PYTHON ) )
			trim_line = false;	// column location and/or white space is significant

		// Update the total number of files and the number of files of each type
		// Because of using a separate instance of CounterForEachLanguage per thread
		// these changes must be accumulated back to the main thread's instancs as well.
		if (useListA)
			pCounter->total_filesA++;
		else
			pCounter->total_filesB++;
			
		if (fileclass == WEB)
		{
			// get web file class
			webCounter = (CWebCounter *)pCounter;
			webType = webCounter->GetWebType(clearCaseFile ? clearCaseCroppedFile : fileName);
			if (webType == WEB_PHP)
			{
				if (useListA)
					webCounter->total_php_filesA++;
				else
					webCounter->total_php_filesB++;
			}
			else if (webType == WEB_ASP)
			{
				if (useListA)
					webCounter->total_asp_filesA++;
				else
					webCounter->total_asp_filesB++;
			}
			else if (webType == WEB_JSP)
			{
				if (useListA)
					webCounter->total_jsp_filesA++;
				else
					webCounter->total_jsp_filesB++;
			}
			else if (webType == WEB_XML)
			{
				if (useListA)
					webCounter->total_xml_filesA++;
				else
					webCounter->total_xml_filesB++;
			}
			else if (webType == WEB_CFM)
			{
				if (useListA)
					webCounter->total_cfm_filesA++;
				else
					webCounter->total_cfm_filesB++;
			}
			else
			{
				if (useListA)
					webCounter->total_htm_filesA++;
				else
					webCounter->total_htm_filesB++;
			}
		}

		if ( 0 == fileName.size() )
			continue;
		
		// Read in a file
		r.e_flag = false;
		r.error_code = "";

		// if the filename was modified by ClearCase, use the cropped name
		if ( clearCaseFile )
			r.file_name = clearCaseCroppedFile;
		else
			r.file_name = fileName;

		/* COMMENTED OUT but kept as example of multithreading that was Inefficient:
		it attempted to use fine grain threading (thread unit of work was a single file)
		which had WAY too much overhead to give good performance for Thousands of files.
		if ( workThreadsCount )
		{
			// Make a placeholder in the source file list.
			// This keeps the list in the same order as if done by a single thread.
			//
			// depending on which source file we are on...
			SourceFileElement s_element(fmap, r);
			mySrcFileList->push_back(s_element);

			// Get pointer to the just created element at the end
			list<SourceFileElement>::iterator it = mySrcFileList->end();
			--it;	// Move back 1 position to valid structure
			SourceFileElement * pElement = &(*it);
			unsigned int error_count = 0;
			files_to_read = ThreadAddFileToRead( fileName, trim_line, 
												pElement, error_count );
			if ( error_count )
			{
				// Iterate from last position used for searching for Errors
				// of mySrcFileList looking for any r.e_flag == true
			}
			continue;
		}
		*/

#ifdef	USE_MEMORY_MAPPED_FILES
		// Use the Memory Mapped file interface created earlier to open the file
		// Open the Memory Mapped source (read only) file
		fr.open( fileName.c_str() );
#else
		// Open the file
		ifstream fr( fileName.c_str(), ios::in );
#endif
		if (!fr.is_open())
		{
			r.e_flag = true;
			r.error_code = "Unable to open file";
			error_count++;
			string err = "Error: ";
			err += r.error_code;
			err += " (";
			err += fileName;
			err += ")";
			if (!isErr)
			{
				_prv_SaveOrAddError( threadIdx, userIF, err_msgs, err, false, 1 );
				isErr = true;
			}
			else
				_prv_SaveOrAddError( threadIdx, userIF, err_msgs, err );
			_prv_SaveOrWriteUncountedFile( threadIdx, userIF, unc_files, "Not Readable", fileName, useListA, print_csv, outDir );
		}
		else
		{
			// File opened OK, read each Physical line into memory
			lineNum = 0;
			lineTooLong = false;

			while (fr.good() || fr.eof())
			{
				getline(fr, oneline);
				if ((!fr.good() && !fr.eof()) || (fr.eof() && oneline.empty()))
					break;

				lineNum++;
				if (oneline.length() > MAX_LINE_LENGTH)
				{
					lineTooLong = true;
					break;
				}
				
				if ( trim_line )
					oneline = CUtil::TrimString(oneline);
				if ( oneline.size() )
					oneline = CUtil::ReplaceSmartQuotes(oneline);
				
				// Moved element(lineNum, oneline); as lineElement  
				// inside call below to prevent Debug build memory leaks.
				fmap.push_back( lineElement( lineNum, oneline ) );
				if (!fr.good())
					break;
			}
			fr.clear();
			fr.close();

			if (lineTooLong)
			{
				r.e_flag = true;
				r.error_code = "Line too long";
				error_count++;
				string err = "Error: ";
				err += r.error_code;
				err += ", file skipped (";
				err += fileName;
				err += ")";
				if (!isErr)
				{
					_prv_SaveOrAddError( threadIdx, userIF, err_msgs, err, false, 1 );
					isErr = true;
				}
				else
					_prv_SaveOrAddError( threadIdx, userIF, err_msgs, err );
				fmap.clear();  // don't bother processing the file if an error is just going to be reported.				
					
				// Add the line number that was too long to the message
				string tmp;
				IntToStr( lineNum, tmp );
				err = "Excessive Line Length on Line " + tmp;
				_prv_SaveOrWriteUncountedFile( threadIdx, userIF, unc_files, err, fileName, useListA, print_csv, outDir );

				// Allow this to set up empty settings that will show as Zero counts in results files.
				OK_to_process = true;
			}
			else if (fmap.size() == 0)
			{
				bool	logOnly = false;
				if ( g_no_warnings_to_stdout )
					logOnly = true;

				string err = "Warning: File is empty (";
				err += fileName;
				err += ")";
				if (!isErr)
				{
					_prv_SaveOrAddError( threadIdx, userIF, err_msgs, err, logOnly, 1 );
					isErr = true;
				}
				else
					_prv_SaveOrAddError( threadIdx, userIF, err_msgs, err, logOnly );

				// Allow this to set up empty settings that will show as Zero counts in results files.
				OK_to_process = true;
			}
			else
				OK_to_process = true;
		}

		// Avoid unnecessary calls to ExtractFilename later on
		if ( ! discard_lines_after_process )
			r.file_name_only = CUtil::ExtractFilename( r.file_name );

		// depending on which source file we are on...
		mySrcFileList->push_back( SourceFileElement( fmap, r ) );

		if ( process_after_read && OK_to_process )
		{
			// set iterator to file entry just appended at end
			SourceFileList::iterator i = mySrcFileList->end();
			i--;
			ProcessSourceListFile( threadIdx, userIF, CounterForEachLanguage, 
								&pCounter, i, mySourceFileA, mySourceFileB, 
								num_inserted,
								err_msgs, unc_files,
								myErrList, useListA );
			if ( num_inserted )
			{
				SourceFileList::iterator itPos = i;
				itPos++;
				for ( unsigned int k = 0; k < num_inserted; k++ )
				{
					
					ProcessSourceListFile( threadIdx, userIF, CounterForEachLanguage, 
								&pCounter, itPos, mySourceFileA, mySourceFileB, 
								num_inserted,
								err_msgs, unc_files,
								myErrList, useListA );

					if ( discard_lines_after_process )
					{
						SourceFileElement & element = (*itPos);
						element.first.clear();				// Empty the file map of Physical source lines
						element.second.mySLOCLines.clear();	// Empty the vector of Logical source lines
					}

					itPos++;
					if ( itPos == mySrcFileList->end() )
						break;
				}

				if ( discard_lines_after_process )
				{
					SourceFileElement & element = (*i);
					element.first.clear();				// Empty the file map of Physical source lines
					element.second.mySLOCLines.clear();	// Empty the vector of Logical source lines
				}

				num_inserted = 0;
			}
		}

		
		if ( threadIdx < MAX_UCC_THREAD_COUNT )
		{
			count_down--;
			if ( 0 == count_down )
			{
			#ifdef	ENABLE_THREADS
				// Save work progress for later display
				int wanted_action = _prv_IncrementWorkDone( threadIdx, inc_amount );

				if ( ( STOP_WORKING == wanted_action )
				  || ( EXIT_NOW     == wanted_action ) )
				{
					break;	// Don't do any more files.  Get out.
				}
			#endif
				count_down = inc_amount;
			}
		}
		else
		{
			// Running from main thread, update UI if needed
			count_done++;
			percent_done = ( count_done * 100 ) / num_in_loop;
			if ( percent_done != prev_percent_done )
			{
				// Percent changed so update UI
				userIF->updateProgress( "", false, percent_done, num_in_loop );
				prev_percent_done = percent_done;
			}
			// processing Canceled ?
			if ( userIF != NULL )
			{
				bool has_cancelled = userIF->isCanceled();
				if ( has_cancelled )
				{
					break;	// Stop Looping and return
				}
			}
		}
	}		//	END		LOOP through the given file list

	return error_count;
}

// Analyze and do Counts for a single Source File
void ProcessSourceListFile( const unsigned int threadIdx, UserIF * userIF, 
							CounterForEachLangType & CounterForEachLanguage,
							CCodeCounter				**	ppCounter,
							SourceFileList::iterator		i, 
							SourceFileList				*	mySourceFileA,
							SourceFileList				*	mySourceFileB,
							unsigned int				&	num_inserted,
							ErrMsgStructVector			&	err_msgs,
							UncountedFileStructVector	&	unc_files,
							string						&	errList,
							const bool						useListA )
{
	CCodeCounter	*	pCounter = NULL; 
	size_t				embFile = string::npos;

	// Exceptions within this File Analysis & Counter upper level Caller
	int		analyze_exception_type = EXCEPTION_DID_NOT_HAPPEN;
	string	analyze_exception_msg;

	// handle any exception that may occur un-handled in the counting functions
	try
	{
		embFile = i->second.file_name.find( EMBEDDED_FILE_PREFIX );	// Get position for later use

		// examines the file and sets counter to the correct language counter
		DecideLanguage( CounterForEachLanguage, &(pCounter), print_cmplx, i->second.file_name );
		i->second.file_type = (pCounter->classtype == UNKNOWN || pCounter->classtype == DATAFILE) ? DATA : CODE;
		i->second.class_type = pCounter->classtype;

		// Call down to Analyze the details of this file
		pCounter->CountSLOC( &(i->first), &(i->second) );

		if (i->second.trunc_lines > 0)
		{
			stringstream ss;
			ss << "Warning: Truncated ";
			ss << i->second.trunc_lines;
			ss << " line(s) in file (";
			if (embFile != string::npos)
				ss << i->second.file_name.substr(0, embFile);
			else
				ss << i->second.file_name;
			ss << ")";
			string err = ss.str();
			errList += err + "\n";
			_prv_SaveOrAddError( threadIdx, userIF, err_msgs, err, true );	// Log only
		}

		// if webcounter, insert the separation file into list
		if (pCounter->classtype == WEB)
		{
			SourceFileList::iterator pos = i;
			pos++;
			map<int, SourceFileElement>* smap = ((CWebCounter*)pCounter)->GetSeparationMap();
			for (map<int, SourceFileElement>::iterator iter = smap->begin(); iter != smap->end(); iter++)
			{
				if (iter->second.first.size() > 0)
				{
					// Set Embedded flag when inserting a file into the List of Files
					// This helps later processing avoid calling find() to see if this is an Embedded file 
					// Set File name only string to help later processing
					size_t tempi = iter->second.second.file_name.find( EMBEDDED_FILE_PREFIX );
					if ( tempi != string::npos )
					{
						iter->second.second.file_name_isEmbedded = true;
						iter->second.second.file_name_only = CUtil::ExtractFilename( iter->second.second.file_name.substr( 0, tempi ) );
					}
					else
					{
						iter->second.second.file_name_isEmbedded = false;
						iter->second.second.file_name_only = CUtil::ExtractFilename( iter->second.second.file_name );
					}

					(useListA) ? mySourceFileA->insert(pos, iter->second) : mySourceFileB->insert(pos, iter->second);

					num_inserted++;
				}
			}
		}
	}
	catch(const std::overflow_error& e) 
	{
		// this executes if above throws std::overflow_error (same type rule)
		analyze_exception_type = EXCEPTION_OVERFLOW_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was an Overflow error exception.\n";
		analyze_exception_msg += e.what();
	}
	catch(const std::underflow_error& e)
	{
		// this executes if above throws std::underflow_error (base class rule)
		analyze_exception_type = EXCEPTION_UNDERFLOW_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was an Underflow error exception.\n";
		analyze_exception_msg += e.what();
	}
	catch(const std::range_error& e)
	{
		// this executes if above throws std::range_error
		analyze_exception_type = EXCEPTION_RANGE_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was a Range error exception.\n";
		analyze_exception_msg += e.what();
	}
	catch(const std::system_error& e)
	{
		// this executes if above throws std::system_error
		analyze_exception_type = EXCEPTION_SYSTEM_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was a System error exception.\n";
		analyze_exception_msg += e.what();
	}
	catch(const std::runtime_error& e) 
	{
		// this executes if above throws std::runtime_error (base class rule)
		analyze_exception_type = EXCEPTION_RUNTIME_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was a Runtime error exception.\n";
		analyze_exception_msg += e.what();
	} 
	catch(const std::domain_error& e) 
	{
		// this executes if above throws std::domain_error (base class rule)
		analyze_exception_type = EXCEPTION_DOMAIN_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was a Domain error exception.\n";
		analyze_exception_msg += e.what();
	} 
	catch(const std::length_error& e) 
	{
		// this executes if above throws std::length_error (base class rule)
		analyze_exception_type = EXCEPTION_LENGTH_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was a Length error exception.\n";
		analyze_exception_msg += e.what();
	} 	
	catch(const std::invalid_argument& e) 
	{
		// this executes if above throws std::invalid_argument (base class rule)
		analyze_exception_type = EXCEPTION_INVALID_ARG_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += "Error: there was an Invalid Argument error exception.\n";
		analyze_exception_msg += e.what();
	} 		
	catch(const std::out_of_range& e) 
	{
		// this executes if above throws std::out_of_range (base class rule)
		analyze_exception_type = EXCEPTION_OUT_OF_RANGE_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was a Out of Range error exception.\n";
		analyze_exception_msg += e.what();
	} 
	catch(const std::logic_error& e) 
	{
		// this executes if above throws std::logic_error (base class rule)
		analyze_exception_type = EXCEPTION_LOGIC_ERROR;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was a Logic error exception.\n";
		analyze_exception_msg += e.what();
	} 
	catch(const std::exception& e) 
	{
		// this executes if above throws std::exception (base class rule)
		analyze_exception_type = EXCEPTION_STD_EXCEPTION;
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: there was a std namespace error exception.\n";
		analyze_exception_msg += e.what();
	} 
	catch(...) 
	{
		// this executes if above throws std::string or int or any other unrelated type
		analyze_exception_type = EXCEPTION_NOT_SPECIFIC;		// Some kind of severe error that prevented normal finish
		IntToStr( threadIdx, analyze_exception_msg );
		analyze_exception_msg = "Thread " + analyze_exception_msg;
		analyze_exception_msg += " Error: a general exception occurred.";
	}

	if ( EXCEPTION_DID_NOT_HAPPEN != analyze_exception_type )
	{
		// TODO: See if there are graceful ways to recover from Exceptions in a thread.
		// Because the error context does not give the call stack (for example) this could get messy in a hurry.
		//
		// For now set the Global values so the thread Exception is known.
		// Note that this is NOT a thread safe way but at this point that is the least of our worries.
		//g_thread_exception_type = analyze_exception_type;
		//g_thread_exception_idx  = threadIdx;
		//g_thread_exception_msg  = analyze_exception_msg;

		string err = "Error: Unable to count file (";
		if (embFile != string::npos)
			err += i->second.file_name.substr(0, embFile);
		else
			err += i->second.file_name;
		err += ")";

		// Append message about File name to existing Exception info message
		err = analyze_exception_msg + "\n" + err;

		errList += err + "\n";
		//userIF->AddError(err, true);
		_prv_SaveOrAddError( threadIdx, userIF, err_msgs, err, false );		// Show ERROR on console as well

		//WriteUncountedFile("Unhandled Counting Error", i->second.file_name, useListA, print_csv);
		_prv_SaveOrWriteUncountedFile( threadIdx, userIF, unc_files,
									"Unhandled Counting Error", i->second.file_name, 
									useListA, print_csv, outDir );
	}

	*ppCounter = pCounter;
}



// Divide a List of Files among multiple threads and start them working
int ReadFilesThreads( const unsigned int				num_files,
					UserIF				*				userIF,
					const		bool					print_cmplx,
					const		bool					print_csv,
					std::vector<std::string>::iterator itStart,		// Start of entire list to divide
					std::vector<std::string>::iterator itEnd,		//  End  of entire list to divide
					const		bool					useListA,
					const		bool					clearCaseFile,
					SourceFileList		*				mySrcFileList,
					string								outDir,
					const		bool					process_after_read,
					const		bool					discard_lines_after_process )
{
	int		error_count = 0;
#ifdef	ENABLE_THREADS
	// Add a Request to Read a List of Files for each thread.
	// Each thread should be ready for work.
	//
	// It is OK (but inefficient) to have fewer files than available threads.
	// If so, there may be threads left blocked and waiting for work.
	//
	int num_to_assign = num_files;
	int num_per_thread = num_files / g_threads_status_in_use;
	if ( 0 == num_per_thread )
		num_per_thread = 1;

	int		isAssigned[ MAX_UCC_THREAD_COUNT ];
	memset( isAssigned, 0, sizeof( isAssigned ) );

	int		toAssign[ MAX_UCC_THREAD_COUNT ];
	memset( toAssign, 0, sizeof( toAssign ) );

	std::vector<std::string>::iterator iStart, iEnd;
	int		num_threads_to_assign = 0;
	int		thisAssign = num_per_thread;
	for ( unsigned int j = 0; j < g_threads_status_in_use; j++ )
	{
		if ( ( j == g_threads_status_in_use - 1 ) 
		  || ( num_to_assign < thisAssign ) )
			thisAssign = num_to_assign;

		toAssign[ j ] = thisAssign;
		num_threads_to_assign++;
		num_to_assign -= thisAssign;
		if ( num_to_assign <= 0 )
			break;
	}

	do
	{
		g_assigned_count = 0;

		// Find a thread ready for Work
		for ( unsigned int j = 0; j < g_threads_status_in_use; j++ )
		{
			if ( EXIT_NOW == g_threads_status[ j ].main_WantsAction )
			{
				continue;		// Should NOT get here
			}

			// Add a file read request for a thread that is ready
			if ( THREAD_WAITING_FOR_WORK != g_threads_status[ j ].work_ThreadState )
				continue;

			if ( isAssigned[ j ] ) 
			{
				g_assigned_count++;
				continue;
			}

			if ( 0 == toAssign[ j ] )
				continue;

			if ( g_threads_status[ j ].thread_had_error )
			{
				error_count = 1;	// some error processing a file
				// Error details are found in the itFileElement first and second elements

			}
			g_threads_file_names[ j ].clear();

			g_threads_status[ j ].print_cmplx                 = print_cmplx;
			g_threads_status[ j ].print_csv                   = print_csv;
			g_threads_status[ j ].process_after_read          = process_after_read;
			g_threads_status[ j ].discard_lines_after_process = discard_lines_after_process;

			iStart = itStart;
			if ( j )
			{
				if ( j < g_threads_status_in_use - 1 )
				{
					advance( iStart, j * ( num_per_thread ) );
					iEnd = iStart;
					advance( iEnd, toAssign[ j ] );
				}
				else
				{
					iEnd = itEnd;
					iStart = iEnd;
					iStart--;
					advance( iStart, - ( toAssign[ j ] - 1 ) );
				}
			}
			else
			{
				// This is only valid for thread index 0
				g_threads_status[ 0 ].mySrcFileList = mySrcFileList;
				iEnd = iStart;
				advance( iEnd, toAssign[ j ] );
			}

			g_threads_status[ j ].itStart       = iStart;
			g_threads_status[ j ].itEnd         = iEnd;
			g_threads_status[ j ].useListA      = useListA;
			g_threads_status[ j ].clearCaseFile = clearCaseFile;
			
			g_threads_status[ j ].outDir        = outDir;

			g_threads_status[ j ].userIF        = userIF;	// thread checks this for != NULL
			g_threads_status[ j ].main_WantsAction = GET_TO_WORK;

			// Toggle semaphore to wake thread
			g_threads_status[ j ].pSem->signal();

			boost::this_thread::yield();	// Allow assigned thread (and maybe others) to run

			isAssigned[ j ] = j + 1;
			g_assigned_count++;
			if ( g_assigned_count == num_threads_to_assign )
				break;

		}	//	END		for ( unsigned int j = 0; j < g_threads_status_in_use; j++ )

		if ( g_assigned_count < num_threads_to_assign )
			boost::this_thread::yield();	//ThreadSleep( 100 );

	} while ( g_assigned_count < num_threads_to_assign );
#else
	// Just do a simple reference to these.  Will not produce any code.  
	// Avoids compiler warning when NOT compiling Thread support.
	num_files;
	userIF;
	print_cmplx;
	print_csv;
	itEnd;
	itStart;
	useListA;
	clearCaseFile;
	mySrcFileList;
	process_after_read;
	discard_lines_after_process;
#endif
	return error_count;
}

/*!
*			StartWorkerThreads
* Set up and Start a number of worker threads.
* Any threads started will be waiting for either work to do or to exit thread if wanted.
*
* \param num_to_start	IN			number of worker threads wanted to start
* \param message		IN/OUT		ref to message giving details for number actually started
*
* \returns							1 if number of threads wanted was started OK
*							or		positive number >= 2 giving number of threads actually started
*							or		negative number giving Severe unrecoverable Error code
*
*	It is the calling code's responsibility to check for Errors.
*
* Recommended actions for Severe unrecoverable Errors: 
*		Log the ERROR with all given details (found in message)
*		Continue running in single thread mode without any worker threads
*
* Recommended actions for the number of threads actually started being lower than wanted:
*		Log an INFO message about wanted and actual worker thread count
*		Go ahead and use the availble (smaller number) of worker threads
*/
int StartWorkerThreads( CounterForEachLangType * pCounterForEachLanguage, 
						UserIF				*	userIF,
						const unsigned int num_to_start, string & message )
{
	int				retVal = THREADS_STARTED_OK;

#ifdef	ENABLE_THREADS

	int				error_code = 0;
	unsigned int	started_thread_count = 0;
	unsigned int	num_wanted = num_to_start;

	message = "";

	// Save the calling (main) thread id
	g_main_thread_id = get_id();

	g_userIF = userIF;
	g_thread0_CounterForEachLanguage = pCounterForEachLanguage;

	if ( num_wanted < MIN_UCC_THREAD_COUNT )
	{
		num_wanted = MIN_UCC_THREAD_COUNT;
		message = "INFO: Number of threads wanted is too few.  Using 2 threads.\n";
	}

	if ( num_wanted > MAX_UCC_THREAD_COUNT )
	{
		num_wanted = MAX_UCC_THREAD_COUNT;
		message = "INFO: Number of threads wanted is too many.  Using ";
		string tmp;
		IntToStr( MAX_UCC_THREAD_COUNT, tmp );
		message += tmp;
		message += " threads.\n";
	}

	// Set up array of thread status structs
	if ( g_threads_status_in_use )
		FinishWorkerThreads();

	_prv_threadStatus		one_thread_status;
	_prv_Init_threadStatus( one_thread_status );

	try
	{
		for ( unsigned int j = 0; j < num_wanted; j++ )
		{
			one_thread_status.thread_idx = j;
			
			// Create a boost semaphore class to block worker threads when not doing work.
			// Initial value of semaphore will block the worker thread when it calls wait.
			one_thread_status.pSem = new Semaphore( 0 );
			
			g_threads_status[ j ] = one_thread_status;

			g_threads_file_names[ j ]      = "";
			g_threads_pSrcFileElement[ j ] = NULL;

		// Create a thread for each struct
			boost::thread myThread( prv_WorkThread_Function, j );

			g_threads_status_in_use++;

			g_threads_status[ j ].threadId = myThread.get_id();

			// Now  the boost::thread class myThread will go out of scope.
			// When the boost::thread class that represents a thread of
			// execution is destroyed the thread becomes detached. 
			// Once a thread is detached, it will continue executing until 
			// the invocation of the function or callable object supplied on 
			// construction has completed, or the program is terminated. 
			//
			//		OR 
			// the way done here, when the main thread tells it to Exit

			started_thread_count++;

			// Pause the calling (main) thread so the created threads start OK
			boost::this_thread::yield();

			g_exclusive_Mutex.lock();
			g_exclusive_Mutex.unlock();
		}

		// Sleep main thread for a bit
		ThreadSleep( ONE_SECOND );

		g_exclusive_Mutex.lock();
		g_exclusive_Mutex.unlock();

		unsigned int thread_started_OK_count;
		do
		{
			thread_started_OK_count = 0;
			for ( unsigned int j = 0; j < started_thread_count; j++ )
			{
				if ( true == g_threads_status[ j ].thread_has_started )
					thread_started_OK_count++;
			}

			if ( thread_started_OK_count < started_thread_count )
			{
				boost::this_thread::yield();
				g_exclusive_Mutex.lock();
				g_exclusive_Mutex.unlock();
			}

		} while ( thread_started_OK_count < started_thread_count );
	}
	catch (...)
	{
		string tmp;
		if ( started_thread_count >= MIN_UCC_THREAD_COUNT )
		{
			// Got enough threads started to work with
			message += "INFO: Number of threads started ";
			IntToStr( started_thread_count, tmp );
			message += tmp;
			message += " is less than number wanted ";
			IntToStr( num_to_start, tmp );
			message += tmp;
			message += "\n";
		}
		else
		{
			error_code = -1;	// Severe problem creating a thread
			if ( message.size() )
				message += "\n";
			message += "ERROR: Unable to start even ";
			IntToStr( MIN_UCC_THREAD_COUNT, tmp );
			message += tmp;
			message += " threads.";

			if ( started_thread_count )
			{
				FinishWorkerThreads();
				started_thread_count = 0;
			}
		}

		g_exclusive_Mutex.unlock();
	}	//	END		catch

	if ( num_wanted > started_thread_count )
	{
		if ( error_code )
		{
			retVal = error_code;
			if ( 0 < started_thread_count )
				FinishWorkerThreads();
		}
		else
			retVal = (int) started_thread_count;
	}
#else
	pCounterForEachLanguage;
	userIF;
	num_to_start;
	message = "Information: worker Threads not compiled.  Edit UCCThread.cpp\n";
	retVal = -2;
#endif
	return retVal;
}


/*!
*			FinishWorkerThreads
* Finish any running worker threads.
* This may block for a few seconds while worker threads exit if they were busy.
*/	
void FinishWorkerThreads()
{
#ifdef	ENABLE_THREADS
	for ( unsigned int j = 0; j < g_threads_status_in_use; j++ )
	{
		g_threads_status[ j ].main_WantsAction = EXIT_NOW;
		
		// Toggle semaphore to wake thread if it was waiting
		g_threads_status[ j ].pSem->signal();

		// Give up rest of calling (main) thread time slice and let worker threads run and exit
		boost::this_thread::yield();
	}

	// Check for all threads having a status of THREAD_HAS_EXITED
	unsigned int threads_to_check = g_threads_status_in_use;
	
	while ( threads_to_check )
	{
		for ( unsigned int j = 0; j < g_threads_status_in_use; j++ )
		{
			if ( THREAD_HAS_EXITED != g_threads_status[ j ].work_ThreadState )
			{
				// Give up rest of calling (main) thread time slice and let worker threads run and exit
				ThreadSleep( 100 );
			}
			else
				threads_to_check--;
		}

		// Is there 1 or more threads not yet Exited?  Ensure they get checked again.
		if ( threads_to_check )
			threads_to_check = g_threads_status_in_use;
	}

	g_threads_status_in_use = 0;

#endif
}


// Propagate back the results from recently finished worker threads
void CombineThreadResults()
{
	// Send back the combined results of Reading a List of Files
	// and perhaps doing Analysis and Counting as well...
#ifdef	ENABLE_THREADS
	bool	any_messages_shown = false;
	bool	my_useListA = g_threads_status[ 0 ].useListA;
	string	writeErr;

	SourceFileList & rDestFiles = (my_useListA) ? SourceFileA: SourceFileB;
	//SourceFileList * pSrcFiles = NULL;
	ErrMsgStructVector::iterator		itErr;
	UncountedFileStructVector::iterator	itUnc;

	CounterForEachLangType::iterator itDestStart = g_thread0_CounterForEachLanguage->begin();
	CounterForEachLangType::iterator itDestEnd   = g_thread0_CounterForEachLanguage->end();

	for ( int j = 0; j < g_assigned_count; j++ )
	{
		// Copy back results to main thread struct
		SourceFileList * pSrcFiles = g_threads_status[ j ].mySrcFileList;

		// Append to existing list

	#ifdef	USE_LESS_MEM_ON_COPY
		// Minimize use of temporary memory.  
		// This can save several MB or GB or more depending on size of File list and Files
		// Which can make the difference between success with Threads or not.
		// So this is the default approach.
		unsigned int num_to_copy = pSrcFiles->size();

		if ( num_to_copy )
		{
			// Determine a chunk of File results to copy / erase at a time
			// This is biased to work with a large number of Files.
			// So it may be somewhat inefficient for small number of Files.
			unsigned int chunk_size = num_to_copy / 20;
			if ( chunk_size > 500 )
				chunk_size = 500;
			if ( 0 == chunk_size )
				chunk_size = num_to_copy;
			
			list<SourceFileElement>::iterator itSrcPos1;
			list<SourceFileElement>::iterator itSrcPos2;
			while ( num_to_copy )
			{
				itSrcPos1 = pSrcFiles->begin();
				itSrcPos2 = itSrcPos1;
				advance( itSrcPos2, chunk_size );
				rDestFiles.insert( rDestFiles.end(), itSrcPos1, itSrcPos2 );
				pSrcFiles->erase( itSrcPos1, itSrcPos2 );

				num_to_copy -= chunk_size;
				if ( chunk_size > num_to_copy )
					chunk_size = num_to_copy;
			}
			/*
			// This works but is TOO SLOW if doing thousands of File results
			for ( unsigned int k = 0; k < num_to_copy; k++ )
			{
				rDestFiles.push_back( *(pSrcFiles->begin()) );	// Append element to end of list
				pSrcFiles->pop_front();						// Get rid of element just copied
			} */
		#else
			// Use Faster but temporarily uses much MORE memory approach
			rDestFiles.insert( rDestFiles.end(), pSrcFiles->begin(), pSrcFiles->end() );
		#endif
			pSrcFiles->clear();	// Release memory from thread struct
		}

		// Handle list of Errors
		for ( itErr = g_threads_status[ j ].thread_err_msgs.begin(); itErr != g_threads_status[ j ].thread_err_msgs.end(); itErr++ )
		{
			errMsgStruct & err_msg = *(itErr);
			// err_msg.threadIdx = threadIdx;
			if ( err_msg.err.size() )	// Don't do empty messages.  So main thread can empty messages before thread finishes.
			{
				g_userIF->AddError( err_msg.err, err_msg.logOnly, err_msg.preNL, err_msg.postNL );
				if ( false == err_msg.logOnly )
					any_messages_shown = true;
			}
		}
		g_threads_status[ j ].thread_err_msgs.clear();

		// Handle list of Uncounted Files
		for ( itUnc = g_threads_status[ j ].thread_unc_files.begin(); itUnc != g_threads_status[ j ].thread_unc_files.end(); itUnc++ )
		{
			uncountedFileStruct & unc_file = *(itUnc);
			if ( unc_file.msg.size() )
			{
				writeErr = CUtil::WriteUncountedFileUtil( unc_file.msg, unc_file.uncFile, 
											unc_file.useListA, unc_file.csvOutput, unc_file.outDir );
				if ( writeErr.size() )
				{
					g_userIF->AddError( writeErr );
					any_messages_shown = true;
				}
			}
		}
		g_threads_status[ j ].thread_unc_files.clear();

		if ( g_threads_status[ j ].errList.size() )
		{
			// Show Errors wanted on the UI found by this thread (skip trailing NewLine)
			bool msg_shown = g_userIF->updateProgress( g_threads_status[ j ].errList, false );
			if ( true == msg_shown )
				any_messages_shown = true;
			g_threads_status[ j ].errList.clear();
		}

		if ( g_process_after_read )
		{
			// Update main thread from each thread's CounterForEachLangType local data
			AddFromOtherLangCounters( g_thread0_CounterForEachLanguage,
									g_threads_status[ j ].pCounterForEachLanguage,
									my_useListA );
		}
	}

	if ( any_messages_shown )
		cout << endl;	// Put in a trailing NewLine after the console messages

	// All results from threads handled, get ready for next request
	g_assigned_count = 0;

#endif
}


// Return the number of threads working
unsigned int NumThreadsBusy( unsigned int & count_done, 
							const unsigned int sleep_milliseconds,
							const bool combine_results_from_threads )
{
	unsigned int	busy_count = 0;

#ifdef	ENABLE_THREADS

	count_done = 0;

	for ( unsigned int j = 0; j < g_threads_status_in_use; j++ )
	{
		if ( THREAD_WORKING == g_threads_status[ j ].work_ThreadState )
			busy_count++;
		
		count_done += g_threads_status[ j ].thread_work_count_done;
	}

	if ( 0 == busy_count )
	{
		if ( combine_results_from_threads )
		{
			// At this point process any Error messages, Uncounted Files, etc.
			CombineThreadResults();
		}
	}
	else
		ThreadSleep( sleep_milliseconds );
#else
	count_done;
	sleep_milliseconds;
	combine_results_from_threads;
#endif

	return busy_count;
}

/*!
*			ThreadAddFileToRead
* Request a thread to read in the Physical lines of a given named file.
* This will return usually before the thread finishes reading.  It is NOT synchronous.
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

/*  COMMENTED OUT.  May be useful sometime.
unsigned int ThreadAddFileToRead( const string fileName, const bool trim_line, 
								SourceFileElement * pSrcFileElement,
								unsigned int & error_count )
{
	unsigned int	retVal = 0;		// Number of pending Requests
	bool			wait_for_NOT_working = false;
	unsigned int	NOT_working_count = 0;

	if ( 0 == fileName.size() )
	{
		wait_for_NOT_working = true;
	}
	
	// Add a file read request for a thread that is ready
	//		OR
	// Wait for all threads to NOT be working
	bool done = false;
	do
	{
		NOT_working_count = 0;

		// Find a thread ready for Work
		for ( unsigned int j = 0; j < g_threads_status_in_use; j++ )
		{
			if ( EXIT_NOW == g_threads_status[ j ].main_WantsAction )
			{
				continue;		// Should NOT get here
			}

			if ( wait_for_NOT_working )
			{
				//  See if the thread is NOT working
				if ( THREAD_WORKING != g_threads_status[ j ].work_ThreadState )
				{
					NOT_working_count++;
					if ( THREAD_HAS_EXITED != g_threads_status[ j ].work_ThreadState )
					{
						// Set the wanted state.  No need to toggle semaphore.
						g_threads_status[ j ].main_WantsAction = NO_WORK_YET;
					}
				}
				
				// boost::this_thread::yield();	//ThreadSleep( 100 );
				continue;
			}
			else
			{
				// Add a file read request for a thread that is ready
				if ( THREAD_WAITING_FOR_WORK != g_threads_status[ j ].work_ThreadState )
					continue;
				
				if ( g_threads_file_names[ j ].size() )
					continue;	// Thread was given a file to read but not given time

				if ( g_threads_status[ j ].thread_had_error )
				{
					error_count = 1;	// some error processing a file
					// Error details are found in the itFileElement first and second elements

				}

				g_threads_pSrcFileElement[ j ]         = pSrcFileElement;
				g_threads_status[ j ].main_trim_line   = trim_line;
				g_threads_file_names[ j ]              = fileName;
				g_threads_status[ j ].main_WantsAction = GET_TO_WORK;

				// Toggle semaphore to wake thread
				g_threads_status[ j ].pSem->signal();

				retVal = 1;		// There will be at least 1 pending request
				done = true;
				boost::this_thread::yield();	// Allow assigned thread (and maybe others) to run
				break;
			}
		}	//	END		for ( unsigned int j = 0; j < g_threads_status_in_use; j++ )

		if ( g_threads_status_in_use == NOT_working_count )
			done = true;

		if ( false == done )
			boost::this_thread::yield();	//ThreadSleep( 100 );

	} while ( false == done );

	return retVal;
}
*/


/*
------------------------------------------------------------------------------
	TOP LEVEL DESIGN  Multiple Thread feature

Primary Design Goal:
	Improve performance ==> less processing time.

Multiple threads: (appropriate Use case)

	There are many files to process and the overall processing time
	takes several minutes (your definition of several) to complete.

Multiple threads: (doing this may give poor results)

	If the processing time without extra threads is less than a minute 
	you may not see less time but rather see increased time. 

	User specifies too many threads for the underlying OS/HW to support.
	Could be where too many threads are competing for:
	CPU, disk resources, memory, etc.  Increased time could result.

Secondary Design Goals:
	Correctness ! ! !
		Produce results as correctly as single threaded implementation.
		Constrain ways for things to go wrong with extra threads (see below).

	Preserve existing single threaded performance
		Try to have minimal impact on existing UCC implementation.

2 DATA POINTS:  (Use cases where Multiple threads may help)

Built Optimized version of UCC_2013_04 for these measurements.
These were gathered on an older Windows 7.1 laptop with AMD 64 bit dual core CPU

TEST 1:
	Linux version 3.11.6  all directories/files using Original version
34,341 C source files plus various other files
Test with Optimized build took 1,884 seconds and CPU use was around 60% to 90%
1,884 seconds = 31.4 minutes
	This is an average of over 18.2 files processed per second

UCC -nowarnings -threads 2 -dir "C:\Linux\Stable_3_11_6\linux-3.11.6" 
-outdir  "C:\Utilities\TEST\UCC\Files_OUT" -ascii
2 Threads started.
        Optimized processing in use ...
Building list of source files...
Read, Analyze & Count keywords in files..DONE
Looking for duplicate files..............DONE
Generating results to files..............DONE
 36,876 files for 52.8 files processed per second

     Processing  Step     :  seconds
------------------------- : --------
 Build file list          :    17
Read, Analyze & Count     :   582
 Find duplicates          :    92
  Generate results files  :     8
               Total time : 11 minutes 39 seconds

TEST 2:
	SMALLER part of Linux (just the arch) directory tree

Original version Optimized build: CPU use was around 60% to 90%
2 minutes 50 seconds or about 73.0 files processed per second

	Using 2 Threads to do Linux arch dir
UCC -threads 2 -dir "C:\Linux\Stable_3_11_6\linux-3.11.6\arch" 
-outdir "C:\Utilities\TEST\UCC\Files_OUT" -ascii
2 Threads started.
        Optimized processing in use ...
Building list of source files...
Read, Analyze & Count keywords in files..DONE
Looking for duplicate files..............DONE
Generating results to files..............DONE
 12418 files for 208.8 files processed per second

     Processing  Step     :  seconds
------------------------- : --------
 Build file list          :     1
Read, Analyze & Count     :    41
 Find duplicates          :    15
  Generate results files  :     2
               Total time :    59 seconds

Overall speed is close to 3x of Original.
Most of the improvement is from changes to Duplicate checking code.
But using 2 threads is definitely faster than original as well.

I have found a fair variation in the time to first build the list of files.
I am guessing this is related to OS file caching.
I usually run the same test 2 times and use the times from the 2nd run.
This allows the OS file caching to work and gives a lower overall time.
This way may not be realistic for Users running UCC 1 time for various directories.

------------------------------------------------------------------------------
	ANALYSIS of Unified Code Count implementation to support Multiple threads

Writing to console from multiple threads:
cout Use:
	Good news is that MainObject.cpp mostly uses cout for Help output
	other uses of cout are for RARE Error messages from CCodeCounter.cpp and a
	few of the Language specific parsing modules.  
	Decision: 
		Leave unchanged except for a few places in MainObject
	TODO: 
		Error messages may be improved by giving more context info.

Messages/Logging Revised:
	Because the total list of files to process is split among N threads:
	there is likely some various messages that each thread may create (Warnings mostly).
	To preserve the same order of messages as if running single threaded, each message
	is saved in a structure with the text and some decision support flags.
	After the threads finish work, the message structures are used to call expected
	message/logging helpers as if in single thread mode.
	Downside to this approach is that messages from threads are no longer "real time"
	Upside is that same order of messages is presented (just a little later).

	The code could be changed so when the main thread wakes up to check for all threads
	finishing, it could also start processing the structures from just the first thread.
	This would give a pseudo "real time" feel but ONLY for messages from the first thread.
	Another possible downside to this would be extra logging disk I/O that may take away 
	from what disk I/O the threads are using.

How Threads update the UI % completion
	Multiple threads updating the UI directly is NOT recommended (unless UI is designed that way).
		Also I did not have access to GUI code to try out.
	
	Threads calculate the count down number of files to process to show 1/2 of 1 % progress changed.
	When the count down reaches Zero, a thread will update a count in a shared structure 
	that the main thread a fraction of a second later will read.  This is a classic example of
	"False Sharing" which can be a big decrease on multithreaded performance.  So I tried to
	minimize the number of counts done updates by a thread while still giving decent UI update info.
*/
