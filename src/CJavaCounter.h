//! Code counter class definition for the Java language.
/*!
* \file CJavaCounter.h
*
* This file contains the code counter class definition for the Java language.
*/

#ifndef CJavaCounter_h
#define CJavaCounter_h

#include "CCJavaCsCounter.h"

//! Java code counter class.
/*!
* \class CJavaCounter
*
* Defines the Java code counter class.
*/
class CJavaCounter : public CCJavaCsCounter
{
public:
	CJavaCounter();
};

//! Java in JSP code counter class.
/*!
* \class CJavaJspCounter
*
* Defines the Java in JSP code counter class.
*/
class CJavaJspCounter : public CJavaCounter
{
public:
	CJavaJspCounter();
};

#endif
