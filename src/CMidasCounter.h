//! Code counter class definition for the Midas macro languages.
/*!
* \file CMidasCounter.h
*
* This file contains the code counter class definition for the Midas macro languages.
*/

#ifndef CMidasCounter_h
#define CMidasCounter_h

#include "CCodeCounter.h"

//! Midas code counter class.
/*!
* \class CMidasCounter
*
* Defines the Midas code counter class.
*/
class CMidasCounter : public CCodeCounter
{
public:
	CMidasCounter();

protected:
	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak,
		bool &data_continue, unsigned int &temp_lines, unsigned int &phys_exec_lines,
		unsigned int &phys_data_lines, StringVector &loopEnd);
};

#endif
