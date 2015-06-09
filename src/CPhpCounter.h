//! Code counter class definition for the PHP language.
/*!
* \file CPhpCounter.h
*
* This file contains the code counter class definition for the PHP language.
*/

#ifndef CPhpCounter_h
#define CPhpCounter_h

#include "CCodeCounter.h"

//! PHP code counter class.
/*!
* \class CPhpCounter
*
* Defines the PHP code counter class.
* NOTE: PHP variables are case sensitive, but PHP functions are case insensitive.
*/
class CPhpCounter : public CCodeCounter
{
public:
	CPhpCounter();

protected:
	StringVector exclude_loop;				//!< List of keywords to exclude for loops

	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, unsigned int &paren_cnt,
		bool &forflag, bool &found_forifwhile, bool &found_while, char &prev_char, bool &data_continue,
		unsigned int &temp_lines, unsigned int &phys_exec_lines, unsigned int &phys_data_lines,
		bool &inArrayDec, bool &found_for, StringVector &loopLevel);
};

#endif
