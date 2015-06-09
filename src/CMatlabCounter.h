//! Code counter class definition for the Matlab language.
/*!
* \file CMatlabCounter.h
*
* This file contains the code counter class definition for the Matlab Language.
*/

#ifndef CMatlabCounter_h
#define CMatlabCounter_h

#include "CCodeCounter.h"

//! Matlab code counter class.
/*!
* \class CMatlabCounter
*
* Defines the Matlab code counter class.
*/
class CMatlabCounter : public CCodeCounter
{
public:
	CMatlabCounter();

protected:
	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak,
		bool &cont_str, unsigned int &openBrackets, StringVector &loopLevel);
};

#endif
