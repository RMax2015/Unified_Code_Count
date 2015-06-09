//! Code counter class methods for the XML language.
/*!
* \file CXmlCounter.cpp
*
* This file contains the code counter class methods for the XML language.
*/

#include "CXmlCounter.h"

/*!
* Constructs a CXmlCounter object.
*/
CXmlCounter::CXmlCounter()
{
	classtype = XML;
	language_name = "XML";

	file_extension.push_back(".*xml");

	BlockCommentStart.push_back("<!--");
	BlockCommentEnd.push_back("-->");
}
