//! Code counter class definition for the Python language.
/*!
* \file CPythonCounter.h
*
* This file contains the code counter class definition for the Python language.
*/

#ifndef CPythonCounter_h
#define CPythonCounter_h

#include "CCodeCounter.h"

//! Python code counter class.
/*!
* \class CPythonCounter
*
* Defines the Python code counter class.
*/
class CPythonCounter : public CCodeCounter
{
public:
	CPythonCounter();

protected:
	StringVector loop_keywords;		//!< List of keywords to indicate the beginning of a loop

	virtual int ReplaceQuote(string &strline, size_t &idx_start, bool &contd, string &CurrentQuoteEnd);
	virtual int CountCommentsSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak,
		unsigned int &paren_cnt, UIntVector &loopWhiteSpace);
};

#endif
