//! Code counter class definition for the JavaScript language.
/*!
* \file CJavascriptCounter.h
*
* This file contains the code counter class definition for the JavaScript language.
*/

#ifndef CJavascriptCounter_h
#define CJavascriptCounter_h

#include "CCodeCounter.h"

//! JavaScript code counter class.
/*!
* \class CJavascriptCounter
*
* Defines the JavaScript code counter class.
*/
class CJavascriptCounter : public CCodeCounter
{
public:
	CJavascriptCounter();

protected:
	virtual int ReplaceQuote(string &strline, size_t &idx_start, bool &contd, char &CurrentQuoteEnd);
	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, unsigned int &paren_cnt,
		bool &forflag, bool &found_forifwhile, bool &found_while, char &prev_char, bool &data_continue,
		unsigned int &temp_lines, unsigned int &phys_exec_lines, unsigned int &phys_data_lines, bool &inArrayDec,
		unsigned int &openBrackets, StringVector &loopLevel);
};

//! JavaScript in PHP code counter class.
/*!
* \class CJavascriptPhpCounter
*
* Defines the JavaScript in PHP code counter class.
*/
class CJavascriptPhpCounter : public CJavascriptCounter
{
public:
	CJavascriptPhpCounter();
};

//! JavaScript in HTML code counter class.
/*!
* \class CJavascriptHtmlCounter
*
* Defines the JavaScript in HTML code counter class.
*/
class CJavascriptHtmlCounter : public CJavascriptCounter
{
public:
	CJavascriptHtmlCounter();
};

//! JavaScript in XML code counter class.
/*!
* \class CJavascriptXmlCounter
*
* Defines the JavaScript in XML code counter class.
*/
class CJavascriptXmlCounter : public CJavascriptCounter
{
public:
	CJavascriptXmlCounter();
};

//! JavaScript in JSP code counter class.
/*!
* \class CJavascriptJspCounter
*
* Defines the JavaScript in JSP code counter class.
*/
class CJavascriptJspCounter : public CJavascriptCounter
{
public:
	CJavascriptJspCounter();
};

//! JavaScript in ASP server code counter class.
/*!
* \class CJavascriptAspServerCounter
*
* Defines the JavaScript in ASP server code counter class.
*/
class CJavascriptAspServerCounter : public CJavascriptCounter
{
public:
	CJavascriptAspServerCounter();
};

//! JavaScript in ASP client code counter class.
/*!
* \class CJavascriptAspClientCounter
*
* Defines the JavaScript in ASP client code counter class.
*/
class CJavascriptAspClientCounter : public CJavascriptCounter
{
public:
	CJavascriptAspClientCounter();
};

//! JavaScript in ColdFusion code counter class.
/*!
* \class CJavascriptColdFusionCounter
*
* Defines the JavaScript in ColdFusion code counter class.
*/
class CJavascriptColdFusionCounter : public CJavascriptCounter
{
public:
	CJavascriptColdFusionCounter();
};

#endif
