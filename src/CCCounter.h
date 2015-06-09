//! Code counter class definition for the C/C++ languages.
/*!
* \file CCCounter.h
*
* This file contains the code counter class definition for the C/C++ languages.
*/

#ifndef CCCounter_h
#define CCCounter_h

#include "CCJavaCsCounter.h"

//! C/C++ code counter class.
/*!
* \class CCCounter
*
* Defines the C/C++ code counter class.
*/
class CCCounter : public CCJavaCsCounter
{
public:
	CCCounter();
};

#endif
