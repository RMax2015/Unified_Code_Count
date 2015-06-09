//! Code counter class definition for the C shell script language.
/*!
* \file CCshCounter.h
*
* This file contains the code counter class definition for the C shell script language.
* This also includes the Tcsh language.
*/

#ifndef CCshCounter_h
#define CCshCounter_h

#include "CCodeCounter.h"

//! C shell script code counter class.
/*!
* \class CCshCounter
*
* Defines the C shell script code counter class.
*/
class CCshCounter : public CCodeCounter
{
public:
	CCshCounter();

protected:
	virtual int PreCountProcess(filemap* fmap);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak,
		bool &data_continue, unsigned int &temp_lines, unsigned int &phys_exec_lines,
		unsigned int &phys_data_lines, unsigned int &loopLevel);

	StringVector continue_keywords;		//!< List of keywords to continue to next line
};

#endif
