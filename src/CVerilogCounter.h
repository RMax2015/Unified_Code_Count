//! Code counter class definition for the Verilog language.
/*!
* \file CVerilogCounter.h
*
* This file contains the code counter class definition for the Verilog hardware definition language (used in FPGA programming).
*/

#ifndef CVerilogCounter_h
#define CVerilogCounter_h

#include "CCodeCounter.h"
#include "CVHDLCounter.h"

//! Verilog code counter class.
/*!
* \class CVerilogCounter
*
* Defines the Verilog code counter class.
*/

class CVerilogCounter : public CCodeCounter
{
public:
	CVerilogCounter();

protected:
	virtual int CountComplexity(filemap* fmap, results* result);
	virtual int CountDirectiveSLOC(filemap* fmap, results* result, filemap* fmapmBak = NULL);
	virtual int LanguageSpecificProcess(filemap* fmap, results* result, filemap* fmapmBak = NULL);
	void LSLOC(results* result, string line, string lineBak, string &strLSLOC, string &strLSLOCBak, unsigned int &paren_cnt,
		bool &forflag, bool &found_forifwhile, bool &found_while, char &prev_char, bool &data_continue,
		unsigned int &temp_lines, unsigned int &phys_exec_lines, unsigned int &phys_data_lines,
		bool &found_for, StringVector &loopLevel, bool &always_flag, bool &case_flag, bool &repeat_flag);
};

#endif
