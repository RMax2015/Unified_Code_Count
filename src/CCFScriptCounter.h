//! Code counter class definition for the CFScript language.
/*!
* \file CCFScriptCounter.h
*
* This file contains the code counter class definition for the CFScript language.
*/

#ifndef CCFScriptCounter_h
#define CCFScriptCounter_h

#include "CCodeCounter.h"

//! CFScript code counter class.
/*!
* \class CCFScriptCounter
*
* Defines the CFScript code counter class.
*/
class CCFScriptCounter : public CCodeCounter
{
public:
	CCFScriptCounter();

protected:
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, unsigned int &paren_cnt,
		bool &forflag, bool &found_forifwhile, bool &found_while, char &prev_char, bool &data_continue,
		unsigned int &temp_lines, unsigned int &phys_exec_lines, unsigned int &phys_data_lines, bool &inArrayDec,
		unsigned int &openBrackets, StringVector &loopLevel);
};

#endif
