/*
 * TimeZone.h
 *
 *       Created on: 20.02.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef TIMEZONE_H_
#define TIMEZONE_H_

#include "Base.h"
#include "Config.h"

class TimeZone: public Base {
public:
	GTimeZone *tz;
	int timeZone;
	TimeZone() {
		/* set timezone in config file because got invalid time for
		 * in summer for Moscow (actually no saving daylight)
		 * For Moscow use "3" which means utc+3 without daylight savings
		 */
		Config c;
		c.readFile();
		timeZone = c.timeZone;
		if (timeZone == DEFAULT_TIME_ZONE) {
			tz = g_time_zone_new_local();
		}
		else {
			//3 is full width with sign
			std::string s = format("%+03d", timeZone);
			tz = g_time_zone_new(s.c_str());
		}
	}

	~TimeZone() {
		g_time_zone_unref(tz);
	}
};

#endif /* TIMEZONE_H_ */
