//! Code counter class definition for the C/C++, Java, and C# languages.
/*!
* \file CCJavaCsCounter.h
*
* This file contains the code counter class definition for the C/C++, Java, and C# languages.
*/

#ifndef CCJavaCsCounter_h
#define CCJavaCsCounter_h

#include "CCodeCounter.h"

//! C/C++, Java, and C# code counter class.
/*!
* \class CCJavaCsCounter
*
* Defines the C/C++, Java, and C# code counter class.
*/
class CCJavaCsCounter : public CCodeCounter
{
public:
	CCJavaCsCounter();

protected:
	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapmBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapmBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, unsigned int &paren_cnt,
		bool &forflag, bool &found_forifwhile, bool &found_while, char &prev_char, bool &data_continue,
		unsigned int &temp_lines, unsigned int &phys_exec_lines, unsigned int &phys_data_lines,
		bool &inArrayDec, bool &found_for, unsigned int &openBrackets, StringVector &loopLevel);
	virtual int ParseFunctionName(const string &line, string &lastline,
		filemap &functionStack, string &functionName, unsigned int &functionCount);
};

#endif
