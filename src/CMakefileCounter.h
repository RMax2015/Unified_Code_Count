//! Code counter class definition for Makefiles.
/*!
* \file CMakefileCounter.h
*
* This file contains the code counter class definition for Makefiles.
*/

#ifndef CMakefileCounter_h
#define CMakefileCounter_h

#include "CCodeCounter.h"

//! Makefile code counter class.
/*!
* \class CMakefileCounter
*
* Defines the Makefile code counter class.
*/
class CMakefileCounter : public CCodeCounter
{
public:
	CMakefileCounter();

protected:
	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak);
};

#endif
