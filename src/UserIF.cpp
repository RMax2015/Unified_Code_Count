//! User interface class methods.
/*!
* \file UserIF.cpp
*
* This file contains the user interface class methods.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_05
*	Moved code that was common no matter if GUI or text UI
*   Fixed defects in text UI updateProgress and allow suppression of Warnings on console
*/

#include <time.h>

#include "UserIF.h"

//
//		These are common no matter how the UI is done
//

/*!
* Sets the execution error file.
*
* \param outDir output directory
*/
void UserIF::SetErrorFile(const string &outDir)
{
	time_t myTime;
	struct tm *myLocalTime;
	time(&myTime);
#if defined UNIX || defined MINGW
	myLocalTime = localtime(&myTime);
#else
	struct tm myLT;
	localtime_s(&myLT, &myTime);
	myLocalTime = &myLT;
#endif
	char s[50];
	strftime(s, 50, "error_log_%m%d%Y_%I%M%S.txt", myLocalTime);
	if (outDir.empty())
		errPath = s;
	else
		errPath = outDir + s;
}


#ifndef QTGUI

/*!
* Constructs a UserIF object.
*/
UserIF::UserIF()
{
	errPath = "";
	execCanceled = false;
}


/*!
* Adds a message entry to the error log.
*
* \param err error string
* \param logOnly flag to only write to log (no user output)
* \param preNL number of preceding new lines in user output (default=0)
* \param postNL number of proceeding new lines in user output (default=1)
*/
void UserIF::AddError(const string &err, bool logOnly, int preNL, int postNL)
{
	if (err.empty() || errPath.empty())
		return;

	// open the error file if not already opened
	if (!errFile.is_open())
	{
		errFile.open(errPath.c_str(), ofstream::out);
		if (!errFile.is_open())
		{
			cerr << "Error: Failed to open error log file" << endl;
			return;
		}
	}
	errFile << err << endl;
	if (!logOnly)
	{
		for (int i = 0; i < preNL; i++)
			cout << endl;
		cout << err;
		for (int i = 0; i < postNL; i++)
			cout << endl;
		cout << flush;
	}
}

/*!
* Updates progress by displaying a message and passing percent completion.
*
* \Global	no_warnings_to_stdout	IN	true will suppress showing a warning
*
* \param msg message string
* \param postNL include a new line (default=true)
* \param pct percent completion -1 if none, else shows 4 chars with possible leading dots
* \param progressTotal progress total  IF 1 will output 100%
*
* \return true if message was shown
*/
// Declare the function we use here without having to include the other Thread interfaces
extern void	IntToStr( const int val, string & result );

bool UserIF::updateProgress(const string &msg, const bool postNL,
	const int pct, const unsigned int progressTotal)
{
	bool	msg_shown = false;

	if ( pct >= 0 )
	{
		// Assumption is that showing Percent done will not be a warning
		if ( !msg.empty() )
		{
			cout << msg << flush;
			msg_shown = true;
		}

		// Changed below to only change the LAST 4 positions
		// And this will always change those 4
		// Always overwrite last 4 positions 
		// so Caller must set up with a little extra for the first time
		string	tmp = "\b\b\b\b";	// Back up to overwrite previous % shown

		if (progressTotal == 1)	// check for special case
			tmp += "100%";
		else
		{
			if (pct < 10)
				tmp += "..";
			else if (pct < 100)
				tmp += ".";
			string tmp2;
			IntToStr( pct, tmp2 );
			tmp += tmp2 + "%";
		}
		cout << tmp;
		cout.flush();
		msg_shown = true;
	}
	else
	{
		bool	show_msg = true;
		if ( !msg.empty() )
		{
			if ( g_no_warnings_to_stdout )
			{
				if ( msg.find( "Warning:" ) != string::npos )
					show_msg = false;
			}
		}
		else
			show_msg = false;

		if ( show_msg )
		{
			if ( !postNL )
				cout << msg << flush;
			else
				cout << msg << endl;
			msg_shown = true;
		}
	}
	return	msg_shown;
}

/*!
* Checks whether execution has been canceled by the user.
*
* \return execution canceled?
*/
bool UserIF::isCanceled()
{
	return(execCanceled);
}

#else

#include <QApplication>

/*!
* Constructs a UserIF object.
*
* \parent parent widget
*/
UserIF::UserIF(QWidget *parent)
	: QObject(parent)
{
	errPath = "";
	execCanceled = false;

	if (parent)
	{
		connect(this, SIGNAL(updatedLog(const QString &)), parent, SLOT(updateLog(const QString &)));
		connect(this, SIGNAL(updatedProgress(const QString &, int)), parent, SLOT(updateProgress(const QString &, int)));
		connect(parent, SIGNAL(canceledExecution()), this, SLOT(cancelExecution()));
	}
}

/*!
* Adds a message entry to the error log.
*
* \param err error string
* \param logOnly flag to only write to log (no user output)
* \param preNL number of preceding new lines in user output (default=0)
* \param postNL number of proceeding new lines in user output (default=1)
*/
void UserIF::AddError(const string &err, bool /*logOnly*/, int preNL, int postNL)
{
	if (err.empty() || errPath.empty())
		return;

	// open the error file if not already opened
	if (!errFile.is_open())
	{
		errFile.open(errPath.c_str(), ofstream::out);
		if (!errFile.is_open())
		{
			emit updatedLog("Error: Failed to open error log file\n");
			return;
		}
	}
	errFile << err << endl;

	QString errStr = err.c_str();
	for (int i = 0; i < preNL; i++)
		errStr = "\n" + errStr;
	for (int i = 0; i < postNL; i++)
		errStr += "\n";
	emit updatedLog(errStr);
	QApplication::processEvents();
}

/*!
* Updates progress by displaying a message and passing percent completion.
*
* \param msg message string
* \param postNL include a new line (default=true)
* \param pct percent completion (-1 if none)
* \param progressCnt progress count <= progress total
* \param progressTotal progress total -> 100%
*/
void UserIF::updateProgress(const string &msg, bool /*postNL*/,
	int pct, unsigned int /*progressCnt*/, unsigned int /*progressTotal*/)
{
	if (!msg.empty() || pct >= 0)
	{
		emit updatedProgress(QString(msg.c_str()), pct);
		QApplication::processEvents();
	}
}

/*!
* Cancels the running execution.
*/
void UserIF::cancelExecution()
{
	execCanceled = true;
}

/*!
* Checks whether execution has been canceled by the user.
*
* \return execution canceled?
*/
bool UserIF::isCanceled()
{
	return(execCanceled);
}

#endif
