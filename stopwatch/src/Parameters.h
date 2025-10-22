/*
 * Parameters.h
 *
 *       Created on: 03.01.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "help.h"
#include "DateTime.h"
#include <set>

enum class StringType {
	FRAME, DIALOG, ENTRY
};

class Parameters {
	VString arg;
	bool _minimize; //has minimize() function so use another name
	static std::set<int> predefinedTime[8]; //used in Frame & Dialog so static
	//begin error information
	std::string message;
	std::string condition;
	std::string file;
	int line;
	/* If errorType=0 argument=number of argument of parsing string
	 * else argument=number of line in stopwatchPredefined.txt file
	 * */
	int argument;
	int errorType;
	//end error information

	bool parse(bool predefined = false);

	static std::string infoString;
	static void staticInit(const char *file);

public:
	static const char *TITLE[int(Mode::MODE_SIZE)];

	/* for stopwatch mode beepTime = seconds
	 * for minute mode beepTime = minutes
	 * for time mode beepTime = YYYYMMDDHHMM
	 */
	std::set<BeepTimeType> beepTime;	//must not be static
	Mode mode;	//must not be static

	static VString predefinedSet;	//don't need to reorder so use vector
	static VPredefinedDateType predefinedDate;
	BeepTimeType lastBeepTime;
	DateTime setupTime;

	bool loadPredefined() {
		return loadPredefined(getPredefinedFileContents());
	}
	bool loadPredefined(std::string const &data);//no static because store error message
	std::string getPredefinedFileContents();
	void setPredefinedFileContents(std::string const &s);

	static std::string getPredefinedTimeString(DialogType dt =
			DialogType::PARAMETERS);
	static std::string getPredefinedTimeString(int wday);
	std::set<int> const& getPredefinedTime(int wday);
	static std::string getPredefinedDateString();
	static std::string getPredefinedSetString();

	static std::string const& getInfoString() {
		return infoString;
	}

	bool parse(VString const &a) {
		arg = a;
		return parse();
	}

	bool parse(int argc, char *argv[]);
	bool parse(const std::string s, bool predefined = false);
	bool parse(int combo, int minimize, const char *p);
	std::string what(bool forLabel = false) const;
	std::string toString(StringType t) const;

	bool minimize() const {
		return _minimize;
	}

	int getMode() const {
		return int(mode);
	}

	bool isStopwatch() const {
		return mode == Mode::STOPWATCH;
	}

	bool isMinute() const {
		return mode == Mode::MINUTE;
	}

	bool isTime() const {
		return mode == Mode::TIME;
	}

	bool hasBeepTime(BeepTimeType i) {
		return beepTime.find(i) != beepTime.end();
	}

	void operator=(Parameters const &p);

	void upcoming();
	void upcomingAll();
	BeepTimeType getNextBeepTime();
	DateTime getUpcoming();

	bool prepare(const char *file) {
		staticInit(file);
		return loadPredefined();
	}

	static bool isPredefinedDate();
	std::string beepTimeFormat(BeepTimeType v, bool icon) const;

	void addDays(Parameters const &p, int days);
	//copy and add days if needs
	void copyAddDays(Parameters const &p);

	static void removeUntilTime(std::set<BeepTimeType> &b,
			BeepTimeType const &t);

	static std::string getPredefinedFileName();
};

#endif /* PARAMETERS_H_ */
