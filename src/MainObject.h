//! Main counting object class definition.
/*!
* \file MainObject.h
*
* This file contains the main counting object class definition.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_
*   Addition of Multithreading performance enhancement 
*	Some Refactoring to move code to other source files to decrease size/complexity
*/

#ifndef MainObject_h
#define MainObject_h

// Get class defs for various Language parsers and support utils
#include "LangUtils.h"

#include "UserIF.h"

#include "UCCThread.h"


//! Main counting object class.
/*!
* \class MainObject
*
* Defines the main counting object class.
*/
class MainObject
{
public:
	MainObject();
	~MainObject();

	// Call this instead of directly checking through userIF
	bool HasUserCancelled();

	// Helper to support reporting times of various steps
	double GetDuplicateThreshold();

	int MainProcess(int argc, char *argv[]);

	void GetLanguageExtensionMap(multimap<const string, string> *lang_ext_map);
	static void ShowUsage(const string &option = "", const bool do_exit = true);
#ifdef QTGUI
	void ConnectUserIF(QWidget *parent = 0);
#endif

protected:

	int ParseCommandLine(int argc, char *argv[]);
	void StartThreads( string & start_result_msg );
	int ReadAllFiles(StringVector &inputFileVector, const string &inputFileList = INPUT_FILE_LIST_NAME, const bool useListA = true);
	void ProcessSourceList( const bool useListA = true );
	bool IsSupportedFileExtension(const string &file_name);
	void FindDuplicateFiles(SourceFileList &fileList, StringVector &dupList1, StringVector &dupList2, const bool checkMatch = false);
	bool FindDuplicateFor(SourceFileList &compareList, SourceFileList::iterator &sfi,
		StringVector &dupList1, StringVector &dupList2, const bool checkMatch = false);
	void CompareForDuplicate(srcLineVector &firstFile, srcLineVector &secondFile, double &changedLines, double &totalLines);
	void ReadUserExtMapping(const string &extMapFile);
	void CreateExtMap();

	string BaselineFileName1;						//!< Baseline file name 1
	string BaselineFileName2;						//!< Baseline file name 2

	string dirnameA;								//!< Directory name for baseline A
	string dirnameB;								//!< Directory name for baseline B

	string userExtMapFile;							//!< User extension map file

	double duplicate_threshold;						//!< % changed threshold for determining duplicate content
	StringVector listFilesToBeSearched;				//!< List of options for files to be searched

	bool use_CommandLine;							//!< Read file names from command line specifications or from input files
	double match_threshold;							//!< % threshold for matching, if greater then added/deleted instead of modified

	bool clearCaseFile;								//!< Target files are extracted from the ClearCase CM
	bool followSymLinks;							//!< Unix symbolic links are followed to their linked files/dirs

	CounterForEachLangType CounterForEachLanguage;	//!< List of language enum and ptr to Parser for that language
	map<string, string> LanguageExtensionMap;		//!< List of languages and their extensions

	CCodeCounter* counter;							//!< Single language code counter
};

#endif
