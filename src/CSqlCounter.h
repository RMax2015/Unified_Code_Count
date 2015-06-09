//! Code counter class definition for the SQL language.
/*!
* \file CSqlCounter.h
*
* This file contains the code counter class definition for the SQL language.
*/

#ifndef CSqlCounter_h
#define CSqlCounter_h

#include "CCodeCounter.h"

//! SQL code counter class.
/*!
* \class CSqlCounter
*
* Defines the SQL code counter class.
*/
class CSqlCounter : public CCodeCounter
{
public:
	CSqlCounter();

protected:
	virtual int ReplaceQuote(string &strline, size_t &idx_start, bool &contd, char &CurrentQuoteEnd);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, bool &data_continue);
};

//! SQL in ColdFusion code counter class.
/*!
* \class CSqlColdFusionCounter
*
* Defines the SQL in ColdFusion code counter class.
*/
class CSqlColdFusionCounter : public CSqlCounter
{
public:
	CSqlColdFusionCounter();
};

#endif
