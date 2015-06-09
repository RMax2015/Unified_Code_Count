//! Language aware Utility procedures.
/*!
* \file LangUtils.cpp
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

#include "UCCGlobals.h"
#include "LangUtils.h"

// See note in UCCThreads.cpp for why this is exciting to call
int Init_CounterForEachLanguage( CounterForEachLangType & CounterForEachLanguage )
{
	int retVal = 0;

	CCodeCounter* tmp;

	tmp = new CCodeCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(UNKNOWN		,tmp));

	tmp = new CDataCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(DATAFILE		,tmp));

	tmp = new CWebCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(WEB			,tmp));

	tmp = new CAdaCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(ADA			,tmp));

	tmp = new CBashCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(BASH			,tmp));

	tmp = new CCshCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(CSH			,tmp));

	tmp = new CCCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(C_CPP			,tmp));

	tmp = new CCsharpCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(CSHARP		,tmp));

	tmp = new CCsharpHtmlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(CSHARP_HTML	,tmp));

	tmp = new CCsharpXmlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(CSHARP_XML	,tmp));

	tmp = new CCsharpAspCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(CSHARP_ASP_S	,tmp));

	tmp = new CColdFusionCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(COLDFUSION	,tmp));

	tmp = new CCFScriptCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(CFSCRIPT		,tmp));

	tmp = new CCssCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(CSS			,tmp));

	tmp = new CFortranCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(FORTRAN		,tmp));

	tmp = new CHtmlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(HTML			,tmp));

	tmp = new CHtmlPhpCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(HTML_PHP		,tmp));

	tmp = new CHtmlJspCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(HTML_JSP		,tmp));

	tmp = new CHtmlAspCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(HTML_ASP		,tmp));

	tmp = new CHtmlColdFusionCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(HTML_CFM		,tmp));

	tmp = new CJavaCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVA			,tmp));

	tmp = new CJavaJspCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVA_JSP		,tmp));

	tmp = new CJavascriptCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVASCRIPT	,tmp));

	tmp = new CJavascriptHtmlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVASCRIPT_HTML,tmp));

	tmp = new CJavascriptXmlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVASCRIPT_XML,tmp));

	tmp = new CJavascriptPhpCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVASCRIPT_PHP,tmp));

	tmp = new CJavascriptJspCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVASCRIPT_JSP,tmp));

	tmp = new CJavascriptAspServerCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVASCRIPT_ASP_S,tmp));

	tmp = new CJavascriptAspClientCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVASCRIPT_ASP_C,tmp));

	tmp = new CJavascriptColdFusionCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(JAVASCRIPT_CFM,tmp));

	tmp = new CMakefileCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(MAKEFILE		,tmp));

	tmp = new CMatlabCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(MATLAB		,tmp));

	tmp = new CNeXtMidasCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(NEXTMIDAS		,tmp));

	tmp = new CXMidasCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(XMIDAS		,tmp));

	tmp = new CPascalCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(PASCAL		,tmp));

	tmp = new CPerlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(PERL			,tmp));

	tmp = new CPhpCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(PHP			,tmp));

	tmp = new CPythonCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(PYTHON		,tmp));

	tmp = new CRubyCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(RUBY			,tmp));

	tmp = new CSqlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(SQL			,tmp));

	tmp = new CSqlColdFusionCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(SQL_CFM		,tmp));

	tmp = new CVbCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VB			,tmp));

	tmp = new CVbscriptCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VBSCRIPT		,tmp));

	tmp = new CVbsHtmlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VBS_HTML		,tmp));

	tmp = new CVbsXmlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VBS_XML		,tmp));

	tmp = new CVbsPhpCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VBS_PHP		,tmp));

	tmp = new CVbsJspCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VBS_JSP		,tmp));

	tmp = new CVbsAspServerCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VBS_ASP_S		,tmp));

	tmp = new CVbsAspClientCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VBS_ASP_C		,tmp));

	tmp = new CVbsColdFusionCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VBS_CFM		,tmp));

	tmp = new CVerilogCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VERILOG		,tmp));

	tmp = new CVHDLCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(VHDL			,tmp));
	
	tmp = new CXmlCounter;
	CounterForEachLanguage.insert(map<int, CCodeCounter*>::value_type(XML			,tmp));

	return retVal;
}

/*!
* Resets all count lists (e.g., directive_count, data_name_count, etc.).
*/
void ResetCounterCounts( CounterForEachLangType & eachLangCounters )
{
	for (CounterForEachLangType::iterator iter = eachLangCounters.begin(); iter != eachLangCounters.end(); iter++)
		iter->second->InitializeCounts();
}


/*!
* Sets the Physical line size Truncation limit
*/
void SetCounterOptions( CounterForEachLangType & eachLangCounters )
{
	if (lsloc_truncate != DEFAULT_TRUNCATE)
	{
		for (CounterForEachLangType::iterator iter = eachLangCounters.begin(); iter != eachLangCounters.end(); iter++)		
			iter->second->lsloc_truncate = lsloc_truncate;
	}	
}


/*!
* Determines which Language counter class can be used for a file.
*
* \SideEffects	This will set print_cmplx flag in a pointed to Code counter class pointer
*
* \param CounterForEachLanguage	IN		structure holding Language counter class instances
* \param ppCounter				IN/OUT	pointer to Code counter class pointer (thread support)
* \param print_cmplx			IN		provide a value if setCounter is true
* \param file_name				IN		file name
* \param setCounter				IN		set print_cmplx bool of the Code counter base class instance
*
* \return language class type
*/
ClassType DecideLanguage( CounterForEachLangType & CounterForEachLanguage,
						CCodeCounter** ppCounter,
						const bool print_cmplx,
						const string &file_name, 
						const bool setCounter )
{
	bool	found = false;
	CCodeCounter* pCounter = NULL; 

	for (map<int, CCodeCounter*>::iterator iter = CounterForEachLanguage.begin(); iter != CounterForEachLanguage.end(); iter++)
	{
		if (iter->second->IsSupportedFileExtension(file_name))
		{
			found = true;
			pCounter = iter->second;
			if (setCounter && print_cmplx)
				pCounter->print_cmplx = true;
			*ppCounter = pCounter;
			break;
		}
	}
	if (!found)
	{
		pCounter = CounterForEachLanguage[UNKNOWN];
		*ppCounter = pCounter;
	}
	return pCounter->classtype;
}

/*!
* Retrieves the language name for a file.
*
* \param class_type file classification type
* \param file_name file name
*
* \return language name
*/
string GetLanguageName( CounterForEachLangType & CounterForEachLanguage, 
						const ClassType class_type, const string &file_name ) 
{
	string classTypeFound = DEF_LANG_NAME;

	if ( ( class_type == WEB ) 
	  && ( file_name.length() > 0 ) )
	{
		CWebCounter *webCounter = (CWebCounter*)CounterForEachLanguage[WEB];
		WebType webType = webCounter->GetWebType( file_name );
		classTypeFound = webCounter->GetWebLangName( webType );
	}
	else
	{
		if ( class_type < (int)CounterForEachLanguage.size() )
			classTypeFound = CounterForEachLanguage[class_type]->language_name;
	}
		
	return classTypeFound;
}


/*!
* Updates count lists (e.g., directive_count, data_name_count, etc.) based on file counts.
*
* \param useListA use file list A? (otherwise use list B)
*/
void UpdateCounterCounts( CounterForEachLangType & eachLangCounters,
						SourceFileList * mySourceFile,
						const bool useListA, const bool resetCounts )
{
	int i;
	ClassType class_type;
	WebType webType;
	SourceFileList::iterator its;
	StringVector::iterator icnts, icnte;
	UIntPairVector::iterator icntc;
	UIntVector::iterator icnt;
	CCodeCounter* codeCounter;
	CWebCounter* webCounter;

	if ( resetCounts )
		ResetCounterCounts( eachLangCounters );

	for (its = mySourceFile->begin(); its != mySourceFile->end(); its++)
	{
		class_type = its->second.class_type;
		codeCounter = eachLangCounters[class_type];
		for (i = 0; i < 12; i++)
		{
			switch (i)
			{
			case 0:
				icnts = codeCounter->directive.begin();
				icnte = codeCounter->directive.end();
				icntc = codeCounter->directive_count.begin();
				icnt = its->second.directive_count.begin();
				break;
			case 1:
				icnts = codeCounter->data_name_list.begin();
				icnte = codeCounter->data_name_list.end();
				icntc = codeCounter->data_name_count.begin();
				icnt = its->second.data_name_count.begin();
				break;
			case 2:
				icnts = codeCounter->exec_name_list.begin();
				icnte = codeCounter->exec_name_list.end();
				icntc = codeCounter->exec_name_count.begin();
				icnt = its->second.exec_name_count.begin();
				break;
			case 3:
				icnts = codeCounter->math_func_list.begin();
				icnte = codeCounter->math_func_list.end();
				icntc = codeCounter->math_func_count.begin();
				icnt = its->second.math_func_count.begin();
				break;
			case 4:
				icnts = codeCounter->trig_func_list.begin();
				icnte = codeCounter->trig_func_list.end();
				icntc = codeCounter->trig_func_count.begin();
				icnt = its->second.trig_func_count.begin();
				break;
			case 5:
				icnts = codeCounter->log_func_list.begin();
				icnte = codeCounter->log_func_list.end();
				icntc = codeCounter->log_func_count.begin();
				icnt = its->second.log_func_count.begin();
				break;
			case 6:
				icnts = codeCounter->cmplx_calc_list.begin();
				icnte = codeCounter->cmplx_calc_list.end();
				icntc = codeCounter->cmplx_calc_count.begin();
				icnt = its->second.cmplx_calc_count.begin();
				break;
			case 7:
				icnts = codeCounter->cmplx_cond_list.begin();
				icnte = codeCounter->cmplx_cond_list.end();
				icntc = codeCounter->cmplx_cond_count.begin();
				icnt = its->second.cmplx_cond_count.begin();
				break;
			case 8:
				icnts = codeCounter->cmplx_logic_list.begin();
				icnte = codeCounter->cmplx_logic_list.end();
				icntc = codeCounter->cmplx_logic_count.begin();
				icnt = its->second.cmplx_logic_count.begin();
				break;
			case 9:
				icnts = codeCounter->cmplx_preproc_list.begin();
				icnte = codeCounter->cmplx_preproc_list.end();
				icntc = codeCounter->cmplx_preproc_count.begin();
				icnt = its->second.cmplx_preproc_count.begin();
				break;
			case 10:
				icnts = codeCounter->cmplx_assign_list.begin();
				icnte = codeCounter->cmplx_assign_list.end();
				icntc = codeCounter->cmplx_assign_count.begin();
				icnt = its->second.cmplx_assign_count.begin();
				break;
			case 11:
				icnts = codeCounter->cmplx_pointer_list.begin();
				icnte = codeCounter->cmplx_pointer_list.end();
				icntc = codeCounter->cmplx_pointer_count.begin();
				icnt = its->second.cmplx_pointer_count.begin();
				break;
			}
			while (icnts != icnte)
			{
				if (its->second.duplicate)
					(*icntc).second += (*icnt);
				else
					(*icntc).first += (*icnt);
				icnts++;
				icnt++;
				icntc++;
			}
		}
		if (its->second.duplicate)
		{
			codeCounter->counted_dupFiles++;

			if (useListA)
				codeCounter->total_dupFilesA++;
			else
				codeCounter->total_dupFilesB++;

			if (class_type == WEB)
			{
				// get web file class
				webCounter = (CWebCounter *)codeCounter;
				webType = webCounter->GetWebType(its->second.file_name);
				if (webType == WEB_PHP)
				{
					if (useListA)
						webCounter->total_php_dupFilesA++;
					else
						webCounter->total_php_dupFilesB++;
				}
				else if (webType == WEB_ASP)
				{
					if (useListA)
						webCounter->total_asp_dupFilesA++;
					else
						webCounter->total_asp_dupFilesB++;
				}
				else if (webType == WEB_JSP)
				{
					if (useListA)
						webCounter->total_jsp_dupFilesA++;
					else
						webCounter->total_jsp_dupFilesB++;
				}
				else if (webType == WEB_XML)
				{
					if (useListA)
						webCounter->total_xml_dupFilesA++;
					else
						webCounter->total_xml_dupFilesB++;
				}
				else if (webType == WEB_CFM)
				{
					if (useListA)
						webCounter->total_cfm_dupFilesA++;
					else
						webCounter->total_cfm_dupFilesB++;
				}
				else
				{
					if (useListA)
						webCounter->total_htm_dupFilesA++;
					else
						webCounter->total_htm_dupFilesB++;
				}
			}
		}
		else
			codeCounter->counted_files++;
	}
}


/*!
* AddFromPairVector add counts to Main thread Counter instance vectors from Threads
* Local procedure
*
* \SideEffects	This will clear Source counters to zero after accumulation
*
* \param destVect reference to Destination base class vector instance holding counts
* \param srcVect  reference to   Source    base class vector instance holding counts
*/
void	AddFromPairVector( UIntPairVector & destVect, UIntPairVector & srcVect )
{
	unsigned int size = srcVect.size();
	for ( unsigned int k = 0; k < size; k++ )
	{
		destVect[ k ].first  += srcVect[ k ].first;
		destVect[ k ].second += srcVect[ k ].second;
	}

	if ( size )
		srcVect.clear();
}


/*!
* AddFromACounter add counts to Main thread Counter instance from Threads
* Local procedure
*
* \param pDestCounter pointer to Destination base class Code counter
* \param pSrcCounter  pointer to   Source    base class Code counter
*/
void	AddFromACounter( CCodeCounter * pDestCounter, CCodeCounter * pSrcCounter )
{
	AddFromPairVector( pDestCounter->directive_count,     pSrcCounter->directive_count     );
	AddFromPairVector( pDestCounter->data_name_count,     pSrcCounter->data_name_count     );
	AddFromPairVector( pDestCounter->exec_name_count,     pSrcCounter->exec_name_count     );

	AddFromPairVector( pDestCounter->math_func_count,     pSrcCounter->math_func_count     );
	AddFromPairVector( pDestCounter->trig_func_count,     pSrcCounter->trig_func_count     );
	AddFromPairVector( pDestCounter->log_func_count,      pSrcCounter->log_func_count      );

	AddFromPairVector( pDestCounter->cmplx_calc_count,    pSrcCounter->cmplx_calc_count    );
	AddFromPairVector( pDestCounter->cmplx_cond_count,    pSrcCounter->cmplx_cond_count    );
	AddFromPairVector( pDestCounter->cmplx_logic_count,   pSrcCounter->cmplx_logic_count   );
	AddFromPairVector( pDestCounter->cmplx_preproc_count, pSrcCounter->cmplx_preproc_count );
	AddFromPairVector( pDestCounter->cmplx_assign_count,  pSrcCounter->cmplx_assign_count  );
	AddFromPairVector( pDestCounter->cmplx_pointer_count, pSrcCounter->cmplx_pointer_count );
}


/*!
* AddFromOtherLangCounters add counts to Main thread structure from Threads
*
* \SideEffects	This will clear Source counters to zero after accumulation
*
* \param	pDestLangCounters	IN/OUT	pointer to Language classes whose info needs updating
* \param	pSrcLangCounters	IN/OUT	pointer to Language classes that has new info
* \param	my_useListA			IN		List A counters (otherwise counters for List B)
*/
void AddFromOtherLangCounters( CounterForEachLangType * pDestLangCounters,
							CounterForEachLangType * pSrcLangCounters,
							const bool my_useListA )
{
	// Update main thread from each thread's CounterForEachLangType local data
	CounterForEachLangType::iterator itSrc     = pSrcLangCounters->begin();
	CounterForEachLangType::iterator itDest    = pDestLangCounters->begin();
	CounterForEachLangType::iterator itDestEnd = pDestLangCounters->end();

	CCodeCounter *	pSrcCodeCounter  = NULL;
	CCodeCounter *	pDestCodeCounter = NULL;
	CWebCounter *	webCounterSrc    = NULL;
	CWebCounter *	webCounterDest   = NULL;
	ClassType		classType        = WEB;

	bool	accumulate_from_web_counters = false;

	for ( ; itDest != itDestEnd; itDest++, itSrc++ )
	{
		pSrcCodeCounter  = (*itSrc).second;
		pDestCodeCounter = (*itDest).second;

		// It is possible not all Language classes in all Threads got called by DecideLanguage
		// So only set to true if Source was true else leave alone
		if ( pSrcCodeCounter->print_cmplx )
			pDestCodeCounter->print_cmplx = true;
		
		// Update values changed by the threaded code or Counters called from threaded code.
		// Update the Web classes values that are separate from CCodeCounter class
		classType = pDestCodeCounter->classtype;
		if (  WEB == classType )
		{
			// Add from a WEB counter which has private counts separate from Code counter base
			accumulate_from_web_counters = true;
			webCounterSrc  = (CWebCounter *)pSrcCodeCounter;
			webCounterDest = (CWebCounter *)pDestCodeCounter;
		}
		else
			accumulate_from_web_counters = false;

		if ( my_useListA )
		{
			pDestCodeCounter->total_filesA += pSrcCodeCounter->total_filesA;
			pSrcCodeCounter->total_filesA = 0;

			if ( accumulate_from_web_counters )
			{
				webCounterDest->total_php_filesA += webCounterSrc->total_php_filesA;
				webCounterSrc->total_php_filesA = 0;

				webCounterDest->total_asp_filesA += webCounterSrc->total_asp_filesA;
				webCounterSrc->total_asp_filesA = 0;

				webCounterDest->total_jsp_filesA += webCounterSrc->total_jsp_filesA;
				webCounterSrc->total_jsp_filesA = 0;

				webCounterDest->total_xml_filesA += webCounterSrc->total_xml_filesA;
				webCounterSrc->total_xml_filesA = 0;

				webCounterDest->total_cfm_filesA += webCounterSrc->total_cfm_filesA;
				webCounterSrc->total_cfm_filesA = 0;

				webCounterDest->total_htm_filesA += webCounterSrc->total_htm_filesA;
				webCounterSrc->total_htm_filesA = 0;
			}
		}
		else
		{
			pDestCodeCounter->total_filesB += pSrcCodeCounter->total_filesB;
			pSrcCodeCounter->total_filesB = 0;

			if ( accumulate_from_web_counters )
			{
				webCounterDest->total_php_filesB += webCounterSrc->total_php_filesB;
				webCounterSrc->total_php_filesB = 0;

				webCounterDest->total_asp_filesB += webCounterSrc->total_asp_filesB;
				webCounterSrc->total_asp_filesB = 0;

				webCounterDest->total_jsp_filesB += webCounterSrc->total_jsp_filesB;
				webCounterSrc->total_jsp_filesB = 0;

				webCounterDest->total_xml_filesB += webCounterSrc->total_xml_filesB;
				webCounterSrc->total_xml_filesB = 0;

				webCounterDest->total_cfm_filesB += webCounterSrc->total_cfm_filesB;
				webCounterSrc->total_cfm_filesB = 0;

				webCounterDest->total_htm_filesB += webCounterSrc->total_htm_filesB;
				webCounterSrc->total_htm_filesB = 0;
			}
		}
		AddFromACounter( pDestCodeCounter, pSrcCodeCounter );
	}
}