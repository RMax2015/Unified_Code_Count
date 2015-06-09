//! Code counter class definition for the VBScript language.
/*!
* \file CVbscriptCounter.h
*
* This file contains the code counter class definition for the VBScript language.
*/

#ifndef CVbscriptCounter_h
#define CVbscriptCounter_h

#include "CVbCounter.h"

//! Visual Basic code counter class.
/*!
* \class CVbscriptCounter
*
* Defines the Visual Basic code counter class.
*/
class CVbscriptCounter : public CVbCounter
{
public:
	CVbscriptCounter();
};

//! VBScript in PHP code counter class.
/*!
* \class CVbsPhpCounter
*
* Defines the VBScript in PHP code counter class.
*/
class CVbsPhpCounter : public CVbscriptCounter
{
public:
	CVbsPhpCounter();
};

//! VBScript in HTML code counter class.
/*!
* \class CVbsHtmlCounter
*
* Defines the VBScript in HTML code counter class.
*/
class CVbsHtmlCounter : public CVbscriptCounter
{
public:
	CVbsHtmlCounter();
};

//! VBScript in XML code counter class.
/*!
* \class CVbsXmlCounter
*
* Defines the VBScript in XML code counter class.
*/
class CVbsXmlCounter : public CVbscriptCounter
{
public:
	CVbsXmlCounter();
};

//! VBScript in JSP code counter class.
/*!
* \class CVbsJspCounter
*
* Defines the VBScript in JSP code counter class.
*/
class CVbsJspCounter : public CVbscriptCounter
{
public:
	CVbsJspCounter();
};

//! VBScript in ASP server code counter class.
/*!
* \class CVbsAspServerCounter
*
* Defines the VBScript in ASP server code counter class.
*/
class CVbsAspServerCounter : public CVbscriptCounter
{
public:
	CVbsAspServerCounter();
};

//! VBScript in ASP client code counter class.
/*!
* \class CVbsAspClientCounter
*
* Defines the VBScript in ASP client code counter class.
*/
class CVbsAspClientCounter : public CVbscriptCounter
{
public:
	CVbsAspClientCounter();
};

//! VBScript in ColdFusion code counter class.
/*!
* \class CVbsColdFusionCounter
*
* Defines the VBScript in ColdFusion code counter class.
*/
class CVbsColdFusionCounter : public CVbscriptCounter
{
public:
	CVbsColdFusionCounter();
};

#endif
