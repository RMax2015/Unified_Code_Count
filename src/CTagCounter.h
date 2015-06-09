//! Code counter class definition for tag languages including HTML, XML, and ColdFusion.
/*!
* \file CHtmlCounter.h
*
* This file contains the code counter class definition for for tag languages including HTML, XML, and ColdFusion.
*/

#ifndef CTagCounter_h
#define CTagCounter_h

#include "CCodeCounter.h"

//! Tag language code counter class.
/*!
* \class CTagCounter
*
* Defines the tag language code counter class.
*/
class CTagCounter : public CCodeCounter
{
public:
	CTagCounter();

protected:
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, char &prev_char,
		bool &data_continue, unsigned int &temp_lines, unsigned int &phys_exec_lines, unsigned int &phys_data_lines);
	void CountTagTally(string base, StringVector& container, unsigned int &count, int mode, string exclude,
		string include1, string include2, UIntVector* counter_container = 0, bool case_sensitive = true);
};

#endif
