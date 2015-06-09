//! Code counter class definition for the C# language.
/*!
* \file CCsharpCounter.h
*
* This file contains the code counter class definition for the C# language.
*/

#ifndef CCsharpCounter_h
#define CCsharpCounter_h

#include "CCJavaCsCounter.h"

//! C# code counter class.
/*!
* \class CCsharpCounter
*
* Defines the C# code counter class.
*/
class CCsharpCounter : public CCJavaCsCounter
{
public:
	CCsharpCounter();

protected:
	virtual int PreCountProcess(filemap* fmap);
	virtual int ReplaceQuote(string &strline, size_t &idx_start, bool &contd, char &CurrentQuoteEnd);

private:
	bool isVerbatim;
};

//! C# in HTML code counter class.
/*!
* \class CCsharpHtmlCounter
*
* Defines the C# in HTML code counter class.
*/
class CCsharpHtmlCounter : public CCsharpCounter
{
public:
	CCsharpHtmlCounter();
};

//! C# in XML code counter class.
/*!
* \class CCsharpXmlCounter
*
* Defines the C# in XML code counter class.
*/
class CCsharpXmlCounter : public CCsharpCounter
{
public:
	CCsharpXmlCounter();
};

//! C# in ASP code counter class.
/*!
* \class CCsharpAspCounter
*
* Defines the C# in ASP code counter class.
*/
class CCsharpAspCounter : public CCsharpCounter
{
public:
	CCsharpAspCounter();
};

#endif
