//! Code counter class definition for the HTML language.
/*!
* \file CHtmlCounter.h
*
* This file contains the code counter class definition for the HTML language.
*/

#ifndef CHtmlCounter_h
#define CHtmlCounter_h

#include "CTagCounter.h"

//! HTML code counter class.
/*!
* \class CHtmlCounter
*
* Defines the HTML code counter class.
*/
class CHtmlCounter : public CTagCounter
{
public:
	CHtmlCounter();
};

//! HTML in PHP code counter class.
/*!
* \class CHtmlPhpCounter
*
* Defines the HTML in PHP code counter class.
*/
class CHtmlPhpCounter : public CHtmlCounter
{
public:
	CHtmlPhpCounter();
};

//! HTML in JSP code counter class.
/*!
* \class CHtmlJspCounter
*
* Defines the HTML in JSP code counter class.
*/
class CHtmlJspCounter : public CHtmlCounter
{
public:
	CHtmlJspCounter();
};

//! HTML in ASP code counter class.
/*!
* \class CHtmlAspCounter
*
* Defines the HTML in ASP code counter class.
*/
class CHtmlAspCounter : public CHtmlCounter
{
public:
	CHtmlAspCounter();
};

//! HTML in ColdFusion code counter class.
/*!
* \class CHtmlColdFusionCounter
*
* Defines the HTML in ColdFusion code counter class.
*/
class CHtmlColdFusionCounter : public CHtmlCounter
{
public:
	CHtmlColdFusionCounter();
};

#endif
