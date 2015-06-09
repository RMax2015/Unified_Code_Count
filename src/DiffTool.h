//! Differencing tool class definition.
/*!
* \file DiffTool.h
*
* This file contains the differencing tool class definition.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_06_02
*   Changes  ended  on 2015_06_06
*	Refactored CompareFileNames to CompareFilePaths to better describe purpose
*/

#ifndef DIFFTOOL_H
#define DIFFTOOL_H

#include "MainObject.h"
#include "CmpMngr.h"
#include <algorithm>
#include <sstream>
#include <time.h>

// Differencing functionality defines
#define BASELINE_INF1 "fileListA.txt"
#define BASELINE_INF2 "fileListB.txt"
#define DIFF_OUTFILE "outfile_diff_results.txt"
#define DIFF_OUTFILE_CSV "outfile_diff_results.csv"
#define MATCH_PAIRS_OUTFILE "MatchedPairs.txt"
#define MATCH_PAIRS_OUTFILE_CSV "MatchedPairs.csv"

#define MAX_MISMATCH_COST	10000

#define BAR_S	"-"

//! Differencing tool class.
/*!
* \class DiffTool
*
* Defines the differencing tool class.
*/
class DiffTool: public MainObject
{
public:
	DiffTool();
	~DiffTool(){}

	int diffToolProcess(int argc, char *argv[]);

	//! Differencing results class.
	/*!
	* \class resultStruct
	*
	* Defines the differencing results class.
	*/
	class resultStruct
	{
	public:
		unsigned int addedLines;			//!< Lines added
		unsigned int deletedLines;			//!< Lines deleted
		unsigned int modifiedLines;			//!< Lines modified
		unsigned int unmodifiedLines;		//!< Lines unmodified

		resultStruct() { addedLines = deletedLines = modifiedLines = unmodifiedLines = 0; }
	};

private:
	int ReadAllDiffFiles();
	void MatchBaseLines(bool webSepFilesOnly = false);
	int CompareFilePaths( const string &file1, const string &file2, const unsigned int file_name_length );
	void ProcessPairs();
	void PrintMatchedPairs();
	void PrintDiffResults();

	//! Pair of matching source files.
	/*!
	* \typedef MatchedFilePair
	*
	* Defines a pair of matching source files.
	*/
	typedef pair<SourceFileElement *, SourceFileElement *> MatchedFilePair;

	//! Pair of file name to source file element
	/*!
	* \typedef FileNamePair
	*
	* Defines a pair for file name and source file element.  Used for matching files
	*/
	typedef pair<const string, SourceFileElement *> FileNamePair;

	//! Vector of matching file pairs and results.
	/*!
	* \typedef MatchingType
	*
	* Defines a vector of matching file pairs and results.
	*/
	typedef vector<pair<resultStruct, MatchedFilePair> > MatchingType;

	//! Structure to contain file element preference value.
	/*!
	* \struct PreferenceStruct
	*
	* Defines a structure to contain file element preference value.
	*/
	struct PreferenceStruct
	{
		SourceFileElement *fileElement;	//!< File element
		int value;						//!< File preference value
	};

	//! Structure to sort the preference vector in descending order.
	/*!
	* \struct CustomCMP
	*
	* Defines a structure to sort the preference vector in descending order.
	*/
	struct CustomCMP
	{
		bool operator() (const PreferenceStruct &a, const PreferenceStruct &b) const
		{
			return a.value < b.value;
		};
	};

	//! Vector of file preferences.
	/*!
	* \typedef MatchingType
	*
	* Defines a vector of file preferences.
	*/
	typedef vector<PreferenceStruct> PreferenceMapType;

	//! Map of file elements and preferences.
	/*!
	* \typedef BaselinePreferenceMapType
	*
	* Defines a map of file elements and preferences.
	*/
	typedef map<SourceFileElement *, PreferenceMapType *> BaselinePreferenceMapType;

	//! Map of file element pairs.
	/*!
	* \typedef BaselineFileMapType
	*
	* Defines a map of file element pairs.
	*/
	typedef map<SourceFileElement *, SourceFileElement *> BaselineFileMapType;

	//! Multimap of file elements, sorted by file name
	/*!
	* \typedef SortedPreferenceMapType
	*
	* Defines a multimap of file elements and names
	*/
	typedef multimap<const string, SourceFileElement*> SortedPreferenceMapType;

	MatchingType matchedFilesList;		//!< List of matching file pairs and differencing results

	unsigned int total_addedLines;		//!< Total lines added
	unsigned int total_deletedLines;	//!< Total lines deleted
	unsigned int total_modifiedLines;	//!< Total lines modified
	unsigned int total_unmodifiedLines;	//!< Total lines unmodified
	unsigned int dup_addedLines;		//!< Total duplicate lines added
	unsigned int dup_deletedLines;		//!< Total duplicate lines deleted
	unsigned int dup_modifiedLines;		//!< Total duplicate lines modified
	unsigned int dup_unmodifiedLines;	//!< Total duplicate lines unmodified

	ofstream outfile_diff_results;		//!< Differencing results text file stream
	ofstream outfile_diff_csv;			//!< Differencing results CSV file stream
	ofstream dup_outfile_diff_results;  //!< Differencing results text file stream for duplicates
	ofstream dup_outfile_diff_csv;		//!< Differencing results CSV file stream for duplicates
	bool printDup;						//!< Print duplicates (only true if unmatched duplicates exist)
};

#endif
