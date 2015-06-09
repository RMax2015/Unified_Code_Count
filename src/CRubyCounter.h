//! Code counter class definition for the Ruby language.
/*!
* \file CRubyCounter.h
*
* This file contains the code counter class definition for the Ruby language.
*/

#ifndef CRubyCounter_h
#define CRubyCounter_h

#include "CCodeCounter.h"

//! Ruby code counter class.
/*!
* \class CRubyCounter
*
* Defines the Ruby code counter class.
*/
class CRubyCounter : public CCodeCounter
{
public:
	CRubyCounter();

protected:
	virtual int ReplaceQuote(string &strline, size_t &idx_start, bool &contd, char &CurrentQuoteEnd);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak);

	string delimiter;	// used to store delimiter of string literals across lines
};

#endif
