//! Utility class definition for static methods.
/*!
* \file CUtil.h
*
* This file contains the utility class definition for static methods.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_06_06
*   Changes  ended  on 2015_06_06
*	Refactored for faster performance
*/

#ifndef CUTIL_H
#define CUTIL_H

#include "cc_main.h"

using namespace std;

#define INVALID_POSITION ((unsigned int)-1)
#define TO_END_OF_STRING INVALID_POSITION

//! Vector containing a list of strings.
/*!
* \typedef vectorString
*
* Defines a vector containing a list of strings.
*/
typedef vector<string> vectorString;

//! Utility class.
/*!
* \class CUtil
*
* Defines a utility class.
*/
class CUtil
{
public:
	static string TrimString(const string &str, int mode = 0);
	static string EraseString(const string &srcstr, const string &erasedstr);

	/*!
	* Initialize an array of lower case chars used by ToLower.  Much faster performance.
	*
	* \Globals	lowerChars	char array is initialized
	*/
	static void InitToLower();

	static string ToLower(const string &string_to_lower);
	static bool CheckBlank(const string &str);
	static bool IsInteger(const string &str);
	static size_t FindStringsCaseInsensitive(const string &target, map<string, int> &table, size_t &pos, size_t preLang = INVALID_POSITION);
	static size_t FindCharAvoidEscape(const string &source, char target, size_t start_idx, char escape);
	static size_t FindKeyword(const string &str, const string &keyword, size_t start = 0, size_t end = TO_END_OF_STRING, bool case_sensitive = true);
	static void CountTally(const string &base, StringVector &container, unsigned int &count, int mode, const string &exclude,
		const string &include1, const string &include2, UIntVector *counter_container = 0, bool case_sensitive = true);
	static string ExtractFilename(const string &filepath);
	static bool ListAllFiles(string &folder, StringVector &fileExtList, StringVector &fileList, bool symLinks);
	static bool GetFileList(StringVector &fileList, const string &path, bool symLinks);
	static bool MatchFilename(const string &filename, const string &matchStr);
	static int MkPath(const string &path);
	static int PrintFileHeader(ofstream &pout, const string &title, const string &cmd = "");
	static int PrintFileHeaderLine(ofstream &pout, const string &line);
	static string ConvertClearCaseFile(const string &fileName);
	static size_t TruncateLine(size_t length, size_t totalLength, size_t truncate, bool &trunc_flag);
	static string ClearRedundantSpaces(const string &str);
	static string ReplaceSmartQuotes(const string &str);
	
	// Returns	string of message to send to UI if not empty string
	static string WriteUncountedFileUtil(const string &msg, const string &uncFile, bool useListA, bool csvOutput, string outDir);
};

#endif
