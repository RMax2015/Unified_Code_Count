//! Code counter class definition for the cascading style sheet (CSS) language.
/*!
* \file CCssCounter.h
*
* This file contains the code counter class definition for the cascading style sheet (CSS) language.
*/

#ifndef CCssCounter_h
#define CCssCounter_h

#include "CCodeCounter.h"

//! CSS code counter class.
/*!
* \class CCssCounter
*
* Defines the CSS code counter class.
*/
class CCssCounter : public CCodeCounter
{
public:
	CCssCounter();

protected:
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string &line, string &strLSLOC, char &lastLinesLastChar,
		unsigned int &phys_exec_lines, unsigned int &phys_data_lines);
};

#endif
