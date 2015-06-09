//! UCC Print (to various files) Utility procedures public declarations
/*!
* \file PrintUtil.h
*
* This file gives interfaces for the majority of UCC file output.
*
* ADDED to UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_06
*   Refactored MainObject.cpp and moved code to PrintUtil.cpp to support:
*		faster scrolling through MainObject.cpp, 
*		reduce SIZE and complexity of MainObject.cpp,
*		and ease future changes, etc.
*/

#ifndef UCC_PRINT_UTILITY_H
#define UCC_PRINT_UTILITY_H

#include	"CWebCounter.h"

	//! Structure to contain count totals.
	/*!
	* \struct TotalValue
	*
	* Defines a structure to contain count totals.
	*/
	struct TotalValue
	{
		TotalValue()
		{
			total_line = 0;
			blank_line = 0;
			whole_comment = 0;
			embed_comment = 0;
			phy_direct = 0;
			phy_decl = 0;
			phy_instr = 0;
			log_direct = 0;
			log_decl = 0;
			log_instr = 0;
			num_of_file = 0;
		}

		unsigned int total_line;					//!< Total lines
		unsigned int blank_line;					//!< Blank lines
		unsigned int whole_comment;					//!< Whole line comments
		unsigned int embed_comment;					//!< Embedded comments
		unsigned int phy_direct;					//!< Physical directive SLOC
		unsigned int phy_decl;						//!< Physical data declaration SLOC
		unsigned int phy_instr;						//!< Physical executable instruction SLOC
		unsigned int log_direct;					//!< Logical directive SLOC
		unsigned int log_decl;						//!< Logical data declaration SLOC
		unsigned int log_instr;						//!< Logical executable instruction SLOC
		unsigned int num_of_file;					//!< Number of files
	};

	//! Map of count totals.
	/*!
	* \typedef TotalValueMap
	*
	* Defines a map of count totals.
	*/
	typedef map<int, TotalValue> TotalValueMap;


	//! Structure to contain web language count totals.
	/*!
	* \struct WebTotalValue
	*
	* Defines a structure to contain web language count totals.
	*
	* TODO: Replace the magic number name with a better named define for whatever 6 means
	*/
	#define	WEB_MAGIC_NUMBER	6
	struct WebTotalValue
	{
		WebTotalValue()
		{
			total_line = 0;
			blank_line = 0;
			whole_comment = 0;
			embed_comment = 0;
			for (int i = 0; i < WEB_MAGIC_NUMBER; i++)
			{
				phy_direct[i] = 0;
				phy_decl[i] = 0;
				phy_instr[i] = 0;
				log_direct[i] = 0;
				log_decl[i] = 0;
				log_instr[i] = 0;
			}
			num_of_file = 0;
		}

		unsigned int total_line;					//!< Total lines
		unsigned int blank_line;					//!< Blank lines
		unsigned int whole_comment;					//!< Whole line comments
		unsigned int embed_comment;					//!< Embedded comments
		unsigned int phy_direct[WEB_MAGIC_NUMBER];	//!< Physical directive SLOC
		unsigned int phy_decl[WEB_MAGIC_NUMBER];	//!< Physical data declaration SLOC
		unsigned int phy_instr[WEB_MAGIC_NUMBER];	//!< Physical executable instruction SLOC
		unsigned int log_direct[WEB_MAGIC_NUMBER];	//!< Logical directive SLOC
		unsigned int log_decl[WEB_MAGIC_NUMBER];	//!< Logical data declaration SLOC
		unsigned int log_instr[WEB_MAGIC_NUMBER];	//!< Logical executable instruction SLOC
		unsigned int num_of_file;					//!< Number of files
	};

	//! Map of web language count totals.
	/*!
	* \typedef WebTotalValueMap
	*
	* Defines a map of web language count totals.
	*/
	typedef map<WebType, WebTotalValue> WebTotalValueMap;


	//! Structure to contain all web language count totals for unified output.
	/*!
	* \struct AllWebTotalValue
	*
	* Defines a structure to contain all web language count totals.
	* If new web language is added, must update the count value here
	*  0 - HTML    1 - XML    2 - JSclnt  3 - VBSclnt
	*  4 - C#clnt  5 - JSsrv  6 - VBSsrv  7 - C#srv
	*  8 - PHP	   9 - Java  10 - SQL    11 - CF      12 - CFS
	*/
	#define WEB_LANG_COUNT	13
	struct AllWebTotalValue
	{
		AllWebTotalValue()
		{
			total_line = 0;
			blank_line = 0;
			whole_comment = 0;
			embed_comment = 0;
			for (int i = 0; i < WEB_LANG_COUNT; i++)
			{
				phy_direct[i] = 0;
				phy_decl[i] = 0;
				phy_instr[i] = 0;
				log_direct[i] = 0;
				log_decl[i] = 0;
				log_instr[i] = 0;
			}
			num_of_file = 0;
		}

		unsigned int total_line;					//!< Total lines
		unsigned int blank_line;					//!< Blank lines
		unsigned int whole_comment;					//!< Whole line comments
		unsigned int embed_comment;					//!< Embedded comments
		unsigned int phy_direct[WEB_LANG_COUNT];	//!< Physical directive SLOC
		unsigned int phy_decl[WEB_LANG_COUNT];		//!< Physical data declaration SLOC
		unsigned int phy_instr[WEB_LANG_COUNT];		//!< Physical executable instruction SLOC
		unsigned int log_direct[WEB_LANG_COUNT];	//!< Logical directive SLOC
		unsigned int log_decl[WEB_LANG_COUNT];		//!< Logical data declaration SLOC
		unsigned int log_instr[WEB_LANG_COUNT];		//!< Logical executable instruction SLOC
		unsigned int num_of_file;					//!< Number of files
	};


// Public print utility procedures

int PrintCountResults( CounterForEachLangType & CounterForEachLanguage,
						const bool useListA = true, const string &outputFileNamePrePend = "",
						StringVector* filesToPrint = NULL, const bool excludeFiles = true );

int PrintTotalCountResults( CounterForEachLangType & CounterForEachLanguage,
							const bool useListA = true, const string &outputFileNamePrePend = "",
							StringVector* filesToPrint = NULL, const bool excludeFiles = true );

int PrintComplexityResults( CounterForEachLangType & CounterForEachLanguage, 
							const bool useListA = true, const string &outputFileNamePrePend = "", 
							const bool printDuplicates = false);

void PrintDuplicateSummary( const bool useListA = true, const string &outputFileNamePrePend = "" );

int PrintCountSummary( CounterForEachLangType & CounterForEachLanguage,
						TotalValueMap &total, WebTotalValueMap &webtotal,
						const string &outputFileNamePrePend = "" );

#endif	//	#ifndef UCC_PRINT_UTILITY_H
