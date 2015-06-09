//! Code counter class definition for the Bash shell script language.
/*!
* \file CBashCounter.h
*
* This file contains the code counter class definition for the Bash shell script language.
* This also includes the Korn shell language.
*/

#ifndef CBashCounter_h
#define CBashCounter_h

#include "CCodeCounter.h"

//! Bash shell script code counter class.
/*!
* \class CBashCounter
*
* Defines the Bash shell script code counter class.
*/
class CBashCounter : public CCodeCounter
{
public:
	CBashCounter();

protected:
	virtual int PreCountProcess(filemap* fmap);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak,
		bool &data_continue, unsigned int &temp_lines, unsigned int &phys_exec_lines,
		unsigned int &phys_data_lines, StringVector &loopLevel);

	StringVector continue_keywords;		//!< List of keywords to continue to next line
};

#endif
