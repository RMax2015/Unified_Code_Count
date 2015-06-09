//! Code counter class definition for the Pascal language.
/*!
* \file CPascalCounter.h
*
* This file contains the code counter class definition for the Pascal language.
*/

#ifndef CPascalCounter_h
#define CPascalCounter_h

#include "CCodeCounter.h"

//! Pascal code counter class.
/*!
* \class CPascalCounter
*
* Defines the Pascal counter class.
*/
class CPascalCounter : public CCodeCounter
{
public:
	CPascalCounter();

protected:
	virtual int CountCommentsSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapmBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, bool &found_block,
		bool &found_forifwhile, bool &found_end, bool &found_loop, StringVector &loopLevel);
	void FoundSLOC(results* result, string &strLSLOC, string &strLSLOCBak, bool &found_block,
		bool &found_forifwhile, bool &found_end, bool &trunc_flag);
};

#endif
