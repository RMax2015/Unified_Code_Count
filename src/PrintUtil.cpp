//! UCC Print (to various files) Utility procedures implementation
/*!
* \file PrintUtil.cpp
*
* This file supports the majority of UCC file output.
*
* ADDED to UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_06
*   Refactored MainObject.cpp and moved Printing related code here to:
*		make scrolling through MainObject faster, 
*		reduce SIZE and complexity of MainObject (less than 1/2 of previous size!),
*		and ease future changes.
*   This code should NOT be called from any thread but the main thread.
*/

#include <string>
#include <sstream>

using namespace std;

#include "UCCGlobals.h"
#include "cc_main.h"
#include "CUtil.h"
#include "UserIF.h"

#include "LangUtils.h"

#include "PrintUtil.h"


#define DUP_PAIRS_OUTFILE		"DuplicatePairs.txt"
#define DUP_PAIRS_OUTFILE_CSV	"DuplicatePairs.csv"

extern	UserIF *	userIF;					//!< User interface for presenting messages/progress to user

//
//		Local Data declarations
//
ofstream output_summary;					//!< Output summary file stream
ofstream output_summary_csv;				//!< Output summary CSV file stream
ofstream output_file;						//!< Total output file stream
ofstream output_file_csv;					//!< Total output CSV file stream

//
//		Local Procedure prototypes
//
int PrintCyclomaticComplexity(const bool useListA, const string &outputFileNamePrePend, const bool printDuplicates);

void PrintDuplicateList(StringVector &myList1, StringVector &myList2, ofstream &outfile, const bool csvFormat = false);

ofstream* GetTotalOutputStream(const string &outputFileNamePrePend = "", const bool csvOutput = false);

ofstream* GetOutputSummaryStream(const string &outputFileNamePrePend = "", const bool csvOutput = false);

void CloseOutputSummaryStream();

void CloseTotalOutputStream();

/*!
* Prints the counting results for a set of files.
*
* \param useListA use file list A? (otherwise use list B)
* \param outputFileNamePrePend name to prepend to the output file
* \param filesToPrint list of files to include or exclude
* \param excludeFiles exclude files? (if true excludes files in filesToPrint; if false includes only files in filesToPrint)
*
* \return method status
*/
int PrintCountResults( CounterForEachLangType & CounterForEachLanguage,
						const bool useListA, const string &outputFileNamePrePend, 
						StringVector *filesToPrint, const bool excludeFiles )
{
	ofstream* pout     = NULL;
	ofstream* pout_csv = NULL;
	TotalValueMap total;
	WebTotalValueMap webtotal;
	StringVector::iterator sit;
	ClassType class_type;
	string file_type;
	CWebCounter *webCounter;
	WebType webType;

	// skip if all files are excluded
	if (filesToPrint != NULL && filesToPrint->size() < 1 && !excludeFiles)
		return 0;

	SourceFileList::iterator its;
	SourceFileList* mySourceFile = (useListA) ? &SourceFileA : &SourceFileB;
	for (its = mySourceFile->begin(); its != mySourceFile->end(); its++)
	{
		if (filesToPrint != NULL && filesToPrint->size() > 0)
		{
			// restrict based on those files in the filesToPrint list
			sit = filesToPrint->begin();
			while (sit != filesToPrint->end() && its->second.file_name.compare((*sit)) != 0)
				sit++;

			if (excludeFiles)
			{
				// skip the file if in the filesToPrint list
				if (sit != filesToPrint->end())
					continue;
			}
			else
			{
				 // skip the file if NOT in the filesToPrint list
				if (sit == filesToPrint->end())
					continue;
			}
		}

		if (its->second.class_type == WEB)
		{
			SourceFileList::iterator startpos = its;
			SourceFileList::iterator endpos = ++startpos;
			for (; endpos!= mySourceFile->end(); endpos++)
			{
				if ( endpos->second.file_name_isEmbedded == false )
					break;
			}
			webType = ((CWebCounter*)CounterForEachLanguage[WEB])->GetWebType(its->second.file_name);
			if (print_ascii || print_legacy)
			{
				pout = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(webType, outDir + outputFileNamePrePend, cmdLine, false, print_legacy);
				if (pout == NULL)
				{
					string err = "Error: Unable to create file (";
					err += outputFileNamePrePend;
					if (webType == WEB_PHP)
						err += "PHP";
					else if (webType == WEB_ASP)
						err += "ASP";
					else if (webType == WEB_JSP)
						err += "JSP";
					else if (webType == WEB_XML)
						err += "XML";
					else if (webType == WEB_CFM)
						err += "ColdFusion";
					else
						err += "HTML";
					err += OUTPUT_FILE_NAME;
					err += "). Operation aborted.";
					userIF->AddError(err);
					return 0;
				}
			}
			if (print_csv)
			{
				pout_csv = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(webType, outDir + outputFileNamePrePend, cmdLine, true);
				if (pout_csv == NULL)
				{
					string err = "Error: Unable to create file (";
					err += outputFileNamePrePend;
					if (webType == WEB_PHP)
						err += "PHP";
					else if (webType == WEB_ASP)
						err += "ASP";
					else if (webType == WEB_JSP)
						err += "JSP";
					else if (webType == WEB_XML)
						err += "XML";
					else if (webType == WEB_CFM)
						err += "ColdFusion";
					else
						err += "HTML";
					err += OUTPUT_FILE_NAME_CSV;
					err += "). Operation aborted.";
					userIF->AddError(err);
					return 0;
				}
			}

			if (webType == WEB_PHP)
			{
				results r_htm;
				results r_js;
				results r_vbs;
				results r_php;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML_PHP:
						r_htm = i->second;
						break;
					case JAVASCRIPT_PHP:
						r_js = i->second;
						break;
					case VBS_PHP:
						r_vbs = i->second;
						break;
					case PHP:
						r_php = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_php.SLOC_lines[PHY] = r_php.directive_lines[PHY] + r_php.exec_lines[PHY] + r_php.data_lines[PHY];
				r_php.SLOC_lines[LOG] = r_php.directive_lines[LOG] + r_php.exec_lines[LOG] + r_php.data_lines[LOG];
				r_php.total_lines = r_php.SLOC_lines[PHY] +	r_php.blank_lines + r_php.comment_lines;

				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_php.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_php.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_php.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_php.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_php.directive_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_php.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_php.exec_lines[LOG];
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_php.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_php.SLOC_lines[LOG];
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_php.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_php.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name << endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_php.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_php.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_php.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_php.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << r_php.directive_lines[LOG] << ",";
					(*pout_csv) << r_php.data_lines[LOG] << ",";
					(*pout_csv) << r_php.exec_lines[LOG] << ",";
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_php.SLOC_lines[LOG] << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_php.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_php.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_php.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_php.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_php.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_php.e_comm_lines);
				
				//0:htm 1:js 2:vbs 3:php
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_direct[3] += r_php.directive_lines[PHY];
				webtotal[webType].phy_decl[3] += r_php.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_php.exec_lines[PHY];
				webtotal[webType].log_direct[3] += r_php.directive_lines[LOG];
				webtotal[webType].log_decl[3] += r_php.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_php.exec_lines[LOG];
			}
			else if (webType == WEB_JSP)
			{
				results r_htm;
				results r_js;
				results r_vbs;
				results r_java;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML_JSP:
						r_htm = i->second;
						break;
					case JAVASCRIPT_JSP:
						r_js = i->second;
						break;
					case VBS_JSP:
						r_vbs = i->second;
						break;
					case JAVA_JSP:
						r_java = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_java.SLOC_lines[PHY] = r_java.directive_lines[PHY] + r_java.exec_lines[PHY] + r_java.data_lines[PHY];
				r_java.SLOC_lines[LOG] = r_java.directive_lines[LOG] + r_java.exec_lines[LOG] + r_java.data_lines[LOG];
				r_java.total_lines = r_java.SLOC_lines[PHY] +	r_java.blank_lines + r_java.comment_lines;

				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_java.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_java.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_java.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_java.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_java.directive_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_java.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_java.exec_lines[LOG];
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_java.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_java.SLOC_lines[LOG];
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_java.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_java.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name << endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_java.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_java.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_java.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_java.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << r_java.directive_lines[LOG] << ",";
					(*pout_csv) << r_java.data_lines[LOG] << ",";
					(*pout_csv) << r_java.exec_lines[LOG] << ",";
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_java.SLOC_lines[LOG] << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_java.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_java.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_java.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_java.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_java.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_java.e_comm_lines);
				
				//0:htm 1:js 2:vbs 3:java
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_direct[3] += r_java.directive_lines[PHY];
				webtotal[webType].phy_decl[3] += r_java.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_java.exec_lines[PHY];
				webtotal[webType].log_direct[3] += r_java.directive_lines[LOG];
				webtotal[webType].log_decl[3] += r_java.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_java.exec_lines[LOG];
			}
			else if (webType == WEB_ASP)
			{
				results r_htm;
				results r_jsc;
				results r_vbsc;
				results r_jss;
				results r_vbss;
				results r_css;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML_ASP:
						r_htm = i->second;
						break;
					case JAVASCRIPT_ASP_C:
						r_jsc = i->second;
						break;
					case VBS_ASP_C:
						r_vbsc = i->second;
						break;
					case JAVASCRIPT_ASP_S:
						r_jss = i->second;
						break;
					case VBS_ASP_S:
						r_vbss = i->second;
						break;
					case CSHARP_ASP_S:
						r_css = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_jsc.SLOC_lines[PHY] = r_jsc.exec_lines[PHY] + r_jsc.data_lines[PHY];
				r_jsc.SLOC_lines[LOG] = r_jsc.exec_lines[LOG] + r_jsc.data_lines[LOG];
				r_jsc.total_lines = r_jsc.SLOC_lines[PHY] +	r_jsc.blank_lines + r_jsc.comment_lines;

				r_vbsc.SLOC_lines[PHY] = r_vbsc.exec_lines[PHY] + r_vbsc.data_lines[PHY];
				r_vbsc.SLOC_lines[LOG] = r_vbsc.exec_lines[LOG] + r_vbsc.data_lines[LOG];
				r_vbsc.total_lines = r_vbsc.SLOC_lines[PHY] +	r_vbsc.blank_lines + r_vbsc.comment_lines;

				r_jss.SLOC_lines[PHY] = r_jss.directive_lines[PHY] + r_jss.exec_lines[PHY] + r_jss.data_lines[PHY];
				r_jss.SLOC_lines[LOG] = r_jss.directive_lines[LOG] + r_jss.exec_lines[LOG] + r_jss.data_lines[LOG];
				r_jss.total_lines = r_jss.SLOC_lines[PHY] +	r_jss.blank_lines + r_jss.comment_lines;

				r_vbss.SLOC_lines[PHY] = r_vbss.exec_lines[PHY] + r_vbss.data_lines[PHY];
				r_vbss.SLOC_lines[LOG] = r_vbss.exec_lines[LOG] + r_vbss.data_lines[LOG];
				r_vbss.total_lines = r_vbss.SLOC_lines[PHY] +	r_vbss.blank_lines + r_vbss.comment_lines;

				r_css.SLOC_lines[PHY] = r_css.exec_lines[PHY] + r_css.data_lines[PHY];
				r_css.SLOC_lines[LOG] = r_css.exec_lines[LOG] + r_css.data_lines[LOG];
				r_css.total_lines = r_css.SLOC_lines[PHY] +	r_css.blank_lines + r_css.comment_lines;

				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_jsc.total_lines + r_vbsc.total_lines + r_jss.total_lines + r_vbss.total_lines + r_css.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_jsc.blank_lines + r_vbsc.blank_lines + r_jss.blank_lines + r_vbss.blank_lines + r_css.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_jsc.comment_lines + r_vbsc.comment_lines + r_jss.comment_lines + r_vbss.comment_lines + r_css.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_jsc.e_comm_lines + r_vbsc.e_comm_lines + r_jss.e_comm_lines + r_vbss.e_comm_lines + r_css.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_jsc.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_jsc.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbsc.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbsc.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_jss.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_jss.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbss.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbss.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_css.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_css.exec_lines[LOG];
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_jsc.SLOC_lines[LOG] + r_vbsc.SLOC_lines[LOG] + r_jss.SLOC_lines[LOG] + r_vbss.SLOC_lines[LOG] + r_css.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_jsc.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbsc.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_jss.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbss.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_css.SLOC_lines[LOG];
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_jsc.SLOC_lines[LOG] + r_vbsc.SLOC_lines[LOG] + r_jss.SLOC_lines[LOG] + r_vbss.SLOC_lines[LOG] + r_css.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_jsc.SLOC_lines[PHY] + r_vbsc.SLOC_lines[PHY] + r_jss.SLOC_lines[PHY] + r_vbss.SLOC_lines[PHY] + r_css.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name<<endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_jsc.total_lines + r_vbsc.total_lines + r_jss.total_lines + r_vbss.total_lines + r_css.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_jsc.blank_lines + r_vbsc.blank_lines + r_jss.blank_lines + r_vbss.blank_lines + r_css.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_jsc.comment_lines + r_vbsc.comment_lines + r_jss.comment_lines + r_vbss.comment_lines + r_css.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_jsc.e_comm_lines + r_vbsc.e_comm_lines + r_jss.e_comm_lines + r_vbss.e_comm_lines + r_css.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << r_jsc.data_lines[LOG] << ",";
					(*pout_csv) << r_jsc.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbsc.data_lines[LOG] << ",";
					(*pout_csv) << r_vbsc.exec_lines[LOG] << ",";
					(*pout_csv) << r_jss.data_lines[LOG] << ",";
					(*pout_csv) << r_jss.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbss.data_lines[LOG] << ",";
					(*pout_csv) << r_vbss.exec_lines[LOG] << ",";
					(*pout_csv) << r_css.data_lines[LOG] << ",";
					(*pout_csv) << r_css.exec_lines[LOG] << ",";
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_jsc.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbsc.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_jss.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbss.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_css.SLOC_lines[LOG] << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_jsc.SLOC_lines[LOG] + r_vbsc.SLOC_lines[LOG] + r_jss.SLOC_lines[LOG] + r_vbss.SLOC_lines[LOG] + r_css.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_jsc.SLOC_lines[PHY] + r_vbsc.SLOC_lines[PHY] + r_jss.SLOC_lines[PHY] + r_vbss.SLOC_lines[PHY] + r_css.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_jsc.total_lines + r_vbsc.total_lines + r_jss.total_lines + r_vbss.total_lines + r_css.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_jsc.blank_lines + r_vbsc.blank_lines + r_jss.blank_lines + r_vbss.blank_lines + r_css.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_jsc.comment_lines + r_vbsc.comment_lines + r_jss.comment_lines + r_vbss.comment_lines + r_css.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_jsc.e_comm_lines + r_vbsc.e_comm_lines + r_jss.e_comm_lines + r_vbss.e_comm_lines + r_css.e_comm_lines);
				
				//0:htm 1:jsc 2:vbsc 3:jss 4:vbss 5:csharps
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_jsc.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_jsc.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_jsc.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_jsc.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbsc.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbsc.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbsc.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbsc.exec_lines[LOG];

				webtotal[webType].phy_decl[3] += r_jss.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_jss.exec_lines[PHY];
				webtotal[webType].log_decl[3] += r_jss.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_jss.exec_lines[LOG];

				webtotal[webType].phy_decl[4] += r_vbss.data_lines[PHY];
				webtotal[webType].phy_instr[4] += r_vbss.exec_lines[PHY];
				webtotal[webType].log_decl[4] += r_vbss.data_lines[LOG];
				webtotal[webType].log_instr[4] += r_vbss.exec_lines[LOG];

				webtotal[webType].phy_decl[5] += r_css.data_lines[PHY];
				webtotal[webType].phy_instr[5] += r_css.exec_lines[PHY];
				webtotal[webType].log_decl[5] += r_css.data_lines[LOG];
				webtotal[webType].log_instr[5] += r_css.exec_lines[LOG];
			}
			else if (webType == WEB_XML)
			{
				results r_xml;
				results r_js;
				results r_vbs;
				results r_cs;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case XML:
						r_xml = i->second;
						break;
					case JAVASCRIPT_XML:
						r_js = i->second;
						break;
					case VBS_XML:
						r_vbs = i->second;
						break;
					case CSHARP_XML:
						r_cs = i->second;
						break;
					default:
						break;
					}
				}
				r_xml.SLOC_lines[PHY] = r_xml.exec_lines[PHY] + r_xml.data_lines[PHY];
				r_xml.SLOC_lines[LOG] = r_xml.exec_lines[LOG] + r_xml.data_lines[LOG];
				r_xml.total_lines = r_xml.SLOC_lines[PHY] +	r_xml.blank_lines + r_xml.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_cs.SLOC_lines[PHY] = r_cs.exec_lines[PHY] + r_cs.data_lines[PHY];
				r_cs.SLOC_lines[LOG] = r_cs.exec_lines[LOG] + r_cs.data_lines[LOG];
				r_cs.total_lines = r_cs.SLOC_lines[PHY] +	r_cs.blank_lines + r_cs.comment_lines;

				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_xml.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_xml.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_xml.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_xml.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_xml.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_xml.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_cs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cs.exec_lines[LOG];
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_xml.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_xml.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cs.SLOC_lines[LOG];
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_xml.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_xml.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_cs.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name<<endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_xml.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines) << ",";
					(*pout_csv) << (r_xml.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines) << ",";
					(*pout_csv) << (r_xml.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines) << ",";
					(*pout_csv) << (r_xml.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines) << ",";
					(*pout_csv) << r_xml.data_lines[LOG] << ",";
					(*pout_csv) << r_xml.exec_lines[LOG] << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << r_cs.data_lines[LOG] << ",";
					(*pout_csv) << r_cs.exec_lines[LOG] << ",";
					(*pout_csv) << r_xml.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_cs.SLOC_lines[LOG] << ",";
					(*pout_csv) << (r_xml.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_xml.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_cs.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_xml.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
				webtotal[webType].blank_line += (r_xml.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
				webtotal[webType].whole_comment += (r_xml.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
				webtotal[webType].embed_comment += (r_xml.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);

				//0:xml 1:js 2:vbs 3:csharp
				webtotal[webType].phy_decl[0] += r_xml.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_xml.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_xml.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_xml.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_decl[3] += r_cs.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_cs.exec_lines[PHY];
				webtotal[webType].log_decl[3] += r_cs.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_cs.exec_lines[LOG];
			}
			else if (webType == WEB_CFM)
			{
				results r_htm;
				results r_js;
				results r_vbs;
				results r_sql;
				results r_cfm;
				results r_cfs;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML_CFM:
						r_htm = i->second;
						break;
					case JAVASCRIPT_CFM:
						r_js = i->second;
						break;
					case VBS_CFM:
						r_vbs = i->second;
						break;
					case SQL_CFM:
						r_sql = i->second;
						break;
					case COLDFUSION:
						r_cfm = i->second;
						break;
					case CFSCRIPT:
						r_cfs = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_sql.SLOC_lines[PHY] = r_sql.exec_lines[PHY] + r_sql.data_lines[PHY];
				r_sql.SLOC_lines[LOG] = r_sql.exec_lines[LOG] + r_sql.data_lines[LOG];
				r_sql.total_lines = r_sql.SLOC_lines[PHY] +	r_sql.blank_lines + r_sql.comment_lines;

				r_cfm.SLOC_lines[PHY] = r_cfm.exec_lines[PHY] + r_cfm.data_lines[PHY];
				r_cfm.SLOC_lines[LOG] = r_cfm.exec_lines[LOG] + r_cfm.data_lines[LOG];
				r_cfm.total_lines = r_cfm.SLOC_lines[PHY] +	r_cfm.blank_lines + r_cfm.comment_lines;

				r_cfs.SLOC_lines[PHY] = r_cfs.exec_lines[PHY] + r_cfs.data_lines[PHY];
				r_cfs.SLOC_lines[LOG] = r_cfs.exec_lines[LOG] + r_cfs.data_lines[LOG];
				r_cfs.total_lines = r_cfs.SLOC_lines[PHY] +	r_cfs.blank_lines + r_cfs.comment_lines;

				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_sql.total_lines + r_cfm.total_lines + r_cfs.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_sql.blank_lines + r_cfm.blank_lines + r_cfs.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_sql.comment_lines + r_cfm.comment_lines + r_cfs.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_sql.e_comm_lines + r_cfm.e_comm_lines + r_cfs.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_sql.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_sql.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_cfm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cfm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_cfs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cfs.exec_lines[LOG];
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_sql.SLOC_lines[LOG] + r_cfm.SLOC_lines[LOG] + r_cfs.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_sql.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(10);		(*pout) << r_cfm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_cfs.SLOC_lines[LOG];
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_sql.SLOC_lines[LOG] + r_cfm.SLOC_lines[LOG] + r_cfs.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_sql.SLOC_lines[PHY] + r_cfm.SLOC_lines[PHY] + r_cfs.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name<<endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_sql.total_lines + r_cfm.total_lines + r_cfs.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_sql.blank_lines + r_cfm.blank_lines + r_cfs.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_sql.comment_lines + r_cfm.comment_lines + r_cfs.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_sql.e_comm_lines + r_cfm.e_comm_lines + r_cfs.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << r_sql.data_lines[LOG] << ",";
					(*pout_csv) << r_sql.exec_lines[LOG] << ",";
					(*pout_csv) << r_cfm.data_lines[LOG] << ",";
					(*pout_csv) << r_cfm.exec_lines[LOG] << ",";
					(*pout_csv) << r_cfs.data_lines[LOG] << ",";
					(*pout_csv) << r_cfs.exec_lines[LOG] << ",";
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_sql.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_cfm.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_cfs.SLOC_lines[LOG] << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_sql.SLOC_lines[LOG] + r_cfm.SLOC_lines[LOG] + r_cfs.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_sql.SLOC_lines[PHY] + r_cfm.SLOC_lines[PHY] + r_cfs.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_sql.total_lines + r_cfm.total_lines + r_cfs.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_sql.blank_lines + r_cfm.blank_lines + r_cfs.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_sql.comment_lines + r_cfm.comment_lines + r_cfs.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_sql.e_comm_lines + r_cfm.e_comm_lines + r_cfs.e_comm_lines);

				//0:htm 1:js 2:vbs 3:sql 5:cfm 6:cfs
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_decl[3] += r_sql.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_sql.exec_lines[PHY];
				webtotal[webType].log_decl[3] += r_sql.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_sql.exec_lines[LOG];

				webtotal[webType].phy_decl[4] += r_cfm.data_lines[PHY];
				webtotal[webType].phy_instr[4] += r_cfm.exec_lines[PHY];
				webtotal[webType].log_decl[4] += r_cfm.data_lines[LOG];
				webtotal[webType].log_instr[4] += r_cfm.exec_lines[LOG];

				webtotal[webType].phy_decl[5] += r_cfs.data_lines[PHY];
				webtotal[webType].phy_instr[5] += r_cfs.exec_lines[PHY];
				webtotal[webType].log_decl[5] += r_cfs.data_lines[LOG];
				webtotal[webType].log_instr[5] += r_cfs.exec_lines[LOG];
			}
			else
			{
				webType = WEB_HTM;
				results r_htm;
				results r_js;
				results r_vbs;
				results r_cs;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML:
						r_htm = i->second;
						break;
					case JAVASCRIPT_HTML:
						r_js = i->second;
						break;
					case VBS_HTML:
						r_vbs = i->second;
						break;
					case CSHARP_HTML:
						r_cs = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_cs.SLOC_lines[PHY] = r_cs.exec_lines[PHY] + r_cs.data_lines[PHY];
				r_cs.SLOC_lines[LOG] = r_cs.exec_lines[LOG] + r_cs.data_lines[LOG];
				r_cs.total_lines = r_cs.SLOC_lines[PHY] +	r_cs.blank_lines + r_cs.comment_lines;

				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_cs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cs.exec_lines[LOG];
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cs.SLOC_lines[LOG];
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_cs.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name << endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << r_cs.data_lines[LOG] << ",";
					(*pout_csv) << r_cs.exec_lines[LOG] << ",";
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_cs.SLOC_lines[LOG] << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_cs.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);
				
				//0:htm 1:js 2:vbs 3:csharp
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_decl[3] += r_cs.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_cs.exec_lines[PHY];
				webtotal[webType].log_decl[3] += r_cs.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_cs.exec_lines[LOG];
			}

			// skip the other web language partial file
			its = --endpos;
		}
		else // languages other than WEB
		{
			class_type = its->second.class_type;
			file_type = (class_type != UNKNOWN && class_type != DATAFILE ? "CODE" : "DATA");

			// do not print temp files that are created to represent embedded code
			if ( its->second.file_name_isEmbedded == true ) 
				continue;

			if (print_ascii || print_legacy)
			{
				pout = CounterForEachLanguage[class_type]->GetOutputStream(outDir + outputFileNamePrePend, cmdLine);
				if (pout == NULL)
				{
					string err = "Error: Unable to create file (";
					err += outputFileNamePrePend + CounterForEachLanguage[class_type]->language_name + OUTPUT_FILE_NAME;
					err += "). Operation aborted.";
					userIF->AddError(err, false, 1);
					return 0;
				}
			}
			if (print_csv)
			{
				pout_csv = CounterForEachLanguage[class_type]->GetOutputStream(outDir + outputFileNamePrePend, cmdLine, true);
				if (pout_csv == NULL)
				{
					string err = "Error: Unable to create file (";
					err += outputFileNamePrePend + CounterForEachLanguage[class_type]->language_name + OUTPUT_FILE_NAME_CSV;
					err += "). Operation aborted.";
					userIF->AddError(err, false, 1);
					return 0;
				}
			}

			if (total.count(class_type) == 0)
				total.insert(TotalValueMap::value_type(class_type, TotalValue()));

			if (its->second.e_flag)
			{
				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::left);
					(*pout) << " Error: ";
					(*pout).width(65);
					(*pout) << its->second.error_code;
					(*pout) << its->second.file_name << endl << endl;
					(*pout).unsetf(ios::left);
				}
				if (print_csv)
				{
					(*pout_csv) << "Error: ";
					(*pout_csv) << its->second.error_code << ",,,,,,,,,";
					(*pout_csv) << its->second.file_name << endl << endl;
				}
				continue;
			}
			else
			{
				its->second.SLOC_lines[PHY] = its->second.directive_lines[PHY] + its->second.exec_lines[PHY] + its->second.data_lines[PHY];
				its->second.SLOC_lines[LOG] = its->second.directive_lines[LOG] + its->second.exec_lines[LOG] + its->second.data_lines[LOG];
				its->second.total_lines = its->second.SLOC_lines[PHY] +	its->second.blank_lines + its->second.comment_lines;
			}

			total[class_type].num_of_file++;

			if (print_ascii || print_legacy)
			{
				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << its->second.total_lines;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << its->second.blank_lines;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << its->second.comment_lines;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << its->second.e_comm_lines;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << its->second.directive_lines[LOG];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << its->second.data_lines[LOG];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << its->second.exec_lines[LOG];
				(*pout) << " |";
				(*pout).width(8);	(*pout) << its->second.SLOC_lines[LOG];
				if (!print_legacy)
				{
					(*pout) << " ";
					(*pout).width(8);	(*pout) << its->second.SLOC_lines[PHY];
				}
				(*pout) << " | " << file_type;
				(*pout) << "  " << its->second.file_name;
				(*pout) << endl;
				(*pout).unsetf(ios::right);
			}
			if (print_csv)
			{
				(*pout_csv) << its->second.total_lines << ",";
				(*pout_csv) << its->second.blank_lines << ",";
				(*pout_csv) << its->second.comment_lines << ",";
				(*pout_csv) << its->second.e_comm_lines << ",";
				(*pout_csv) << its->second.directive_lines[LOG] << ",";
				(*pout_csv) << its->second.data_lines[LOG] << ",";
				(*pout_csv) << its->second.exec_lines[LOG] << ",";
				(*pout_csv) << its->second.SLOC_lines[LOG] << ",";
				(*pout_csv) << its->second.SLOC_lines[PHY] << ",";
				(*pout_csv) << file_type << ",";
				(*pout_csv) << its->second.file_name << endl;
			}

			// total count for physical lines
			total[class_type].total_line += its->second.total_lines;
			total[class_type].blank_line += its->second.blank_lines;
			total[class_type].whole_comment += its->second.comment_lines;
			total[class_type].embed_comment += its->second.e_comm_lines;
			total[class_type].phy_direct += its->second.directive_lines[PHY];
			total[class_type].phy_decl += its->second.data_lines[PHY];
			total[class_type].phy_instr += its->second.exec_lines[PHY];
			total[class_type].log_direct += its->second.directive_lines[LOG];
			total[class_type].log_decl += its->second.data_lines[LOG];
			total[class_type].log_instr += its->second.exec_lines[LOG];
		}
	}

	// display summary for WEB languages
	for (WebTotalValueMap::iterator itto = webtotal.begin(); itto != webtotal.end(); itto++)
	{
		webCounter = (CWebCounter*)CounterForEachLanguage[WEB];
		if (itto->first == WEB_PHP)
		{
			if (print_ascii || print_legacy)
			{
				pout = webCounter->GetOutputStream(WEB_PHP, outDir + outputFileNamePrePend, cmdLine);

				(*pout) << endl;
				CUtil::PrintFileHeaderLine(*pout, "RESULTS SUMMARY");
				(*pout) << endl;
				(*pout) << "                                    |       HTML      |     JS-Clnt     |    VBS-Clnt     |           PHP           |" << endl;
				if (print_legacy)
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  | Compiler  Data   Exec.  |                    SLOC                    | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. | Direct.   Decl.  Instr. |   Total      HTML JS-Clnt VBS-Clnt     PHP | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-------------------------+---------+----------------------------------+------------------" << endl;
				}
				else
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  | Compiler  Data   Exec.  |                    SLOC                    | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. | Direct.   Decl.  Instr. |    HTML JS-Clnt VBS-Clnt     PHP     Total | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-------------------------+----------------------------------+---------+------------------" << endl;
				}

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_direct[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[3];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				}
				(*pout) << " | CODE  Physical"<<endl;

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_direct[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[3];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[1] + itto->second.log_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[2] + itto->second.log_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]);
				}
				(*pout) << " | CODE  Logical" << endl;
			}
			if (print_csv)
			{
				pout_csv = webCounter->GetOutputStream(WEB_PHP, outDir + outputFileNamePrePend, cmdLine, true);

				(*pout_csv) << endl;
				CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS SUMMARY");
				(*pout_csv) << endl;
				(*pout_csv) << ",,,,HTML,,JS-Clnt,,VBS-Clnt,,PHP" << endl;
				(*pout_csv) << "Total,Blank,Comments,,Word,Exec.,Data,Exec.,Data,Exec.,Compiler,Data,Exec.,SLOC,,,,,File,SLOC" << endl;
				(*pout_csv) << "Lines,Lines,Whole,Embedded,LOC,Instr.,Decl.,Instr.,Decl.,Instr.,Direct.,Decl.,Instr.,HTML,JS-Clnt,VBS-Clnt,PHP,Total,Type,Definition" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.phy_decl[0] << ",";
				(*pout_csv) << itto->second.phy_instr[0] << ",";
				(*pout_csv) << itto->second.phy_decl[1] << ",";
				(*pout_csv) << itto->second.phy_instr[1] << ",";
				(*pout_csv) << itto->second.phy_decl[2] << ",";
				(*pout_csv) << itto->second.phy_instr[2] << ",";
				(*pout_csv) << itto->second.phy_direct[3] << ",";
				(*pout_csv) << itto->second.phy_decl[3] << ",";
				(*pout_csv) << itto->second.phy_instr[3] << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]) << ",";
				(*pout_csv) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]) << ",";
				(*pout_csv) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]) << ",";
				(*pout_csv) << (itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
					itto->second.phy_decl[1] + itto->second.phy_instr[1] +
					itto->second.phy_decl[2] + itto->second.phy_instr[2] +
					itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << "CODE,Physical" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.log_decl[0] << ",";
				(*pout_csv) << itto->second.log_instr[0] << ",";
				(*pout_csv) << itto->second.log_decl[1] << ",";
				(*pout_csv) << itto->second.log_instr[1] << ",";
				(*pout_csv) << itto->second.log_decl[2] << ",";
				(*pout_csv) << itto->second.log_instr[2] << ",";
				(*pout_csv) << itto->second.log_direct[3] << ",";
				(*pout_csv) << itto->second.log_decl[3] << ",";
				(*pout_csv) << itto->second.log_instr[3] << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0]) << ",";
				(*pout_csv) << (itto->second.log_decl[1] + itto->second.log_instr[1]) << ",";
				(*pout_csv) << (itto->second.log_decl[2] + itto->second.log_instr[2]) << ",";
				(*pout_csv) << (itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
					itto->second.log_decl[1] + itto->second.log_instr[1] +
					itto->second.log_decl[2] + itto->second.log_instr[2] +
					itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << "CODE,Logical" << endl;
			}
		}
		else if (itto->first == WEB_JSP)
		{
			if (print_ascii || print_legacy)
			{
				pout = webCounter->GetOutputStream(WEB_JSP, outDir + outputFileNamePrePend, cmdLine);

				(*pout) << endl;
				CUtil::PrintFileHeaderLine(*pout, "RESULTS SUMMARY");
				(*pout) << endl;
				(*pout) << "                                    |       HTML      |     JS-Clnt     |    VBS-Clnt     |          Java           |" << endl;
				if (print_legacy)
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  | Compiler  Data   Exec.  |                    SLOC                    | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. | Direct.   Decl.  Instr. |   Total      HTML JS-Clnt VBS-Clnt    Java | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-------------------------+---------+----------------------------------+------------------" << endl;
				}
				else
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  | Compiler  Data   Exec.  |                    SLOC                    | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. | Direct.   Decl.  Instr. |    HTML JS-Clnt VBS-Clnt    Java     Total | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-------------------------+----------------------------------+---------+------------------" << endl;
				}

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_direct[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[3];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				}
				(*pout) << " | CODE  Physical"<<endl;

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_direct[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[3];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[1] + itto->second.log_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[2] + itto->second.log_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]);
				}
				(*pout) << " | CODE  Logical" << endl;
			}
			if (print_csv)
			{
				pout_csv = webCounter->GetOutputStream(WEB_JSP, outDir + outputFileNamePrePend, cmdLine, true);

				(*pout_csv) << endl;
				CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS SUMMARY");
				(*pout_csv) << endl;
				(*pout_csv) << ",,,,HTML,,JS-Clnt,,VBS-Clnt,,Java" << endl;
				(*pout_csv) << "Total,Blank,Comments,,Word,Exec.,Data,Exec.,Data,Exec.,Compiler,Data,Exec.,SLOC,,,,,File,SLOC" << endl;
				(*pout_csv) << "Lines,Lines,Whole,Embedded,LOC,Instr.,Decl.,Instr.,Decl.,Instr.,Direct.,Decl.,Instr.,HTML,JS-Clnt,VBS-Clnt,Java,Total,Type,Definition" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.phy_decl[0] << ",";
				(*pout_csv) << itto->second.phy_instr[0] << ",";
				(*pout_csv) << itto->second.phy_decl[1] << ",";
				(*pout_csv) << itto->second.phy_instr[1] << ",";
				(*pout_csv) << itto->second.phy_decl[2] << ",";
				(*pout_csv) << itto->second.phy_instr[2] << ",";
				(*pout_csv) << itto->second.phy_direct[3] << ",";
				(*pout_csv) << itto->second.phy_decl[3] << ",";
				(*pout_csv) << itto->second.phy_instr[3] << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]) << ",";
				(*pout_csv) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]) << ",";
				(*pout_csv) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]) << ",";
				(*pout_csv) << (itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
					itto->second.phy_decl[1] + itto->second.phy_instr[1] +
					itto->second.phy_decl[2] + itto->second.phy_instr[2] +
					itto->second.phy_direct[3] + itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << "CODE,Physical" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.log_decl[0] << ",";
				(*pout_csv) << itto->second.log_instr[0] << ",";
				(*pout_csv) << itto->second.log_decl[1] << ",";
				(*pout_csv) << itto->second.log_instr[1] << ",";
				(*pout_csv) << itto->second.log_decl[2] << ",";
				(*pout_csv) << itto->second.log_instr[2] << ",";
				(*pout_csv) << itto->second.log_direct[3] << ",";
				(*pout_csv) << itto->second.log_decl[3] << ",";
				(*pout_csv) << itto->second.log_instr[3] << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0]) << ",";
				(*pout_csv) << (itto->second.log_decl[1] + itto->second.log_instr[1]) << ",";
				(*pout_csv) << (itto->second.log_decl[2] + itto->second.log_instr[2]) << ",";
				(*pout_csv) << (itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
					itto->second.log_decl[1] + itto->second.log_instr[1] +
					itto->second.log_decl[2] + itto->second.log_instr[2] +
					itto->second.log_direct[3] + itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << "CODE,Logical" << endl;
			}
		}
		else if (itto->first == WEB_ASP)
		{
			if (print_ascii || print_legacy)
			{
				pout = webCounter->GetOutputStream(WEB_ASP, outDir + outputFileNamePrePend, cmdLine);

				(*pout) << endl;
				CUtil::PrintFileHeaderLine(*pout, "RESULTS SUMMARY");
				(*pout) << endl;
				(*pout) << "                                    |       HTML      |     JS-Clnt     |    VBS-Clnt     |      JS-Svr     |     VBS-Svr     |      C#-Svr     |" << endl;
				if (print_legacy)
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                            SLOC                            | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Total      HTML JS-Clnt VBS-Clnt  JS-Svr VBS-Svr  C#-Svr | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+---------+--------------------------------------------------+------------------" << endl;
				}
				else
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                            SLOC                            | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |    HTML JS-Clnt VBS-Clnt  JS-Svr VBS-Svr  C#-Svr     Total | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+--------------------------------------------------+---------+------------------" << endl;
				}

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[3];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[4];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[4];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[5];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[5];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_decl[3] + itto->second.phy_instr[3] +
						itto->second.phy_decl[4] + itto->second.phy_instr[4] +
						itto->second.phy_decl[5] + itto->second.phy_instr[5]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[4] + itto->second.phy_instr[4]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[5] + itto->second.phy_instr[5]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_decl[3] + itto->second.phy_instr[3] +
						itto->second.phy_decl[4] + itto->second.phy_instr[4] +
						itto->second.phy_decl[5] + itto->second.phy_instr[5]);
				}
				(*pout) << " | CODE  Physical"<<endl;

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[3];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[4];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[4];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[5];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[5];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_decl[3] + itto->second.log_instr[3] +
						itto->second.log_decl[4] + itto->second.log_instr[4] +
						itto->second.log_decl[5] + itto->second.log_instr[5]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[1] + itto->second.log_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[2] + itto->second.log_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[3] + itto->second.log_instr[3]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[4] + itto->second.log_instr[4]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[5] + itto->second.log_instr[5]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_decl[3] + itto->second.log_instr[3] +
						itto->second.log_decl[4] + itto->second.log_instr[4] +
						itto->second.log_decl[5] + itto->second.log_instr[5]);
				}
				(*pout) << " | CODE  Logical" << endl;
			}
			if (print_csv)
			{
				pout_csv = webCounter->GetOutputStream(WEB_ASP, outDir + outputFileNamePrePend, cmdLine, true);

				(*pout_csv) << endl;
				CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS SUMMARY");
				(*pout_csv) << endl;
				(*pout_csv) << ",,,,HTML,,JS-Clnt,,VBS-Clnt,,JS-Svr,,VBS-Svr,,C#-Svr" << endl;
				(*pout_csv) << "Total,Blank,Comments,,Word,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,SLOC,,,,,,,File,SLOC" << endl;
				(*pout_csv) << "Lines,Lines,Whole,Embedded,LOC,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,HTML,JS-Clnt,VBS-Clnt,JS-Svr,VBS-Svr,C#-Svr,Total,Type,Definition" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.phy_decl[0] << ",";
				(*pout_csv) << itto->second.phy_instr[0] << ",";
				(*pout_csv) << itto->second.phy_decl[1] << ",";
				(*pout_csv) << itto->second.phy_instr[1] << ",";
				(*pout_csv) << itto->second.phy_decl[2] << ",";
				(*pout_csv) << itto->second.phy_instr[2] << ",";
				(*pout_csv) << itto->second.phy_decl[3] << ",";
				(*pout_csv) << itto->second.phy_instr[3] << ",";
				(*pout_csv) << itto->second.phy_decl[4] << ",";
				(*pout_csv) << itto->second.phy_instr[4] << ",";
				(*pout_csv) << itto->second.phy_decl[5] << ",";
				(*pout_csv) << itto->second.phy_instr[5] << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]) << ",";
				(*pout_csv) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]) << ",";
				(*pout_csv) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]) << ",";
				(*pout_csv) << (itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << (itto->second.phy_decl[4] + itto->second.phy_instr[4]) << ",";
				(*pout_csv) << (itto->second.phy_decl[5] + itto->second.phy_instr[5]) << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
					itto->second.phy_decl[1] + itto->second.phy_instr[1] +
					itto->second.phy_decl[2] + itto->second.phy_instr[2] +
					itto->second.phy_decl[3] + itto->second.phy_instr[3] +
					itto->second.phy_decl[4] + itto->second.phy_instr[4] +
					itto->second.phy_decl[5] + itto->second.phy_instr[5]) << ",";
				(*pout_csv) << "CODE,Physical" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.log_decl[0] << ",";
				(*pout_csv) << itto->second.log_instr[0] << ",";
				(*pout_csv) << itto->second.log_decl[1] << ",";
				(*pout_csv) << itto->second.log_instr[1] << ",";
				(*pout_csv) << itto->second.log_decl[2] << ",";
				(*pout_csv) << itto->second.log_instr[2] << ",";
				(*pout_csv) << itto->second.log_decl[3] << ",";
				(*pout_csv) << itto->second.log_instr[3] << ",";
				(*pout_csv) << itto->second.log_decl[4] << ",";
				(*pout_csv) << itto->second.log_instr[4] << ",";
				(*pout_csv) << itto->second.log_decl[5] << ",";
				(*pout_csv) << itto->second.log_instr[5] << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0]) << ",";
				(*pout_csv) << (itto->second.log_decl[1] + itto->second.log_instr[1]) << ",";
				(*pout_csv) << (itto->second.log_decl[2] + itto->second.log_instr[2]) << ",";
				(*pout_csv) << (itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << (itto->second.log_decl[4] + itto->second.log_instr[4]) << ",";
				(*pout_csv) << (itto->second.log_decl[5] + itto->second.log_instr[5]) << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
					itto->second.log_decl[1] + itto->second.log_instr[1] +
					itto->second.log_decl[2] + itto->second.log_instr[2] +
					itto->second.log_decl[3] + itto->second.log_instr[3] +
					itto->second.log_decl[4] + itto->second.log_instr[4] +
					itto->second.log_decl[5] + itto->second.log_instr[5]) << ",";
				(*pout_csv) << "CODE,Logical" << endl;
			}
		}
		else if (itto->first == WEB_XML)
		{
			if (print_ascii || print_legacy)
			{
				pout = webCounter->GetOutputStream(WEB_XML, outDir + outputFileNamePrePend, cmdLine);

				(*pout) << endl;
				CUtil::PrintFileHeaderLine(*pout, "RESULTS SUMMARY");
				(*pout) << endl;
				(*pout) << "                                    |        XML      |     JS-Clnt     |    VBS-Clnt     |     C#-Clnt     |" << endl;
				if (print_legacy)
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                    SLOC                    | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Total       XML JS-Clnt VBS-Clnt C#-Clnt | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+---------+----------------------------------+------------------" << endl;
				}
				else
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                    SLOC                    | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |     XML JS-Clnt VBS-Clnt C#-Clnt     Total | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+----------------------------------------------------+---------+------------------" << endl;
				}

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[3];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				}
				(*pout) << " | CODE  Physical" << endl;

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[3];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_decl[3] + itto->second.log_instr[3]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[1] + itto->second.log_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[2] + itto->second.log_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[3] + itto->second.log_instr[3]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_decl[3] + itto->second.log_instr[3]);
				}
				(*pout) << " | CODE  Logical" << endl;
			}
			if (print_csv)
			{
				pout_csv = webCounter->GetOutputStream(WEB_XML, outDir + outputFileNamePrePend, cmdLine, true);

				(*pout_csv) << endl;
				CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS SUMMARY");
				(*pout_csv) << endl;
				(*pout_csv) << ",,,,XML,,JS-Clnt,,VBS-Clnt,,C#-Clnt" << endl;
				(*pout_csv) << "Total,Blank,Comments,,Word,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,SLOC,,,,,File,SLOC" << endl;
				(*pout_csv) << "Lines,Lines,Whole,Embedded,LOC,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,XML,JS-Clnt,VBS-Clnt,C#-Clnt,Total,Type,Definition" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.phy_decl[0] << ",";
				(*pout_csv) << itto->second.phy_instr[0] << ",";
				(*pout_csv) << itto->second.phy_decl[1] << ",";
				(*pout_csv) << itto->second.phy_instr[1] << ",";
				(*pout_csv) << itto->second.phy_decl[2] << ",";
				(*pout_csv) << itto->second.phy_instr[2] << ",";
				(*pout_csv) << itto->second.phy_decl[3] << ",";
				(*pout_csv) << itto->second.phy_instr[3] << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]) << ",";
				(*pout_csv) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]) << ",";
				(*pout_csv) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]) << ",";
				(*pout_csv) << (itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
					itto->second.phy_decl[1] + itto->second.phy_instr[1] +
					itto->second.phy_decl[2] + itto->second.phy_instr[2] +
					itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << "CODE,Physical" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.log_decl[0] << ",";
				(*pout_csv) << itto->second.log_instr[0] << ",";
				(*pout_csv) << itto->second.log_decl[1] << ",";
				(*pout_csv) << itto->second.log_instr[1] << ",";
				(*pout_csv) << itto->second.log_decl[2] << ",";
				(*pout_csv) << itto->second.log_instr[2] << ",";
				(*pout_csv) << itto->second.log_decl[3] << ",";
				(*pout_csv) << itto->second.log_instr[3] << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0]) << ",";
				(*pout_csv) << (itto->second.log_decl[1] + itto->second.log_instr[1]) << ",";
				(*pout_csv) << (itto->second.log_decl[2] + itto->second.log_instr[2]) << ",";
				(*pout_csv) << (itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
					itto->second.log_decl[1] + itto->second.log_instr[1] +
					itto->second.log_decl[2] + itto->second.log_instr[2] +
					itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << "CODE,Logical" << endl;
			}
		}
		else if (itto->first == WEB_CFM)
		{
			if (print_ascii || print_legacy)
			{
				pout = webCounter->GetOutputStream(WEB_CFM, outDir + outputFileNamePrePend, cmdLine);

				(*pout) << endl;
				CUtil::PrintFileHeaderLine(*pout, "RESULTS SUMMARY");
				(*pout) << endl;
				(*pout) << "                                    |       HTML      |     JS-Clnt     |    VBS-Clnt     |       SQL       |    ColdFusion   |     CFScript    |" << endl;
				if (print_legacy)
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                              SLOC                              | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Total      HTML JS-Clnt VBS-Clnt     SQL ColdFusion CFScript | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+---------+------------------------------------------------------+------------------" << endl;
				}
				else
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                              SLOC                              | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |    HTML JS-Clnt VBS-Clnt     SQL ColdFusion CFScript     Total | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+------------------------------------------------------+---------+------------------" << endl;
				}

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[3];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[4];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[4];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[5];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[5];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_decl[3] + itto->second.phy_instr[3] +
						itto->second.phy_decl[4] + itto->second.phy_instr[4] +
						itto->second.phy_decl[5] + itto->second.phy_instr[5]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				(*pout) << " ";
				(*pout).width(10);		(*pout) << (itto->second.phy_decl[4] + itto->second.phy_instr[4]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[5] + itto->second.phy_instr[5]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_decl[3] + itto->second.phy_instr[3] +
						itto->second.phy_decl[4] + itto->second.phy_instr[4] +
						itto->second.phy_decl[5] + itto->second.phy_instr[5]);
				}
				(*pout) << " | CODE  Physical"<<endl;

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[3];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[4];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[4];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[5];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[5];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_decl[3] + itto->second.log_instr[3] +
						itto->second.log_decl[4] + itto->second.log_instr[4] +
						itto->second.log_decl[5] + itto->second.log_instr[5]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[1] + itto->second.log_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[2] + itto->second.log_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[3] + itto->second.log_instr[3]);
				(*pout) << " ";
				(*pout).width(10);		(*pout) << (itto->second.log_decl[4] + itto->second.log_instr[4]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[5] + itto->second.log_instr[5]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_decl[3] + itto->second.log_instr[3] +
						itto->second.log_decl[4] + itto->second.log_instr[4] +
						itto->second.log_decl[5] + itto->second.log_instr[5]);
				}
				(*pout) << " | CODE  Logical" << endl;
			}
			if (print_csv)
			{
				pout_csv = webCounter->GetOutputStream(WEB_CFM, outDir + outputFileNamePrePend, cmdLine, true);

				(*pout_csv) << endl;
				CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS SUMMARY");
				(*pout_csv) << endl;
				(*pout_csv) << ",,,,HTML,,JS-Clnt,,VBS-Clnt,,SQL,,ColdFusion,,CFScript" << endl;
				(*pout_csv) << "Total,Blank,Comments,,Word,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,SLOC,,,,,,,File,SLOC" << endl;
				(*pout_csv) << "Lines,Lines,Whole,Embedded,LOC,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,HTML,JS-Clnt,VBS-Clnt,SQL,ColdFusion,CFScript,Total,Type,Definition" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.phy_decl[0] << ",";
				(*pout_csv) << itto->second.phy_instr[0] << ",";
				(*pout_csv) << itto->second.phy_decl[1] << ",";
				(*pout_csv) << itto->second.phy_instr[1] << ",";
				(*pout_csv) << itto->second.phy_decl[2] << ",";
				(*pout_csv) << itto->second.phy_instr[2] << ",";
				(*pout_csv) << itto->second.phy_decl[3] << ",";
				(*pout_csv) << itto->second.phy_instr[3] << ",";
				(*pout_csv) << itto->second.phy_decl[4] << ",";
				(*pout_csv) << itto->second.phy_instr[4] << ",";
				(*pout_csv) << itto->second.phy_decl[5] << ",";
				(*pout_csv) << itto->second.phy_instr[5] << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]) << ",";
				(*pout_csv) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]) << ",";
				(*pout_csv) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]) << ",";
				(*pout_csv) << (itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << (itto->second.phy_decl[4] + itto->second.phy_instr[4]) << ",";
				(*pout_csv) << (itto->second.phy_decl[5] + itto->second.phy_instr[5]) << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
					itto->second.phy_decl[1] + itto->second.phy_instr[1] +
					itto->second.phy_decl[2] + itto->second.phy_instr[2] +
					itto->second.phy_decl[3] + itto->second.phy_instr[3] +
					itto->second.phy_decl[4] + itto->second.phy_instr[4] +
					itto->second.phy_decl[5] + itto->second.phy_instr[5]) << ",";
				(*pout_csv) << "CODE,Physical" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.log_decl[0] << ",";
				(*pout_csv) << itto->second.log_instr[0] << ",";
				(*pout_csv) << itto->second.log_decl[1] << ",";
				(*pout_csv) << itto->second.log_instr[1] << ",";
				(*pout_csv) << itto->second.log_decl[2] << ",";
				(*pout_csv) << itto->second.log_instr[2] << ",";
				(*pout_csv) << itto->second.log_decl[3] << ",";
				(*pout_csv) << itto->second.log_instr[3] << ",";
				(*pout_csv) << itto->second.log_decl[4] << ",";
				(*pout_csv) << itto->second.log_instr[4] << ",";
				(*pout_csv) << itto->second.log_decl[5] << ",";
				(*pout_csv) << itto->second.log_instr[5] << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0]) << ",";
				(*pout_csv) << (itto->second.log_decl[1] + itto->second.log_instr[1]) << ",";
				(*pout_csv) << (itto->second.log_decl[2] + itto->second.log_instr[2]) << ",";
				(*pout_csv) << (itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << (itto->second.log_decl[4] + itto->second.log_instr[4]) << ",";
				(*pout_csv) << (itto->second.log_decl[5] + itto->second.log_instr[5]) << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
					itto->second.log_decl[1] + itto->second.log_instr[1] +
					itto->second.log_decl[2] + itto->second.log_instr[2] +
					itto->second.log_decl[3] + itto->second.log_instr[3] +
					itto->second.log_decl[4] + itto->second.log_instr[4] +
					itto->second.log_decl[5] + itto->second.log_instr[5]) << ",";
				(*pout_csv) << "CODE,Logical" << endl;
			}
		}
		else if (itto->first == WEB_HTM)
		{
			if (print_ascii || print_legacy)
			{
				pout = webCounter->GetOutputStream(WEB_HTM, outDir + outputFileNamePrePend, cmdLine);

				(*pout) << endl;
				CUtil::PrintFileHeaderLine(*pout, "RESULTS SUMMARY");
				(*pout) << endl;
				(*pout) << "                                    |       HTML      |     JS-Clnt     |    VBS-Clnt     |     C#-Clnt     |" << endl;
				if (print_legacy)
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                    SLOC                    | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Total      HTML JS-Clnt VBS-Clnt C#-Clnt | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+---------+----------------------------------+------------------" << endl;
				}
				else
				{
					(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                    SLOC                    | File  SLOC" << endl;
					(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |    HTML JS-Clnt VBS-Clnt C#-Clnt     Total | Type  Definition" << endl;
					(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+----------------------------------------------------+---------+------------------" << endl;
				}

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.phy_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.phy_instr[3];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
						itto->second.phy_decl[1] + itto->second.phy_instr[1] +
						itto->second.phy_decl[2] + itto->second.phy_instr[2] +
						itto->second.phy_decl[3] + itto->second.phy_instr[3]);
				}
				(*pout) << " | CODE  Physical" << endl;

				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[0];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[0];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[1];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[1];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[2];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[2];
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_decl[3];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr[3];
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_decl[3] + itto->second.log_instr[3]);
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[1] + itto->second.log_instr[1]);
				(*pout) << " ";
				(*pout).width(8);		(*pout) << (itto->second.log_decl[2] + itto->second.log_instr[2]);
				(*pout) << " ";
				(*pout).width(7);		(*pout) << (itto->second.log_decl[3] + itto->second.log_instr[3]);
				if (!print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);	(*pout) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
						itto->second.log_decl[1] + itto->second.log_instr[1] +
						itto->second.log_decl[2] + itto->second.log_instr[2] +
						itto->second.log_decl[3] + itto->second.log_instr[3]);
				}
				(*pout) << " | CODE  Logical" << endl;
			}
			if (print_csv)
			{
				pout_csv = webCounter->GetOutputStream(WEB_HTM, outDir + outputFileNamePrePend, cmdLine, true);

				(*pout_csv) << endl;
				CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS SUMMARY");
				(*pout_csv) << endl;
				(*pout_csv) << ",,,,HTML,,JS-Clnt,,VBS-Clnt,,C#-Clnt" << endl;
				(*pout_csv) << "Total,Blank,Comments,,Word,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,SLOC,,,,,File,SLOC" << endl;
				(*pout_csv) << "Lines,Lines,Whole,Embedded,LOC,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,HTML,JS-Clnt,VBS-Clnt,C#-Clnt,Total,Type,Definition" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.phy_decl[0] << ",";
				(*pout_csv) << itto->second.phy_instr[0] << ",";
				(*pout_csv) << itto->second.phy_decl[1] << ",";
				(*pout_csv) << itto->second.phy_instr[1] << ",";
				(*pout_csv) << itto->second.phy_decl[2] << ",";
				(*pout_csv) << itto->second.phy_instr[2] << ",";
				(*pout_csv) << itto->second.phy_decl[3] << ",";
				(*pout_csv) << itto->second.phy_instr[3] << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0]) << ",";
				(*pout_csv) << (itto->second.phy_decl[1] + itto->second.phy_instr[1]) << ",";
				(*pout_csv) << (itto->second.phy_decl[2] + itto->second.phy_instr[2]) << ",";
				(*pout_csv) << (itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << (itto->second.phy_decl[0] + itto->second.phy_instr[0] +
					itto->second.phy_decl[1] + itto->second.phy_instr[1] +
					itto->second.phy_decl[2] + itto->second.phy_instr[2] +
					itto->second.phy_decl[3] + itto->second.phy_instr[3]) << ",";
				(*pout_csv) << "CODE,Physical" << endl;

				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.log_decl[0] << ",";
				(*pout_csv) << itto->second.log_instr[0] << ",";
				(*pout_csv) << itto->second.log_decl[1] << ",";
				(*pout_csv) << itto->second.log_instr[1] << ",";
				(*pout_csv) << itto->second.log_decl[2] << ",";
				(*pout_csv) << itto->second.log_instr[2] << ",";
				(*pout_csv) << itto->second.log_decl[3] << ",";
				(*pout_csv) << itto->second.log_instr[3] << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0]) << ",";
				(*pout_csv) << (itto->second.log_decl[1] + itto->second.log_instr[1]) << ",";
				(*pout_csv) << (itto->second.log_decl[2] + itto->second.log_instr[2]) << ",";
				(*pout_csv) << (itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << (itto->second.log_decl[0] + itto->second.log_instr[0] +
					itto->second.log_decl[1] + itto->second.log_instr[1] +
					itto->second.log_decl[2] + itto->second.log_instr[2] +
					itto->second.log_decl[3] + itto->second.log_instr[3]) << ",";
				(*pout_csv) << "CODE,Logical" << endl;
			}
		}

		// display statistics
		if (print_ascii || print_legacy)
		{
			(*pout) << endl << "Number of files successfully accessed........................ ";
			(*pout).width(5);		(*pout) << itto->second.num_of_file;
			(*pout) << " out of ";

			if (filesToPrint != NULL)
			{
				if (excludeFiles)
				{
					if (useListA)
					{
						if (itto->first == WEB_PHP)
							(*pout) << webCounter->total_php_filesA - webCounter->total_php_dupFilesA << endl;
						else if (itto->first == WEB_ASP)
							(*pout) << webCounter->total_asp_filesA - webCounter->total_asp_dupFilesA << endl;
						else if (itto->first == WEB_JSP)
							(*pout) << webCounter->total_jsp_filesA - webCounter->total_jsp_dupFilesA << endl;
						else if (itto->first == WEB_XML)
							(*pout) << webCounter->total_xml_filesA - webCounter->total_xml_dupFilesA << endl;
						else if (itto->first == WEB_CFM)
							(*pout) << webCounter->total_cfm_filesA - webCounter->total_cfm_dupFilesA << endl;
						else
							(*pout) << webCounter->total_htm_filesA - webCounter->total_htm_dupFilesA << endl;
					}
					else
					{
						if (itto->first == WEB_PHP)
							(*pout) << webCounter->total_php_filesB - webCounter->total_php_dupFilesB << endl;
						else if (itto->first == WEB_ASP)
							(*pout) << webCounter->total_asp_filesB - webCounter->total_asp_dupFilesB << endl;
						else if (itto->first == WEB_JSP)
							(*pout) << webCounter->total_jsp_filesB - webCounter->total_jsp_dupFilesB << endl;
						else if (itto->first == WEB_XML)
							(*pout) << webCounter->total_xml_filesB - webCounter->total_xml_dupFilesB << endl;
						else if (itto->first == WEB_CFM)
							(*pout) << webCounter->total_cfm_filesB - webCounter->total_cfm_dupFilesB << endl;
						else
							(*pout) << webCounter->total_htm_filesB - webCounter->total_htm_dupFilesB << endl;
					}
				}
				else
				{
					if (useListA)
					{
						if (itto->first == WEB_PHP)
							(*pout) << webCounter->total_php_dupFilesA << endl;
						else if (itto->first == WEB_ASP)
							(*pout) << webCounter->total_asp_dupFilesA << endl;
						else if (itto->first == WEB_JSP)
							(*pout) << webCounter->total_jsp_dupFilesA << endl;
						else if (itto->first == WEB_XML)
							(*pout) << webCounter->total_xml_dupFilesA << endl;
						else if (itto->first == WEB_CFM)
							(*pout) << webCounter->total_cfm_dupFilesA << endl;
						else
							(*pout) << webCounter->total_htm_dupFilesA << endl;
					}
					else
					{
						if (itto->first == WEB_PHP)
							(*pout) << webCounter->total_php_dupFilesB << endl;
						else if (itto->first == WEB_ASP)
							(*pout) << webCounter->total_asp_dupFilesB << endl;
						else if (itto->first == WEB_JSP)
							(*pout) << webCounter->total_jsp_dupFilesB << endl;
						else if (itto->first == WEB_XML)
							(*pout) << webCounter->total_xml_dupFilesB << endl;
						else if (itto->first == WEB_CFM)
							(*pout) << webCounter->total_cfm_dupFilesB << endl;
						else
							(*pout) << webCounter->total_htm_dupFilesB << endl;
					}
				}
			}
			else if (useListA)
			{
				if (itto->first == WEB_PHP)
					(*pout) << webCounter->total_php_filesA << endl;
				else if (itto->first == WEB_ASP)
					(*pout) << webCounter->total_asp_filesA << endl;
				else if (itto->first == WEB_JSP)
					(*pout) << webCounter->total_jsp_filesA << endl;
				else if (itto->first == WEB_XML)
					(*pout) << webCounter->total_xml_filesA << endl;
				else if (itto->first == WEB_CFM)
					(*pout) << webCounter->total_cfm_filesA << endl;
				else
					(*pout) << webCounter->total_htm_filesA << endl;
			}
			else
			{
				if (itto->first == WEB_PHP)
					(*pout) << webCounter->total_php_filesB << endl;
				else if (itto->first == WEB_ASP)
					(*pout) << webCounter->total_asp_filesB << endl;
				else if (itto->first == WEB_JSP)
					(*pout) << webCounter->total_jsp_filesB << endl;
				else if (itto->first == WEB_XML)
					(*pout) << webCounter->total_xml_filesB << endl;
				else if (itto->first == WEB_CFM)
					(*pout) << webCounter->total_cfm_filesB << endl;
				else
					(*pout) << webCounter->total_htm_filesB << endl;
			}

			(*pout) << endl << "Ratio of Physical to Logical SLOC............................ ";
		}
		if (print_csv)
		{
			(*pout_csv) << endl << "Number of files successfully accessed,";
			(*pout_csv) << itto->second.num_of_file;
			(*pout_csv) << ",out of,";

			if (filesToPrint != NULL)
			{
				if (excludeFiles)
				{
					if (useListA)
					{
						if (itto->first == WEB_PHP)
							(*pout_csv) << webCounter->total_php_filesA - webCounter->total_php_dupFilesA << endl;
						else if (itto->first == WEB_ASP)
							(*pout_csv) << webCounter->total_asp_filesA - webCounter->total_asp_dupFilesA << endl;
						else if (itto->first == WEB_JSP)
							(*pout_csv) << webCounter->total_jsp_filesA - webCounter->total_jsp_dupFilesA << endl;
						else if (itto->first == WEB_XML)
							(*pout_csv) << webCounter->total_xml_filesA - webCounter->total_xml_dupFilesA << endl;
						else if (itto->first == WEB_CFM)
							(*pout_csv) << webCounter->total_cfm_filesA - webCounter->total_cfm_dupFilesA << endl;
						else
							(*pout_csv) << webCounter->total_htm_filesA - webCounter->total_htm_dupFilesA << endl;
					}
					else
					{
						if (itto->first == WEB_PHP)
							(*pout_csv) << webCounter->total_php_filesB - webCounter->total_php_dupFilesB << endl;
						else if (itto->first == WEB_ASP)
							(*pout_csv) << webCounter->total_asp_filesB - webCounter->total_asp_dupFilesB << endl;
						else if (itto->first == WEB_JSP)
							(*pout_csv) << webCounter->total_jsp_filesB - webCounter->total_jsp_dupFilesB << endl;
						else if (itto->first == WEB_XML)
							(*pout_csv) << webCounter->total_xml_filesB - webCounter->total_xml_dupFilesB << endl;
						else if (itto->first == WEB_CFM)
							(*pout_csv) << webCounter->total_cfm_filesB - webCounter->total_cfm_dupFilesB << endl;
						else
							(*pout_csv) << webCounter->total_htm_filesB - webCounter->total_htm_dupFilesB << endl;
					}
				}
				else
				{
					if (useListA)
					{
						if (itto->first == WEB_PHP)
							(*pout_csv) << webCounter->total_php_dupFilesA << endl;
						else if (itto->first == WEB_ASP)
							(*pout_csv) << webCounter->total_asp_dupFilesA << endl;
						else if (itto->first == WEB_JSP)
							(*pout_csv) << webCounter->total_jsp_dupFilesA << endl;
						else if (itto->first == WEB_XML)
							(*pout_csv) << webCounter->total_xml_dupFilesA << endl;
						else if (itto->first == WEB_CFM)
							(*pout_csv) << webCounter->total_cfm_dupFilesA << endl;
						else
							(*pout_csv) << webCounter->total_htm_dupFilesA << endl;
					}
					else
					{
						if (itto->first == WEB_PHP)
							(*pout_csv) << webCounter->total_php_dupFilesB << endl;
						else if (itto->first == WEB_ASP)
							(*pout_csv) << webCounter->total_asp_dupFilesB << endl;
						else if (itto->first == WEB_JSP)
							(*pout_csv) << webCounter->total_jsp_dupFilesB << endl;
						else if (itto->first == WEB_XML)
							(*pout_csv) << webCounter->total_xml_dupFilesB << endl;
						else if (itto->first == WEB_CFM)
							(*pout_csv) << webCounter->total_cfm_dupFilesB << endl;
						else
							(*pout_csv) << webCounter->total_htm_dupFilesB << endl;
					}
				}
			}
			else if (useListA)
			{
				if (itto->first == WEB_PHP)
					(*pout_csv) << webCounter->total_php_filesA << endl;
				else if (itto->first == WEB_ASP)
					(*pout_csv) << webCounter->total_asp_filesA << endl;
				else if (itto->first == WEB_JSP)
					(*pout_csv) << webCounter->total_jsp_filesA << endl;
				else if (itto->first == WEB_XML)
					(*pout_csv) << webCounter->total_xml_filesA << endl;
				else if (itto->first == WEB_CFM)
					(*pout_csv) << webCounter->total_cfm_filesA << endl;
				else
					(*pout_csv) << webCounter->total_htm_filesA << endl;
			}
			else
			{
				if (itto->first == WEB_PHP)
					(*pout_csv) << webCounter->total_php_filesB << endl;
				else if (itto->first == WEB_ASP)
					(*pout_csv) << webCounter->total_asp_filesB << endl;
				else if (itto->first == WEB_JSP)
					(*pout_csv) << webCounter->total_jsp_filesB << endl;
				else if (itto->first == WEB_XML)
					(*pout_csv) << webCounter->total_xml_filesB << endl;
				else if (itto->first == WEB_CFM)
					(*pout_csv) << webCounter->total_cfm_filesB << endl;
				else
					(*pout_csv) << webCounter->total_htm_filesB << endl;
			}

			(*pout_csv) << endl << "Ratio of Physical to Logical SLOC,";
		}

		unsigned int tlsloc = itto->second.log_decl[0] + itto->second.log_instr[0] +
			itto->second.log_decl[1] + itto->second.log_instr[1] +
			itto->second.log_decl[2] + itto->second.log_instr[2] +
			itto->second.log_decl[3] + itto->second.log_instr[3] +
			itto->second.log_decl[4] + itto->second.log_instr[4] +
			itto->second.log_decl[5] + itto->second.log_instr[5];
		if (tlsloc > 0)
		{
			unsigned int tpsloc = itto->second.phy_decl[0] + itto->second.phy_instr[0] +
				itto->second.phy_decl[1] + itto->second.phy_instr[1] +
				itto->second.phy_decl[2] + itto->second.phy_instr[2] +
				itto->second.phy_decl[3] + itto->second.phy_instr[3] +
				itto->second.phy_decl[4] + itto->second.phy_instr[4] +
				itto->second.phy_decl[5] + itto->second.phy_instr[5];
			float tslocrat = (float)tpsloc / (float)tlsloc;

			if (print_ascii || print_legacy)
			{
				(*pout).setf(ios::fixed,ios::floatfield);
				(*pout).width(8);
				(*pout).precision(2);
				(*pout) << tslocrat << endl;
				(*pout).unsetf(ios::floatfield);
			}
			if (print_csv)
			{
				(*pout_csv).setf(ios::fixed,ios::floatfield);
				(*pout_csv).width(8);
				(*pout_csv).precision(2);
				(*pout_csv) << tslocrat << endl;
				(*pout_csv).unsetf(ios::floatfield);
			}
		}
		else
		{
			if (print_ascii || print_legacy)
				(*pout) << "    NA" << endl;
			if (print_csv)
				(*pout_csv) << "NA" << endl;
		}
	}

	// display summary for other languages, other than WEB
	for (TotalValueMap::iterator itto = total.begin(); itto != total.end(); itto++)
	{
		if (CounterForEachLanguage[itto->first]->classtype == UNKNOWN || CounterForEachLanguage[itto->first]->classtype == DATA)
			file_type = "DATA";
		else
			file_type = "CODE";

		if (print_ascii || print_legacy)
		{
			pout = CounterForEachLanguage[itto->first]->GetOutputStream(outDir + outputFileNamePrePend, cmdLine);

			(*pout) << endl;
			CUtil::PrintFileHeaderLine(*pout, "RESULTS SUMMARY");
			(*pout) << endl;
			if (print_legacy)
			{
				(*pout) << "   Total   Blank |      Comments    | Compiler  Data   Exec.  |  Number  |         | File  SLOC" << endl;
				(*pout) << "   Lines   Lines |   Whole Embedded | Direct.   Decl.  Instr. | of Files |   SLOC  | Type  Definition" << endl;
				(*pout) << "-----------------+------------------+-------------------------+----------+---------+------------------" << endl;
			}
			else
			{
				(*pout) << "   Total   Blank |      Comments    | Compiler  Data   Exec.  |         | File  SLOC" << endl;
				(*pout) << "   Lines   Lines |   Whole Embedded | Direct.   Decl.  Instr. |   SLOC  | Type  Definition" << endl;
				(*pout) << "-----------------+------------------+-------------------------+---------+------------------" << endl;
			}

			(*pout).setf(ios::right);
			(*pout).width(8);		(*pout) << itto->second.total_line;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << itto->second.blank_line;
			(*pout) << " |";
			(*pout).width(8);		(*pout) << itto->second.whole_comment;
			(*pout) << " ";
			(*pout).width(8);		(*pout) << itto->second.embed_comment;
			(*pout) << " |";
			(*pout).width(8);		(*pout) << itto->second.phy_direct;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << itto->second.phy_decl;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << itto->second.phy_instr;
			if (print_legacy)
			{
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.num_of_file;
			}
			(*pout) << " |";
			(*pout).width(8);		(*pout) << itto->second.phy_direct + itto->second.phy_decl + itto->second.phy_instr;
			(*pout) << " | " << file_type << "  Physical" << endl;

			if (file_type != "DATA")
			{
				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << itto->second.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << itto->second.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_direct;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_decl;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << itto->second.log_instr;
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);		(*pout) << itto->second.num_of_file;
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << itto->second.log_direct + itto->second.log_decl + itto->second.log_instr;
				(*pout) << " | CODE  Logical" << endl;
			}

			// display statistics
			(*pout) << endl << "Number of files successfully accessed........................ ";
			(*pout).width(5);		(*pout) << itto->second.num_of_file;
			(*pout) << " out of ";
			if (filesToPrint != NULL)
			{
				if (excludeFiles)
				{
					if (useListA)
						(*pout) << CounterForEachLanguage[itto->first]->total_filesA - CounterForEachLanguage[itto->first]->total_dupFilesA << endl;
					else
						(*pout) << CounterForEachLanguage[itto->first]->total_filesB - CounterForEachLanguage[itto->first]->total_dupFilesB << endl;
				}
				else
				{
					if (useListA)
						(*pout) << CounterForEachLanguage[itto->first]->total_dupFilesA << endl;
					else
						(*pout) << CounterForEachLanguage[itto->first]->total_dupFilesB << endl;
				}
			}
			else if (useListA)
				(*pout) << CounterForEachLanguage[itto->first]->total_filesA << endl;
			else
				(*pout) << CounterForEachLanguage[itto->first]->total_filesB << endl;

			(*pout) << endl << "Ratio of Physical to Logical SLOC............................ ";
		}
		if (print_csv)
		{
			pout_csv = CounterForEachLanguage[itto->first]->GetOutputStream(outDir + outputFileNamePrePend, cmdLine, true);

			(*pout_csv) << endl;
			CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS SUMMARY");
			(*pout_csv) << endl;
			(*pout_csv) << "Total,Blank,Comments,,Compiler,Data,Exec.,,File,SLOC" << endl;
			(*pout_csv) << "Lines,Lines,Whole,Embedded,Direct.,Decl.,Instr.,SLOC,Type,Definition" << endl;

			(*pout_csv) << itto->second.total_line << ",";
			(*pout_csv) << itto->second.blank_line << ",";
			(*pout_csv) << itto->second.whole_comment << ",";
			(*pout_csv) << itto->second.embed_comment << ",";
			(*pout_csv) << itto->second.phy_direct << ",";
			(*pout_csv) << itto->second.phy_decl << ",";
			(*pout_csv) << itto->second.phy_instr << ",";
			(*pout_csv) << itto->second.phy_direct + itto->second.phy_decl + itto->second.phy_instr << ",";
			(*pout_csv) << file_type << ",Physical" << endl;

			if (file_type != "DATA")
			{
				(*pout_csv) << itto->second.total_line << ",";
				(*pout_csv) << itto->second.blank_line << ",";
				(*pout_csv) << itto->second.whole_comment << ",";
				(*pout_csv) << itto->second.embed_comment << ",";
				(*pout_csv) << itto->second.log_direct << ",";
				(*pout_csv) << itto->second.log_decl << ",";
				(*pout_csv) << itto->second.log_instr << ",";
				(*pout_csv) << itto->second.log_direct + itto->second.log_decl + itto->second.log_instr << ",";
				(*pout_csv) << "CODE,Logical" << endl;
			}

			// display statistics
			(*pout_csv) << endl << "Number of files successfully accessed,";
			(*pout_csv) << itto->second.num_of_file;
			(*pout_csv) << ",out of,";

			if (filesToPrint != NULL)
			{
				if (excludeFiles)
				{
					if (useListA)
						(*pout_csv) << CounterForEachLanguage[itto->first]->total_filesA - CounterForEachLanguage[itto->first]->total_dupFilesA << endl;
					else
						(*pout_csv) << CounterForEachLanguage[itto->first]->total_filesB - CounterForEachLanguage[itto->first]->total_dupFilesB << endl;
				}
				else
				{
					if (useListA)
						(*pout_csv) << CounterForEachLanguage[itto->first]->total_dupFilesA << endl;
					else
						(*pout_csv) << CounterForEachLanguage[itto->first]->total_dupFilesB << endl;
				}
			}
			else if (useListA)
				(*pout_csv) << CounterForEachLanguage[itto->first]->total_filesA << endl;
			else
				(*pout_csv) << CounterForEachLanguage[itto->first]->total_filesB << endl;

			(*pout_csv) << endl << "Ratio of Physical to Logical SLOC,";
		}
		
		unsigned int tlsloc = itto->second.log_direct + itto->second.log_decl + itto->second.log_instr;
		if (tlsloc > 0)
		{
			unsigned int tpsloc = itto->second.phy_direct + itto->second.phy_decl + itto->second.phy_instr;
			float tslocrat = (float)tpsloc / (float)tlsloc;

			if (print_ascii || print_legacy)
			{
				(*pout).setf(ios::fixed,ios::floatfield);
				(*pout).width(8);
				(*pout).precision(2);
				(*pout) << tslocrat << endl;
				(*pout).unsetf(ios::floatfield);
			}
			if (print_csv)
			{
				(*pout_csv).setf(ios::fixed,ios::floatfield);
				(*pout_csv).width(8);
				(*pout_csv).precision(2);
				(*pout_csv) << tslocrat << endl;
				(*pout_csv).unsetf(ios::floatfield);
			}
		}
		else
		{
			if (print_ascii || print_legacy)
				(*pout) << "    NA" << endl;
			if (print_csv)
				(*pout_csv) << "NA" << endl;
		}
	}

	if (print_cmplx)
	{
		string dots = "........................";
		UIntPairVector::iterator idirc;
		UIntPairVector::iterator idatc;
		UIntPairVector::iterator iexec;
		StringVector::iterator idir;
		StringVector::iterator idat;
		StringVector::iterator iexe;

		// display keywords counts
		ClassType ty;
		for (map<int, CCodeCounter*>::iterator iter = ++CounterForEachLanguage.begin(); iter != CounterForEachLanguage.end(); iter++)
		{
			if (iter->second->directive.size() == 0 && iter->second->data_name_list.size() == 0 && iter->second->exec_name_list.size()  == 0)
				continue;
			else if (total.count(iter->second->classtype) == 0)
			{
				if (iter->second->print_cmplx && (excludeFiles ||
					(useListA && iter->second->total_dupFilesA > 0) || (!useListA && iter->second->total_dupFilesB > 0)))
				{
					ty = iter->second->classtype;
					if (ty == PHP || ty == HTML_PHP || ty == JAVASCRIPT_PHP || ty == VBS_PHP)
					{
						if (print_ascii || print_legacy)
							pout = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_PHP, outDir + outputFileNamePrePend, cmdLine);
						if (print_csv)
							pout_csv = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_PHP, outDir + outputFileNamePrePend, cmdLine, true);
					}
					else if (ty == JAVA_JSP || ty == HTML_JSP || ty == JAVASCRIPT_JSP || ty == VBS_JSP)
					{
						if (print_ascii || print_legacy)
							pout = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_JSP, outDir + outputFileNamePrePend, cmdLine);
						if (print_csv)
							pout_csv = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_JSP, outDir + outputFileNamePrePend, cmdLine, true);
					}
					else if (ty == HTML_ASP || ty == JAVASCRIPT_ASP_C || ty == VBS_ASP_C || ty == JAVASCRIPT_ASP_S || ty == VBS_ASP_S || ty == CSHARP_ASP_S)
					{
						if (print_ascii || print_legacy)
							pout = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_ASP, outDir + outputFileNamePrePend, cmdLine);
						if (print_csv)
							pout_csv = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_ASP, outDir + outputFileNamePrePend, cmdLine, true);
					}
					else if (ty == HTML || ty == JAVASCRIPT_HTML || ty == VBS_HTML || ty == CSHARP_HTML)
					{
						if (print_ascii || print_legacy)
							pout = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_HTM, outDir + outputFileNamePrePend, cmdLine);
						if (print_csv)
							pout_csv = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_HTM, outDir + outputFileNamePrePend, cmdLine, true);
					}
					else if (ty == XML || ty == JAVASCRIPT_XML || ty == VBS_XML || ty == CSHARP_XML)
					{
						if (print_ascii || print_legacy)
							pout = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_XML, outDir + outputFileNamePrePend, cmdLine);
						if (print_csv)
							pout_csv = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_XML, outDir + outputFileNamePrePend, cmdLine, true);
					}
					else if (ty == COLDFUSION || ty == CFSCRIPT || ty == HTML_CFM || ty == JAVASCRIPT_CFM || ty == VBS_CFM || ty == SQL_CFM)
					{
						if (print_ascii || print_legacy)
							pout = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_CFM, outDir + outputFileNamePrePend, cmdLine);
						if (print_csv)
							pout_csv = ((CWebCounter*)CounterForEachLanguage[WEB])->GetOutputStream(WEB_CFM, outDir + outputFileNamePrePend, cmdLine, true);
					}
					else
						continue;
				}
				else
					continue;
			}
			else
			{
				if (print_ascii || print_legacy)
					pout = CounterForEachLanguage[iter->second->classtype]->GetOutputStream(outDir + outputFileNamePrePend, cmdLine);
				if (print_csv)
					pout_csv = CounterForEachLanguage[iter->second->classtype]->GetOutputStream(outDir + outputFileNamePrePend, cmdLine, true);
			}

			if (print_ascii || print_legacy)
			{
				(*pout) << endl;
				CUtil::PrintFileHeaderLine(*pout, "TOTAL OCCURRENCES OF " + iter->second->language_name + " KEYWORDS");
				(*pout) << "--------------------------------------------------------------------------------------------------------------" << endl;
				(*pout) << "    Compiler Directives                  Data Keywords                        Executable Keywords" << endl;
				(*pout).setf(ios::right);
			}
			if (print_csv)
			{
				(*pout_csv) << endl;
				CUtil::PrintFileHeaderLine(*pout_csv, "TOTAL OCCURRENCES OF " + iter->second->language_name + " KEYWORDS");
				(*pout_csv) << "Compiler Directives,,Data Keywords,,Executable Keywords" << endl;
			}

			idirc = iter->second->directive_count.begin();
			idatc = iter->second->data_name_count.begin();
			iexec = iter->second->exec_name_count.begin();
			idir = iter->second->directive.begin();
			idat = iter->second->data_name_list.begin();
			iexe = iter->second->exec_name_list.begin();

			while ( (idir != iter->second->directive.end()) || (idat != iter->second->data_name_list.end()) ||
				(iexe != iter->second->exec_name_list.end()) )
			{
				if (idir != iter->second->directive.end())
				{
					if (print_ascii || print_legacy)
					{
						(*pout) << ' ' << *idir << dots.substr(idir->length());
						(*pout).width(7);
						if (filesToPrint != NULL && !excludeFiles)
							(*pout) << (*idirc).second;
						else
							(*pout) << (*idirc).first;
					}
					if (print_csv)
					{
						(*pout_csv) << *idir << ",";
						if (filesToPrint != NULL && !excludeFiles)
							(*pout_csv) << (*idirc).second << ",";
						else
							(*pout_csv) << (*idirc).first << ",";
					}
					idir++;
					idirc++;
				}
				else
				{
					if (print_ascii || print_legacy)
						(*pout) << "                                ";
					if (print_csv)
						(*pout_csv) << ",,";
				}
				if (print_ascii || print_legacy)
					(*pout) << "     ";

				if (idat != iter->second->data_name_list.end())
				{
					if (print_ascii || print_legacy)
					{
						(*pout) << ' ' << *idat << dots.substr(idat->length());
						(*pout).width(7);
						if (filesToPrint != NULL && !excludeFiles)
							(*pout) << (*idatc).second;
						else
							(*pout) << (*idatc).first;
					}
					if (print_csv)
					{
						(*pout_csv) << *idat << ",";
						if (filesToPrint != NULL && !excludeFiles)
							(*pout_csv) << (*idatc).second << ",";
						else
							(*pout_csv) << (*idatc).first << ",";
					}
					idat++;
					idatc++;
				}
				else
				{
					if (print_ascii || print_legacy)
						(*pout) << "                                ";
					if (print_csv)
						(*pout_csv) << ",,";
				}
				if (print_ascii || print_legacy)
					(*pout) << "     ";

				if (iexe != iter->second->exec_name_list.end())
				{
					if (print_ascii || print_legacy)
					{
						(*pout) << ' ' << *iexe << dots.substr(iexe->length());
						(*pout).width(7);
						if (filesToPrint != NULL && !excludeFiles)
							(*pout) << (*iexec).second;
						else
							(*pout) << (*iexec).first;
					}
					if (print_csv)
					{
						(*pout_csv) << *iexe << ",";
						if (filesToPrint != NULL && !excludeFiles)
							(*pout_csv) << (*iexec).second;
						else
							(*pout_csv) << (*iexec).first;
					}
					iexe++;
					iexec++;
				}
				else
				{
					if (print_ascii || print_legacy)
						(*pout) << "                                ";
				}
				if (print_ascii || print_legacy)
					(*pout) << endl;
				if (print_csv)
					(*pout_csv) << endl;
			}
		}
	}

	// close all files
	for (map<int,CCodeCounter*>::iterator iter=CounterForEachLanguage.begin(); iter!=CounterForEachLanguage.end(); iter++)
		iter->second->CloseOutputStream();

	// print out language count summary
	if (!PrintCountSummary( CounterForEachLanguage, total, webtotal, outputFileNamePrePend))
		return 0;

	return 1;
}

/*!
* Prints the total counting results.
*
* \param useListA use file list A? (otherwise use list B)
* \param outputFileNamePrePend name to prepend to the output file
* \param filesToPrint list of files to include or exclude
* \param excludeFiles exclude files? (if true excludes files in filesToPrint; if false includes only files in filesToPrint)
*
* \return method status
*/
int PrintTotalCountResults( CounterForEachLangType & CounterForEachLanguage,
							bool useListA, const string &outputFileNamePrePend, 
							StringVector *filesToPrint, bool excludeFiles)
{
	ofstream* pout     = NULL;
	ofstream* pout_csv = NULL;
	TotalValueMap total;
	WebTotalValueMap webtotal;
	TotalValue allLangCode, allLangData;
	AllWebTotalValue allWebLang;
	unsigned int totalNonDupFileA = 0, totalFileA = 0;
	unsigned int totalNonDupFileB = 0, totalFileB  = 0;
	bool printHeader = false;
	unsigned int tlsloc = 0, tpsloc = 0;
	float tslocrat;
	StringVector::iterator sit;
	ClassType class_type;
	string file_type;
	CWebCounter *webCounter;
	WebType webType;

	// skip if all files are excluded
	if (filesToPrint != NULL && filesToPrint->size() < 1 && !excludeFiles)
		return 0;

	// display each non-web count
	SourceFileList::iterator its;
	SourceFileList* mySourceFile = (useListA) ? &SourceFileA : &SourceFileB;
	for (its = mySourceFile->begin(); its != mySourceFile->end(); its++)
	{
		if (filesToPrint != NULL && filesToPrint->size() > 0)
		{
			// restrict based on those files in the filesToPrint list
			sit = filesToPrint->begin();
			while (sit != filesToPrint->end() && its->second.file_name.compare((*sit)) != 0)
				sit++;

			if (excludeFiles)
			{
				// skip the file if in the filesToPrint list
				if (sit != filesToPrint->end())
					continue;
			}
			else
			{
				// skip the file if NOT in the filesToPrint list
				if (sit == filesToPrint->end())
					continue;
			}
		}

		// languages other than WEB
		if (its->second.class_type != WEB)
		{
			class_type = its->second.class_type;
			file_type = (class_type != UNKNOWN && class_type != DATAFILE ? "CODE" : "DATA");

			// do not print temp files that are created to represent embedded code
			if ( its->second.file_name_isEmbedded == true ) 
				continue;

			if (total.count(class_type) == 0)
				total.insert(TotalValueMap::value_type(class_type, TotalValue()));

			// get the stream for non-web language, skip if already printed
			if (!printHeader)
			{
				if (print_ascii || print_legacy)
				{
					pout = GetTotalOutputStream(outDir + outputFileNamePrePend);
					if (pout == NULL)
					{
						string err = "Error: Unable to create file (";
						err += outputFileNamePrePend + "TOTAL" + OUTPUT_FILE_NAME;
						err += "). Operation aborted.";
						userIF->AddError(err, false, 1);
						return 0;
					}
					// if there exists non-web language, print the header
					CUtil::PrintFileHeaderLine(*pout, "RESULTS FOR ALL NON-WEB LANGUAGE FILES");
					(*pout) << endl;
					if (print_legacy)
					{
						(*pout) << "   Total   Blank |      Comments    | Compiler  Data   Exec.  | Logical | File  Module" << endl;
						(*pout) << "   Lines   Lines |   Whole Embedded | Direct.   Decl.  Instr. |   SLOC  | Type  Name" << endl;
						(*pout) << "-----------------+------------------+-------------------------+---------+---------------------------" << endl;
					}
					else
					{
						(*pout) << "   Total   Blank |      Comments    | Compiler  Data   Exec.  | Logical Physical | File  Module" << endl;
						(*pout) << "   Lines   Lines |   Whole Embedded | Direct.   Decl.  Instr. |   SLOC    SLOC   | Type  Name" << endl;
						(*pout) << "-----------------+------------------+-------------------------+------------------+---------------------------" << endl;
					}
				}
				if (print_csv)
				{
					pout_csv = GetTotalOutputStream(outDir + outputFileNamePrePend, true);
					if (pout_csv == NULL)
					{
						string err = "Error: Unable to create file (";
						err += outputFileNamePrePend + "TOTAL" + OUTPUT_FILE_NAME_CSV;
						err += "). Operation aborted.";
						userIF->AddError(err, false, 1);
						return 0;
					}
					// if there exists non-web language, print the header
					(*pout_csv) << endl << "          RESULTS FOR ALL NON-WEB LANGUAGE FILES" << endl;
					(*pout_csv) << "Total,Blank,Comments,,Compiler,Data,Exec.,Logical,Physical,File,Module" << endl;
					(*pout_csv) << "Lines,Lines,Whole,Embedded,Direct.,Decl.,Instr.,SLOC,SLOC,Type,Name" << endl;
				}
				printHeader = true;
			}

			if (its->second.e_flag)
			{
				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::left);
					(*pout) << " Error: ";
					(*pout).width(65);
					(*pout) << its->second.error_code;
					(*pout) << its->second.file_name << endl << endl;
					(*pout).unsetf(ios::left);
				}
				if (print_csv)
				{
					(*pout_csv) << "Error: ";
					(*pout_csv) << its->second.error_code << ",,,,,,,,,";
					(*pout_csv) << its->second.file_name << endl << endl;
				}
				continue;
			}
			else
			{
				its->second.SLOC_lines[PHY] = its->second.directive_lines[PHY] + its->second.exec_lines[PHY] + its->second.data_lines[PHY];
				its->second.SLOC_lines[LOG] = its->second.directive_lines[LOG] + its->second.exec_lines[LOG] + its->second.data_lines[LOG];
				its->second.total_lines = its->second.SLOC_lines[PHY] +	its->second.blank_lines + its->second.comment_lines;
			}

			total[class_type].num_of_file++;

			if (print_ascii || print_legacy)
			{
				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << its->second.total_lines;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << its->second.blank_lines;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << its->second.comment_lines;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << its->second.e_comm_lines;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << its->second.directive_lines[LOG];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << its->second.data_lines[LOG];
				(*pout) << " ";
				(*pout).width(7);		(*pout) << its->second.exec_lines[LOG];
				(*pout) << " |";
				if (print_legacy)
				{
					(*pout).width(8);	(*pout) << its->second.SLOC_lines[LOG];
				}
				else
				{
					(*pout).width(8);	(*pout) << its->second.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);	(*pout) << its->second.SLOC_lines[PHY];
				}
				(*pout) << " | " << file_type;
				(*pout) << " " << its->second.file_name;
				(*pout) << endl;
				(*pout).unsetf(ios::right);
			}
			if (print_csv)
			{
				(*pout_csv) << its->second.total_lines << ",";
				(*pout_csv) << its->second.blank_lines << ",";
				(*pout_csv) << its->second.comment_lines << ",";
				(*pout_csv) << its->second.e_comm_lines << ",";
				(*pout_csv) << its->second.directive_lines[LOG] << ",";
				(*pout_csv) << its->second.data_lines[LOG] << ",";
				(*pout_csv) << its->second.exec_lines[LOG] << ",";
				(*pout_csv) << its->second.SLOC_lines[LOG] << ",";
				(*pout_csv) << its->second.SLOC_lines[PHY] << ",";
				(*pout_csv) << file_type << ",";
				(*pout_csv) << its->second.file_name << endl;
			}

			// total count for physical lines of this class type
			total[class_type].total_line += its->second.total_lines;
			total[class_type].blank_line += its->second.blank_lines;
			total[class_type].whole_comment += its->second.comment_lines;
			total[class_type].embed_comment += its->second.e_comm_lines;
			total[class_type].phy_direct += its->second.directive_lines[PHY];
			total[class_type].phy_decl += its->second.data_lines[PHY];
			total[class_type].phy_instr += its->second.exec_lines[PHY];
			total[class_type].log_direct += its->second.directive_lines[LOG];
			total[class_type].log_decl += its->second.data_lines[LOG];
			total[class_type].log_instr += its->second.exec_lines[LOG];

			// total count for physical lines of ALL languages
			if (file_type == "DATA")
			{
				allLangData.total_line += its->second.total_lines;
				allLangData.blank_line += its->second.blank_lines;
				allLangData.whole_comment += its->second.comment_lines;
				allLangData.embed_comment += its->second.e_comm_lines;
				allLangData.phy_direct += its->second.directive_lines[PHY];
				allLangData.phy_decl += its->second.data_lines[PHY];
				allLangData.phy_instr += its->second.exec_lines[PHY];
				allLangData.log_direct += its->second.directive_lines[LOG];
				allLangData.log_decl += its->second.data_lines[LOG];
				allLangData.log_instr += its->second.exec_lines[LOG];
				allLangData.num_of_file++;
			}
			else
			{
				allLangCode.total_line += its->second.total_lines;
				allLangCode.blank_line += its->second.blank_lines;
				allLangCode.whole_comment += its->second.comment_lines;
				allLangCode.embed_comment += its->second.e_comm_lines;
				allLangCode.phy_direct += its->second.directive_lines[PHY];
				allLangCode.phy_decl += its->second.data_lines[PHY];
				allLangCode.phy_instr += its->second.exec_lines[PHY];
				allLangCode.log_direct += its->second.directive_lines[LOG];
				allLangCode.log_decl += its->second.data_lines[LOG];
				allLangCode.log_instr += its->second.exec_lines[LOG];
				allLangCode.num_of_file++;
			}
		}
	}

	// display summary for other languages, other than WEB
	for (TotalValueMap::iterator itto = total.begin(); itto != total.end(); itto++)
	{
		if (filesToPrint != NULL)
		{
			if (excludeFiles)
			{
				if (useListA)
					totalNonDupFileA += CounterForEachLanguage[itto->first]->total_filesA - CounterForEachLanguage[itto->first]->total_dupFilesA;
				else
					totalNonDupFileB += CounterForEachLanguage[itto->first]->total_filesB - CounterForEachLanguage[itto->first]->total_dupFilesB;
			}
			else
			{
				if (useListA)
					totalNonDupFileA += CounterForEachLanguage[itto->first]->total_dupFilesA;
				else
					totalNonDupFileB += CounterForEachLanguage[itto->first]->total_dupFilesB;
			}
		}
		else if (useListA)
			totalFileA += CounterForEachLanguage[itto->first]->total_filesA;
		else
			totalFileB += CounterForEachLanguage[itto->first]->total_filesB;

		tlsloc += itto->second.log_direct + itto->second.log_decl + itto->second.log_instr;
		tpsloc += itto->second.phy_direct + itto->second.phy_decl + itto->second.phy_instr;
	}

	// print other languages count summary
	if (total.size() > 0)
	{
		if (print_ascii || print_legacy)
		{
			(*pout) << endl;
			CUtil::PrintFileHeaderLine(*pout, "RESULTS SUMMARY FOR NON-WEB LANGUAGES");
			(*pout) << endl;
			if (print_legacy)
			{
				(*pout) << "   Total   Blank |      Comments    | Compiler  Data   Exec.  |  Number  |         | File  SLOC" << endl;
				(*pout) << "   Lines   Lines |   Whole Embedded | Direct.   Decl.  Instr. | of Files |   SLOC  | Type  Definition" << endl;
				(*pout) << "-----------------+------------------+-------------------------+----------+---------+------------------" << endl;
			}
			else
			{
				(*pout) << "   Total   Blank |      Comments    | Compiler  Data   Exec.  |         | File  SLOC" << endl;
				(*pout) << "   Lines   Lines |   Whole Embedded | Direct.   Decl.  Instr. |   SLOC  | Type  Definition" << endl;
				(*pout) << "-----------------+------------------+-------------------------+---------+------------------" << endl;
			}

			(*pout).setf(ios::right);
			(*pout).width(8);		(*pout) << allLangCode.total_line;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allLangCode.blank_line;
			(*pout) << " |";
			(*pout).width(8);		(*pout) << allLangCode.whole_comment;
			(*pout) << " ";
			(*pout).width(8);		(*pout) << allLangCode.embed_comment;
			(*pout) << " |";
			(*pout).width(8);		(*pout) << allLangCode.phy_direct;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allLangCode.phy_decl;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allLangCode.phy_instr;
			if (print_legacy)
			{
				(*pout) << " |";
				(*pout).width(8);		(*pout) << allLangCode.num_of_file;
			}
			(*pout) << " |";
			(*pout).width(8);		(*pout) << allLangCode.phy_direct + allLangCode.phy_decl + allLangCode.phy_instr;
			(*pout) << " | CODE  Physical" << endl;

			(*pout).setf(ios::right);
			(*pout).width(8);		(*pout) << allLangCode.total_line;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allLangCode.blank_line;
			(*pout) << " |";
			(*pout).width(8);		(*pout) << allLangCode.whole_comment;
			(*pout) << " ";
			(*pout).width(8);		(*pout) << allLangCode.embed_comment;
			(*pout) << " |";

			(*pout).width(8);		(*pout) << allLangCode.log_direct;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allLangCode.log_decl;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allLangCode.log_instr;
			if (print_legacy)
			{
				(*pout) << " |";
				(*pout).width(8);		(*pout) << allLangCode.num_of_file;
			}
			(*pout) << " |";
			(*pout).width(8);		(*pout) << allLangCode.log_direct + allLangCode.log_decl + allLangCode.log_instr;
			(*pout) << " | CODE  Logical" << endl;

			if (allLangData.num_of_file > 0)
			{
				(*pout).setf(ios::right);
				(*pout).width(8);		(*pout) << allLangData.total_line;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << allLangData.blank_line;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << allLangData.whole_comment;
				(*pout) << " ";
				(*pout).width(8);		(*pout) << allLangData.embed_comment;
				(*pout) << " |";
				(*pout).width(8);		(*pout) << allLangData.phy_direct;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << allLangData.phy_decl;
				(*pout) << " ";
				(*pout).width(7);		(*pout) << allLangData.phy_instr;
				if (print_legacy)
				{
					(*pout) << " |";
					(*pout).width(8);		(*pout) << allLangData.num_of_file;
				}
				(*pout) << " |";
				(*pout).width(8);		(*pout) << allLangData.phy_direct + allLangData.phy_decl + allLangData.phy_instr;
				(*pout) << " | DATA  Physical" << endl;
			}

			// display statistics
			(*pout) << endl << "Number of files successfully accessed........................ ";
			(*pout).width(5);		(*pout) << allLangCode.num_of_file + allLangData.num_of_file;
			(*pout) << " out of ";
			if (filesToPrint != NULL)
			{
				if (useListA)
					(*pout) << totalNonDupFileA << endl;
				else
					(*pout) << totalNonDupFileB << endl;
			}
			else if (useListA)
				(*pout) << totalFileA << endl;
			else
				(*pout) << totalFileB << endl;

			(*pout) << endl << "Ratio of Physical to Logical SLOC............................ ";

			if (tlsloc > 0)
			{
				tslocrat = (float)tpsloc / (float)tlsloc;
				(*pout).setf(ios::fixed,ios::floatfield);
				(*pout).width(8);
				(*pout).precision(2);
				(*pout) << tslocrat << endl;
				(*pout).unsetf(ios::floatfield);
			}
			else
				(*pout) << "    NA" << endl;
		}
		if (print_csv)
		{
			(*pout_csv) << endl;
			CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS SUMMARY FOR NON-WEB LANGUAGES");
			(*pout_csv) << "Total,Blank,Comments,,Compiler,Data,Exec.,,File,SLOC" << endl;
			(*pout_csv) << "Lines,Lines,Whole,Embedded,Direct.,Decl.,Instr.,SLOC,Type,Definition" << endl;

			(*pout_csv) << allLangCode.total_line << ",";
			(*pout_csv) << allLangCode.blank_line << ",";
			(*pout_csv) << allLangCode.whole_comment << ",";
			(*pout_csv) << allLangCode.embed_comment << ",";
			(*pout_csv) << allLangCode.phy_direct << ",";
			(*pout_csv) << allLangCode.phy_decl << ",";
			(*pout_csv) << allLangCode.phy_instr << ",";
			(*pout_csv) << allLangCode.phy_direct + allLangCode.phy_decl + allLangCode.phy_instr << ",";
			(*pout_csv) << "CODE,Physical" << endl;

			(*pout_csv) << allLangCode.total_line << ",";
			(*pout_csv) << allLangCode.blank_line << ",";
			(*pout_csv) << allLangCode.whole_comment << ",";
			(*pout_csv) << allLangCode.embed_comment << ",";
			(*pout_csv) << allLangCode.log_direct << ",";
			(*pout_csv) << allLangCode.log_decl << ",";
			(*pout_csv) << allLangCode.log_instr << ",";
			(*pout_csv) << allLangCode.log_direct + allLangCode.log_decl + allLangCode.log_instr << ",";
			(*pout_csv) << "CODE,Logical" << endl;

			if (allLangData.num_of_file > 0)
			{
				(*pout_csv) << allLangData.total_line << ",";
				(*pout_csv) << allLangData.blank_line << ",";
				(*pout_csv) << allLangData.whole_comment << ",";
				(*pout_csv) << allLangData.embed_comment << ",";
				(*pout_csv) << allLangData.phy_direct << ",";
				(*pout_csv) << allLangData.phy_decl << ",";
				(*pout_csv) << allLangData.phy_instr << ",";
				(*pout_csv) << allLangData.phy_direct + allLangData.phy_decl + allLangData.phy_instr << ",";
				(*pout_csv) << "DATA,Physical" << endl;
			}

			// display statistics
			(*pout_csv) << endl << "Number of files successfully accessed,";
			(*pout_csv) << allLangCode.num_of_file + allLangData.num_of_file;
			(*pout_csv) << ",out of,";

			if (filesToPrint != NULL)
			{
				if (useListA)
					(*pout_csv) << totalNonDupFileA << endl;
				else
					(*pout_csv) << totalNonDupFileB << endl;
			}
			else if (useListA)
				(*pout_csv) << totalFileA << endl;
			else
				(*pout_csv) << totalFileB << endl;

			(*pout_csv) << endl << "Ratio of Physical to Logical SLOC,";

			if (tlsloc > 0)
			{
				tslocrat = (float)tpsloc / (float)tlsloc;
				(*pout_csv).setf(ios::fixed,ios::floatfield);
				(*pout_csv).width(8);
				(*pout_csv).precision(2);
				(*pout_csv) << tslocrat << endl;
				(*pout_csv).unsetf(ios::floatfield);
			}
			else
				(*pout_csv) << "NA" << endl;
		}
	}

	// display web language count
	printHeader = false;
	for (its = mySourceFile->begin(); its != mySourceFile->end(); its++)
	{
		if (filesToPrint != NULL && filesToPrint->size() > 0)
		{
			// restrict based on those files in the filesToPrint list
			sit = filesToPrint->begin();
			while (sit != filesToPrint->end() && its->second.file_name.compare((*sit)) != 0)
				sit++;

			if (excludeFiles)
			{
				// skip the file if in the filesToPrint list
				if (sit != filesToPrint->end())
					continue;
			}
			else
			{
				// skip the file if NOT in the filesToPrint list
				if (sit == filesToPrint->end())
					continue;
			}
		}

		if (its->second.class_type == WEB)
		{
			SourceFileList::iterator startpos = its;
			SourceFileList::iterator endpos = ++startpos;
			for (; endpos!= mySourceFile->end(); endpos++)
			{
				if ( endpos->second.file_name_isEmbedded == false )
					break;
			}
			webType = ((CWebCounter*)CounterForEachLanguage[WEB])->GetWebType(its->second.file_name);

			// get the stream for web languages
			if (!printHeader)
			{
				if (print_ascii || print_legacy)
				{
					pout = GetTotalOutputStream(outDir + outputFileNamePrePend);
					if (pout == NULL)
					{
						string err = "Error: Unable to create file (";
						err += outputFileNamePrePend + "TOTAL" + OUTPUT_FILE_NAME;
						err += "). Operation aborted.";
						userIF->AddError(err, false, 1);
						return 0;
					}
					// if there exists web language, append the header to the file
					(*pout) << endl;
					CUtil::PrintFileHeaderLine(*pout, "RESULTS FOR ALL WEB LANGUAGE FILES");
					(*pout) << endl;
					(*pout) << "                                    |       HTML      |        XML      |     JS-Clnt     |    VBS-Clnt     |     C#-Clnt     |      JS-Svr     |     VBS-Svr     |      C#-Svr     |           PHP           |          Java           |       SQL       |    ColdFusion   |     CFScript    |" << endl;
					if (print_legacy)
					{
						(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  | Compiler  Data   Exec.  | Compiler  Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                                                          Logical SLOC                                                  | File  Module" << endl;
						(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. | Direct.   Decl.  Instr. | Direct.   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Total      HTML     XML JS-Clnt VBS-Clnt C#-Clnt  JS-Svr VBS-Svr  C#-Svr     PHP    Java     SQL ColdFusion CFScript | Type  Name" << endl;
						(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-------------------------+-------------------------+-----------------+-----------------+-----------------+---------+--------------------------------------------------------------------------------------------------------------+---------------------------" << endl;
					}
					else
					{
						(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  | Compiler  Data   Exec.  | Compiler  Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                                                          Logical SLOC                                                  | Physical | File  Module" << endl;
						(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. | Direct.   Decl.  Instr. | Direct.   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |    HTML     XML JS-Clnt VBS-Clnt C#-Clnt  JS-Svr VBS-Svr  C#-Svr     PHP    Java     SQL ColdFusion CFScript     Total |   SLOC   | Type  Name" << endl;
						(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-------------------------+-------------------------+-----------------+-----------------+-----------------+--------------------------------------------------------------------------------------------------------------+---------+----------+---------------------------" << endl;
					}
				}
				if (print_csv)
				{
					pout_csv = GetTotalOutputStream(outDir + outputFileNamePrePend, true);
					if (pout_csv == NULL)
					{
						string err = "Error: Unable to create file (";
						err += outputFileNamePrePend + "Total" + OUTPUT_FILE_NAME_CSV;
						err += "). Operation aborted.";
						userIF->AddError(err, false, 1);
						return 0;
					}
					// if there exists web lang, append the header to the file
					(*pout_csv) << endl;
					CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS FOR ALL WEB LANGUAGE FILES");
					(*pout_csv) << endl;
					(*pout_csv) << ",,,,HTML,,XML,,JS-Clnt,,VBS-Clnt,,C#-Clnt,,JS-Svr,,VBS-Svr,,C#-Svr,,PHP,,,Java,,,SQL,,ColdFusion,,CFScript" << endl;
					(*pout_csv) << "Total,Blank,Comments,,Word,Exec.,Word,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Compiler,Data,Exec.,Compiler,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Logical SLOC,,,,,,,,,,,,,,Physical,File,Module" << endl;
					(*pout_csv) << "Lines,Lines,Whole,Embedded,LOC,Instr.,LOC,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Direct.,Decl.,Instr.,Direct.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,HTML,XML,JS-Clnt,VBS-Clnt,C#-Clnt,JS-Svr,VBS-Svr,C#-Svr,PHP,Java,SQL,ColdFusion,CFScript,Total,SLOC,Type,Name" << endl;
				}
				printHeader = true;
			}

			if (webType == WEB_PHP)
			{
				results r_htm;
				results r_js;
				results r_vbs;
				results r_php;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML_PHP:
						r_htm = i->second;
						break;
					case JAVASCRIPT_PHP:
						r_js = i->second;
						break;
					case VBS_PHP:
						r_vbs = i->second;
						break;
					case PHP:
						r_php = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_php.SLOC_lines[PHY] = r_php.directive_lines[PHY] + r_php.exec_lines[PHY] + r_php.data_lines[PHY];
				r_php.SLOC_lines[LOG] = r_php.directive_lines[LOG] + r_php.exec_lines[LOG] + r_php.data_lines[LOG];
				r_php.total_lines = r_php.SLOC_lines[PHY] +	r_php.blank_lines + r_php.comment_lines;

				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_php.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_php.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_php.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_php.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";					
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_php.directive_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_php.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_php.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_php.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_php.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(10);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(8);		(*pout) << "0";
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_php.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_php.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name << endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_php.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_php.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_php.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_php.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_php.directive_lines[LOG] << ",";
					(*pout_csv) << r_php.data_lines[LOG] << ",";
					(*pout_csv) << r_php.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_php.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_php.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_php.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_php.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_php.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_php.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_php.e_comm_lines);

				//0:htm 1:js 2:vbs 3:php
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_direct[3] += r_php.directive_lines[PHY];
				webtotal[webType].phy_decl[3] += r_php.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_php.exec_lines[PHY];
				webtotal[webType].log_direct[3] += r_php.directive_lines[LOG];
				webtotal[webType].log_decl[3] += r_php.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_php.exec_lines[LOG];

				allWebLang.num_of_file++;

				allWebLang.total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_php.total_lines);
				allWebLang.blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_php.blank_lines);
				allWebLang.whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_php.comment_lines);
				allWebLang.embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_php.e_comm_lines);

				allWebLang.phy_decl[0] += r_htm.data_lines[PHY];
				allWebLang.phy_instr[0] += r_htm.exec_lines[PHY];
				allWebLang.log_decl[0] += r_htm.data_lines[LOG];
				allWebLang.log_instr[0] += r_htm.exec_lines[LOG];

				allWebLang.phy_decl[2] += r_js.data_lines[PHY];
				allWebLang.phy_instr[2] += r_js.exec_lines[PHY];
				allWebLang.log_decl[2] += r_js.data_lines[LOG];
				allWebLang.log_instr[2] += r_js.exec_lines[LOG];

				allWebLang.phy_decl[3] += r_vbs.data_lines[PHY];
				allWebLang.phy_instr[3] += r_vbs.exec_lines[PHY];
				allWebLang.log_decl[3] += r_vbs.data_lines[LOG];
				allWebLang.log_instr[3] += r_vbs.exec_lines[LOG];

				allWebLang.phy_direct[8] += r_php.directive_lines[PHY];
				allWebLang.phy_decl[8] += r_php.data_lines[PHY];
				allWebLang.phy_instr[8] += r_php.exec_lines[PHY];
				allWebLang.log_direct[8] += r_php.directive_lines[LOG];
				allWebLang.log_decl[8] += r_php.data_lines[LOG];
				allWebLang.log_instr[8] += r_php.exec_lines[LOG];
			}
			else if (webType == WEB_JSP)
			{
				results r_htm;
				results r_js;
				results r_vbs;
				results r_java;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML_JSP:
						r_htm = i->second;
						break;
					case JAVASCRIPT_JSP:
						r_js = i->second;
						break;
					case VBS_JSP:
						r_vbs = i->second;
						break;
					case JAVA_JSP:
						r_java = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_java.SLOC_lines[PHY] = r_java.directive_lines[PHY] + r_java.exec_lines[PHY] + r_java.data_lines[PHY];
				r_java.SLOC_lines[LOG] = r_java.directive_lines[LOG] + r_java.exec_lines[LOG] + r_java.data_lines[LOG];
				r_java.total_lines = r_java.SLOC_lines[PHY] +	r_java.blank_lines + r_java.comment_lines;

				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_java.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_java.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_java.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_java.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_java.directive_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_java.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_java.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_java.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_java.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(10);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(8);		(*pout) << "0";
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_java.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_java.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name << endl;
					(*pout).unsetf(ios::right);				
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_java.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_java.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_java.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_java.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_java.directive_lines[LOG] << ",";
					(*pout_csv) << r_java.data_lines[LOG] << ",";
					(*pout_csv) << r_java.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_java.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_java.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_java.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_java.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_java.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_java.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_java.e_comm_lines);

				//0:htm 1:js 2:vbs 3:java
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_direct[3] += r_java.directive_lines[PHY];
				webtotal[webType].phy_decl[3] += r_java.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_java.exec_lines[PHY];
				webtotal[webType].log_direct[3] += r_java.directive_lines[LOG];
				webtotal[webType].log_decl[3] += r_java.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_java.exec_lines[LOG];

				allWebLang.num_of_file++;

				allWebLang.total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_java.total_lines);
				allWebLang.blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_java.blank_lines);
				allWebLang.whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_java.comment_lines);
				allWebLang.embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_java.e_comm_lines);

				allWebLang.phy_decl[0] += r_htm.data_lines[PHY];
				allWebLang.phy_instr[0] += r_htm.exec_lines[PHY];
				allWebLang.log_decl[0] += r_htm.data_lines[LOG];
				allWebLang.log_instr[0] += r_htm.exec_lines[LOG];

				allWebLang.phy_decl[2] += r_js.data_lines[PHY];
				allWebLang.phy_instr[2] += r_js.exec_lines[PHY];
				allWebLang.log_decl[2] += r_js.data_lines[LOG];
				allWebLang.log_instr[2] += r_js.exec_lines[LOG];

				allWebLang.phy_decl[3] += r_vbs.data_lines[PHY];
				allWebLang.phy_instr[3] += r_vbs.exec_lines[PHY];
				allWebLang.log_decl[3] += r_vbs.data_lines[LOG];
				allWebLang.log_instr[3] += r_vbs.exec_lines[LOG];

				allWebLang.phy_direct[9] += r_java.directive_lines[PHY];
				allWebLang.phy_decl[9] += r_java.data_lines[PHY];
				allWebLang.phy_instr[9] += r_java.exec_lines[PHY];
				allWebLang.log_direct[9] += r_java.directive_lines[LOG];
				allWebLang.log_decl[9] += r_java.data_lines[LOG];
				allWebLang.log_instr[9] += r_java.exec_lines[LOG];
			}
			else if (webType == WEB_ASP)
			{
				results r_htm;
				results r_jsc;
				results r_vbsc;
				results r_jss;
				results r_vbss;
				results r_css;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML_ASP:
						r_htm = i->second;
						break;
					case JAVASCRIPT_ASP_C:
						r_jsc = i->second;
						break;
					case VBS_ASP_C:
						r_vbsc = i->second;
						break;
					case JAVASCRIPT_ASP_S:
						r_jss = i->second;
						break;
					case VBS_ASP_S:
						r_vbss = i->second;
						break;
					case CSHARP_ASP_S:
						r_css = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_jsc.SLOC_lines[PHY] = r_jsc.exec_lines[PHY] + r_jsc.data_lines[PHY];
				r_jsc.SLOC_lines[LOG] = r_jsc.exec_lines[LOG] + r_jsc.data_lines[LOG];
				r_jsc.total_lines = r_jsc.SLOC_lines[PHY] +	r_jsc.blank_lines + r_jsc.comment_lines;

				r_vbsc.SLOC_lines[PHY] = r_vbsc.exec_lines[PHY] + r_vbsc.data_lines[PHY];
				r_vbsc.SLOC_lines[LOG] = r_vbsc.exec_lines[LOG] + r_vbsc.data_lines[LOG];
				r_vbsc.total_lines = r_vbsc.SLOC_lines[PHY] +	r_vbsc.blank_lines + r_vbsc.comment_lines;

				r_jss.SLOC_lines[PHY] = r_jss.directive_lines[PHY] + r_jss.exec_lines[PHY] + r_jss.data_lines[PHY];
				r_jss.SLOC_lines[LOG] = r_jss.directive_lines[LOG] + r_jss.exec_lines[LOG] + r_jss.data_lines[LOG];
				r_jss.total_lines = r_jss.SLOC_lines[PHY] +	r_jss.blank_lines + r_jss.comment_lines;

				r_vbss.SLOC_lines[PHY] = r_vbss.exec_lines[PHY] + r_vbss.data_lines[PHY];
				r_vbss.SLOC_lines[LOG] = r_vbss.exec_lines[LOG] + r_vbss.data_lines[LOG];
				r_vbss.total_lines = r_vbss.SLOC_lines[PHY] +	r_vbss.blank_lines + r_vbss.comment_lines;

				r_css.SLOC_lines[PHY] = r_css.exec_lines[PHY] + r_css.data_lines[PHY];
				r_css.SLOC_lines[LOG] = r_css.exec_lines[LOG] + r_css.data_lines[LOG];
				r_css.total_lines = r_css.SLOC_lines[PHY] +	r_css.blank_lines + r_css.comment_lines;

				if (print_ascii || print_legacy)
				{										
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_jsc.total_lines + r_vbsc.total_lines + r_jss.total_lines + r_vbss.total_lines + r_css.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_jsc.blank_lines + r_vbsc.blank_lines + r_jss.blank_lines + r_vbss.blank_lines + r_css.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_jsc.comment_lines + r_vbsc.comment_lines + r_jss.comment_lines + r_vbss.comment_lines + r_css.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_jsc.e_comm_lines + r_vbsc.e_comm_lines + r_jss.e_comm_lines + r_vbss.e_comm_lines + r_css.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_jsc.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_jsc.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbsc.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbsc.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";					
					(*pout).width(8);		(*pout) << r_jss.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_jss.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbss.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbss.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_css.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_css.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_jsc.SLOC_lines[LOG] + r_vbsc.SLOC_lines[LOG] + r_jss.SLOC_lines[LOG] + r_vbss.SLOC_lines[LOG] + r_css.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_jsc.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbsc.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_jss.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbss.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_css.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(10);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(8);		(*pout) << "0";
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_jsc.SLOC_lines[LOG] + r_vbsc.SLOC_lines[LOG] + r_jss.SLOC_lines[LOG] + r_vbss.SLOC_lines[LOG] + r_css.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_jsc.SLOC_lines[PHY] + r_vbsc.SLOC_lines[PHY] + r_jss.SLOC_lines[PHY] + r_vbss.SLOC_lines[PHY] + r_css.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name << endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_jsc.total_lines + r_vbsc.total_lines + r_jss.total_lines + r_vbss.total_lines + r_css.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_jsc.blank_lines + r_vbsc.blank_lines + r_jss.blank_lines + r_vbss.blank_lines + r_css.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_jsc.comment_lines + r_vbsc.comment_lines + r_jss.comment_lines + r_vbss.comment_lines + r_css.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_jsc.e_comm_lines + r_vbsc.e_comm_lines + r_jss.e_comm_lines + r_vbss.e_comm_lines + r_css.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_jsc.data_lines[LOG] << ",";
					(*pout_csv) << r_jsc.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbsc.data_lines[LOG] << ",";
					(*pout_csv) << r_vbsc.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << r_jss.data_lines[LOG] << ",";
					(*pout_csv) << r_jss.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbss.data_lines[LOG] << ",";
					(*pout_csv) << r_vbss.exec_lines[LOG] << ",";
					(*pout_csv) << r_css.data_lines[LOG] << ",";
					(*pout_csv) << r_css.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_jsc.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbsc.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_jss.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbss.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_css.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_jsc.SLOC_lines[LOG] + r_vbsc.SLOC_lines[LOG] + r_jss.SLOC_lines[LOG] + r_vbss.SLOC_lines[LOG] + r_css.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_jsc.SLOC_lines[PHY] + r_vbsc.SLOC_lines[PHY] + r_jss.SLOC_lines[PHY] + r_vbss.SLOC_lines[PHY] + r_css.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_jsc.total_lines + r_vbsc.total_lines + r_jss.total_lines + r_vbss.total_lines + r_css.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_jsc.blank_lines + r_vbsc.blank_lines + r_jss.blank_lines + r_vbss.blank_lines + r_css.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_jsc.comment_lines + r_vbsc.comment_lines + r_jss.comment_lines + r_vbss.comment_lines + r_css.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_jsc.e_comm_lines + r_vbsc.e_comm_lines + r_jss.e_comm_lines + r_vbss.e_comm_lines + r_css.e_comm_lines);

				//0:htm 1:jsc 2:vbsc 3:jss 4:vbss 5:csharps
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_jsc.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_jsc.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_jsc.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_jsc.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbsc.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbsc.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbsc.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbsc.exec_lines[LOG];

				webtotal[webType].phy_decl[3] += r_jss.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_jss.exec_lines[PHY];
				webtotal[webType].log_decl[3] += r_jss.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_jss.exec_lines[LOG];

				webtotal[webType].phy_decl[4] += r_vbss.data_lines[PHY];
				webtotal[webType].phy_instr[4] += r_vbss.exec_lines[PHY];
				webtotal[webType].log_decl[4] += r_vbss.data_lines[LOG];
				webtotal[webType].log_instr[4] += r_vbss.exec_lines[LOG];

				webtotal[webType].phy_decl[5] += r_css.data_lines[PHY];
				webtotal[webType].phy_instr[5] += r_css.exec_lines[PHY];
				webtotal[webType].log_decl[5] += r_css.data_lines[LOG];
				webtotal[webType].log_instr[5] += r_css.exec_lines[LOG];

				allWebLang.num_of_file++;

				allWebLang.total_line += (r_htm.total_lines + r_jsc.total_lines + r_vbsc.total_lines + r_jss.total_lines + r_vbss.total_lines + r_css.total_lines);
				allWebLang.blank_line += (r_htm.blank_lines + r_jsc.blank_lines + r_vbsc.blank_lines + r_jss.blank_lines + r_vbss.blank_lines + r_css.blank_lines);
				allWebLang.whole_comment += (r_htm.comment_lines + r_jsc.comment_lines + r_vbsc.comment_lines + r_jss.comment_lines + r_vbss.comment_lines + r_css.comment_lines);
				allWebLang.embed_comment += (r_htm.e_comm_lines + r_jsc.e_comm_lines + r_vbsc.e_comm_lines + r_jss.e_comm_lines + r_vbss.e_comm_lines + r_css.e_comm_lines);

				allWebLang.phy_decl[0] += r_htm.data_lines[PHY];
				allWebLang.phy_instr[0] += r_htm.exec_lines[PHY];
				allWebLang.log_decl[0] += r_htm.data_lines[LOG];
				allWebLang.log_instr[0] += r_htm.exec_lines[LOG];

				allWebLang.phy_decl[2] += r_jsc.data_lines[PHY];
				allWebLang.phy_instr[2] += r_jsc.exec_lines[PHY];
				allWebLang.log_decl[2] += r_jsc.data_lines[LOG];
				allWebLang.log_instr[2] += r_jsc.exec_lines[LOG];

				allWebLang.phy_decl[3] += r_vbsc.data_lines[PHY];
				allWebLang.phy_instr[3] += r_vbsc.exec_lines[PHY];
				allWebLang.log_decl[3] += r_vbsc.data_lines[LOG];
				allWebLang.log_instr[3] += r_vbsc.exec_lines[LOG];

				allWebLang.phy_decl[5] += r_jss.data_lines[PHY];
				allWebLang.phy_instr[5] += r_jss.exec_lines[PHY];
				allWebLang.log_decl[5] += r_jss.data_lines[LOG];
				allWebLang.log_instr[5] += r_jss.exec_lines[LOG];

				allWebLang.phy_decl[6] += r_vbss.data_lines[PHY];
				allWebLang.phy_instr[6] += r_vbss.exec_lines[PHY];
				allWebLang.log_decl[6] += r_vbss.data_lines[LOG];
				allWebLang.log_instr[6] += r_vbss.exec_lines[LOG];

				allWebLang.phy_decl[7] += r_css.data_lines[PHY];
				allWebLang.phy_instr[7] += r_css.exec_lines[PHY];
				allWebLang.log_decl[7] += r_css.data_lines[LOG];
				allWebLang.log_instr[7] += r_css.exec_lines[LOG];
			}
			else if (webType == WEB_XML)
			{
				results r_xml;
				results r_js;
				results r_vbs;
				results r_cs;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case XML:
						r_xml = i->second;
						break;
					case JAVASCRIPT_XML:
						r_js = i->second;
						break;
					case VBS_XML:
						r_vbs = i->second;
						break;
					case CSHARP_XML:
						r_cs = i->second;
						break;
					default:
						break;
					}
				}
				r_xml.SLOC_lines[PHY] = r_xml.exec_lines[PHY] + r_xml.data_lines[PHY];
				r_xml.SLOC_lines[LOG] = r_xml.exec_lines[LOG] + r_xml.data_lines[LOG];
				r_xml.total_lines = r_xml.SLOC_lines[PHY] +	r_xml.blank_lines + r_xml.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_cs.SLOC_lines[PHY] = r_cs.exec_lines[PHY] + r_cs.data_lines[PHY];
				r_cs.SLOC_lines[LOG] = r_cs.exec_lines[LOG] + r_cs.data_lines[LOG];
				r_cs.total_lines = r_cs.SLOC_lines[PHY] +	r_cs.blank_lines + r_cs.comment_lines;

				if (print_ascii || print_legacy)
				{		
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_xml.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_xml.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_xml.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_xml.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_xml.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_xml.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_cs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					if (print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_xml.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_xml.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(10);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(8);		(*pout) << "0";
					if (!print_legacy)
					{
						(*pout) << " |";
						(*pout).width(8);	(*pout) << (r_xml.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_xml.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_cs.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name << endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_xml.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines) << ",";
					(*pout_csv) << (r_xml.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines) << ",";
					(*pout_csv) << (r_xml.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines) << ",";
					(*pout_csv) << (r_xml.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines) << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_xml.data_lines[LOG] << ",";
					(*pout_csv) << r_xml.exec_lines[LOG] << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << r_cs.data_lines[LOG] << ",";
					(*pout_csv) << r_cs.exec_lines[LOG] << ",";					
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_xml.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_cs.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";	
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << (r_xml.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_xml.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_cs.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_xml.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
				webtotal[webType].blank_line += (r_xml.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
				webtotal[webType].whole_comment += (r_xml.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
				webtotal[webType].embed_comment += (r_xml.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);

				//0:xml 1:js 2:vbs 3:csharp
				webtotal[webType].phy_decl[0] += r_xml.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_xml.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_xml.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_xml.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_decl[3] += r_cs.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_cs.exec_lines[PHY];
				webtotal[webType].log_decl[3] += r_cs.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_cs.exec_lines[LOG];

				allWebLang.num_of_file++;

				allWebLang.total_line += (r_xml.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
				allWebLang.blank_line += (r_xml.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
				allWebLang.whole_comment += (r_xml.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
				allWebLang.embed_comment += (r_xml.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);

				allWebLang.phy_decl[1] += r_xml.data_lines[PHY];
				allWebLang.phy_instr[1] += r_xml.exec_lines[PHY];
				allWebLang.log_decl[1] += r_xml.data_lines[LOG];
				allWebLang.log_instr[1] += r_xml.exec_lines[LOG];

				allWebLang.phy_decl[2] += r_js.data_lines[PHY];
				allWebLang.phy_instr[2] += r_js.exec_lines[PHY];
				allWebLang.log_decl[2] += r_js.data_lines[LOG];
				allWebLang.log_instr[2] += r_js.exec_lines[LOG];

				allWebLang.phy_decl[3] += r_vbs.data_lines[PHY];
				allWebLang.phy_instr[3] += r_vbs.exec_lines[PHY];
				allWebLang.log_decl[3] += r_vbs.data_lines[LOG];
				allWebLang.log_instr[3] += r_vbs.exec_lines[LOG];

				allWebLang.phy_decl[4] += r_cs.data_lines[PHY];
				allWebLang.phy_instr[4] += r_cs.exec_lines[PHY];
				allWebLang.log_decl[4] += r_cs.data_lines[LOG];
				allWebLang.log_instr[4] += r_cs.exec_lines[LOG];
			}
			else if (webType == WEB_CFM)
			{
				results r_htm;
				results r_js;
				results r_vbs;
				results r_sql;
				results r_cfm;
				results r_cfs;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML_CFM:
						r_htm = i->second;
						break;
					case JAVASCRIPT_CFM:
						r_js = i->second;
						break;
					case VBS_CFM:
						r_vbs = i->second;
						break;
					case SQL_CFM:
						r_sql = i->second;
						break;
					case COLDFUSION:
						r_cfm = i->second;
						break;
					case CFSCRIPT:
						r_cfs = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_sql.SLOC_lines[PHY] = r_sql.exec_lines[PHY] + r_sql.data_lines[PHY];
				r_sql.SLOC_lines[LOG] = r_sql.exec_lines[LOG] + r_sql.data_lines[LOG];
				r_sql.total_lines = r_sql.SLOC_lines[PHY] +	r_sql.blank_lines + r_sql.comment_lines;

				r_cfm.SLOC_lines[PHY] = r_cfm.exec_lines[PHY] + r_cfm.data_lines[PHY];
				r_cfm.SLOC_lines[LOG] = r_cfm.exec_lines[LOG] + r_cfm.data_lines[LOG];
				r_cfm.total_lines = r_cfm.SLOC_lines[PHY] +	r_cfm.blank_lines + r_cfm.comment_lines;

				r_cfs.SLOC_lines[PHY] = r_cfs.exec_lines[PHY] + r_cfs.data_lines[PHY];
				r_cfs.SLOC_lines[LOG] = r_cfs.exec_lines[LOG] + r_cfs.data_lines[LOG];
				r_cfs.total_lines = r_cfs.SLOC_lines[PHY] +	r_cfs.blank_lines + r_cfs.comment_lines;

				if (print_ascii || print_legacy)
				{
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_sql.total_lines + r_cfm.total_lines + r_cfs.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_sql.blank_lines + r_cfm.blank_lines + r_cfs.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_sql.comment_lines + r_cfm.comment_lines + r_cfs.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_sql.e_comm_lines + r_cfm.e_comm_lines + r_cfs.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";					
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_sql.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_sql.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_cfm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cfm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_cfs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cfs.exec_lines[LOG];
					if (print_legacy)
					{
						(*pout) << " |";					
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_sql.SLOC_lines[LOG] + r_cfm.SLOC_lines[LOG] + r_cfs.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_sql.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(10);		(*pout) << r_cfm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_cfs.SLOC_lines[LOG];
					if (!print_legacy)
					{
						(*pout) << " |";					
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_sql.SLOC_lines[LOG] + r_cfm.SLOC_lines[LOG] + r_cfs.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_sql.SLOC_lines[PHY] + r_cfm.SLOC_lines[PHY] + r_cfs.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name << endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_sql.total_lines + r_cfm.total_lines + r_cfs.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_sql.blank_lines + r_cfm.blank_lines + r_cfs.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_sql.comment_lines + r_cfm.comment_lines + r_cfs.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_sql.e_comm_lines + r_cfm.e_comm_lines + r_cfs.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_sql.data_lines[LOG] << ",";
					(*pout_csv) << r_sql.exec_lines[LOG] << ",";
					(*pout_csv) << r_cfm.data_lines[LOG] << ",";
					(*pout_csv) << r_cfm.exec_lines[LOG] << ",";
					(*pout_csv) << r_cfs.data_lines[LOG] << ",";
					(*pout_csv) << r_cfs.exec_lines[LOG] << ",";					
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_sql.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_cfm.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_cfs.SLOC_lines[LOG] << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_sql.SLOC_lines[LOG] + r_cfm.SLOC_lines[LOG] + r_cfs.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_sql.SLOC_lines[PHY] + r_cfm.SLOC_lines[PHY] + r_cfs.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_sql.total_lines + r_cfm.total_lines + r_cfs.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_sql.blank_lines + r_cfm.blank_lines + r_cfs.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_sql.comment_lines + r_cfm.comment_lines + r_cfs.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_sql.e_comm_lines + r_cfm.e_comm_lines + r_cfs.e_comm_lines);

				//0:htm 1:js 2:vbs 3:sql 5:cfm 6:cfs
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_decl[3] += r_sql.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_sql.exec_lines[PHY];
				webtotal[webType].log_decl[3] += r_sql.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_sql.exec_lines[LOG];

				webtotal[webType].phy_decl[4] += r_cfm.data_lines[PHY];
				webtotal[webType].phy_instr[4] += r_cfm.exec_lines[PHY];
				webtotal[webType].log_decl[4] += r_cfm.data_lines[LOG];
				webtotal[webType].log_instr[4] += r_cfm.exec_lines[LOG];

				webtotal[webType].phy_decl[5] += r_cfs.data_lines[PHY];
				webtotal[webType].phy_instr[5] += r_cfs.exec_lines[PHY];
				webtotal[webType].log_decl[5] += r_cfs.data_lines[LOG];
				webtotal[webType].log_instr[5] += r_cfs.exec_lines[LOG];

				allWebLang.num_of_file++;

				allWebLang.total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_sql.total_lines + r_cfm.total_lines + r_cfs.total_lines);
				allWebLang.blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_sql.blank_lines + r_cfm.blank_lines + r_cfs.blank_lines);
				allWebLang.whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_sql.comment_lines + r_cfm.comment_lines + r_cfs.comment_lines);
				allWebLang.embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_sql.e_comm_lines + r_cfm.e_comm_lines + r_cfs.e_comm_lines);

				allWebLang.phy_decl[0] += r_htm.data_lines[PHY];
				allWebLang.phy_instr[0] += r_htm.exec_lines[PHY];
				allWebLang.log_decl[0] += r_htm.data_lines[LOG];
				allWebLang.log_instr[0] += r_htm.exec_lines[LOG];

				allWebLang.phy_decl[2] += r_js.data_lines[PHY];
				allWebLang.phy_instr[2] += r_js.exec_lines[PHY];
				allWebLang.log_decl[2] += r_js.data_lines[LOG];
				allWebLang.log_instr[2] += r_js.exec_lines[LOG];

				allWebLang.phy_decl[3] += r_vbs.data_lines[PHY];
				allWebLang.phy_instr[3] += r_vbs.exec_lines[PHY];
				allWebLang.log_decl[3] += r_vbs.data_lines[LOG];
				allWebLang.log_instr[3] += r_vbs.exec_lines[LOG];

				allWebLang.phy_decl[10] += r_sql.data_lines[PHY];
				allWebLang.phy_instr[10] += r_sql.exec_lines[PHY];
				allWebLang.log_decl[10] += r_sql.data_lines[LOG];
				allWebLang.log_instr[10] += r_sql.exec_lines[LOG];

				allWebLang.phy_decl[11] += r_cfm.data_lines[PHY];
				allWebLang.phy_instr[11] += r_cfm.exec_lines[PHY];
				allWebLang.log_decl[11] += r_cfm.data_lines[LOG];
				allWebLang.log_instr[11] += r_cfm.exec_lines[LOG];

				allWebLang.phy_decl[12] += r_cfs.data_lines[PHY];
				allWebLang.phy_instr[12] += r_cfs.exec_lines[PHY];
				allWebLang.log_decl[12] += r_cfs.data_lines[LOG];
				allWebLang.log_instr[12] += r_cfs.exec_lines[LOG];
			}
			else
			{
				webType = WEB_HTM;
				results r_htm;
				results r_js;
				results r_vbs;
				results r_cs;
				for (SourceFileList::iterator i = startpos; i!= endpos; i++)
				{
					switch (i->second.class_type)
					{
					case HTML:
						r_htm = i->second;
						break;
					case JAVASCRIPT_HTML:
						r_js = i->second;
						break;
					case VBS_HTML:
						r_vbs = i->second;
						break;
					case CSHARP_HTML:
						r_cs = i->second;
						break;
					default:
						break;
					}
				}
				r_htm.SLOC_lines[PHY] = r_htm.exec_lines[PHY] + r_htm.data_lines[PHY];
				r_htm.SLOC_lines[LOG] = r_htm.exec_lines[LOG] + r_htm.data_lines[LOG];
				r_htm.total_lines = r_htm.SLOC_lines[PHY] +	r_htm.blank_lines + r_htm.comment_lines;

				r_js.SLOC_lines[PHY] = r_js.exec_lines[PHY] + r_js.data_lines[PHY];
				r_js.SLOC_lines[LOG] = r_js.exec_lines[LOG] + r_js.data_lines[LOG];
				r_js.total_lines = r_js.SLOC_lines[PHY] +	r_js.blank_lines + r_js.comment_lines;

				r_vbs.SLOC_lines[PHY] = r_vbs.exec_lines[PHY] + r_vbs.data_lines[PHY];
				r_vbs.SLOC_lines[LOG] = r_vbs.exec_lines[LOG] + r_vbs.data_lines[LOG];
				r_vbs.total_lines = r_vbs.SLOC_lines[PHY] +	r_vbs.blank_lines + r_vbs.comment_lines;

				r_cs.SLOC_lines[PHY] = r_cs.exec_lines[PHY] + r_cs.data_lines[PHY];
				r_cs.SLOC_lines[LOG] = r_cs.exec_lines[LOG] + r_cs.data_lines[LOG];
				r_cs.total_lines = r_cs.SLOC_lines[PHY] +	r_cs.blank_lines + r_cs.comment_lines;

				if (print_ascii || print_legacy)
				{		
					(*pout).setf(ios::right);
					(*pout).width(8);		(*pout) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
					(*pout) << " ";
					(*pout).width(7);		(*pout) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
					(*pout) << " ";
					(*pout).width(8);		(*pout) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_htm.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_js.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_vbs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_vbs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_cs.data_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cs.exec_lines[LOG];
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " |";
					(*pout).width(8);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					if (print_legacy)
					{
						(*pout) << " |";					
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]);
					}
					(*pout) << " |";
					(*pout).width(8);		(*pout) << r_htm.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_js.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(8);		(*pout) << r_vbs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << r_cs.SLOC_lines[LOG];
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(7);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(10);		(*pout) << "0";
					(*pout) << " ";
					(*pout).width(8);		(*pout) << "0";
					if (!print_legacy)
					{
						(*pout) << " |";					
						(*pout).width(8);	(*pout) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]);
						(*pout) << " |";
						(*pout).width(9);	(*pout) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_cs.SLOC_lines[PHY]);
					}
					(*pout) << " | CODE  ";
					(*pout) << its->second.file_name << endl;
					(*pout).unsetf(ios::right);
				}
				if (print_csv)
				{
					(*pout_csv) << (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines) << ",";
					(*pout_csv) << (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines) << ",";
					(*pout_csv) << (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines) << ",";
					(*pout_csv) << (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines) << ",";
					(*pout_csv) << r_htm.data_lines[LOG] << ",";
					(*pout_csv) << r_htm.exec_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_js.data_lines[LOG] << ",";
					(*pout_csv) << r_js.exec_lines[LOG] << ",";
					(*pout_csv) << r_vbs.data_lines[LOG] << ",";
					(*pout_csv) << r_vbs.exec_lines[LOG] << ",";
					(*pout_csv) << r_cs.data_lines[LOG] << ",";
					(*pout_csv) << r_cs.exec_lines[LOG] << ",";					
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";					
					(*pout_csv) << r_htm.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << r_js.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_vbs.SLOC_lines[LOG] << ",";
					(*pout_csv) << r_cs.SLOC_lines[LOG] << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";	
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << "0" << ",";
					(*pout_csv) << (r_htm.SLOC_lines[LOG] + r_js.SLOC_lines[LOG] + r_vbs.SLOC_lines[LOG] + r_cs.SLOC_lines[LOG]) << ",";
					(*pout_csv) << (r_htm.SLOC_lines[PHY] + r_js.SLOC_lines[PHY] + r_vbs.SLOC_lines[PHY] + r_cs.SLOC_lines[PHY]) << ",";
					(*pout_csv) << "CODE," << its->second.file_name << endl;
				}

				webtotal[webType].num_of_file++;

				webtotal[webType].total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
				webtotal[webType].blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
				webtotal[webType].whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
				webtotal[webType].embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);

				//0:htm 1:js 2:vbs 3:csharp
				webtotal[webType].phy_decl[0] += r_htm.data_lines[PHY];
				webtotal[webType].phy_instr[0] += r_htm.exec_lines[PHY];
				webtotal[webType].log_decl[0] += r_htm.data_lines[LOG];
				webtotal[webType].log_instr[0] += r_htm.exec_lines[LOG];

				webtotal[webType].phy_decl[1] += r_js.data_lines[PHY];
				webtotal[webType].phy_instr[1] += r_js.exec_lines[PHY];
				webtotal[webType].log_decl[1] += r_js.data_lines[LOG];
				webtotal[webType].log_instr[1] += r_js.exec_lines[LOG];

				webtotal[webType].phy_decl[2] += r_vbs.data_lines[PHY];
				webtotal[webType].phy_instr[2] += r_vbs.exec_lines[PHY];
				webtotal[webType].log_decl[2] += r_vbs.data_lines[LOG];
				webtotal[webType].log_instr[2] += r_vbs.exec_lines[LOG];

				webtotal[webType].phy_decl[3] += r_cs.data_lines[PHY];
				webtotal[webType].phy_instr[3] += r_cs.exec_lines[PHY];
				webtotal[webType].log_decl[3] += r_cs.data_lines[LOG];
				webtotal[webType].log_instr[3] += r_cs.exec_lines[LOG];

				allWebLang.num_of_file++;

				allWebLang.total_line += (r_htm.total_lines + r_js.total_lines + r_vbs.total_lines + r_cs.total_lines);
				allWebLang.blank_line += (r_htm.blank_lines + r_js.blank_lines + r_vbs.blank_lines + r_cs.blank_lines);
				allWebLang.whole_comment += (r_htm.comment_lines + r_js.comment_lines + r_vbs.comment_lines + r_cs.comment_lines);
				allWebLang.embed_comment += (r_htm.e_comm_lines + r_js.e_comm_lines + r_vbs.e_comm_lines + r_cs.e_comm_lines);

				allWebLang.phy_decl[0] += r_htm.data_lines[PHY];
				allWebLang.phy_instr[0] += r_htm.exec_lines[PHY];
				allWebLang.log_decl[0] += r_htm.data_lines[LOG];
				allWebLang.log_instr[0] += r_htm.exec_lines[LOG];

				allWebLang.phy_decl[2] += r_js.data_lines[PHY];
				allWebLang.phy_instr[2] += r_js.exec_lines[PHY];
				allWebLang.log_decl[2] += r_js.data_lines[LOG];
				allWebLang.log_instr[2] += r_js.exec_lines[LOG];

				allWebLang.phy_decl[3] += r_vbs.data_lines[PHY];
				allWebLang.phy_instr[3] += r_vbs.exec_lines[PHY];
				allWebLang.log_decl[3] += r_vbs.data_lines[LOG];
				allWebLang.log_instr[3] += r_vbs.exec_lines[LOG];

				allWebLang.phy_decl[4] += r_cs.data_lines[PHY];
				allWebLang.phy_instr[4] += r_cs.exec_lines[PHY];
				allWebLang.log_decl[4] += r_cs.data_lines[LOG];
				allWebLang.log_instr[4] += r_cs.exec_lines[LOG];
			}

			// skip the other web language partial file
			its = --endpos;
		}
	}

	// display summary for WEB languages
	totalNonDupFileA = 0; totalFileA = 0;
	totalNonDupFileB = 0; totalFileB = 0;
	tpsloc = 0; tlsloc = 0;
	for (WebTotalValueMap::iterator itto = webtotal.begin(); itto != webtotal.end(); itto++)
	{
		webCounter = (CWebCounter*)CounterForEachLanguage[WEB];
		if (filesToPrint != NULL)
		{
			if (excludeFiles)
			{
				if (useListA)
				{
					if (itto->first == WEB_PHP)
						totalNonDupFileA += webCounter->total_php_filesA - webCounter->total_php_dupFilesA;
					else if (itto->first == WEB_ASP)
						totalNonDupFileA += webCounter->total_asp_filesA - webCounter->total_asp_dupFilesA;
					else if (itto->first == WEB_JSP)
						totalNonDupFileA += webCounter->total_jsp_filesA - webCounter->total_jsp_dupFilesA;
					else if (itto->first == WEB_XML)
						totalNonDupFileA += webCounter->total_xml_filesA - webCounter->total_xml_dupFilesA;
					else if (itto->first == WEB_CFM)
						totalNonDupFileA += webCounter->total_cfm_filesA - webCounter->total_cfm_dupFilesA;
					else
						totalNonDupFileA += webCounter->total_htm_filesA - webCounter->total_htm_dupFilesA;
				}
				else
				{
					if (itto->first == WEB_PHP)
						totalNonDupFileB += webCounter->total_php_filesB - webCounter->total_php_dupFilesB;
					else if (itto->first == WEB_ASP)
						totalNonDupFileB += webCounter->total_asp_filesB - webCounter->total_asp_dupFilesB;
					else if (itto->first == WEB_JSP)
						totalNonDupFileB += webCounter->total_jsp_filesB - webCounter->total_jsp_dupFilesB;
					else if (itto->first == WEB_XML)
						totalNonDupFileA += webCounter->total_xml_filesB - webCounter->total_xml_dupFilesB;
					else if (itto->first == WEB_CFM)
						totalNonDupFileB += webCounter->total_cfm_filesB - webCounter->total_cfm_dupFilesB;
					else
						totalNonDupFileB += webCounter->total_htm_filesB - webCounter->total_htm_dupFilesB;
				}
			}
			else
			{
				if (useListA)
				{
					if (itto->first == WEB_PHP)
						totalNonDupFileA += webCounter->total_php_dupFilesA;
					else if (itto->first == WEB_ASP)
						totalNonDupFileA += webCounter->total_asp_dupFilesA;
					else if (itto->first == WEB_JSP)
						totalNonDupFileA += webCounter->total_jsp_dupFilesA;
					else if (itto->first == WEB_XML)
						totalNonDupFileA += webCounter->total_xml_dupFilesA;
					else if (itto->first == WEB_CFM)
						totalNonDupFileA += webCounter->total_cfm_dupFilesA;
					else
						totalNonDupFileA += webCounter->total_htm_dupFilesA;
				}
				else
				{
					if (itto->first == WEB_PHP)
						totalNonDupFileB += webCounter->total_php_dupFilesB;
					else if (itto->first == WEB_ASP)
						totalNonDupFileB += webCounter->total_asp_dupFilesB;
					else if (itto->first == WEB_JSP)
						totalNonDupFileB += webCounter->total_jsp_dupFilesB;
					else if (itto->first == WEB_XML)
						totalNonDupFileA += webCounter->total_xml_dupFilesB;
					else if (itto->first == WEB_CFM)
						totalNonDupFileB += webCounter->total_cfm_dupFilesB;
					else
						totalNonDupFileB += webCounter->total_htm_dupFilesB;
				}
			}
		}
		else if (useListA)
		{
			if (itto->first == WEB_PHP)
				totalFileA += webCounter->total_php_filesA;
			else if (itto->first == WEB_ASP)
				totalFileA += webCounter->total_asp_filesA;
			else if (itto->first == WEB_JSP)
				totalFileA += webCounter->total_jsp_filesA;
			else if (itto->first == WEB_XML)
				totalFileA += webCounter->total_xml_filesA;
			else if (itto->first == WEB_CFM)
				totalFileA += webCounter->total_cfm_filesA;
			else
				totalFileA += webCounter->total_htm_filesA;
		}
		else
		{
			if (itto->first == WEB_PHP)
				totalFileB += webCounter->total_php_filesB;
			else if (itto->first == WEB_ASP)
				totalFileB += webCounter->total_asp_filesB;
			else if (itto->first == WEB_JSP)
				totalFileB += webCounter->total_jsp_filesB;
			else if (itto->first == WEB_XML)
				totalFileA += webCounter->total_xml_filesB;
			else if (itto->first == WEB_CFM)
				totalFileB += webCounter->total_cfm_filesB;
			else
				totalFileB += webCounter->total_htm_filesB;
		}

		tlsloc += itto->second.log_decl[0] + itto->second.log_instr[0] +
			itto->second.log_decl[1] + itto->second.log_instr[1] +
			itto->second.log_decl[2] + itto->second.log_instr[2] +
			itto->second.log_decl[3] + itto->second.log_instr[3] +
			itto->second.log_decl[4] + itto->second.log_instr[4] +
			itto->second.log_decl[5] + itto->second.log_instr[5];

		tpsloc += itto->second.phy_decl[0] + itto->second.phy_instr[0] +
			itto->second.phy_decl[1] + itto->second.phy_instr[1] +
			itto->second.phy_decl[2] + itto->second.phy_instr[2] +
			itto->second.phy_decl[3] + itto->second.phy_instr[3] +
			itto->second.phy_decl[4] + itto->second.phy_instr[4] +
			itto->second.phy_decl[5] + itto->second.phy_instr[5];
	}

	if (webtotal.size() > 0) 
	{
		if (print_ascii || print_legacy)
		{
			(*pout) << endl;
			CUtil::PrintFileHeaderLine(*pout, "RESULTS SUMMARY FOR WEB LANGUAGES");
			(*pout) << endl;
			(*pout) << "                                    |       HTML      |        XML      |     JS-Clnt     |    VBS-Clnt     |     C#-Clnt     |      JS-Svr     |     VBS-Svr     |      C#-Svr     |           PHP           |          Java           |       SQL       |    ColdFusion   |     CFScript    |" << endl;
			if (print_legacy)
			{
				(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  | Compiler  Data   Exec.  | Compiler  Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                                                              SLOC                                                      | File  SLOC" << endl;
				(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. | Direct.   Decl.  Instr. | Direct.   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Total      HTML     XML JS-Clnt VBS-Clnt C#-Clnt  JS-Svr VBS-Svr  C#-Svr     PHP    Java     SQL ColdFusion CFScript | Type  Definition" << endl;
				(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-------------------------+-------------------------+-----------------+-----------------+-----------------+---------+--------------------------------------------------------------------------------------------------------------+------------------" << endl;
			}
			else
			{
				(*pout) << "   Total   Blank |      Comments    |    Word  Exec.  |    Word  Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  | Compiler  Data   Exec.  | Compiler  Data   Exec.  |   Data   Exec.  |   Data   Exec.  |   Data   Exec.  |                                                              SLOC                                                      | File  SLOC" << endl;
				(*pout) << "   Lines   Lines |   Whole Embedded |     LOC  Instr. |     LOC  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. | Direct.   Decl.  Instr. | Direct.   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |   Decl.  Instr. |    HTML     XML JS-Clnt VBS-Clnt C#-Clnt  JS-Svr VBS-Svr  C#-Svr     PHP    Java     SQL ColdFusion CFScript     Total | Type  Definition" << endl;
				(*pout) << "-----------------+------------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-----------------+-------------------------+-------------------------+-----------------+-----------------+-----------------+--------------------------------------------------------------------------------------------------------------+---------+------------------" << endl;
			}

			(*pout).setf(ios::right);
			(*pout).width(8);		(*pout) << allWebLang.total_line;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.blank_line;
			(*pout) << " |";
			(*pout).width(8);		(*pout) << allWebLang.whole_comment;
			(*pout) << " ";
			(*pout).width(8);		(*pout) << allWebLang.embed_comment;
			(*pout) << " |";	// 0 - HTML
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[0];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[0];
			(*pout) << " |";	// 1 - XML
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[1];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[1];
			(*pout) << " |";	// 2 - JSclnt
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[2];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[2];
			(*pout) << " |";	// 3 - VBSclnt
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[3];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[3];
			(*pout) << " |";	// 4 - C#clnt
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[4];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[4];
			(*pout) << " |";	// 5 - JSsrv
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[5];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[5];
			(*pout) << " |";	// 6 - VBSsrv
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[6];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[6];
			(*pout) << " |";	// 7 - C#srv
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[7];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[7];
			(*pout) << " |";	// 8 - PHP
			(*pout).width(8);		(*pout) << allWebLang.phy_direct[8];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_decl[8];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[8];
			(*pout) << " |";	// 9 - Java
			(*pout).width(8);		(*pout) << allWebLang.phy_direct[9];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_decl[9];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[9];
			(*pout) << " |";	//10 - SQL
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[10];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[10];
			(*pout) << " |";	//11 - CF
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[11];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[11];
			(*pout) << " |";	//12 - CFS
			(*pout).width(8);		(*pout) << allWebLang.phy_decl[12];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.phy_instr[12];
			if (print_legacy)
			{
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (allWebLang.phy_decl[0] + allWebLang.phy_instr[0] +
					allWebLang.phy_decl[1] + allWebLang.phy_instr[1] +
					allWebLang.phy_decl[2] + allWebLang.phy_instr[2] +
					allWebLang.phy_decl[3] + allWebLang.phy_instr[3] +
					allWebLang.phy_decl[4] + allWebLang.phy_instr[4] + 
					allWebLang.phy_decl[5] + allWebLang.phy_instr[5] +
					allWebLang.phy_decl[6] + allWebLang.phy_instr[6] +
					allWebLang.phy_decl[7] + allWebLang.phy_instr[7] +
					allWebLang.phy_direct[8] + allWebLang.phy_decl[8] + allWebLang.phy_instr[8] +
					allWebLang.phy_direct[9] + allWebLang.phy_decl[9] + allWebLang.phy_instr[9] +
					allWebLang.phy_decl[10] + allWebLang.phy_instr[10] +
					allWebLang.phy_decl[11] + allWebLang.phy_instr[11] +
					allWebLang.phy_decl[12] + allWebLang.phy_instr[12]);
			}
			(*pout) << " |";
			(*pout).width(8);		(*pout) << (allWebLang.phy_decl[0] + allWebLang.phy_instr[0]);		// HTML
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.phy_decl[1] + allWebLang.phy_instr[1]);		// XML
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.phy_decl[2] + allWebLang.phy_instr[2]);		// JSclnt
			(*pout) << " ";
			(*pout).width(8);		(*pout) << (allWebLang.phy_decl[3] + allWebLang.phy_instr[3]);		// VBSclnt
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.phy_decl[4] + allWebLang.phy_instr[4]);		// C#clnt
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.phy_decl[5] + allWebLang.phy_instr[5]);		// JSsrv
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.phy_decl[6] + allWebLang.phy_instr[6]);		// VBSsrv
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.phy_decl[7] + allWebLang.phy_instr[7]);		// C#srv
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.phy_direct[8] + allWebLang.phy_decl[8] + allWebLang.phy_instr[8]);	// PHP
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.phy_direct[9] + allWebLang.phy_decl[9] + allWebLang.phy_instr[9]);	// Java
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.phy_decl[10] + allWebLang.phy_instr[10]);		// SQL
			(*pout) << " ";
			(*pout).width(10);		(*pout) << (allWebLang.phy_decl[11] + allWebLang.phy_instr[11]);	// CF
			(*pout) << " ";
			(*pout).width(8);		(*pout) << (allWebLang.phy_decl[12] + allWebLang.phy_instr[12]);	// CFS
			if (!print_legacy)
			{
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (allWebLang.phy_decl[0] + allWebLang.phy_instr[0] +
					allWebLang.phy_decl[1] + allWebLang.phy_instr[1] +
					allWebLang.phy_decl[2] + allWebLang.phy_instr[2] +
					allWebLang.phy_decl[3] + allWebLang.phy_instr[3] +
					allWebLang.phy_decl[4] + allWebLang.phy_instr[4] + 
					allWebLang.phy_decl[5] + allWebLang.phy_instr[5] +
					allWebLang.phy_decl[6] + allWebLang.phy_instr[6] +
					allWebLang.phy_decl[7] + allWebLang.phy_instr[7] +
					allWebLang.phy_direct[8] + allWebLang.phy_decl[8] + allWebLang.phy_instr[8] +
					allWebLang.phy_direct[9] + allWebLang.phy_decl[9] + allWebLang.phy_instr[9] +
					allWebLang.phy_decl[10] + allWebLang.phy_instr[10] +
					allWebLang.phy_decl[11] + allWebLang.phy_instr[11] +
					allWebLang.phy_decl[12] + allWebLang.phy_instr[12]);
			}
			(*pout) << " | CODE  Physical" << endl;

			(*pout).setf(ios::right);
			(*pout).width(8);		(*pout) << allWebLang.total_line;
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.blank_line;
			(*pout) << " |";
			(*pout).width(8);		(*pout) << allWebLang.whole_comment;
			(*pout) << " ";
			(*pout).width(8);		(*pout) << allWebLang.embed_comment;
			(*pout) << " |";	// 0 - HTML
			(*pout).width(8);		(*pout) << allWebLang.log_decl[0];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[0];
			(*pout) << " |";	// 1 - XML
			(*pout).width(8);		(*pout) << allWebLang.log_decl[1];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[1];
			(*pout) << " |";	// 2 - JSclnt
			(*pout).width(8);		(*pout) << allWebLang.log_decl[2];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[2];
			(*pout) << " |";	// 3 - VBSclnt
			(*pout).width(8);		(*pout) << allWebLang.log_decl[3];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[3];
			(*pout) << " |";	// 4 - C#clnt
			(*pout).width(8);		(*pout) << allWebLang.log_decl[4];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[4];
			(*pout) << " |";	// 5 - JSsrv
			(*pout).width(8);		(*pout) << allWebLang.log_decl[5];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[5];
			(*pout) << " |";	// 6 - VBSsrv
			(*pout).width(8);		(*pout) << allWebLang.log_decl[6];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[6];
			(*pout) << " |";	// 7 - C#srv
			(*pout).width(8);		(*pout) << allWebLang.log_decl[7];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[7];
			(*pout) << " |";	// 8 - PHP
			(*pout).width(8);		(*pout) << allWebLang.log_direct[8];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_decl[8];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[8];
			(*pout) << " |";	// 9 - Java
			(*pout).width(8);		(*pout) << allWebLang.log_direct[9];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_decl[9];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[9];
			(*pout) << " |";	//10 - SQL
			(*pout).width(8);		(*pout) << allWebLang.log_decl[10];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[10];
			(*pout) << " |";	//11 - CF
			(*pout).width(8);		(*pout) << allWebLang.log_decl[11];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[11];
			(*pout) << " |";	//12 - CFS
			(*pout).width(8);		(*pout) << allWebLang.log_decl[12];
			(*pout) << " ";
			(*pout).width(7);		(*pout) << allWebLang.log_instr[12];
			if (print_legacy)
			{
				(*pout) << " |";
				(*pout).width(7);		(*pout) << (allWebLang.log_decl[0] + allWebLang.log_instr[0] +
					allWebLang.log_decl[1] + allWebLang.log_instr[1] +
					allWebLang.log_decl[2] + allWebLang.log_instr[2] +
					allWebLang.log_decl[3] + allWebLang.log_instr[3] +
					allWebLang.log_decl[4] + allWebLang.log_instr[4] + 
					allWebLang.log_decl[5] + allWebLang.log_instr[5] +
					allWebLang.log_decl[6] + allWebLang.log_instr[6] +
					allWebLang.log_decl[7] + allWebLang.log_instr[7] +
					allWebLang.log_direct[8] + allWebLang.log_decl[8] + allWebLang.log_instr[8] +
					allWebLang.log_direct[9] + allWebLang.log_decl[9] + allWebLang.log_instr[9] +
					allWebLang.log_decl[10] + allWebLang.log_instr[10] +
					allWebLang.log_decl[11] + allWebLang.log_instr[11] +
					allWebLang.log_decl[12] + allWebLang.log_instr[12]);
			}
			(*pout) << " |";
			(*pout).width(8);		(*pout) << (allWebLang.log_decl[0] + allWebLang.log_instr[0]);		// HTML
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.log_decl[1] + allWebLang.log_instr[1]);		// XML
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.log_decl[2] + allWebLang.log_instr[2]);		// JSclnt
			(*pout) << " ";
			(*pout).width(8);		(*pout) << (allWebLang.log_decl[3] + allWebLang.log_instr[3]);		// VBSclnt
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.log_decl[4] + allWebLang.log_instr[4]);		// C#clnt
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.log_decl[5] + allWebLang.log_instr[5]);		// JSsvr
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.log_decl[6] + allWebLang.log_instr[6]);		// VBSsrv
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.log_decl[7] + allWebLang.log_instr[7]);		// C#srv
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.log_direct[8] + allWebLang.log_decl[8] + allWebLang.log_instr[8]);	// PHP
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.log_direct[9] + allWebLang.log_decl[9] + allWebLang.log_instr[9]);	// Java
			(*pout) << " ";
			(*pout).width(7);		(*pout) << (allWebLang.log_decl[10] + allWebLang.log_instr[10]);		// SQL
			(*pout) << " ";
			(*pout).width(10);		(*pout) << (allWebLang.log_decl[11] + allWebLang.log_instr[11]);	// CF
			(*pout) << " ";
			(*pout).width(8);		(*pout) << (allWebLang.log_decl[12] + allWebLang.log_instr[12]);	// CFS
			if (!print_legacy)
			{
				(*pout) << " |";
				(*pout).width(8);		(*pout) << (allWebLang.log_decl[0] + allWebLang.log_instr[0] +
					allWebLang.log_decl[1] + allWebLang.log_instr[1] +
					allWebLang.log_decl[2] + allWebLang.log_instr[2] +
					allWebLang.log_decl[3] + allWebLang.log_instr[3] +
					allWebLang.log_decl[4] + allWebLang.log_instr[4] + 
					allWebLang.log_decl[5] + allWebLang.log_instr[5] +
					allWebLang.log_decl[6] + allWebLang.log_instr[6] +
					allWebLang.log_decl[7] + allWebLang.log_instr[7] +
					allWebLang.log_direct[8] + allWebLang.log_decl[8] + allWebLang.log_instr[8] +
					allWebLang.log_direct[9] + allWebLang.log_decl[9] + allWebLang.log_instr[9] +
					allWebLang.log_decl[10] + allWebLang.log_instr[10] +
					allWebLang.log_decl[11] + allWebLang.log_instr[11] +
					allWebLang.log_decl[12] + allWebLang.log_instr[12]);
			}
			(*pout) << " | CODE  Logical" << endl;

			// display statistics
			(*pout) << endl << "Number of files successfully accessed........................ ";
			(*pout).width(5);		(*pout) << allWebLang.num_of_file;
			(*pout) << " out of ";
			if (filesToPrint != NULL)
			{
				if (useListA)
					(*pout) << totalNonDupFileA << endl;
				else
					(*pout) << totalNonDupFileB << endl;
			}
			else if (useListA)
				(*pout) << totalFileA << endl;
			else
				(*pout) << totalFileB << endl;

			(*pout) << endl << "Ratio of Physical to Logical SLOC............................ ";

			if (tlsloc > 0)
			{
				tslocrat = (float)tpsloc / (float)tlsloc;
				(*pout).setf(ios::fixed,ios::floatfield);
				(*pout).width(8);
				(*pout).precision(2);
				(*pout) << tslocrat << endl;
				(*pout).unsetf(ios::floatfield);
			}
			else
				(*pout) << "    NA" << endl;
		}
		if (print_csv)
		{
			(*pout_csv) << endl;
			CUtil::PrintFileHeaderLine(*pout_csv, "RESULTS SUMMARY FOR WEB LANGUAGES");
			(*pout_csv) << ",,,,HTML,,XML,,JS-Clnt,,VBS-Clnt,,C#-Clnt,,JS-Svr,,VBS-Svr,,C#-Svr,,PHP,,,Java,,,SQL,,ColdFusion,,CFScript" << endl;
			(*pout_csv) << "Total,Blank,Comments,,Word,Exec.,Word,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,Compiler,Data,Exec.,Compiler,Data,Exec.,Data,Exec.,Data,Exec.,Data,Exec.,SLOC,,,,,,,,,,,,,File,SLOC" << endl;
			(*pout_csv) << "Lines,Lines,Whole,Embedded,LOC,Instr.,LOC,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Direct.,Decl.,Instr.,Direct.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,Decl.,Instr.,HTML,XML,JS-Clnt,VBS-Clnt,C#-Clnt,JS-Svr,VBS-Svr,C#-Svr,PHP,Java,SQL,ColdFusion,CFScript,Total,Type,Definition" << endl;

			(*pout_csv) << allWebLang.total_line << ",";
			(*pout_csv) << allWebLang.blank_line << ",";
			(*pout_csv) << allWebLang.whole_comment << ",";
			(*pout_csv) << allWebLang.embed_comment << ",";
			(*pout_csv) << allWebLang.phy_decl[0] << ",";
			(*pout_csv) << allWebLang.phy_instr[0] << ",";
			(*pout_csv) << allWebLang.phy_decl[1] << ",";
			(*pout_csv) << allWebLang.phy_instr[1] << ",";
			(*pout_csv) << allWebLang.phy_decl[2] << ",";
			(*pout_csv) << allWebLang.phy_instr[2] << ",";
			(*pout_csv) << allWebLang.phy_decl[3] << ",";
			(*pout_csv) << allWebLang.phy_instr[3] << ",";
			(*pout_csv) << allWebLang.phy_decl[4] << ",";
			(*pout_csv) << allWebLang.phy_instr[4] << ",";
			(*pout_csv) << allWebLang.phy_decl[5] << ",";
			(*pout_csv) << allWebLang.phy_instr[5] << ",";
			(*pout_csv) << allWebLang.phy_decl[6] << ",";
			(*pout_csv) << allWebLang.phy_instr[6] << ",";
			(*pout_csv) << allWebLang.phy_decl[7] << ",";
			(*pout_csv) << allWebLang.phy_instr[7] << ",";
			(*pout_csv) << allWebLang.phy_direct[8] << ",";
			(*pout_csv) << allWebLang.phy_decl[8] << ",";
			(*pout_csv) << allWebLang.phy_instr[8] << ",";
			(*pout_csv) << allWebLang.phy_direct[9] << ",";
			(*pout_csv) << allWebLang.phy_decl[9] << ",";
			(*pout_csv) << allWebLang.phy_instr[9] << ",";
			(*pout_csv) << allWebLang.phy_decl[10] << ",";
			(*pout_csv) << allWebLang.phy_instr[10] << ",";
			(*pout_csv) << allWebLang.phy_decl[11] << ",";
			(*pout_csv) << allWebLang.phy_instr[11] << ",";
			(*pout_csv) << allWebLang.phy_decl[12] << ",";
			(*pout_csv) << allWebLang.phy_instr[12] << ",";
			(*pout_csv) << (allWebLang.phy_decl[0] + allWebLang.phy_instr[0]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[1] + allWebLang.phy_instr[1]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[2] + allWebLang.phy_instr[2]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[3] + allWebLang.phy_instr[3]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[4] + allWebLang.phy_instr[4]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[5] + allWebLang.phy_instr[5]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[6] + allWebLang.phy_instr[6]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[7] + allWebLang.phy_instr[7]) << ",";
			(*pout_csv) << (allWebLang.phy_direct[8] + allWebLang.phy_decl[8] + allWebLang.phy_instr[8]) << ",";
			(*pout_csv) << (allWebLang.phy_direct[9] + allWebLang.phy_decl[9] + allWebLang.phy_instr[9]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[10] + allWebLang.phy_instr[10]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[11] + allWebLang.phy_instr[11]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[12] + allWebLang.phy_instr[12]) << ",";
			(*pout_csv) << (allWebLang.phy_decl[0] + allWebLang.phy_instr[0] +
				allWebLang.phy_decl[1] + allWebLang.phy_instr[1] +
				allWebLang.phy_decl[2] + allWebLang.phy_instr[2] +
				allWebLang.phy_decl[3] + allWebLang.phy_instr[3] +
				allWebLang.phy_decl[4] + allWebLang.phy_instr[4] + 
				allWebLang.phy_decl[5] + allWebLang.phy_instr[5] +
				allWebLang.phy_decl[6] + allWebLang.phy_instr[6] +
				allWebLang.phy_decl[7] + allWebLang.phy_instr[7] +
				allWebLang.phy_direct[8] + allWebLang.phy_decl[8] + allWebLang.phy_instr[8] +
				allWebLang.phy_direct[9] + allWebLang.phy_decl[9] + allWebLang.phy_instr[9] +
				allWebLang.phy_decl[10] + allWebLang.phy_instr[10] +
				allWebLang.phy_decl[11] + allWebLang.phy_instr[11] +
				allWebLang.phy_decl[12] + allWebLang.phy_instr[12]) << ",";
			(*pout_csv) << "CODE,Physical" << endl;

			(*pout_csv) << allWebLang.total_line << ",";
			(*pout_csv) << allWebLang.blank_line << ",";
			(*pout_csv) << allWebLang.whole_comment << ",";
			(*pout_csv) << allWebLang.embed_comment << ",";
			(*pout_csv) << allWebLang.log_decl[0] << ",";
			(*pout_csv) << allWebLang.log_instr[0] << ",";
			(*pout_csv) << allWebLang.log_decl[1] << ",";
			(*pout_csv) << allWebLang.log_instr[1] << ",";
			(*pout_csv) << allWebLang.log_decl[2] << ",";
			(*pout_csv) << allWebLang.log_instr[2] << ",";
			(*pout_csv) << allWebLang.log_decl[3] << ",";
			(*pout_csv) << allWebLang.log_instr[3] << ",";
			(*pout_csv) << allWebLang.log_decl[4] << ",";
			(*pout_csv) << allWebLang.log_instr[4] << ",";
			(*pout_csv) << allWebLang.log_decl[5] << ",";
			(*pout_csv) << allWebLang.log_instr[5] << ",";
			(*pout_csv) << allWebLang.log_decl[6] << ",";
			(*pout_csv) << allWebLang.log_instr[6] << ",";
			(*pout_csv) << allWebLang.log_decl[7] << ",";
			(*pout_csv) << allWebLang.log_instr[7] << ",";
			(*pout_csv) << allWebLang.log_direct[8] << ",";
			(*pout_csv) << allWebLang.log_decl[8] << ",";
			(*pout_csv) << allWebLang.log_instr[8] << ",";
			(*pout_csv) << allWebLang.log_direct[9] << ",";
			(*pout_csv) << allWebLang.log_decl[9] << ",";
			(*pout_csv) << allWebLang.log_instr[9] << ",";
			(*pout_csv) << allWebLang.log_decl[10] << ",";
			(*pout_csv) << allWebLang.log_instr[10] << ",";
			(*pout_csv) << allWebLang.log_decl[11] << ",";
			(*pout_csv) << allWebLang.log_instr[11] << ",";
			(*pout_csv) << allWebLang.log_decl[12] << ",";
			(*pout_csv) << allWebLang.log_instr[12] << ",";
			(*pout_csv) << (allWebLang.log_decl[0] + allWebLang.log_instr[0]) << ",";
			(*pout_csv) << (allWebLang.log_decl[1] + allWebLang.log_instr[1]) << ",";
			(*pout_csv) << (allWebLang.log_decl[2] + allWebLang.log_instr[2]) << ",";
			(*pout_csv) << (allWebLang.log_decl[3] + allWebLang.log_instr[3]) << ",";
			(*pout_csv) << (allWebLang.log_decl[4] + allWebLang.log_instr[4]) << ",";
			(*pout_csv) << (allWebLang.log_decl[5] + allWebLang.log_instr[5]) << ",";
			(*pout_csv) << (allWebLang.log_decl[6] + allWebLang.log_instr[6]) << ",";
			(*pout_csv) << (allWebLang.log_decl[7] + allWebLang.log_instr[7]) << ",";
			(*pout_csv) << (allWebLang.log_direct[8] + allWebLang.log_decl[8] + allWebLang.log_instr[8]) << ",";
			(*pout_csv) << (allWebLang.log_direct[9] + allWebLang.log_decl[9] + allWebLang.log_instr[9]) << ",";
			(*pout_csv) << (allWebLang.log_decl[10] + allWebLang.log_instr[10]) << ",";
			(*pout_csv) << (allWebLang.log_decl[11] + allWebLang.log_instr[11]) << ",";
			(*pout_csv) << (allWebLang.log_decl[12] + allWebLang.log_instr[12]) << ",";
			(*pout_csv) << (allWebLang.log_decl[0] + allWebLang.log_instr[0] +
				allWebLang.log_decl[1] + allWebLang.log_instr[1] +
				allWebLang.log_decl[2] + allWebLang.log_instr[2] +
				allWebLang.log_decl[3] + allWebLang.log_instr[3] +
				allWebLang.log_decl[4] + allWebLang.log_instr[4] + 
				allWebLang.log_decl[5] + allWebLang.log_instr[5] +
				allWebLang.log_decl[6] + allWebLang.log_instr[6] +
				allWebLang.log_decl[7] + allWebLang.log_instr[7] +
				allWebLang.log_direct[8] + allWebLang.log_decl[8] + allWebLang.log_instr[8] +
				allWebLang.log_direct[9] + allWebLang.log_decl[9] + allWebLang.log_instr[9] +
				allWebLang.log_decl[10] + allWebLang.log_instr[10] +
				allWebLang.log_decl[11] + allWebLang.log_instr[11] +
				allWebLang.log_decl[12] + allWebLang.log_instr[12]) << ",";
			(*pout_csv) << "CODE,Logical" << endl;

			// display statistics
			(*pout_csv) << endl << "Number of files successfully accessed,";
			(*pout_csv) << allWebLang.num_of_file;
			(*pout_csv) << ",out of,";

			if (filesToPrint != NULL)
			{
				if (useListA)
					(*pout_csv) << totalNonDupFileA << endl;
				else
					(*pout_csv) << totalNonDupFileB << endl;
			}
			else if (useListA)
				(*pout_csv) << totalFileA << endl;
			else
				(*pout_csv) << totalFileB << endl;

			(*pout_csv) << endl << "Ratio of Physical to Logical SLOC,";

			if (tlsloc > 0)
			{
				tslocrat = (float)tpsloc / (float)tlsloc;
				(*pout_csv).setf(ios::fixed,ios::floatfield);
				(*pout_csv).width(8);
				(*pout_csv).precision(2);
				(*pout_csv) << tslocrat << endl;
				(*pout_csv).unsetf(ios::floatfield);
			}
			else
				(*pout_csv) << "NA" << endl;
		}
	}

	if (print_cmplx)
	{
		string dots = "........................";
		UIntPairVector::iterator idirc;
		UIntPairVector::iterator idatc;
		UIntPairVector::iterator iexec;
		StringVector::iterator idir;
		StringVector::iterator idat;
		StringVector::iterator iexe;

		// display keyword counts
		ClassType ty;
		for (map<int, CCodeCounter*>::iterator iter = ++CounterForEachLanguage.begin(); iter != CounterForEachLanguage.end(); iter++)
		{
			if (iter->second->directive.size() == 0 && iter->second->data_name_list.size() == 0 && iter->second->exec_name_list.size()  == 0)
				continue;
			else if (total.count(iter->second->classtype) == 0)
			{
				if (iter->second->print_cmplx && (excludeFiles ||
					(useListA && iter->second->total_dupFilesA > 0) || (!useListA && iter->second->total_dupFilesB > 0)))
				{
					ty = iter->second->classtype;
					if (ty == PHP || ty == HTML_PHP || ty == JAVASCRIPT_PHP || ty == VBS_PHP ||
						ty == JAVA_JSP || ty == HTML_JSP || ty == JAVASCRIPT_JSP || ty == VBS_JSP ||
						ty == HTML_ASP || ty == JAVASCRIPT_ASP_C || ty == VBS_ASP_C || ty == JAVASCRIPT_ASP_S || ty == VBS_ASP_S || ty == CSHARP_ASP_S ||
						ty == COLDFUSION || ty == CFSCRIPT || ty == HTML_CFM || ty == JAVASCRIPT_CFM || ty == VBS_CFM || ty == SQL_CFM ||
						ty == XML || ty == JAVASCRIPT_XML || ty == VBS_XML || ty == CSHARP_XML ||
						ty == HTML || ty == JAVASCRIPT_HTML || ty == VBS_HTML || ty == CSHARP_HTML)
					{
						if (print_ascii || print_legacy)
							pout = GetTotalOutputStream(outDir + outputFileNamePrePend);
						if (print_csv)
							pout_csv = GetTotalOutputStream(outDir + outputFileNamePrePend, true);
					}
					else
						continue;
				}
				else
					continue;
			}
			else
			{
				if (print_ascii || print_legacy)
					pout = GetTotalOutputStream(outDir + outputFileNamePrePend);
				if (print_csv)
					pout_csv = GetTotalOutputStream(outDir + outputFileNamePrePend, true);
			}

			if (print_ascii || print_legacy)
			{
				(*pout) << endl;
				CUtil::PrintFileHeaderLine(*pout, "TOTAL OCCURRENCES OF " + iter->second->language_name + " KEYWORDS");
				(*pout) << "--------------------------------------------------------------------------------------------------------------" << endl;
				(*pout) << "    Compiler Directives                  Data Keywords                        Executable Keywords" << endl;
				(*pout).setf(ios::right);
			}
			if (print_csv)
			{
				(*pout_csv) << endl;
				CUtil::PrintFileHeaderLine(*pout_csv, "TOTAL OCCURRENCES OF " + iter->second->language_name + " KEYWORDS");
				(*pout_csv) << "Compiler Directives,,Data Keywords,,Executable Keywords" << endl;
			}

			idirc = iter->second->directive_count.begin();
			idatc = iter->second->data_name_count.begin();
			iexec = iter->second->exec_name_count.begin();
			idir = iter->second->directive.begin();
			idat = iter->second->data_name_list.begin();
			iexe = iter->second->exec_name_list.begin();

			while ( (idir != iter->second->directive.end()) || (idat != iter->second->data_name_list.end()) ||
				(iexe != iter->second->exec_name_list.end()) )
			{
				if (idir != iter->second->directive.end())
				{
					if (print_ascii || print_legacy)
					{
						(*pout) << ' ' << *idir << dots.substr(idir->length());
						(*pout).width(7);
						if (filesToPrint != NULL && !excludeFiles)
							(*pout) << (*idirc).second;
						else
							(*pout) << (*idirc).first;
					}
					if (print_csv)
					{
						(*pout_csv) << *idir << ",";
						if (filesToPrint != NULL && !excludeFiles)
							(*pout_csv) << (*idirc).second << ",";
						else
							(*pout_csv) << (*idirc).first << ",";
					}
					idir++;
					idirc++;
				}
				else
				{
					if (print_ascii || print_legacy)
						(*pout) << "                                ";
					if (print_csv)
						(*pout_csv) << ",,";
				}
				if (print_ascii || print_legacy)
					(*pout) << "     ";

				if (idat != iter->second->data_name_list.end())
				{
					if (print_ascii || print_legacy)
					{
						(*pout) << ' ' << *idat << dots.substr(idat->length());
						(*pout).width(7);
						if (filesToPrint != NULL && !excludeFiles)
							(*pout) << (*idatc).second;
						else
							(*pout) << (*idatc).first;
					}
					if (print_csv)
					{
						(*pout_csv) << *idat << ",";
						if (filesToPrint != NULL && !excludeFiles)
							(*pout_csv) << (*idatc).second << ",";
						else
							(*pout_csv) << (*idatc).first << ",";
					}
					idat++;
					idatc++;
				}
				else
				{
					if (print_ascii || print_legacy)
						(*pout) << "                                ";
					if (print_csv)
						(*pout_csv) << ",,";
				}
				if (print_ascii || print_legacy)
					(*pout) << "     ";

				if (iexe != iter->second->exec_name_list.end())
				{
					if (print_ascii || print_legacy)
					{
						(*pout) << ' ' << *iexe << dots.substr(iexe->length());
						(*pout).width(7);
						if (filesToPrint != NULL && !excludeFiles)
							(*pout) << (*iexec).second;
						else
							(*pout) << (*iexec).first;
					}
					if (print_csv)
					{
						(*pout_csv) << *iexe << ",";
						if (filesToPrint != NULL && !excludeFiles)
							(*pout_csv) << (*iexec).second;
						else
							(*pout_csv) << (*iexec).first;
					}
					iexe++;
					iexec++;
				}
				else
				{
					if (print_ascii || print_legacy)
						(*pout) << "                                ";
				}
				if (print_ascii || print_legacy)
					(*pout) << endl;
				if (print_csv)
					(*pout_csv) << endl;
			}
		}
	}

	// close all files
	for (map<int, CCodeCounter*>::iterator iter=CounterForEachLanguage.begin(); iter!=CounterForEachLanguage.end(); iter++)
		iter->second->CloseOutputStream();

	CloseTotalOutputStream();

	// print out language count summary
	if ( !PrintCountSummary( CounterForEachLanguage, total, webtotal, outputFileNamePrePend ) )
		return 0;

	return 1;
}

/*!
* Prints the language counting summary for a set of files.
*
* \param total map of count totals
* \param webtotal map of web language count totals
* \param outputFileNamePrePend name to prepend to the output file
*
* \return method status
*/
int PrintCountSummary( CounterForEachLangType & CounterForEachLanguage,
						TotalValueMap &total, WebTotalValueMap &webtotal,
						const string &outputFileNamePrePend )
{
	ofstream *pout;
	ofstream *pout_csv;
	ClassType class_type;
	WebType webType;
	size_t i;
	unsigned int fCnt, pCnt, lCnt, rCnt, fTot, pTot, lTot;

	if (print_ascii || print_legacy)
	{
		pout = GetOutputSummaryStream(outDir + outputFileNamePrePend);
		if (pout == NULL)
		{
			string err = "Error: Failed to open language count summary output file (";
			err += outputFileNamePrePend + OUTPUT_FILE_SUM;
			err += "). Operation aborted.";
			userIF->AddError(err);
			return 0;
		}

		(*pout) << "Language                      |   Number   |  Physical  |   Logical" << endl;
		(*pout) << "Name                          |  of Files  |    SLOC    |    SLOC" << endl;
		(*pout) << "------------------------------+------------+------------+-------------" << endl;

		rCnt = fTot = pTot = lTot = 0;
		for (map<int, CCodeCounter*>::iterator itso = ++CounterForEachLanguage.begin(); itso != CounterForEachLanguage.end(); itso++)
		{
			class_type = itso->second->classtype;

			if (itso->second->file_extension.size() > 0)
			{
				for (i = 0; i < itso->second->file_extension.size(); i++)
				{
					if (itso->second->file_extension[i].find(".*") == 0)
					{
						class_type = WEB;
						break;
					}
				}
				if (class_type == WEB)
					continue;

				fCnt = total[class_type].num_of_file;
				if (fCnt > 0)
				{
					pCnt = total[class_type].phy_direct + total[class_type].phy_decl + total[class_type].phy_instr;
					lCnt = total[class_type].log_direct + total[class_type].log_decl + total[class_type].log_instr;
					fTot += fCnt;
					pTot += pCnt;
					lTot += lCnt;

					(*pout).setf(ofstream::left);
					(*pout).width(30);
					(*pout) << itso->second->language_name;
					(*pout).unsetf(ofstream::left);
					(*pout) << "|";
					(*pout).setf(ofstream::right);
					(*pout).width(10);
					(*pout) << fCnt;
					(*pout) << "  |";
					(*pout).width(10);
					(*pout) << pCnt;
					(*pout) << "  |";
					(*pout).width(10);
					(*pout) << lCnt << endl;
					(*pout).unsetf(ofstream::right);

					rCnt++;
				}
			}
		}
		for (WebTotalValueMap::iterator itsw = webtotal.begin(); itsw != webtotal.end(); itsw++)
		{
			webType = itsw->first;
			fCnt = webtotal[webType].num_of_file;
			if (fCnt > 0)
			{
				pCnt = 0;
				lCnt = 0;
				for (i = 0; i < 6; i++)
				{
					pCnt += webtotal[webType].phy_decl[i] + webtotal[webType].phy_instr[i];
					lCnt += webtotal[webType].log_decl[i] + webtotal[webType].log_instr[i];
				}
				fTot += fCnt;
				pTot += pCnt;
				lTot += lCnt;

				(*pout).setf(ofstream::left);
				(*pout).width(30);
				(*pout) << CWebCounter::GetWebLangName(webType);
				(*pout).unsetf(ofstream::left);
				(*pout) << "|";
				(*pout).setf(ofstream::right);
				(*pout).width(10);
				(*pout) << fCnt;
				(*pout) << "  |";
				(*pout).width(10);
				(*pout) << pCnt;
				(*pout) << "  |";
				(*pout).width(10);
				(*pout) << lCnt << endl;
				(*pout).unsetf(ofstream::right);

				rCnt++;
			}
		}

		if (rCnt > 0)
		{
			(*pout) << "------------------------------+------------+------------+-------------" << endl;
			(*pout).setf(ofstream::left);
			(*pout).width(30);
			(*pout) << "Total";
			(*pout).unsetf(ofstream::left);
			(*pout) << "|";
			(*pout).setf(ofstream::right);
			(*pout).width(10);
			(*pout) << fTot;
			(*pout) << "  |";
			(*pout).width(10);
			(*pout) << pTot;
			(*pout) << "  |";
			(*pout).width(10);
			(*pout) << lTot << endl;
			(*pout).unsetf(ofstream::right);
		}
	}
	if (print_csv)
	{
		pout_csv = GetOutputSummaryStream(outDir + outputFileNamePrePend, true);
		if (pout_csv == NULL)
		{
			string err = "Error: Failed to open language count summary output file (";
			err += outputFileNamePrePend + OUTPUT_FILE_SUM_CSV;
			err += "). Operation aborted.";
			userIF->AddError(err);
			return 0;
		}

		(*pout_csv) << "Language,Number,Physical,Logical" << endl;
		(*pout_csv) << "Name,of Files,SLOC,SLOC" << endl;

		rCnt = fTot = pTot = lTot = 0;
		for (map<int, CCodeCounter*>::iterator itso = ++CounterForEachLanguage.begin(); itso != CounterForEachLanguage.end(); itso++)
		{
			class_type = itso->second->classtype;

			if (itso->second->file_extension.size() > 0)
			{
				for (i = 0; i < itso->second->file_extension.size(); i++)
				{
					if (itso->second->file_extension[i].find(".*") == 0)
					{
						class_type = WEB;
						break;
					}
				}
				if (class_type == WEB)
					continue;

				fCnt = total[class_type].num_of_file;
				if (fCnt > 0)
				{
					pCnt = total[class_type].phy_direct + total[class_type].phy_decl + total[class_type].phy_instr;
					lCnt = total[class_type].log_direct + total[class_type].log_decl + total[class_type].log_instr;
					fTot += fCnt;
					pTot += pCnt;
					lTot += lCnt;

					(*pout_csv) << itso->second->language_name << "," << fCnt << "," << pCnt << "," << lCnt << endl;

					rCnt++;
				}
			}
		}
		for (WebTotalValueMap::iterator itsw = webtotal.begin(); itsw != webtotal.end(); itsw++)
		{
			webType = itsw->first;
			fCnt = webtotal[webType].num_of_file;
			if (fCnt > 0)
			{
				pCnt = 0;
				lCnt = 0;
				for (i = 0; i < 6; i++)
				{
					pCnt += webtotal[webType].phy_decl[i] + webtotal[webType].phy_instr[i];
					lCnt += webtotal[webType].log_decl[i] + webtotal[webType].log_instr[i];
				}
				fTot += fCnt;
				pTot += pCnt;
				lTot += lCnt;

				(*pout_csv) << CWebCounter::GetWebLangName(webType) << "," << fCnt << "," << pCnt << "," << lCnt << endl;

				rCnt++;
			}
		}

		if (rCnt > 0)
			(*pout_csv) << endl << "Total," << fTot << "," << pTot << "," << lTot << endl;
	}
	CloseOutputSummaryStream();

	return 1;
}


/*!
* Prints the complexity results.
*
* \param useListA use file list A? (otherwise use list B)
* \param outputFileNamePrePend name to prepend to the output file
* \param printDuplicates print duplicate files? (otherwise print unique files)
*
* \return method status
*/
int PrintComplexityResults( CounterForEachLangType & CounterForEachLanguage,
							bool useListA, const string &outputFileNamePrePend, 
							bool printDuplicates ) 
{
	if (useListA)
	{
		if (printDuplicates && duplicateFilesInA2.size() < 1)
			return 1;
	}
	else
	{
		if (printDuplicates && duplicateFilesInB2.size() < 1)
			return 1;
	}

	size_t i;
	string dots = "......................";
	string COL_SP = "                           ";
	string SP_BW_COL = "    ";
	string nestedLoops = "";
	SourceFileList* mySourceFile = (useListA) ? &SourceFileA : &SourceFileB;
	size_t loopCol = 0;

	StringVector::iterator imath;		// math
	StringVector::iterator itrig;		// trig
	StringVector::iterator ilog;		// logarithmic
	StringVector::iterator icalc;		// calculation
	StringVector::iterator icond;		// conditionals
	StringVector::iterator ilogic;		// logical
	StringVector::iterator ipreproc;	// preprocessor
	StringVector::iterator iassign;		// assign
	StringVector::iterator ipointer;	// pointers
	UIntPairVector::iterator imathc;
	UIntPairVector::iterator itrigc;
	UIntPairVector::iterator ilogc;
	UIntPairVector::iterator icalcc;
	UIntPairVector::iterator icondc;
	UIntPairVector::iterator ilogicc;
	UIntPairVector::iterator ipreprocc;
	UIntPairVector::iterator iassignc;
	UIntPairVector::iterator ipointerc;
	results cmplxTotal;

	cmplxTotal.cmplx_math_lines = 0;
	cmplxTotal.cmplx_trig_lines = 0;
	cmplxTotal.cmplx_logarithm_lines = 0;
	cmplxTotal.cmplx_calc_lines = 0;
	cmplxTotal.cmplx_cond_lines = 0;
	cmplxTotal.cmplx_logic_lines = 0;
	cmplxTotal.cmplx_preproc_lines = 0;
	cmplxTotal.cmplx_assign_lines = 0;
	cmplxTotal.cmplx_pointer_lines = 0;

	// check for counts
	for (SourceFileList::iterator itt2 = mySourceFile->begin(); itt2 != mySourceFile->end(); itt2++)
	{
		if ((!printDuplicates && !itt2->second.duplicate) || (printDuplicates && itt2->second.duplicate))
		{
			if (itt2->second.cmplx_nestloop_count.size() > loopCol)
				loopCol = itt2->second.cmplx_nestloop_count.size();
		}
	}

	// open file and print headers
	ofstream cplxOutputFile, cplxOutputFileCSV;
	if (print_ascii || print_legacy)
	{
		string cplxOutputFileName = outDir + outputFileNamePrePend;
		cplxOutputFileName.append(OUTPUT_FILE_CPLX);
		cplxOutputFile.open(cplxOutputFileName.c_str(), ofstream::out);
		if (!cplxOutputFile.is_open())
		{
			string err = "Error: Unable to create file (";
			err += cplxOutputFileName;
			err += "). Operation aborted";
			userIF->AddError(err, false, 1);
			return 0;
		}

		CUtil::PrintFileHeader(cplxOutputFile, "COMPLEXITY COUNT RESULTS", cmdLine);

		if (print_legacy)
		{
			cplxOutputFile << "#| S | Cond  |Logical| Trig  |  Log  |Preproc| Math  |Assign |  Ptr  | Nesting levels              | Filename" << endl;
			cplxOutputFile << "-------------------------------------------------------------------------------------------------------------------------------------" << endl;
		}
		else
		{
			cplxOutputFile << " Math    Trig  Logarithm  Calculation     Cond.   Logical   Preproc.  Assignment   Pointer   ";
			for (i = 1; i <= loopCol; i++)
			{
				cplxOutputFile << "L" << i << "-Loops  ";
				if (i < 10)
					cplxOutputFile << " ";
			}
			cplxOutputFile << "|   File Name" << endl;
			cplxOutputFile << " --------------------------------------------------------------------------------------------";
			for (i = 1; i <= loopCol; i++)
				cplxOutputFile << "-----------";
			cplxOutputFile << "------------------------------------" << endl;
		}
	}
	if (print_csv)
	{
		string cplxOutputFileNameCSV = outDir + outputFileNamePrePend;
		cplxOutputFileNameCSV.append(OUTPUT_FILE_CPLX_CSV);
		cplxOutputFileCSV.open(cplxOutputFileNameCSV.c_str(), ofstream::out);
		if (!cplxOutputFileCSV.is_open())
		{
			string err = "Error: Unable to create file (";
			err += cplxOutputFileNameCSV;
			err += "). Operation aborted";
			userIF->AddError(err, false, 1);
			return 0;
		}

		CUtil::PrintFileHeader(cplxOutputFileCSV, "COMPLEXITY COUNT RESULTS", cmdLine);

		cplxOutputFileCSV << "Math,Trigonometric,Logarithmic,Calculations,Conditionals,Logical,Preprocessor,Assignment,Pointer";
		for (i = 1; i <= loopCol; i++)
			cplxOutputFileCSV << ",L" << i << "-Loops";
		cplxOutputFileCSV << ",File Name" << endl;
	}

	// print counts for each file
	for (SourceFileList::iterator itt2 = mySourceFile->begin(); itt2 != mySourceFile->end(); itt2++)
	{
		if ( itt2->second.file_name_isEmbedded == true )
			continue;

		if ((!printDuplicates && !itt2->second.duplicate) || (printDuplicates && itt2->second.duplicate))
		{
			// do not print temporary files, but sum all of their counts
			SourceFileList::iterator it = itt2;
			for (it++; it != mySourceFile->end(); it++)
			{
				if ( it->second.file_name_isEmbedded == false ) 
					break;
				itt2->second.cmplx_math_lines += it->second.cmplx_math_lines;
				itt2->second.cmplx_trig_lines += it->second.cmplx_trig_lines;
				itt2->second.cmplx_logarithm_lines += it->second.cmplx_logarithm_lines;
				itt2->second.cmplx_calc_lines += it->second.cmplx_calc_lines;
				itt2->second.cmplx_cond_lines += it->second.cmplx_cond_lines;
				itt2->second.cmplx_logic_lines += it->second.cmplx_logic_lines;
				itt2->second.cmplx_preproc_lines += it->second.cmplx_preproc_lines;
				itt2->second.cmplx_assign_lines += it->second.cmplx_assign_lines;
				itt2->second.cmplx_pointer_lines += it->second.cmplx_pointer_lines;

				for (i = 0; i < it->second.cmplx_nestloop_count.size(); i++)
				{
					if (itt2->second.cmplx_nestloop_count.size() < i + 1)
						itt2->second.cmplx_nestloop_count.push_back(it->second.cmplx_nestloop_count[i]);
					else
						itt2->second.cmplx_nestloop_count[i] += it->second.cmplx_nestloop_count[i];
				}
			}

			if (print_legacy)
			{
				if (useListA)
					cplxOutputFile << "A ";
				else
					cplxOutputFile << "B ";
				if (printDuplicates)
					cplxOutputFile << " X ";
				else if (SourceFileB.size() > 0 && !itt2->second.matched)
				{
					if (useListA)
						cplxOutputFile << " D ";
					else
						cplxOutputFile << " N ";
				}
				else
					cplxOutputFile << " C ";
				cplxOutputFile.width(8);	cplxOutputFile << itt2->second.cmplx_cond_lines;
				cplxOutputFile.width(8);	cplxOutputFile << itt2->second.cmplx_logic_lines;
				cplxOutputFile.width(8);	cplxOutputFile << itt2->second.cmplx_trig_lines;
				cplxOutputFile.width(8);	cplxOutputFile << itt2->second.cmplx_logarithm_lines;
				cplxOutputFile.width(8);	cplxOutputFile << itt2->second.cmplx_preproc_lines;
				cplxOutputFile.width(8);	cplxOutputFile << itt2->second.cmplx_math_lines + itt2->second.cmplx_calc_lines;
				cplxOutputFile.width(8);	cplxOutputFile << itt2->second.cmplx_assign_lines;
				cplxOutputFile.width(8);	cplxOutputFile << itt2->second.cmplx_pointer_lines;

				nestedLoops = "";
				if (itt2->second.cmplx_nestloop_count.size() > 0)
				{
					std::stringstream ss;
					ss << itt2->second.cmplx_nestloop_count[0];
					for (i = 1; i < itt2->second.cmplx_nestloop_count.size(); i++)
						ss << "," << itt2->second.cmplx_nestloop_count[i];
					nestedLoops = ss.str();
				}
				cplxOutputFile << "  ";
				cplxOutputFile.width(28);		cplxOutputFile.setf(ios::left);
				cplxOutputFile << nestedLoops;	cplxOutputFile.unsetf(ios::left);
				cplxOutputFile << "  ";
				cplxOutputFile << itt2->second.file_name << endl;
			}
			else if (print_ascii)
			{
				cplxOutputFile.width(5);	cplxOutputFile << itt2->second.cmplx_math_lines;
				cplxOutputFile.width(8);	cplxOutputFile << itt2->second.cmplx_trig_lines;
				cplxOutputFile.width(11);	cplxOutputFile << itt2->second.cmplx_logarithm_lines;
				cplxOutputFile.width(11);	cplxOutputFile << itt2->second.cmplx_calc_lines;
				cplxOutputFile.width(11);	cplxOutputFile << itt2->second.cmplx_cond_lines;
				cplxOutputFile.width(11);	cplxOutputFile << itt2->second.cmplx_logic_lines;
				cplxOutputFile.width(11);	cplxOutputFile << itt2->second.cmplx_preproc_lines;
				cplxOutputFile.width(11);	cplxOutputFile << itt2->second.cmplx_assign_lines;
				cplxOutputFile.width(11);	cplxOutputFile << itt2->second.cmplx_pointer_lines;
				for (i = 0; i < itt2->second.cmplx_nestloop_count.size(); i++)
				{
					cplxOutputFile.width(11);
					cplxOutputFile << itt2->second.cmplx_nestloop_count[i];
				}
				for (i = itt2->second.cmplx_nestloop_count.size(); i < loopCol; i++)
					cplxOutputFile << "          0";
				cplxOutputFile << "       " << itt2->second.file_name << endl;
			}
			if (print_csv)
			{
				cplxOutputFileCSV << itt2->second.cmplx_math_lines << ",";
				cplxOutputFileCSV << itt2->second.cmplx_trig_lines << ",";
				cplxOutputFileCSV << itt2->second.cmplx_logarithm_lines << ",";
				cplxOutputFileCSV << itt2->second.cmplx_calc_lines << ",";
				cplxOutputFileCSV << itt2->second.cmplx_cond_lines << ",";
				cplxOutputFileCSV << itt2->second.cmplx_logic_lines << ",";
				cplxOutputFileCSV << itt2->second.cmplx_preproc_lines << ",";
				cplxOutputFileCSV << itt2->second.cmplx_assign_lines << ",";
				cplxOutputFileCSV << itt2->second.cmplx_pointer_lines << ",";
				for (i = 0; i < itt2->second.cmplx_nestloop_count.size(); i++)
					cplxOutputFileCSV << itt2->second.cmplx_nestloop_count[i] << ",";
				for (i = itt2->second.cmplx_nestloop_count.size(); i < loopCol; i++)
					cplxOutputFileCSV << "0,";
				cplxOutputFileCSV << itt2->second.file_name << endl;
			}

			cmplxTotal.cmplx_math_lines += itt2->second.cmplx_math_lines;
			cmplxTotal.cmplx_trig_lines += itt2->second.cmplx_trig_lines;
			cmplxTotal.cmplx_logarithm_lines += itt2->second.cmplx_logarithm_lines;
			cmplxTotal.cmplx_calc_lines += itt2->second.cmplx_calc_lines;
			cmplxTotal.cmplx_cond_lines += itt2->second.cmplx_cond_lines;
			cmplxTotal.cmplx_logic_lines += itt2->second.cmplx_logic_lines;
			cmplxTotal.cmplx_preproc_lines += itt2->second.cmplx_preproc_lines;
			cmplxTotal.cmplx_assign_lines += itt2->second.cmplx_assign_lines;
			cmplxTotal.cmplx_pointer_lines += itt2->second.cmplx_pointer_lines;

			itt2 = --it;
		}
	}

	if (print_legacy)
	{
		cplxOutputFile << endl << "Note: # represents the baseline; S represents N=New, D=Deleted, C=Common, X=Duplicate" << endl;
	}
	else if (print_ascii)
	{
		cplxOutputFile << endl << endl << " -----------------------------------------------< totals >-----------------------------------------------------------------------";
		for (i = 1; i <= loopCol; i++)
			cplxOutputFile << "-----------";
		cplxOutputFile << endl << endl;

		cplxOutputFile.width(5);	cplxOutputFile << cmplxTotal.cmplx_math_lines;
		cplxOutputFile.width(8);	cplxOutputFile << cmplxTotal.cmplx_trig_lines;
		cplxOutputFile.width(11);	cplxOutputFile << cmplxTotal.cmplx_logarithm_lines;
		cplxOutputFile.width(11);	cplxOutputFile << cmplxTotal.cmplx_calc_lines;
		cplxOutputFile.width(11);	cplxOutputFile << cmplxTotal.cmplx_cond_lines;
		cplxOutputFile.width(11);	cplxOutputFile << cmplxTotal.cmplx_logic_lines;
		cplxOutputFile.width(11);	cplxOutputFile << cmplxTotal.cmplx_preproc_lines;
		cplxOutputFile.width(11);	cplxOutputFile << cmplxTotal.cmplx_assign_lines;
		cplxOutputFile.width(11);	cplxOutputFile << cmplxTotal.cmplx_pointer_lines << endl << endl;
	}
	if (print_csv)
	{
		cplxOutputFileCSV << endl << "Totals" << endl;

		cplxOutputFileCSV << cmplxTotal.cmplx_math_lines << ",";
		cplxOutputFileCSV << cmplxTotal.cmplx_trig_lines << ",";
		cplxOutputFileCSV << cmplxTotal.cmplx_logarithm_lines << ",";
		cplxOutputFileCSV << cmplxTotal.cmplx_calc_lines << ",";
		cplxOutputFileCSV << cmplxTotal.cmplx_cond_lines << ",";
		cplxOutputFileCSV << cmplxTotal.cmplx_logic_lines << ",";
		cplxOutputFileCSV << cmplxTotal.cmplx_preproc_lines << ",";
		cplxOutputFileCSV << cmplxTotal.cmplx_assign_lines << ",";
		cplxOutputFileCSV << cmplxTotal.cmplx_pointer_lines << endl;
	}

	for (map<int, CCodeCounter*>::iterator iter = ++CounterForEachLanguage.begin(); iter != CounterForEachLanguage.end(); iter++)
	{
		// only print for languages found in files
		if (printDuplicates)
		{
			if (iter->second->counted_dupFiles < 1)
				continue;
		}
		else
		{
			if (iter->second->counted_files < 1)
				continue;
		}

		imath = iter->second->math_func_list.begin();
		itrig = iter->second->trig_func_list.begin();
		ilog = iter->second->log_func_list.begin();
		icalc = iter->second->cmplx_calc_list.begin();
		icond = iter->second->cmplx_cond_list.begin();
		ilogic = iter->second->cmplx_logic_list.begin();
		ipreproc = iter->second->cmplx_preproc_list.begin();
		iassign = iter->second->cmplx_assign_list.begin();
		ipointer = iter->second->cmplx_pointer_list.begin();

		imathc = iter->second->math_func_count.begin();
		itrigc = iter->second->trig_func_count.begin();
		ilogc = iter->second->log_func_count.begin();
		icalcc = iter->second->cmplx_calc_count.begin();
		icondc = iter->second->cmplx_cond_count.begin();
		ilogicc = iter->second->cmplx_logic_count.begin();
		ipreprocc = iter->second->cmplx_preproc_count.begin();
		iassignc = iter->second->cmplx_assign_count.begin();
		ipointerc = iter->second->cmplx_pointer_count.begin();

		if ( (imathc != iter->second->math_func_count.end()) ||
			 (itrigc != iter->second->trig_func_count.end()) ||
			 (ilogc  != iter->second->log_func_count.end()) ||
			 (icalcc != iter->second->cmplx_calc_count.end()) ||
			 (icondc != iter->second->cmplx_cond_count.end()) ||
			 (ilogicc != iter->second->cmplx_logic_count.end()) ||
			 (ipreprocc != iter->second->cmplx_preproc_count.end()) ||
			 (iassignc != iter->second->cmplx_assign_count.end()) ||
			 (ipointerc != iter->second->cmplx_pointer_count.end()) )
		{
			if (print_ascii)
			{
				cplxOutputFile << endl;
				CUtil::PrintFileHeaderLine(cplxOutputFile, "TOTAL OCCURRENCES OF " + iter->second->language_name + " COMPLEXITY COUNTS");
				cplxOutputFile << "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;
				cplxOutputFile << "    Math Functions                 Trigonometric                  Logarithmic                    Calculations                   Conditionals                   Logical                        Preprocessor                   Assignment                     Pointer" << endl;
				cplxOutputFile.setf(ios::right);
			}
			if (print_csv)
			{
				cplxOutputFileCSV << endl;
				CUtil::PrintFileHeaderLine(cplxOutputFileCSV, "TOTAL OCCURRENCES OF " + iter->second->language_name + " COMPLEXITY COUNTS");
				cplxOutputFileCSV << "Math Functions,,Trigonometric,,Logarithmic,,Calculations,,Conditionals,,Logical,,Preprocessor,,Assignment,,Pointer" << endl;
			}
		}

		while (	(imathc != iter->second->math_func_count.end()) ||
				(itrigc != iter->second->trig_func_count.end()) ||
				(ilogc  != iter->second->log_func_count.end()) ||
				(icalcc != iter->second->cmplx_calc_count.end()) ||
				(icondc != iter->second->cmplx_cond_count.end()) ||
				(ilogicc != iter->second->cmplx_logic_count.end()) ||
				(ipreprocc != iter->second->cmplx_preproc_count.end()) ||
				(iassignc != iter->second->cmplx_assign_count.end()) ||
				(ipointerc != iter->second->cmplx_pointer_count.end()) )
		{
			if (imath != iter->second->math_func_list.end())  // imath
			{
				if (print_ascii)
				{
					cplxOutputFile << ' ' << *imath << dots.substr(imath->length());
					cplxOutputFile.width(4);
					if (printDuplicates)
						cplxOutputFile << (*imathc).second;
					else
						cplxOutputFile << (*imathc).first;
				}
				if (print_csv)
				{
					cplxOutputFileCSV << *imath << ",";
					if (printDuplicates)
						cplxOutputFileCSV << (*imathc).second;
					else
						cplxOutputFileCSV << (*imathc).first;
				}
				imath++;
				imathc++;
			}
			else
			{
				if (print_ascii)
					cplxOutputFile << COL_SP;
				if (print_csv)
					cplxOutputFileCSV << ",";
			}
			if (print_ascii)
				cplxOutputFile << SP_BW_COL;

			if (itrig != iter->second->trig_func_list.end())  // itrig
			{
				if (print_ascii)
				{
					cplxOutputFile << ' ' << *itrig << dots.substr(itrig->length());
					cplxOutputFile.width(4);
					if (printDuplicates)
						cplxOutputFile << (*itrigc).second;
					else
						cplxOutputFile << (*itrigc).first;
				}
				if (print_csv)
				{
					cplxOutputFileCSV << "," << *itrig << ",";
					if (printDuplicates)
						cplxOutputFileCSV << (*itrigc).second;
					else
						cplxOutputFileCSV << (*itrigc).first;
				}
				itrig++;
				itrigc++;
			}
			else
			{
				if (print_ascii)
					cplxOutputFile << COL_SP;
				if (print_csv)
					cplxOutputFileCSV << ",,";
			}
			if (print_ascii)
				cplxOutputFile << SP_BW_COL;

			if (ilog != iter->second->log_func_list.end())   // ilog
			{
				if (print_ascii)
				{
					cplxOutputFile << ' ' << *ilog << dots.substr(ilog->length());
					cplxOutputFile.width(4);
					if (printDuplicates)
						cplxOutputFile << (*ilogc).second;
					else
						cplxOutputFile << (*ilogc).first;
				}
				if (print_csv)
				{
					cplxOutputFileCSV << "," << *ilog << ",";
					if (printDuplicates)
						cplxOutputFileCSV << (*ilogc).second;
					else
						cplxOutputFileCSV << (*ilogc).first;
				}
				ilog++;
				ilogc++;
			}
			else
			{
				if (print_ascii)
					cplxOutputFile << COL_SP;
				if (print_csv)
					cplxOutputFileCSV << ",,";
			}
			if (print_ascii)
				cplxOutputFile << SP_BW_COL;

			if (icalc != iter->second->cmplx_calc_list.end())   // icalc
			{
				if (print_ascii)
				{
					cplxOutputFile << ' ' << *icalc << dots.substr(icalc->length());
					cplxOutputFile.width(4);
					if (printDuplicates)
						cplxOutputFile << (*icalcc).second;
					else
						cplxOutputFile << (*icalcc).first;
				}
				if (print_csv)
				{
					cplxOutputFileCSV << "," << *icalc << ",";
					if (printDuplicates)
						cplxOutputFileCSV << (*icalcc).second;
					else
						cplxOutputFileCSV << (*icalcc).first;
				}
				icalc++;
				icalcc++;
			}
			else
			{
				if (print_ascii)
					cplxOutputFile << COL_SP;
				if (print_csv)
					cplxOutputFileCSV << ",,";
			}
			if (print_ascii)
				cplxOutputFile << SP_BW_COL;

			if (icond != iter->second->cmplx_cond_list.end())   // icond
			{
				if (print_ascii)
				{
					cplxOutputFile << ' ' << *icond << dots.substr(icond->length());
					cplxOutputFile.width(4);
					if (printDuplicates)
						cplxOutputFile << (*icondc).second;
					else
						cplxOutputFile << (*icondc).first;
				}
				if (print_csv)
				{
					cplxOutputFileCSV << "," << *icond << ",";
					if (printDuplicates)
						cplxOutputFileCSV << (*icondc).second;
					else
						cplxOutputFileCSV << (*icondc).first;
				}
				icond++;
				icondc++;
			}
			else
			{
				if (print_ascii)
					cplxOutputFile << COL_SP;
				if (print_csv)
					cplxOutputFileCSV << ",,";
			}
			if (print_ascii)
				cplxOutputFile << SP_BW_COL;

			if (ilogic != iter->second->cmplx_logic_list.end())   // ilogic
			{
				if (print_ascii)
				{
					cplxOutputFile << ' ' << *ilogic << dots.substr(ilogic->length());
					cplxOutputFile.width(4);
					if (printDuplicates)
						cplxOutputFile << (*ilogicc).second;
					else
						cplxOutputFile << (*ilogicc).first;
				}
				if (print_csv)
				{
					cplxOutputFileCSV << "," << *ilogic << ",";
					if (printDuplicates)
						cplxOutputFileCSV << (*ilogicc).second;
					else
						cplxOutputFileCSV << (*ilogicc).first;
				}
				ilogic++;
				ilogicc++;
			}
			else
			{
				if (print_ascii)
					cplxOutputFile << COL_SP;
				if (print_csv)
					cplxOutputFileCSV << ",,";
			}
			if (print_ascii)
				cplxOutputFile << SP_BW_COL;

			if (ipreproc != iter->second->cmplx_preproc_list.end())   // ipreproc
			{
				if (print_ascii)
				{
					cplxOutputFile << ' ' << *ipreproc << dots.substr(ipreproc->length());
					cplxOutputFile.width(4);
					if (printDuplicates)
						cplxOutputFile << (*ipreprocc).second;
					else
						cplxOutputFile << (*ipreprocc).first;
				}
				if (print_csv)
				{
					cplxOutputFileCSV << "," << *ipreproc << ",";
					if (printDuplicates)
						cplxOutputFileCSV << (*ipreprocc).second;
					else
						cplxOutputFileCSV << (*ipreprocc).first;
				}
				ipreproc++;
				ipreprocc++;
			}
			else
			{
				if (print_ascii)
					cplxOutputFile << COL_SP;
				if (print_csv)
					cplxOutputFileCSV << ",,";
			}
			if (print_ascii)
				cplxOutputFile << SP_BW_COL;

			if (iassign != iter->second->cmplx_assign_list.end())   // ipreproc
			{
				if (print_ascii)
				{
					cplxOutputFile << ' ' << *iassign << dots.substr(iassign->length());
					cplxOutputFile.width(4);
					if (printDuplicates)
						cplxOutputFile << (*iassignc).second;
					else
						cplxOutputFile << (*iassignc).first;
				}
				if (print_csv)
				{
					cplxOutputFileCSV << "," << *iassign << ",";
					if (printDuplicates)
						cplxOutputFileCSV << (*iassignc).second;
					else
						cplxOutputFileCSV << (*iassignc).first;
				}
				iassign++;
				iassignc++;
			}
			else
			{
				if (print_ascii)
					cplxOutputFile << COL_SP;
				if (print_csv)
					cplxOutputFileCSV << ",,";
			}
			if (print_ascii)
				cplxOutputFile << SP_BW_COL;

			if (ipointer != iter->second->cmplx_pointer_list.end())   // RST Pointers
			{
				if (print_ascii)
				{
					cplxOutputFile << ' ' << *ipointer << dots.substr(ipointer->length());
					cplxOutputFile.width(4);
					if (printDuplicates)
						cplxOutputFile << (*ipointerc).second;
					else
						cplxOutputFile << (*ipointerc).first;
				}
				if (print_csv)
				{
					cplxOutputFileCSV << "," << *ipointer << ",";
					if (printDuplicates)
						cplxOutputFileCSV << (*ipointerc).second;
					else
						cplxOutputFileCSV << (*ipointerc).first;
				}
				ipointer++;
				ipointerc++;
			}
			else
			{
				if (print_ascii)
					cplxOutputFile << COL_SP;
			}
			if (print_ascii)
				cplxOutputFile << endl;
			if (print_csv)
				cplxOutputFileCSV << endl;
		}
	}
	if (print_ascii || print_legacy)
		cplxOutputFile.close();
	if (print_csv)
		cplxOutputFileCSV.close();

	// print cyclomatic complexity
	PrintCyclomaticComplexity(useListA, outputFileNamePrePend, printDuplicates);

	return 1;
}

/*!
* Prints the cyclomatic complexity results.
*
* \param useListA use file list A? (otherwise use list B)
* \param outputFileNamePrePend name to prepend to the output file
* \param printDuplicates print duplicate files? (otherwise print unique files)
*
* \return method status
*/
int PrintCyclomaticComplexity(const bool useListA, const string &outputFileNamePrePend, const bool printDuplicates) 
{
	if (useListA)
	{
		if (printDuplicates && duplicateFilesInA2.size() < 1)
			return 1;
	}
	else
	{
		if (printDuplicates && duplicateFilesInB2.size() < 1)
			return 1;
	}

	SourceFileList* mySourceFile = (useListA) ? &SourceFileA : &SourceFileB;
	bool found = false;

	// check for counts
	for (SourceFileList::iterator itt2 = mySourceFile->begin(); itt2 != mySourceFile->end(); itt2++)
	{
		if ((!printDuplicates && !itt2->second.duplicate) || (printDuplicates && itt2->second.duplicate))
		{
			if (itt2->second.cmplx_cycfunct_count.size() > 0)
			{
				found = true;
				break;
			}
		}
	}
	if (!found)
		return 1;

	// open file and print headers
	ofstream cycCplxOutputFile, cycCplxOutputFileCSV;
	if (print_ascii || print_legacy)
	{
		string cycCplxOutputFileName = outDir + outputFileNamePrePend;
		cycCplxOutputFileName.append(OUTPUT_FILE_CYC_CPLX);
		cycCplxOutputFile.open(cycCplxOutputFileName.c_str(), ofstream::out);
		if (!cycCplxOutputFile.is_open())
		{
			string err = "Error: Unable to create file (";
			err += cycCplxOutputFileName;
			err += "). Operation aborted";
			userIF->AddError(err, false, 1);
			return 0;
		}

		CUtil::PrintFileHeader(cycCplxOutputFile, "CYCLOMATIC COMPLEXITY RESULTS", cmdLine);

		CUtil::PrintFileHeaderLine(cycCplxOutputFile, "RESULTS BY FILE");
		cycCplxOutputFile << endl;
		cycCplxOutputFile << "Cyclomatic Complexity               |   File" << endl;
		cycCplxOutputFile << "     Total   Average    Risk        |   Name" << endl;
		cycCplxOutputFile << "------------------------------------+-----------------------" << endl;
	}
	if (print_csv)
	{
		string cycCplxOutputFileNameCSV = outDir + outputFileNamePrePend;
		cycCplxOutputFileNameCSV.append(OUTPUT_FILE_CYC_CPLX_CSV);
		cycCplxOutputFileCSV.open(cycCplxOutputFileNameCSV.c_str(), ofstream::out);
		if (!cycCplxOutputFileCSV.is_open())
		{
			string err = "Error: Unable to create file (";
			err += cycCplxOutputFileNameCSV;
			err += "). Operation aborted";
			userIF->AddError(err, false, 1);
			return 0;
		}

		CUtil::PrintFileHeader(cycCplxOutputFileCSV, "CYCLOMATIC COMPLEXITY RESULTS", cmdLine);

		CUtil::PrintFileHeaderLine(cycCplxOutputFileCSV, "RESULTS BY FILE");
		cycCplxOutputFileCSV << endl;
		cycCplxOutputFileCSV << "Cyclomatic Complexity" << endl;
		cycCplxOutputFileCSV << "Total,Average,Risk,File Name" << endl;
	}

	// print cyclomatic complexity for each file
	unsigned int fcc, tcc, nffunc, ntfunc;
	float afcc, atcc;
	tcc = ntfunc = 0;
	for (SourceFileList::iterator itt2 = mySourceFile->begin(); itt2 != mySourceFile->end(); itt2++)
	{
		if ( itt2->second.file_name_isEmbedded == true )
			continue;

		fcc = nffunc = 0;
		afcc = 0.;
		if ((!printDuplicates && !itt2->second.duplicate) || (printDuplicates && itt2->second.duplicate))
		{
			filemap::iterator it;
			if (itt2->second.class_type == WEB)
			{
				// sum embedded web files
				SourceFileList::iterator startpos = itt2;
				SourceFileList::iterator endpos = ++startpos;
				for (; endpos != mySourceFile->end(); endpos++)
				{
					if ( endpos->second.file_name_isEmbedded == false )
						break;
					for (it = endpos->second.cmplx_cycfunct_count.begin(); it != endpos->second.cmplx_cycfunct_count.end(); it++)
					{
						fcc += (*it).lineNumber;
						nffunc++;
					}
				}
			}
			else
			{
				for (it = itt2->second.cmplx_cycfunct_count.begin(); it != itt2->second.cmplx_cycfunct_count.end(); it++)
				{
					fcc += (*it).lineNumber;
					nffunc++;
				}
			}
			if (fcc < 1)
				continue;

			tcc += fcc;
			ntfunc += nffunc;
			if (nffunc > 0)
				afcc = (float)fcc / (float)nffunc;

			if (print_ascii || print_legacy)
			{
				cycCplxOutputFile.setf(ofstream::right);
				cycCplxOutputFile.width(10);
				cycCplxOutputFile << fcc;
				cycCplxOutputFile.setf(ios::fixed,ios::floatfield);
				cycCplxOutputFile.width(10);
				cycCplxOutputFile.precision(2);
				cycCplxOutputFile << afcc;
				cycCplxOutputFile.unsetf(ofstream::right);
				cycCplxOutputFile << "    ";
				cycCplxOutputFile.setf(ofstream::left);
				cycCplxOutputFile.width(9);
				if (afcc <= 10.)
					cycCplxOutputFile << "Low";
				else if (afcc <= 20.)
					cycCplxOutputFile << "Medium";
				else if (afcc <= 50.)
					cycCplxOutputFile << "High";
				else
					cycCplxOutputFile << "Very High";
				cycCplxOutputFile.unsetf(ofstream::right);
				cycCplxOutputFile.setf(ofstream::left);
				cycCplxOutputFile << "   |   " << itt2->second.file_name << endl;
			}
			if (print_csv)
			{
				cycCplxOutputFileCSV << fcc << ",";
				cycCplxOutputFileCSV.setf(ios::fixed,ios::floatfield);
				cycCplxOutputFileCSV.width(10);
				cycCplxOutputFileCSV.precision(2);
				cycCplxOutputFileCSV << afcc << ",";
				if (afcc <= 10.)
					cycCplxOutputFileCSV << "Low,";
				else if (afcc <= 20.)
					cycCplxOutputFileCSV << "Medium,";
				else if (afcc <= 50.)
					cycCplxOutputFileCSV << "High,";
				else
					cycCplxOutputFileCSV << "Very High,";
				cycCplxOutputFileCSV << itt2->second.file_name << endl;
			}
		}
	}

	// print total and average cyclomatic complexity
	atcc = 0.;
	if (ntfunc > 0)
		atcc = (float)tcc / (float)ntfunc;
	if (print_ascii || print_legacy)
	{
		cycCplxOutputFile << "------------------------------------+-----------------------" << endl;
		cycCplxOutputFile.setf(ofstream::right);
		cycCplxOutputFile.width(10);
		cycCplxOutputFile << tcc;
		cycCplxOutputFile.setf(ios::fixed,ios::floatfield);
		cycCplxOutputFile.width(10);
		cycCplxOutputFile.precision(2);
		cycCplxOutputFile << atcc << endl << endl;
	}
	if (print_csv)
	{
		cycCplxOutputFileCSV << endl;
		cycCplxOutputFileCSV << tcc << "," << atcc << endl << endl;
	}
	
	if (print_ascii || print_legacy)
	{
		CUtil::PrintFileHeaderLine(cycCplxOutputFile, "RESULTS BY FUNCTION");
		cycCplxOutputFile << endl;
		cycCplxOutputFile << "Cyclomatic   Risk        Function                                                       |   File" << endl;
		cycCplxOutputFile << "Complexity               Name                                                           |   Name" << endl;
		cycCplxOutputFile << "----------------------------------------------------------------------------------------+-----------------------" << endl;
	}
	if (print_csv)
	{
		CUtil::PrintFileHeaderLine(cycCplxOutputFileCSV, "RESULTS BY FUNCTION");
		cycCplxOutputFileCSV << endl;
		cycCplxOutputFileCSV << "Cyclomatic Complexity,Risk,Function Name,File Name" << endl;
	}

	// print cyclomatic complexity for each file by function
	for (SourceFileList::iterator itt2 = mySourceFile->begin(); itt2 != mySourceFile->end(); itt2++)
	{
		if ( itt2->second.file_name_isEmbedded == true )
			continue;

		if ((!printDuplicates && !itt2->second.duplicate) || (printDuplicates && itt2->second.duplicate))
		{
			filemap::iterator it;
			if (itt2->second.class_type == WEB)
			{
				// sum embedded web files
				SourceFileList::iterator startpos = itt2;
				SourceFileList::iterator endpos = ++startpos;
				if (print_ascii || print_legacy)
				{
					for (; endpos != mySourceFile->end(); endpos++)
					{
						if ( endpos->second.file_name_isEmbedded == false )
							break;
						for (it = endpos->second.cmplx_cycfunct_count.begin(); it != endpos->second.cmplx_cycfunct_count.end(); it++)
						{
							cycCplxOutputFile.setf(ofstream::right);
							cycCplxOutputFile.width(10);
							cycCplxOutputFile << (*it).lineNumber;
							cycCplxOutputFile.unsetf(ofstream::right);
							cycCplxOutputFile << "   ";
							cycCplxOutputFile.setf(ofstream::left);
							cycCplxOutputFile.width(9);
							if ((*it).lineNumber <= 10)
								cycCplxOutputFile << "Low";
							else if ((*it).lineNumber <= 20)
								cycCplxOutputFile << "Medium";
							else if ((*it).lineNumber <= 50)
								cycCplxOutputFile << "High";
							else
								cycCplxOutputFile << "Very High";
							cycCplxOutputFile.unsetf(ofstream::right);
							cycCplxOutputFile.setf(ofstream::left);
							cycCplxOutputFile << "   ";
							cycCplxOutputFile.width(60);
							cycCplxOutputFile << (*it).line;
							cycCplxOutputFile << "   |   " << itt2->second.file_name << endl;
						}
					}
				}
				if (print_csv)
				{
					for (; endpos != mySourceFile->end(); endpos++)
					{
						if ( endpos->second.file_name_isEmbedded == false )
							break;
						for (it = endpos->second.cmplx_cycfunct_count.begin(); it != endpos->second.cmplx_cycfunct_count.end(); it++)
						{
							cycCplxOutputFileCSV << (*it).lineNumber << ",";
							if ((*it).lineNumber <= 10)
								cycCplxOutputFileCSV << "Low,";
							else if ((*it).lineNumber <= 20)
								cycCplxOutputFileCSV << "Medium,";
							else if ((*it).lineNumber <= 50)
								cycCplxOutputFileCSV << "High,";
							else
								cycCplxOutputFileCSV << "Very High,";
							cycCplxOutputFileCSV << (*it).line << "," << itt2->second.file_name << endl;
						}
					}
				}
			}
			else
			{
				if (print_ascii || print_legacy)
				{
					for (it = itt2->second.cmplx_cycfunct_count.begin(); it != itt2->second.cmplx_cycfunct_count.end(); it++)
					{
						cycCplxOutputFile.setf(ofstream::right);
						cycCplxOutputFile.width(10);
						cycCplxOutputFile << (*it).lineNumber;
						cycCplxOutputFile.unsetf(ofstream::right);
						cycCplxOutputFile << "   ";
						cycCplxOutputFile.setf(ofstream::left);
						cycCplxOutputFile.width(9);
						if ((*it).lineNumber <= 10)
							cycCplxOutputFile << "Low";
						else if ((*it).lineNumber <= 20)
							cycCplxOutputFile << "Medium";
						else if ((*it).lineNumber <= 50)
							cycCplxOutputFile << "High";
						else
							cycCplxOutputFile << "Very High";
						cycCplxOutputFile.unsetf(ofstream::right);
						cycCplxOutputFile.setf(ofstream::left);
						cycCplxOutputFile << "   ";
						cycCplxOutputFile.width(60);
						cycCplxOutputFile << (*it).line;
						cycCplxOutputFile << "   |   " << itt2->second.file_name << endl;
					}
				}
				if (print_csv)
				{
					for (it = itt2->second.cmplx_cycfunct_count.begin(); it != itt2->second.cmplx_cycfunct_count.end(); it++)
					{
						cycCplxOutputFileCSV << (*it).lineNumber << ",";
						if ((*it).lineNumber <= 10)
							cycCplxOutputFileCSV << "Low,";
						else if ((*it).lineNumber <= 20)
							cycCplxOutputFileCSV << "Medium,";
						else if ((*it).lineNumber <= 50)
							cycCplxOutputFileCSV << "High,";
						else
							cycCplxOutputFileCSV << "Very High,";
						cycCplxOutputFileCSV << (*it).line << "," << itt2->second.file_name << endl;
					}
				}
			}
		}
	}

	// print total and average cyclomatic complexity
	if (print_ascii || print_legacy)
	{
		cycCplxOutputFile << "----------------------------------------------------------------------------------------+-----------------------" << endl;
		cycCplxOutputFile.setf(ofstream::right);
		cycCplxOutputFile.width(10);
		cycCplxOutputFile << tcc;
		cycCplxOutputFile.unsetf(ofstream::right);
		cycCplxOutputFile << "   Total" << endl;
		cycCplxOutputFile.setf(ofstream::right);
		cycCplxOutputFile.setf(ios::fixed,ios::floatfield);
		cycCplxOutputFile.width(10);
		cycCplxOutputFile.precision(2);
		cycCplxOutputFile << atcc;
		cycCplxOutputFile.unsetf(ios::floatfield);
		cycCplxOutputFile << "   Average";
	}
	if (print_csv)
	{
		cycCplxOutputFileCSV << endl;
		cycCplxOutputFileCSV << tcc << ",Total" << endl;		
		cycCplxOutputFileCSV.setf(ios::fixed,ios::floatfield);
		cycCplxOutputFileCSV.width(10);
		cycCplxOutputFileCSV.precision(2);
		cycCplxOutputFileCSV << atcc;
		cycCplxOutputFileCSV.unsetf(ios::floatfield);
		cycCplxOutputFileCSV << ",Average" << endl;
	}

	if (print_ascii || print_legacy)
		cycCplxOutputFile.close();
	if (print_csv)
		cycCplxOutputFileCSV.close();

	return 1;
}

/*!
* Prints the duplicate summary output file.
* Shows which files were duplicates, regardless of file name/path.
*
* \param useListA use file list A? (otherwise use list B)
* \param outputFileNamePrePend name to prepend to the output file
*/
void PrintDuplicateSummary(const bool useListA, const string &outputFileNamePrePend)
{
	if (useListA)
	{
		if (duplicateFilesInA2.size() < 1)
			return;
	}
	else
	{
		if (duplicateFilesInB2.size() < 1)
			return;
	}

	if (print_ascii || print_legacy)
	{
		ofstream dupFile;
		string dupFileName = outDir + outputFileNamePrePend;
		dupFileName.append(DUP_PAIRS_OUTFILE);
		dupFile.open(dupFileName.c_str(), ofstream::out);
		if (!dupFile.is_open())
		{
			string err = "Error: Failed to open duplicate file pair summary output file (";
			err += dupFileName;
			err += ")";
			userIF->AddError(err);
			return;
		}

		CUtil::PrintFileHeader(dupFile, "DUPLICATE FILE PAIRS", cmdLine);
		dupFile.setf(ofstream::left);
		dupFile.width(45);
		dupFile << "Original";
		dupFile.unsetf(ofstream::left);
		dupFile.width(5);
		dupFile << "  |  ";
		dupFile.width(3);
		dupFile.setf(ofstream::left);
		dupFile.width(45);
		dupFile << "Duplicate";
		dupFile << endl;
		if (useListA)
			PrintDuplicateList(duplicateFilesInA1, duplicateFilesInA2, dupFile);
		else
			PrintDuplicateList(duplicateFilesInB1, duplicateFilesInB2, dupFile);
		dupFile.close();
	}
	if (print_csv)
	{
		ofstream dupFileCSV;
		string dupFileNameCSV = outDir + outputFileNamePrePend;
		dupFileNameCSV.append(DUP_PAIRS_OUTFILE_CSV);
		dupFileCSV.open(dupFileNameCSV.c_str(), ofstream::out);
		if (!dupFileCSV.is_open())
		{
			string err = "Error: Failed to open duplicate file pair summary output file (";
			err += dupFileNameCSV;
			err += ")";
			userIF->AddError(err);
			return;
		}

		CUtil::PrintFileHeader(dupFileCSV, "DUPLICATE FILE PAIRS", cmdLine);
		dupFileCSV << "Original,Duplicate" << endl;
		if (useListA)
			PrintDuplicateList(duplicateFilesInA1, duplicateFilesInA2, dupFileCSV, true);
		else
			PrintDuplicateList(duplicateFilesInB1, duplicateFilesInB2, dupFileCSV, true);
		dupFileCSV.close();
	}
}

/*!
* Prints the duplicate file list.
*
* \param myList1 original files list
* \param myList2 duplicate files list
* \param outfile output file stream
* \param csvFormat print CSV file format
*/
void PrintDuplicateList(StringVector &myList1, StringVector &myList2, ofstream &outfile, const bool csvFormat)
{
	if (myList1.size() > 0)
	{
		if (csvFormat)
		{
			StringVector::iterator myI2 = myList2.begin();
			for (StringVector::iterator myI1 = myList1.begin(); myI1 != myList1.end(); myI1++, myI2++)
				outfile << (*myI1) << "," << (*myI2) << endl;
		}
		else
		{
			for (int y = 0; y < 90; y++)
				outfile << "-";
			outfile << endl;
			StringVector::iterator myI2 = myList2.begin();
			for (StringVector::iterator myI1 = myList1.begin(); myI1 != myList1.end(); myI1++, myI2++)
			{
				outfile.setf(ofstream::left);
				outfile.width(45);
				outfile << (*myI1);
				outfile.unsetf(ofstream::left);
				outfile.width(5);
				outfile << "  |  ";
				outfile.setf(ofstream::left);
				outfile.width(45);
				outfile << (*myI2) << endl;
			}
		}
	}
}

/*!
* Retrieves the total languages output file stream.
* Opens a new stream if it has not been opened already.
*
* \param outputFileNamePrePend name to prepend to the output file
* \param csvOutput CSV file stream? (otherwise ASCII text file)
*
* \return output file stream
*/
ofstream* GetTotalOutputStream(const string &outputFileNamePrePend, const bool csvOutput)
{
	if (csvOutput)
	{
		if (!output_file_csv.is_open())
		{
			string fname = outputFileNamePrePend + "TOTAL" + OUTPUT_FILE_NAME_CSV;
			output_file_csv.open(fname.c_str(), ofstream::out);

			if (!output_file_csv.is_open()) return NULL;

			CUtil::PrintFileHeader(output_file_csv, "SLOC COUNT RESULTS FOR ALL SOURCE FILES", cmdLine);
		}
		return &output_file_csv;
	}
	else
	{
		if (!output_file.is_open())
		{
			string fname = outputFileNamePrePend + "TOTAL" + OUTPUT_FILE_NAME;
			output_file.open(fname.c_str(), ofstream::out);

			if (!output_file.is_open()) return NULL;

			CUtil::PrintFileHeader(output_file, "SLOC COUNT RESULTS FOR ALL SOURCE FILES", cmdLine);
		}
		return &output_file;
	}
}

/*!
* Retrieves the output summary file stream.
* Opens a new stream if it has not been opened already.
*
* \param outputFileNamePrePend name to prepend to the output file
* \param csvOutput CSV file stream? (otherwise ASCII text file)
*
* \return output summary file stream
*/
ofstream* GetOutputSummaryStream(const string &outputFileNamePrePend, const bool csvOutput)
{
	if (csvOutput)
	{
		if (!output_summary_csv.is_open())
		{
			string fname = outputFileNamePrePend + OUTPUT_FILE_SUM_CSV;
			output_summary_csv.open(fname.c_str(), ofstream::out);

			if (!output_summary_csv.is_open()) return NULL;

			CUtil::PrintFileHeader(output_summary_csv, "LANGUAGE COUNT SUMMARY", cmdLine);
		}
		return &output_summary_csv;
	}
	else
	{
		if (!output_summary.is_open())
		{
			string fname = outputFileNamePrePend + OUTPUT_FILE_SUM;
			output_summary.open(fname.c_str(), ofstream::out);

			if (!output_summary.is_open()) return NULL;

			CUtil::PrintFileHeader(output_summary, "LANGUAGE COUNT SUMMARY", cmdLine);
		}
		return &output_summary;
	}
}

/*!
* Closes the output summary file stream.
*/
void CloseOutputSummaryStream()
{
	if (output_summary.is_open())
		output_summary.close();
	if (output_summary_csv.is_open())
		output_summary_csv.close();
}

/*!
* Closes the total languages output file stream.
*/
void CloseTotalOutputStream()
{
	if (output_file.is_open())
		output_file.close();
	if (output_file_csv.is_open())
		output_file_csv.close();
}
