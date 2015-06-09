//! Differencing tool class methods.
/*!
* \file DiffTool.cpp
*
* This file contains the differencing tool class methods.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_06
*   Addition of Multithreading performance enhancement 
*	Refactored Differencing for much faster performance
*	Added support to Time various steps
*/

#include "UCCGlobals.h"
#include "DiffTool.h"
#include "CUtil.h"
#include "PrintUtil.h"

using namespace std;

extern	UserIF *	userIF;								//!< User interface for presenting messages/progress to user

// Fill in some variables used to get timing of various steps
// These are in order of when time is set for them
extern	time_t	time_end_list_built;
extern	time_t	time_end_files_read;
extern	time_t	time_end_files_readB;
extern	time_t	time_end_match_baselines;	// Differencing step only
extern	time_t	time_end_files_analysis;
extern	time_t	time_end_find_duplicates;
extern	time_t	time_end_print_results;

// Differencing steps only
extern	time_t	time_end_files_analysisB;
extern	time_t	time_end_find_duplicatesB;
extern	time_t	time_end_print_resultsB;
extern	time_t	time_end_match_baselines_web;
extern	time_t	time_end_process_pairs;
extern	time_t	time_end_print_resultsLast;

/*!
* Constructs a DiffTool object.
*/
DiffTool::DiffTool()
{
	// this is the summary count information
	isDiff = true;
	total_addedLines = total_deletedLines = total_modifiedLines = total_unmodifiedLines = 0;
	dup_addedLines = dup_deletedLines = dup_modifiedLines = dup_unmodifiedLines = 0;
	printDup = false;
	match_threshold = 60;	// default modified lines threshold
}

/*!
* DiffTool process to replace MainProcess when differencing.
*
* \param argc number of arguments
* \param argv argument list
*
* \return function status
*/
int DiffTool::diffToolProcess(int argc, char *argv[])
{
	/*
	Original Differencing Process:
		1. Parse options
		2. Find files to work on
		3. Read input files found into memory
		4. Match initial file pairs
		5. Run counter on each set of baseline files
		6. Identify duplicate files within same baseline
		7. Print all counting results data
		8. Match file pairs for web separation files
		9. For each pair, run differencing functionality on SLOC
		10. Print all remaining results data

	Optimized or Multithreaded Differencing (threaded ops like this):
		1. Parse options, User sets either -threads 1 or more
		2. Start worker Threads if 2 or more wanted.  
			1 uses Optimized path here with a single thread.
		3. Find A & B files to work on
		4  Divide list A of Files for woker threads ... and Repeat once for list B
	(	5.	Threads use their part of list to:
			Read A & B input files found into memory AND ... )
	(	6. Analyze and do Counts for each file read )
		7. Match initial file pairs
		8. Identify duplicate files within same baseline
		9. Print all counting results data
		10. Match file pairs for web separation files
		11. For each pair, run differencing functionality on SLOC
		12. Stop worker threads if started
		13. Print all remaining results data

	TODO: Figure out how to divide and conquer the Match, Duplicate, Difference steps !
		Printing by its nature should remain single threaded and done by the main thread.
		Suggest moving all the Printing to be at step 13 instead of 9 and 13 now.
	*/

	// create a user interface object
	if (userIF == NULL)
		userIF = new UserIF();

	// parse the command line input
	if (!ParseCommandLine(argc, argv))
		ShowUsage();
	SetCounterOptions( CounterForEachLanguage );

	// handle input file lists
	BaselineFileName1 = BASELINE_INF1;
	BaselineFileName2 = BASELINE_INF2;
#ifdef QTGUI
	if (outDir != "")
	{
		BaselineFileName1 = outDir + BASELINE_INF1;
		BaselineFileName2 = outDir + BASELINE_INF2;
	}
#endif
	if (HasUserCancelled())
		return 0;

	// generate user-defined language extension map
	if (userExtMapFile.length() != 0)
		ReadUserExtMapping(userExtMapFile);

	// Start worker threads or Enable Optimizations if wanted
	string	start_threads_result_msg;
	StartThreads( start_threads_result_msg );
	if ( start_threads_result_msg.size() )
		userIF->updateProgress( start_threads_result_msg, false );

	if ( g_process_after_read )
		userIF->updateProgress("\tOptimized processing in use ...");

	// Build List and Read (and maybe Analyze/Count) both A and B source file lists
	if ( !ReadAllDiffFiles() )	// Set times & Shows 2 msgs: Making list and Reading files
		return 0;

	if ( ! g_process_after_read )
	{
		// count Baseline A and prepare SLOC for differencing
		userIF->updateProgress("Performing files analysis and counting: A.........", false);
		ProcessSourceList(true);
		if (HasUserCancelled())
			return 0;

		time( &time_end_files_analysis );

		// count Baseline B and prepare SLOC for differencing
		userIF->updateProgress("Performing files analysis and counting: B ........", false);
		ProcessSourceList(false);
		if (HasUserCancelled())
			return 0;

		time( &time_end_files_analysisB );
	}

	// match files in BaselineA to BaselineB (does not include web separation files)
	userIF->updateProgress("Performing files matching.....................", false);
	if (HasUserCancelled())
		return 0;

	MatchBaseLines();
	PrintMatchedPairs();	// Print time included as part of Match Baselines

#ifndef QTGUI
	userIF->updateProgress("DONE");
#endif

	if (duplicate_threshold >= 0.0)
	{
		time( &time_end_match_baselines );

		// This will show % done
		userIF->updateProgress("Looking for duplicate files in Baseline-A ........", false);
		FindDuplicateFiles(SourceFileA, duplicateFilesInA1, duplicateFilesInA2, true);

		if (HasUserCancelled())
			return 0;

		// Include this small time as part of A Duplicates	0 to 1 second (rounded)
		UpdateCounterCounts( CounterForEachLanguage, &SourceFileA, true, true );
		time( &time_end_find_duplicates );
	}
	else
	{
		// Include this small time as part of Match Baselines	0 to 1 second (rounded)
		UpdateCounterCounts( CounterForEachLanguage, &SourceFileA, true, true );
		time( &time_end_match_baselines );
	}

	// print the counting results for BaselineA (SLOC counts and complexity counts)
	if (print_unified)
		PrintTotalCountResults( CounterForEachLanguage, true, "Baseline-A-", &duplicateFilesInA2 );
	else
		PrintCountResults( CounterForEachLanguage, true, "Baseline-A-", &duplicateFilesInA2);

	if (print_cmplx)
		PrintComplexityResults( CounterForEachLanguage, true, "Baseline-A-" );

	if (duplicate_threshold >= 0.0)
	{
		if (print_unified)
			PrintTotalCountResults( CounterForEachLanguage, true, "Duplicates-A-", &duplicateFilesInA2, false);
		else
			PrintCountResults( CounterForEachLanguage, true, "Duplicates-A-", &duplicateFilesInA2, false);

		if (print_cmplx)
			PrintComplexityResults( CounterForEachLanguage, true, "Duplicates-A-", true);
		PrintDuplicateSummary(true, "Duplicates-A-");
	}

	if (duplicate_threshold >= 0.0)
	{
		time( &time_end_print_results );

		// This will show % done
		userIF->updateProgress("Looking for duplicate files in Baseline-B ........", false);
		FindDuplicateFiles(SourceFileB, duplicateFilesInB1, duplicateFilesInB2, true);

		if (HasUserCancelled())
			return 0;

		// Include this small time as part of B Duplicates	0 to 1 second (rounded)
		UpdateCounterCounts( CounterForEachLanguage, &SourceFileB, false, true );
		time( &time_end_find_duplicatesB );
	}
	else
	{
		// Include this small time as part of print results A	0 to 1 second (rounded)
		UpdateCounterCounts( CounterForEachLanguage, &SourceFileB, false, true );
		time( &time_end_print_results );
	}

	// print the counting results for BaselineB (SLOC counts and complexity counts)
	if (print_unified)
		PrintTotalCountResults( CounterForEachLanguage, false, "Baseline-B-", &duplicateFilesInB2);
	else
		PrintCountResults( CounterForEachLanguage, false, "Baseline-B-", &duplicateFilesInB2);

	if (print_cmplx)
		PrintComplexityResults( CounterForEachLanguage, false, "Baseline-B-");
	if (duplicate_threshold >= 0.0)
	{
		if (print_unified)
			PrintTotalCountResults( CounterForEachLanguage, false, "Duplicates-B-", &duplicateFilesInB2, false);
		else
			PrintCountResults( CounterForEachLanguage, false, "Duplicates-B-", &duplicateFilesInB2, false);

		if (print_cmplx)
			PrintComplexityResults( CounterForEachLanguage, false, "Duplicates-B-", true);
		PrintDuplicateSummary(false, "Duplicates-B-");
	}

	time( &time_end_print_resultsB );

	// compare the matched files in BaselineA to BaselineB
	// perform matching on web separation files since these do not exist when initially matched
	// This will show % completion
	userIF->updateProgress("Performing files comparison.......................", false);
	if (HasUserCancelled())
		return 0;
	if (CounterForEachLanguage[WEB]->total_filesA > 0 ||
		CounterForEachLanguage[WEB]->total_filesB > 0)
	{
		MatchBaseLines(true);

		time( &time_end_match_baselines_web );
	}
	else
		time_end_match_baselines_web = time_end_print_resultsB;

	ProcessPairs();

#ifndef QTGUI
	userIF->updateProgress("\b\b\b\bDONE");
#endif

	time( &time_end_process_pairs );

	// print the comparison results
	userIF->updateProgress("Generating results to files...................", false);
	if (HasUserCancelled())
		return 0;
	PrintDiffResults();
#ifndef QTGUI
	userIF->updateProgress("DONE");
#endif

	time( &time_end_print_resultsLast );

	return 1;
}

/*!
* Reads all files into memory for both baselines.  If wanted Analysis & Counting are done also.
*
* \Global	g_process_after_read	true will also do Analysis and Counting keywords
*
* \return method status		1 = OK else something else
*/
int DiffTool::ReadAllDiffFiles()
{
	int		retVal = 1;
	// flag to indicate if the files are to be read from fileList<A|B>.txt
	// or from the parameters mentioned at command line

	// Make a list of input source files
	userIF->updateProgress("Building list of source files...\n", false);

	vector<string> listFileNamesA;
	vector<string> listFileNamesB;
	if (use_CommandLine)
	{
		CUtil::ListAllFiles(dirnameA, listFilesToBeSearched, listFileNamesA, followSymLinks);
		CUtil::ListAllFiles(dirnameB, listFilesToBeSearched, listFileNamesB, followSymLinks);
		BaselineFileName1 = BaselineFileName2 = "";
	}

	time( &time_end_list_built );

	// read the source files
	if ( g_process_after_read )
		userIF->updateProgress("Read, Analyze & Count keywords A files............", false);
	else
		userIF->updateProgress("Reading source A files............................", false);
	if (HasUserCancelled())
		return 0;

	retVal = ReadAllFiles( listFileNamesA, BaselineFileName1, true );
	if ( g_process_after_read )
	{
		time( &time_end_files_analysis );
		time_end_files_read = time_end_files_analysis;
	}
	else
		time( &time_end_files_read );

	if ( 1 == retVal )
	{
		if ( g_process_after_read )
		{
			userIF->updateProgress("Read, Analyze & Count keywords B files............", false);
			time( &time_end_files_analysisB );
		}
		else
			userIF->updateProgress("Reading source B files............................", false);

		retVal = ReadAllFiles( listFileNamesB, BaselineFileName2, false );
		if ( g_process_after_read )
		{
			time( &time_end_files_analysisB );
			time_end_files_readB = time_end_files_analysisB;
		}
		else
			time( &time_end_files_readB );
	}

	return retVal;
}

/*!
* Matches each file between the two baselines with the best possible match.
* 1. Build a preference list for each file
*    1a. Preference list is an ordered list of files in the other baseline in order of the quality of the match
* 2. Run the Gale-Shapley algorithm to create a matching
* 3. Files that are unmatched will be paired with an empty file to be compared to
* Two files are matched if and only if they have the same sort name.
*
* \param webSepFilesOnly matching only web separation files?
*/
void DiffTool::MatchBaseLines(bool webSepFilesOnly)
{
	// list of files each with a preferences list
	BaselinePreferenceMapType BaseAPrefs, BaseBPrefs;

	// go through the SourceFileA list and compare each to SourceFileB to generate a preference
	// BaselineA, but first check if it is empty
	if (SourceFileA.size() == 0) 
	{
		for (SourceFileList::iterator j = SourceFileB.begin(); j != SourceFileB.end(); j++) 
			BaseBPrefs[&(*j)] = NULL;
	}

	// create the multimaps that will be used
	// the key for the map will be the file name
	SortedPreferenceMapType mapA;
	SortedPreferenceMapType mapB;

	// build multimap of all the file names for A
	for (SourceFileList::iterator i = SourceFileA.begin(); i != SourceFileA.end(); i++)
		mapA.insert(FileNamePair((*i).second.file_name_only, &(*i)));

	// build multimap of all the file names for B
	for (SourceFileList::iterator i = SourceFileB.begin(); i != SourceFileB.end(); i++)
		mapB.insert(FileNamePair((*i).second.file_name_only, &(*i)));

	PreferenceMapType *myAPrefs, *myBPrefs;
	BaselinePreferenceMapType::iterator sfBIterator;
	PreferenceStruct myAPS, myBPS;
	int ourPreference;
	for (SortedPreferenceMapType::iterator i = mapA.begin(); i != mapA.end(); i++)
	{
		if ( webSepFilesOnly && ( (*i).second->second.file_name_isEmbedded == false ) )
			continue;

		myBPrefs = new PreferenceMapType;

		// BaselineB
		SortedPreferenceMapType::iterator stop_point = mapB.upper_bound((*i).second->second.file_name_only);
		for (SortedPreferenceMapType::iterator j = mapB.lower_bound((*i).second->second.file_name_only); j != stop_point; j++)
		{
			if ( webSepFilesOnly && ( (*j).second->second.file_name_isEmbedded == false ) )
				continue;

			// need to check if this file has a preference list, if so use it, if not create it
			sfBIterator = BaseBPrefs.find((*j).second);
			if (sfBIterator != BaseBPrefs.end())
			{
				// already exists, use it
				myAPrefs = (*sfBIterator).second;
			}
			else
			{
				// does not exist, create one
				myAPrefs = new PreferenceMapType;
			}

			if ( (*i).second->second.file_name_only.compare( (*j).second->second.file_name_only ) )
			{
				// Names different so no need to call CompareFilePaths
				ourPreference = MAX_MISMATCH_COST;	// this should instead be based on the size of file1 and/or file2.
			}
			else
			{
				// compare this file path to the file path from A
				ourPreference = CompareFilePaths( (*i).second->second.file_name, 
												(*j).second->second.file_name, 
												(*i).second->second.file_name_only.size() );
			}

			// update preference lists for both files
			myBPS.fileElement = (*j).second;
			myBPS.value = ourPreference;
			myAPS.fileElement = (*i).second;
			myAPS.value = ourPreference;
			myBPrefs->push_back(myBPS);
			myAPrefs->push_back(myAPS);

			// make sure its sorted by preference value
			sort(myAPrefs->begin(), myAPrefs->end(), CustomCMP());

			// save the new values for the BaseBPrefs
			BaseBPrefs[(*j).second] = myAPrefs;
		}

		// make sure its sorted by preference value
		sort(myBPrefs->begin(), myBPrefs->end(), CustomCMP());

		// save the new values for the BaseAPrefs
		BaseAPrefs[(*i).second] = myBPrefs;
	}

	// add empty preference lists for unmatched B files
	for (SortedPreferenceMapType::iterator j = mapB.begin(); j != mapB.end(); j++)
	{
		if ( webSepFilesOnly && ( (*j).second->second.file_name_isEmbedded == false ) )
			continue;

		// need to check if this file has a preference list, if so use it, if not create it
		sfBIterator = BaseBPrefs.find((*j).second);
		if (sfBIterator == BaseBPrefs.end())
		{
			// does not exist, create one
			myBPrefs = new PreferenceMapType;

			// save the new values for the BaseBPrefs
			BaseBPrefs[(*j).second] = myBPrefs;
		}
	}

	// at this point we have a complete sorted preference list for all pairs
	// we will want to match preferences that have the smallest value as it is actually the value of optimal alignment
	// now run the Gale-Shapley algorithm

	list<SourceFileElement *> FreeBaseAList;
	map<SourceFileElement *, PreferenceMapType::iterator> NextSelectionMapBaseA;
	BaselineFileMapType BaseAMatches, BaseBMatches;
	list<SourceFileElement *>::iterator myAFile;
	PreferenceMapType::iterator myBFile;
	for (BaselinePreferenceMapType::iterator myI = BaseAPrefs.begin(); myI != BaseAPrefs.end(); myI++)
	{
		BaseAMatches[(*myI).first] = NULL;
		FreeBaseAList.push_back((*myI).first);
		NextSelectionMapBaseA.insert(make_pair((*myI).first, (*myI).second->begin()));
	}
	
	// do this for completeness later on
	for (BaselinePreferenceMapType::iterator myI = BaseBPrefs.begin(); myI != BaseBPrefs.end(); myI++)
		BaseBMatches[(*myI).first] = NULL;

	BaselineFileMapType::iterator mLocation;
	SourceFileElement *myAFilePrime;
	bool foundAPrimeFirst;
	while (!FreeBaseAList.empty())
	{
		// find a file in BaselineA that is unmatched and has not tried to match against all of files in BaselineB
		myAFile = FreeBaseAList.begin();
		myBFile = NextSelectionMapBaseA[(*myAFile)];
		if (myBFile == BaseAPrefs[(*myAFile)]->end())
		{
			// it has checked all and is not a good match, match to NULL
			BaseAMatches[(*myAFile)] = NULL;

			// remove it from the "Free" list
			FreeBaseAList.pop_front();
			continue;
		}

		// if BFile is unmatched, then pair them...
		mLocation = BaseBMatches.find((*myBFile).fileElement);
		if (mLocation == BaseBMatches.end() || (*mLocation).second == NULL)
		{
			// not matched up, pair it!
			// if the filename is the same, match them
			if ((*myBFile).fileElement->second.file_name_only.compare(
				(*myAFile)->second.file_name_only) == 0)
			{
				BaseAMatches[(*myAFile)] = (*myBFile).fileElement;
				BaseBMatches[(*myBFile).fileElement] = (*myAFile);
			}
			else
			{
				// otherwise, match them to NULL
				BaseAMatches[(*myAFile)] = NULL;
				BaseBMatches[(*myBFile).fileElement] = NULL;
			}

			// now increase NextSelection for A as it has now examined this preference file entry
			NextSelectionMapBaseA[(*myAFile)] = ++myBFile;

			// remove this A file from free list
			FreeBaseAList.pop_front();
		}
		else
		{
			// this preference is already matched, check stability
			// this is the file in A that B is matched to right now
			myAFilePrime = (*mLocation).second;

			// check to see if FileB prefers AFile over AFilePrime
			foundAPrimeFirst = false;
			for (PreferenceMapType::iterator myI = BaseBPrefs[(*myBFile).fileElement]->begin();
				myI != BaseBPrefs[(*myBFile).fileElement]->end(); myI++)
			{
				if ((*myI).fileElement == myAFilePrime)
				{
					// found the Prime
					foundAPrimeFirst = true;
					break;
				}
				else if ((*myI).fileElement == (*myAFile))
				{
					// found the AFile
					break;
				}
			}
			if (foundAPrimeFirst)
			{
				// file APrime and FileB are a better match, keep it that way
				// now FileA remains unmatched but increment its next selection
				NextSelectionMapBaseA[(*myAFile)] = ++myBFile;

				//now check to see if it has checked all the files, if so match it with NULL
				if (myBFile == BaseAPrefs[(*myAFile)]->end())
				{
					// it has checked all and is not a good match, match to NULL
					BaseAMatches[(*myAFile)] = NULL;

					// remove it from the "Free" list
					FreeBaseAList.pop_front();
				}
			}
			else
			{
				// FileB and FileA are better matches....
				// free FileAPrime or match it to NULL
				BaseAMatches[myAFilePrime] = NULL;

				// now check to see if it has checked all the files
				if (NextSelectionMapBaseA[myAFilePrime] != BaseAPrefs[myAFilePrime]->end())
				{
					// has not checked all, put back on free list
					FreeBaseAList.push_back(myAFilePrime);
				}

				// if FileA and FileB have the same name, match them
				if ((*myBFile).fileElement->second.file_name_only.compare(
					(*myAFile)->second.file_name_only) == 0)
				{
					// match FileA and FileB
					BaseAMatches[(*myAFile)] = (*myBFile).fileElement;
					BaseBMatches[(*myBFile).fileElement] = (*myAFile);
				}
				else
				{
					// match FileA and FileB to NULL
					BaseAMatches[(*myAFile)] = NULL;
					BaseBMatches[(*myBFile).fileElement] = NULL;
				}
				NextSelectionMapBaseA[(*myAFile)] = ++myBFile;

				// remove this AFile from free list.
				FreeBaseAList.pop_front();				
			}
		}
	}

	// now attempt to match unmatched files by checking content
	vector<lineElement>::iterator baseLine, compareLine;
	for (BaselineFileMapType::iterator i = BaseAMatches.begin(); i != BaseAMatches.end(); i++)
	{
		if ((*i).second == NULL)
		{
			for (BaselineFileMapType::iterator j = BaseBMatches.begin(); j != BaseBMatches.end(); j++)
			{
				if ((*j).second == NULL)
				{
					SourceFileElement* fileA = (*i).first;
					SourceFileElement* fileB = (*j).first;
					if (fileA->first.size() == fileB->first.size())
					{
						if (fileA->first.size() < 1 || fileB->first.size() < 1)
							continue;	//don't match empty file with different names

						baseLine = fileA->first.begin();
						compareLine = fileB->first.begin();
						while (baseLine != fileA->first.end() && compareLine != fileB->first.end())
						{
							if ((*baseLine).line.compare((*compareLine).line) != 0)
								break;
							baseLine++;
							compareLine++;
						}
						if (baseLine == fileA->first.end() && compareLine == fileB->first.end())
						{
							// match files
							(*i).second = fileB;
							(*j).second = fileA;
						}
					}
				}
			}
		}
	}

	// now we have our matches in BaseAMatches
	// now go through all files in A, if matched create pair, if unmatched pair with NULL
	// then go through B, if matched, ignore since already added from A, if unmatched pair with NULL
	// process ordering based on input lists
	for (SourceFileList::iterator i = SourceFileA.begin(); i != SourceFileA.end(); i++)
	{
		if ( webSepFilesOnly && ( (*i).second.file_name_isEmbedded == false ) )
			continue;

		// get the corresponding match
		mLocation = BaseAMatches.find(&(*i));
		if (mLocation != BaseAMatches.end())
		{
			resultStruct myResults;
			MatchedFilePair myPair = make_pair((*mLocation).first, (*mLocation).second);
			matchedFilesList.push_back(make_pair(myResults, myPair));
			if (mLocation->second != NULL)
				(*i).second.matched = true;
		}
	}

	for (SourceFileList::iterator i = SourceFileB.begin(); i != SourceFileB.end(); i++)
	{
		if ( webSepFilesOnly && ( (*i).second.file_name_isEmbedded == false ) )
			continue;

		// get the corresponding match
		mLocation = BaseBMatches.find(&(*i));
		if (mLocation != BaseBMatches.end())
		{
			if (mLocation->second == NULL)
			{
				resultStruct myResults;
				MatchedFilePair myPair = make_pair((*mLocation).second, (*mLocation).first);
				matchedFilesList.push_back(make_pair(myResults, myPair));
			}
			else
				(*i).second.matched = true;
		}
	}

	// at this point we have the complete match set in matchedFilesList
	for (BaselinePreferenceMapType::iterator myI = BaseAPrefs.begin(); myI != BaseAPrefs.end(); myI++)
		delete (*myI).second;
	for (BaselinePreferenceMapType::iterator myI = BaseBPrefs.begin(); myI != BaseBPrefs.end(); myI++)
		delete (*myI).second;
}

/*!
* Calculates the Levenshtein distance between the two file names.
* Only compare two files that have the same sort name (excluding the path).
* Calling code must compare the file name only part and NOT call this if not equal names.
*
* \param file1 original file
* \param file2 comparison file
*
* \return case-sensitive Levenshtein distance between file names
*/
int DiffTool::CompareFilePaths( const string &file1, const string &file2, const unsigned int file_name_length )
{
// Profiling Using new 2 threads to do optimized Read and Analyze and NO Duplicate checks
// Command line: split for readability
// -nodup -threads 2 -d
// -dir "C:\Linux\Stable_3_11_6\linux-3.11.6\arch" 
//      "C:\Linux\linux_3_13_4\arch" 
// -outdir "C:\TEST\UCC\Files_OUT" -ascii
//
// 22.8% of the TOTAL Time is spent here ! ! !  This included all the time for Read/Analyze
// 16.7% of the total Time now after changes below.  So 136% as fast as before.
//
	int delta = 1;
	int mismatchCost = 1;

//	if (CUtil::ExtractFilename(file1).compare(CUtil::ExtractFilename(file2)) != 0)
//		return MAX_MISMATCH_COST;	// this should instead be based on the size of file1 and/or file2.
	// it would be faster to strip off the filenames, since they must match.

	// Strip off the filename from the rest of the paths
	string	path1 = file1.substr( 0, file1.size() - file_name_length );
	string	path2 = file2.substr( 0, file2.size() - file_name_length );

	// Find values here that do NOT change within the LOOP
	int		path1_size = path1.size();
	int		path2_size = path2.size();

	// Declare values here that are set inside the LOOP
	int		min_B_B    = 0;
	int		min_Cost_B = 0;

	// This function compares the file paths.
	// Compute the Levenshtein distance between the two file paths
	// comparison is always case-sensitive, even in Windows
	//
	// Initialize an array of size 2 arrays.  This is very inefficient.  It is
	// much better to define a 1-D array and use an inline function or macro
	// to simulate two indices.
	//
	// Inefficient in what way?  Memory or Time or ?
	// Were other approaches implemented and benchmarked?  Thanks!  Randy Maxwell
	//
	int** B = new int*[(int)path1_size+1];

	// initialize B[i,0] = i*delta
	for (int i = 0; i <= path1_size; i++)
	{
		B[i] = new int[2];
		B[i][0] = i * delta;
	}
	for (int j = 1; j <= path2_size; j++)
	{
		B[0][1] = j * delta;
		for (int i = 1; i <= path1_size; i++)
		{
			int ourMatchCost = (path1[i-1] == path2[j-1]) ? 0 : mismatchCost;

		// The below code is the main HOTSPOT in this procedure
		// Looking at the C++ optimized code emitted there were 2 calls to min() C library procedure
		// Replaced the calls with if comparisons and now faster (about 136% of original)
		//
		//	B[i][1] = min(ourMatchCost + B[i-1][0],
		//		min(delta + B[i-1][1], delta + B[i][0]));
		//
			min_B_B = B[i-1][1];
			if ( B[i][0] < min_B_B )
				min_B_B = B[i][0];
			
			min_Cost_B = ourMatchCost + B[i-1][0];
			if ( delta + min_B_B < min_Cost_B )
				min_Cost_B = delta + min_B_B;
			
			B[i][1] = min_Cost_B;
		}
		for (int i = 0; i <= path1_size; i++)
			B[i][0] = B[i][1];
	}

	// the optimal alignment value is in B[ path1_size, 1 ]
	int rValue = B[path1_size][1];
	for (int i = 0; i <= path1_size; i++)
		delete[] B[i];
	delete[] B;

	return rValue;
}

/*!
* Executes the differencing comparison on the mapped pairs.
* Assumes that the SLOC lines have been put int the mySLOCLines object in the results object of each file.
* Shows % completed for the User
*/
void DiffTool::ProcessPairs()
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

	CmpMngr myDiffManager;
	srcLineVector* myNullFile = new srcLineVector();
	srcLineVector* firstFile;
	srcLineVector* secondFile;

	string current_file_name;
	typedef vector< pair<bool, MatchingType::iterator> > WebFilesVector;
	WebFilesVector web_files_vector;
	string web_file_name, base_file_name;
	size_t web_marker;
	bool unmatchedDup, found;

	unsigned int	count_done        = 0;
	unsigned int	percent_done      = 0;
	unsigned int	prev_percent_done = 0;
	unsigned int	num_in_list       = matchedFilesList.size();

	for (MatchingType::iterator myI = matchedFilesList.begin(); myI != matchedFilesList.end(); myI++)
	{
		// each source file elements results object has a mySLOCLines object with the SLOC to be diffed
		// check for unmatched duplicates
		firstFile = myNullFile;
		secondFile = myNullFile;
		unmatchedDup = false;
		web_file_name = "";

		if ((*myI).second.first != NULL)
		{
			firstFile = &(*myI).second.first->second.mySLOCLines;

			// keep track of classes of type WEB
			current_file_name = myI->second.first->second.file_name;
			if (myI->second.first->second.class_type == WEB)
				web_file_name = current_file_name;

			if ((*myI).second.first->second.duplicate || (*myI).second.first->second.firstDuplicate)
				unmatchedDup = true;
		}
		if ((*myI).second.second != NULL)
		{
			secondFile = &(*myI).second.second->second.mySLOCLines;

			// keep track of classes of type WEB
			current_file_name = myI->second.second->second.file_name;
			if (myI->second.second->second.class_type == WEB)
				web_file_name = current_file_name;	// always overwrite with the second file if it exists

			if ((*myI).second.second->second.duplicate || (*myI).second.second->second.firstDuplicate)
			{
				if ((*myI).second.first == NULL)
					unmatchedDup = true;
				else
					unmatchedDup = false;
			}
			else
				unmatchedDup = false;
		}

		// this makes sure that if one of the files was unmatched it will just compare it against an empty set
		myDiffManager.Compare(firstFile, secondFile, match_threshold);

		// if the file is a class of type WEB, keep a list
		if (web_file_name.length() > 0)
		{
			// always use the WEB file status, not subsets
			web_files_vector.push_back(pair<bool, MatchingType::iterator>(unmatchedDup, myI));
		}
		else
		{
			// check to see if file is a web subset
			web_marker = current_file_name.find(EMBEDDED_FILE_PREFIX);
			if (web_marker != string::npos)
			{
				// if it is a web subset, search through the captured WEB files
				// assumes the web files are populated prior to the web subsets
				found = false;
				for (WebFilesVector::iterator web_iter = web_files_vector.begin();
					web_iter != web_files_vector.end(); ++web_iter)
				{
					base_file_name = current_file_name.substr(0, web_marker);
					if (web_iter->second->second.second != NULL)
					{
						if (base_file_name.compare(web_iter->second->second.second->second.file_name) == 0)
							found = true;
					}
					if (!found && web_iter->second->second.first != NULL)
					{
						if (base_file_name.compare(web_iter->second->second.first->second.file_name) == 0)
							found = true;
					}
					if (found)
					{
						web_iter->second->first.addedLines += myDiffManager.nAddedLines;
						web_iter->second->first.deletedLines += myDiffManager.nDeletedLines;
						web_iter->second->first.modifiedLines += myDiffManager.nChangedLines;
						web_iter->second->first.unmodifiedLines += myDiffManager.nNochangedLines;
						unmatchedDup = web_iter->first;
						break;
					}
				}
			}
		}

		if (unmatchedDup)
		{
			printDup = true;
			this->dup_addedLines += (*myI).first.addedLines = myDiffManager.nAddedLines;
			this->dup_deletedLines += (*myI).first.deletedLines = myDiffManager.nDeletedLines;
			this->dup_modifiedLines += (*myI).first.modifiedLines = myDiffManager.nChangedLines;
			this->dup_unmodifiedLines += (*myI).first.unmodifiedLines = myDiffManager.nNochangedLines;
		}
		else
		{
			this->total_addedLines += (*myI).first.addedLines = myDiffManager.nAddedLines;
			this->total_deletedLines += (*myI).first.deletedLines = myDiffManager.nDeletedLines;
			this->total_modifiedLines += (*myI).first.modifiedLines = myDiffManager.nChangedLines;
			this->total_unmodifiedLines += (*myI).first.unmodifiedLines = myDiffManager.nNochangedLines;
		}

		count_done++;
		percent_done = ( count_done * 100 ) / num_in_list;
		if ( percent_done != prev_percent_done )
		{
			// Percent changed so update UI
			userIF->updateProgress( "", false, percent_done, num_in_list );
			prev_percent_done = percent_done;
		}
	}
	delete myNullFile;

#ifdef	_DEBUG
	if ( call_again )
	{
		validate = true;
		HeapInUse( errorRet, end_block_count, end_total_sizes, validate );
		if ( ( end_block_count != start_block_count ) 
			|| ( end_total_sizes != start_total_sizes ) )
		{
			printf( "\n\tMemory LEAK or SHRINK likely inside ProcessPairs\nIncrease Number of blocks is %d\nIncrease Allocation size is %d",
					end_block_count - start_block_count, end_total_sizes - start_total_sizes );
		}
	}
#endif

}

/*!
* Print the matching pairs of files.
*/
void DiffTool::PrintMatchedPairs()
{
	ofstream pairFile, pairFileCSV;
	if (print_ascii || print_legacy)
	{
		string fName = outDir + MATCH_PAIRS_OUTFILE;
		pairFile.open(fName.c_str(), ofstream::out);
		if (!pairFile.is_open())
		{
			string err = "Error: Failed to open matched file pairs summary output file (";
			err += MATCH_PAIRS_OUTFILE;
			err += ")";
			userIF->AddError(err);
			return;
		}

		CUtil::PrintFileHeader(pairFile, "MATCHED FILE PAIRS", cmdLine);
		pairFile.setf(ofstream::left);
		pairFile.width(45);
		pairFile << "Baseline A";
		pairFile.unsetf(ofstream::left);
		pairFile.width(5);
		pairFile << "  |  ";
		pairFile.width(3);
		pairFile.setf(ofstream::left);
		pairFile.width(45);
		pairFile << "Baseline B";
		pairFile << endl;
		for (int y = 0; y < 90; y++)
			pairFile << "-";
	}
	if (print_csv)
	{
		string fName = outDir + MATCH_PAIRS_OUTFILE_CSV;
		pairFileCSV.open(fName.c_str(), ofstream::out);
		if (!pairFileCSV.is_open())
		{
			string err = "Error: Failed to open matched file pairs summary output file (";
			err += MATCH_PAIRS_OUTFILE_CSV;
			err += ")";
			userIF->AddError(err);
			return;
		}

		CUtil::PrintFileHeader(pairFileCSV, "MATCHED FILE PAIRS", cmdLine);
		pairFileCSV << "Baseline A,Baseline B" << endl;
	}

	// print the results of matching pairs in the file in formatted manner
	string fileA, fileB;
	for (MatchingType::iterator myI = matchedFilesList.begin(); myI != matchedFilesList.end(); myI++)
	{
		if ((*myI).second.first == NULL)
			fileA = "NA";
		else
			fileA = (*myI).second.first->second.file_name;

		if ((*myI).second.second == NULL)
			fileB = "NA";
		else
			fileB = (*myI).second.second->second.file_name;

		if (print_ascii || print_legacy)
		{
			pairFile << endl;
			pairFile.setf(ofstream::left);
			pairFile.width(45);
			pairFile << fileA;
			pairFile.unsetf(ofstream::left);
			pairFile.width(5);
			pairFile << "  |  ";
			pairFile.setf(ofstream::left);
			pairFile.width(45);
			pairFile << fileB;
		}
		if (print_csv)
			pairFileCSV << fileA << "," << fileB << endl;
	}
	if (print_ascii || print_legacy)
		pairFile.close();
	if (print_csv)
		pairFileCSV.close();
}

/*!
* Prints the results of the file differencing.
*/
void DiffTool::PrintDiffResults()
{
	// open the diff results output file
	if (print_ascii || print_legacy)
	{
		string fName = outDir + DIFF_OUTFILE;
		outfile_diff_results.open(fName.c_str(), ofstream::out);
		if (!outfile_diff_results.is_open())
		{
			string err = "Error: Failed to open diff results output file (";
			err += DIFF_OUTFILE;
			err += ")";
			userIF->AddError(err);
			return;
		}
		if (printDup)
		{
			fName = outDir + "Duplicates-";
			fName += DIFF_OUTFILE;
			dup_outfile_diff_results.open(fName.c_str(), ofstream::out);
			if (!dup_outfile_diff_results.is_open())
			{
				string err = "Error: Failed to open duplicates diff results output file (";
				err += fName;
				err += ")";
				userIF->AddError(err);
				return;
			}
		}
	}
	if (print_csv)
	{
		string fName = outDir + DIFF_OUTFILE_CSV;
		outfile_diff_csv.open(fName.c_str(), ofstream::out);
		if (!outfile_diff_csv.is_open())
		{
			string err = "Error: Failed to open diff results output file (";
			err += DIFF_OUTFILE_CSV;
			err += ")";
			userIF->AddError(err);
			return;
		}
		if (printDup)
		{
			fName = outDir + "Duplicates-";
			fName += DIFF_OUTFILE_CSV;
			dup_outfile_diff_csv.open(fName.c_str(), ofstream::out);
			if (!dup_outfile_diff_csv.is_open())
			{
				string err = "Error: Failed to open duplicates diff results output file (";
				err += fName;
				err += ")";
				userIF->AddError(err);
				return;
			}
		}
	}

	// print the diff results header.
	string myCats[] = {"New", "Deleted", "Modified", "Unmodified", "Module"};
	int i, y, numCats = 5;
	if (print_ascii || print_legacy)
	{
		CUtil::PrintFileHeader(outfile_diff_results, "SOURCE CODE DIFFERENTIAL RESULTS", cmdLine);
		for (i = 0; i < numCats; i++)
		{
			outfile_diff_results.setf(ofstream::left);
			outfile_diff_results.width(15);
			outfile_diff_results << myCats[i];
			outfile_diff_results.unsetf(ofstream::left);
			if (i + 1 < numCats)
			{
				outfile_diff_results.width(3);
				outfile_diff_results << " | ";
			}
		}
		outfile_diff_results << endl;
		for (i = 0; i < numCats - 1; i++)
		{
			outfile_diff_results.setf(ofstream::left);
			outfile_diff_results.width(15);
			outfile_diff_results << "Lines";
			outfile_diff_results.unsetf(ofstream::left);
			outfile_diff_results.width(3);
			outfile_diff_results << " | ";
		}
		outfile_diff_results.setf(ofstream::left);
		outfile_diff_results.width(15);
		outfile_diff_results << "Name";
		outfile_diff_results.unsetf(ofstream::left);
		outfile_diff_results << endl;
		for (y = 0; y < 120; y++)
			outfile_diff_results << BAR_S;
		outfile_diff_results << endl;

		if (printDup)
		{
			CUtil::PrintFileHeader(dup_outfile_diff_results, "SOURCE CODE DIFFERENTIAL RESULTS", cmdLine);
			for (i = 0; i < numCats; i++)
			{
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				dup_outfile_diff_results << myCats[i];
				dup_outfile_diff_results.unsetf(ofstream::left);
				if (i + 1 < numCats)
				{
					dup_outfile_diff_results.width(3);
					dup_outfile_diff_results << " | ";
				}
			}
			dup_outfile_diff_results << endl;
			for (i = 0; i < numCats - 1; i++)
			{
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				dup_outfile_diff_results << "Lines";
				dup_outfile_diff_results.unsetf(ofstream::left);
				dup_outfile_diff_results.width(3);
				dup_outfile_diff_results << " | ";
			}
			dup_outfile_diff_results.setf(ofstream::left);
			dup_outfile_diff_results.width(15);
			dup_outfile_diff_results << "Name";
			dup_outfile_diff_results.unsetf(ofstream::left);
			dup_outfile_diff_results << endl;
			for (y = 0; y < 120; y++)
				dup_outfile_diff_results << BAR_S;
			dup_outfile_diff_results << endl;
		}
	}
	if (print_csv)
	{
		CUtil::PrintFileHeader(outfile_diff_csv, "SOURCE CODE DIFFERENTIAL RESULTS", cmdLine);
		outfile_diff_csv << "New Lines,Deleted Lines,Modified Lines,Unmodified Lines,Modification Type,Language,Module A,Module B" << endl;
		if (printDup)
		{
			CUtil::PrintFileHeader(dup_outfile_diff_csv, "SOURCE CODE DIFFERENTIAL RESULTS", cmdLine);
			dup_outfile_diff_csv << "New Lines,Deleted Lines,Modified Lines,Unmodified Lines,Modification Type,Language,Module A,Module B" << endl;
		}
	}

	// print pair results
	resultStruct *myResults;
	string filenameA, filenameB, lang, modType;
	bool unmatchedDup;
	lang = DEF_LANG_NAME;
	for (MatchingType::iterator myI = matchedFilesList.begin(); myI != matchedFilesList.end(); myI++)
	{
		// select the filename, choose the baselineA unless this one was only in baselineB
		filenameA = ((*myI).second.first != NULL) ? (*myI).second.first->second.file_name : "NA";
		filenameB = ((*myI).second.second != NULL) ? (*myI).second.second->second.file_name : "NA";

		if (((*myI).first.addedLines == 0 && (*myI).first.deletedLines == 0 &&
			 (*myI).first.modifiedLines == 0 && (*myI).first.unmodifiedLines == 0) ||
			 filenameA.find(EMBEDDED_FILE_PREFIX) != string::npos ||
			 filenameB.find(EMBEDDED_FILE_PREFIX) != string::npos)
		{
			// do not print out if the file is empty or file not supported
			// only print file if it is the actual file (not having *.* in the filename)
			continue;
		}
		else
		{
			if ((*myI).second.first != NULL)
				lang = GetLanguageName( CounterForEachLanguage, (*myI).second.first->second.class_type, (*myI).second.first->second.file_name);
			else
				lang = GetLanguageName( CounterForEachLanguage, (*myI).second.second->second.class_type, (*myI).second.second->second.file_name);
		}

		unmatchedDup = false;
		if ((*myI).second.first != NULL)
		{
			if ((*myI).second.first->second.duplicate || (*myI).second.first->second.firstDuplicate)
				unmatchedDup = true;
		}
		if ((*myI).second.second != NULL)
		{
			if ((*myI).second.second->second.duplicate || (*myI).second.second->second.firstDuplicate)
			{
				if ((*myI).second.first == NULL)
					unmatchedDup = true;
				else
					unmatchedDup = false;
			}
			else
				unmatchedDup = false;
		}

		// print pair results
		myResults = &((*myI).first);
		if (!unmatchedDup)
		{
			if (print_ascii || print_legacy)
			{
				outfile_diff_results.setf(ofstream::left);
				outfile_diff_results.width(15);
				outfile_diff_results << myResults->addedLines;

				outfile_diff_results.unsetf(ofstream::left);
				outfile_diff_results.width(3);
				outfile_diff_results << "| ";
				outfile_diff_results.setf(ofstream::left);
				outfile_diff_results.width(15);
				outfile_diff_results << myResults->deletedLines;	// modified from addedLines

				outfile_diff_results.unsetf(ofstream::left);
				outfile_diff_results.width(3);
				outfile_diff_results << "| ";
				outfile_diff_results.setf(ofstream::left);
				outfile_diff_results.width(15);
				outfile_diff_results << myResults->modifiedLines;

				outfile_diff_results.unsetf(ofstream::left);
				outfile_diff_results.width(3);
				outfile_diff_results << "| ";
				outfile_diff_results.setf(ofstream::left);
				outfile_diff_results.width(15);
				outfile_diff_results << myResults->unmodifiedLines;

				outfile_diff_results.unsetf(ofstream::left);
				outfile_diff_results.width(3);
				outfile_diff_results << "| ";
				outfile_diff_results.setf(ofstream::left);
				outfile_diff_results.width(15);

				outfile_diff_results.setf(ofstream::left);
				outfile_diff_results.width(15);
				if (filenameB.compare("NA") != 0)
					outfile_diff_results << filenameB;
				else
					outfile_diff_results << filenameA;

				outfile_diff_results.unsetf(ofstream::left);
				outfile_diff_results << endl;
			}
			if (print_csv)
			{
				outfile_diff_csv << myResults->addedLines;
				outfile_diff_csv << "," << myResults->deletedLines;
				outfile_diff_csv << "," << myResults->modifiedLines;
				outfile_diff_csv << "," << myResults->unmodifiedLines;

				if (myResults->deletedLines == 0 && myResults->unmodifiedLines == 0 && myResults->modifiedLines == 0)
					modType = "Add";
				else if (myResults->addedLines == 0 && myResults->unmodifiedLines == 0 && myResults->modifiedLines == 0)
					modType = "Del";
				else if (myResults->addedLines == 0 && myResults->deletedLines == 0 && myResults->modifiedLines == 0)
					modType = "Unmod";
				else
					modType = "Mod";

				outfile_diff_csv << "," << modType;
				outfile_diff_csv << ",\"" << lang << "\"";
				outfile_diff_csv << ",\"" + filenameA + "\",\"" + filenameB + "\"";
				outfile_diff_csv << endl;
			}
		}
		else
		{
			if (print_ascii || print_legacy)
			{
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				dup_outfile_diff_results << myResults->addedLines;

				dup_outfile_diff_results.unsetf(ofstream::left);
				dup_outfile_diff_results.width(3);
				dup_outfile_diff_results << "| ";
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				dup_outfile_diff_results << myResults->deletedLines;	// modified from addedLines

				dup_outfile_diff_results.unsetf(ofstream::left);
				dup_outfile_diff_results.width(3);
				dup_outfile_diff_results << "| ";
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				dup_outfile_diff_results << myResults->modifiedLines;

				dup_outfile_diff_results.unsetf(ofstream::left);
				dup_outfile_diff_results.width(3);
				dup_outfile_diff_results << "| ";
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				dup_outfile_diff_results << myResults->unmodifiedLines;

				dup_outfile_diff_results.unsetf(ofstream::left);
				dup_outfile_diff_results.width(3);
				dup_outfile_diff_results << "| ";
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);

				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				if (filenameB.compare("NA") != 0)
					dup_outfile_diff_results << filenameB;
				else
					dup_outfile_diff_results << filenameA;

				dup_outfile_diff_results.unsetf(ofstream::left);
				dup_outfile_diff_results << endl;
			}
			if (print_csv)
			{
				dup_outfile_diff_csv << myResults->addedLines;
				dup_outfile_diff_csv << "," << myResults->deletedLines;
				dup_outfile_diff_csv << "," << myResults->modifiedLines;
				dup_outfile_diff_csv << "," << myResults->unmodifiedLines;

				if (myResults->deletedLines == 0 && myResults->unmodifiedLines == 0 && myResults->modifiedLines == 0)
					modType = "Add";
				else if (myResults->addedLines == 0 && myResults->unmodifiedLines == 0 && myResults->modifiedLines == 0)
					modType = "Del";
				else if (myResults->addedLines == 0 && myResults->deletedLines == 0 && myResults->modifiedLines == 0)
					modType = "Unmod";
				else
					modType = "Mod";

				dup_outfile_diff_csv << "," << modType;
				dup_outfile_diff_csv << ",\"" << lang << "\"";
				dup_outfile_diff_csv << ",\"" + filenameA + "\",\"" + filenameB + "\"";
				dup_outfile_diff_csv << endl;
			}
		}
	}

	// print the diff results summary
	int ti;
	numCats--;
	if (print_ascii || print_legacy)
	{
		outfile_diff_results << endl << endl;
		for (ti = 0; ti < numCats; ti++)
		{
			outfile_diff_results.setf(ofstream::left);
			outfile_diff_results.width(15);
			outfile_diff_results << "Total";
			outfile_diff_results.unsetf(ofstream::left);
			if (ti + 1 < numCats)
			{
				outfile_diff_results.width(3);
				outfile_diff_results << "| ";
			}
		}
		outfile_diff_results << endl;
		for (ti = 0; ti < numCats; ti++)
		{
			outfile_diff_results.setf(ofstream::left);
			outfile_diff_results.width(15);
			outfile_diff_results << myCats[ti];
			outfile_diff_results.unsetf(ofstream::left);
			if (ti + 1 < numCats)
			{
				outfile_diff_results.width(3);
				outfile_diff_results << "| ";
			}
		}
		outfile_diff_results << endl;
		for (ti = 0; ti < numCats; ti++)
		{
			outfile_diff_results.setf(ofstream::left);
			outfile_diff_results.width(15);
			outfile_diff_results << "Lines";
			outfile_diff_results.unsetf(ofstream::left);
			if (ti + 1 < numCats)
			{
				outfile_diff_results.width(3);
				outfile_diff_results << "| ";
			}
		}
		outfile_diff_results << endl;
		for (y = 0; y < 73; y++)
			outfile_diff_results << BAR_S;
		outfile_diff_results << endl;

		outfile_diff_results.setf(ofstream::left);
		outfile_diff_results.width(15);
		outfile_diff_results << this->total_addedLines;

		outfile_diff_results.unsetf(ofstream::left);  
		outfile_diff_results.width(3);
		outfile_diff_results << "| ";
		outfile_diff_results.setf(ofstream::left);
		outfile_diff_results.width(15);
		outfile_diff_results << this->total_deletedLines;

		outfile_diff_results.unsetf(ofstream::left);  
		outfile_diff_results.width(3);
		outfile_diff_results << "| ";
		outfile_diff_results.setf(ofstream::left);
		outfile_diff_results.width(15);
		outfile_diff_results << this->total_modifiedLines;

		outfile_diff_results.unsetf(ofstream::left);  
		outfile_diff_results.width(3);
		outfile_diff_results << "| ";
		outfile_diff_results.setf(ofstream::left);
		outfile_diff_results.width(15);
		outfile_diff_results << this->total_unmodifiedLines;

		outfile_diff_results << endl << endl;

		if (printDup)
		{
			dup_outfile_diff_results << endl << endl;
			for (ti = 0; ti < numCats; ti++)
			{
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				dup_outfile_diff_results << "Total";
				dup_outfile_diff_results.unsetf(ofstream::left);
				if (ti + 1 < numCats)
				{
					dup_outfile_diff_results.width(3);
					dup_outfile_diff_results << "| ";
				}
			}
			dup_outfile_diff_results << endl;
			for (ti = 0; ti < numCats; ti++)
			{
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				dup_outfile_diff_results << myCats[ti];
				dup_outfile_diff_results.unsetf(ofstream::left);
				if ((ti + 1) < numCats)
				{
					dup_outfile_diff_results.width(3);
					dup_outfile_diff_results << "| ";
				}
			}
			dup_outfile_diff_results << endl;
			for (ti = 0; ti < numCats; ti++)
			{
				dup_outfile_diff_results.setf(ofstream::left);
				dup_outfile_diff_results.width(15);
				dup_outfile_diff_results << "Lines";
				dup_outfile_diff_results.unsetf(ofstream::left);
				if (ti + 1 < numCats)
				{
					dup_outfile_diff_results.width(3);
					dup_outfile_diff_results << "| ";
				}
			}
			dup_outfile_diff_results << endl;
			for (y = 0; y < 73; y++)
				dup_outfile_diff_results << BAR_S;
			dup_outfile_diff_results << endl;

			dup_outfile_diff_results.setf(ofstream::left);
			dup_outfile_diff_results.width(15);
			dup_outfile_diff_results << this->dup_addedLines;

			dup_outfile_diff_results.unsetf(ofstream::left);  
			dup_outfile_diff_results.width(3);
			dup_outfile_diff_results << "| ";
			dup_outfile_diff_results.setf(ofstream::left);
			dup_outfile_diff_results.width(15);
			dup_outfile_diff_results << this->dup_deletedLines;

			dup_outfile_diff_results.unsetf(ofstream::left);  
			dup_outfile_diff_results.width(3);
			dup_outfile_diff_results << "| ";
			dup_outfile_diff_results.setf(ofstream::left);
			dup_outfile_diff_results.width(15);
			dup_outfile_diff_results << this->dup_modifiedLines;

			dup_outfile_diff_results.unsetf(ofstream::left);  
			dup_outfile_diff_results.width(3);
			dup_outfile_diff_results << "| ";
			dup_outfile_diff_results.setf(ofstream::left);
			dup_outfile_diff_results.width(15);
			dup_outfile_diff_results << this->dup_unmodifiedLines;

			dup_outfile_diff_results << endl << endl;
		}
	}
	if (print_csv)
	{
		outfile_diff_csv << endl << "Total New Lines,Total Deleted Lines,Total Modified Lines,Total Unmodified Lines" << endl;
		outfile_diff_csv << this->total_addedLines;
		outfile_diff_csv << "," << this->total_deletedLines;
		outfile_diff_csv << "," << this->total_modifiedLines;
		outfile_diff_csv << "," << this->total_unmodifiedLines << endl;

		if (printDup)
		{
			dup_outfile_diff_csv << endl << "Total New Lines,Total Deleted Lines,Total Modified Lines,Total Unmodified Lines" << endl;
			dup_outfile_diff_csv << this->dup_addedLines;
			dup_outfile_diff_csv << "," << this->dup_deletedLines;
			dup_outfile_diff_csv << "," << this->dup_modifiedLines;
			dup_outfile_diff_csv << "," << this->dup_unmodifiedLines << endl;
		}
	}

	if (print_ascii || print_legacy)
	{
		outfile_diff_results.close();
		if (printDup)
			dup_outfile_diff_results.close();
	}
	if (print_csv)
	{
		outfile_diff_csv.close();
		if (printDup)
			dup_outfile_diff_csv.close();
	}
}
