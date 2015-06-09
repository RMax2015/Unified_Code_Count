//! Language aware Utility structures and procedures.
/*!
* \file LangUtils.h
*
* This file contains support procedures that can be used by:
* the main thread or by worker threads
*
* Secondary goal is to move some code out of the very large MainObject.cpp file
*
* ADDED to UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_06
*/

#ifndef LANG_UTILS_PUBLIC_H
#define LANG_UTILS_PUBLIC_H

// List of all known Language specific parsers
#include "CCodeCounter.h"
#include "CAdaCounter.h"
#include "CBashCounter.h"
#include "CCCounter.h"
#include "CCFScriptCounter.h"
#include "CColdFusionCounter.h"
#include "CCsharpCounter.h"
#include "CCshCounter.h"
#include "CCssCounter.h"
#include "CDataCounter.h"
#include "CFortranCounter.h"
#include "CHtmlCounter.h"
#include "CJavaCounter.h"
#include "CJavascriptCounter.h"
#include "CMakefileCounter.h"
#include "CMatlabCounter.h"
#include "CNeXtMidasCounter.h"
#include "CPascalCounter.h"
#include "CPerlCounter.h"
#include "CPhpCounter.h"
#include "CPythonCounter.h"
#include "CRubyCounter.h"
#include "CSqlCounter.h"
#include "CVbCounter.h"
#include "CVbscriptCounter.h"
#include "CVerilogCounter.h"
#include "CVHDLCounter.h"
#include "CWebCounter.h"
#include "CXMidasCounter.h"
#include "CXmlCounter.h"


//! Map containing a logical Language enum and a pointer to Parser class for that language.
/*!
* \typedef CounterForEachLangType
*
* Defines a map containing a list of strings.
*/
typedef map<int, CCodeCounter*> CounterForEachLangType;


int Init_CounterForEachLanguage( CounterForEachLangType & CounterForEachLanguage );

/*!
* Resets all count lists (e.g., directive_count, data_name_count, etc.).
*/
void ResetCounterCounts( CounterForEachLangType & eachLangCounters );

/*!
* Sets the Physical line size Truncation limit
*/
void SetCounterOptions( CounterForEachLangType & eachLangCounters );

// Returns the related Language specific parsing class for a given file name
ClassType DecideLanguage( CounterForEachLangType & CounterForEachLanguage,
						CCodeCounter** counter,
						const bool print_cmplx,
						const string &file_name, 
						const bool setCounter = true );

string GetLanguageName( CounterForEachLangType & CounterForEachLanguage, 
						const ClassType class_type, const string &file_name );

/*!
* Updates count lists (e.g., directive_count, data_name_count, etc.) based on file counts.
*
* \param useListA use file list A? (otherwise use list B)
*/
void UpdateCounterCounts( CounterForEachLangType & eachLangCounters,
						SourceFileList * mySourceFile,
						const bool useListA = true, const bool resetCounts = true );


/*!
* AddFromOtherLangCounters add counts to Main thread structure from Threads
*
* \SideEffects	This will clear Source counters to zero after accumulation
*
* \param	pDestLangCounters	IN/OUT	pointer to Language classes whose info needs updating
* \param	pSrcLangCounters	IN/OUT	pointer to Language classes that has new info
* \param	my_useListA			IN		List A counters (otherwise counters for List B)
*/
void AddFromOtherLangCounters( CounterForEachLangType * destLangCounters,
							CounterForEachLangType * srcLangCounters,
							const bool useListA = true );

#endif		//	END		#ifndef		LANG_UTILS_PUBLIC_H