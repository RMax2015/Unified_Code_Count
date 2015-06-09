//! Code counter class definition for the Perl language.
/*!
* \file CPerlCounter.h
*
* This file contains the code counter class definition for the Perl language.
*/

#ifndef CPerlCounter_h
#define CPerlCounter_h

#include "CCodeCounter.h"

//! Perl code counter class.
/*!
* \class CPerlCounter
*
* Defines the Perl code counter class.
*/
class CPerlCounter : public CCodeCounter
{
public:
	CPerlCounter();

protected:
	virtual int PreCountProcess(filemap* fmap);
	virtual int ReplaceQuote(string &strline, size_t &idx_start, bool &contd, char &CurrentQuoteEnd);
	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, unsigned int &paren_cnt,
		bool &forflag, bool &found_forifwhile, bool &found_while, char &prev_char, bool &data_continue,
		unsigned int &temp_lines, unsigned int &phys_exec_lines, unsigned int &phys_data_lines,
		unsigned int &openBrackets, StringVector &loopLevel);
	int ParseFunctionName(const string &line, string &lastline,
		filemap &functionStack, string &functionName, unsigned int &functionCount);
};

#endif
