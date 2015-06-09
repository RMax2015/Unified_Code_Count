//! Code counter class definition for the Ada language.
/*!
* \file CAdaCounter.h
*
* This file contains the code counter class definition for the Ada language.
*/

#ifndef AdaCounter_h
#define AdaCounter_h

#include "CCodeCounter.h"

//! Ada code counter class.
/*!
* \class CAdaCounter
*
* Defines the Ada code counter class.
*/
class CAdaCounter : public CCodeCounter
{
public:
	CAdaCounter();

protected:
	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int ReplaceQuote(string &strline, size_t &idx_start, bool &contd, char &CurrentQuoteEnd);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, unsigned int &paren_cnt,
		bool &forflag, bool &found_forifwhile, bool &found_while, bool &found_type, bool &found_is, bool &found_accept,
		unsigned int &loopLevel);
	void FoundSLOC(results* result, string &strLSLOC, string &strLSLOCBak, bool &found_block, bool &found_forifwhile,
		bool &found_end, bool &found_type, bool &found_is, bool &found_accept, bool &trunc_flag);
};

#endif
