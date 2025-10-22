/*
 * DateTime.h
 *
 *       Created on: 20.02.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#ifndef DATETIME_H_
#define DATETIME_H_

#include "help.h"
#include <algorithm>

class DateTime {
	GDateTime *d;
	void init(GDateTime *p) {
		if (d) {
			g_date_time_unref(d);
		}
		d = p;
	}

	void init(std::string const &iso);

public:
	DateTime() :
			d(0) {
		setNow();
	}

	DateTime(std::string const s) :
			d(0) {
		init(s);
	}

	/*
	 make copy constructor to avoid errors on copy objects
	 */
	DateTime(DateTime const &q) :
			d(0) {
		*this = q;
	}

	DateTime(int day, int month, int year) :
			d(0) {
		init(::format("%4d%02d%02dT000000", year, month, day));
	}

	DateTime(int time, int addDays);

	//this=dt+minutes (negative values of minutes are allowed)
	DateTime(DateTime const &dt, int minutes) :
			d(0) {
		init(g_date_time_add_minutes(dt.d, minutes));
	}

	void setNow();

	void initTimeZone(gint64 v);

	void operator=(DateTime const &v) {
		fromBeepTime(v.toBeepTime());
	}

	int toInt(bool fullTime = true) const {
		int i = minute();
		if (fullTime) {
			i += hour() * 100;
		}
		return i;
	}

	int monthDayToInt() const {
		return day_of_month() + month() * 100;
	}

	int yearMonthDayToInt() const {
		return day_of_month() + (month() + year() * 100) * 100;
	}

	BeepTimeType toBeepTime() const {
		auto s = format("%Y%m%d%H%M"); //YYYYMMDDHHMM
		return strtoll(s.c_str(), 0, 10);
	}

	void fromBeepTime(BeepTimeType v) {
		//time can be <1000 so use %04d
		init(::format("%dT%04d00", getYYYYMMDD(v), getHHMM(v)));
	}

	/* return difference in days *this-v
	 * time isn't important (ignored)
	 * *this=6apr2019 1:00
	 * v=5apr2019 23:00
	 * result=1 day
	 */
	int diffDays(BeepTimeType v);

	//*this-v
	int diffDays(DateTime const &v) {
		return diffDays(v.toBeepTime());
	}

#define M(f) int f()const{return g_date_time_get_##f(d);}
	M(second)
	M(minute)
	M(hour)
	M(day_of_month)
	M(month)
	M(year)
	M(day_of_week)	//1 is Monday, 2 is Tuesday... 7 is Sunday
#undef M

	std::string format(const char *format, bool lower = false) const {
		gchar *dt = g_date_time_format(d, format);
		std::string s = dt;
		g_free(dt);
		if (lower) {
			std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		}
		return s;
	}

	std::string sformat() const {
		//%e gives " 1" for 1st, so use %-e
		return format("%A %-e %B %Y", true);
	}

	static std::string weekdayName(int w) {
		//1jul2019 - monday
		return DateTime(w, 7, 2019).format("%A", true);
	}

	~DateTime() {
		g_date_time_unref(d);
	}
};

#endif /* DATETIME_H_ */
