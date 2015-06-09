#ifndef USERIF_H
#define USERIF_H

//! User interface class definition.
/*!
* \file UserIF.h
*
* This file contains the user interface class definition.
*
* Changed from UCC 2013_04 release by Randy Maxwell
*   Changes started on 2015_04_22
*   Changes  ended  on 2015_06_05
*	Moved code that was common no matter if GUI or text UI
*/

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "UCCGlobals.h"

#ifndef QTGUI

//! Console user interface class.
/*!
* \class UserIF
*
* Defines a console user interface class.
*/
class UserIF
{
public:
	UserIF();
	~UserIF(){}

	void SetErrorFile(const string &outDir);
	void AddError(const string &err, bool logOnly = false, int preNL = 0, int postNL = 1);
	bool updateProgress(const string &msg, const bool postNL = true,
						const int pct = -1, const unsigned int progressTotal = 0);
	bool isCanceled();

private:
	ofstream errFile;			//!< Stream for logging errors
	string errPath;				//!< Error log file path
	bool execCanceled;			//!< Flag indicating canceled execution
};

#else

#include <QWidget>

//! Graphical user interface class.
/*!
* \class UserIF
*
* Defines a graphical user interface class.
*/
class UserIF : public QObject
{
	Q_OBJECT

public:
	UserIF(QWidget *parent = 0);
	~UserIF(){}

	void SetErrorFile(const string &outDir);
	void AddError( errMsgStruct & err );
	void updateProgress(const string &msg, bool postNL = true,
		int pct = -1, unsigned int progressCnt = 0, unsigned int progressTotal = 0);
	bool isCanceled();

signals:
	void updatedLog(const QString &err);
	void updatedProgress(const QString &msg, int pct);

	public slots:
		void cancelExecution();

private:
	ofstream errFile;			//!< Stream for logging errors
	string errPath;				//!< Error log file path
	bool execCanceled;			//!< Flag indicating canceled execution
};

#endif

#endif
