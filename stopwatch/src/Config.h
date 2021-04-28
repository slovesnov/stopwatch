/*
 * Config.h
 *
 *       Created on: 24.11.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <set>
#include "Base.h"
#include "DigitalFontParameters.h"

const int DEFAULT_TIME_ZONE = 100;

class Config: public Base {
public:
	GTimeZone *tz;
	Point captionsSize;
	int timeZone;
	int digitalMode;
	int closeWarning;
	int additionalHeight;
	std::set<BeepTimeType> lastSetTime;
	DigitalFontParameters maxDigitalClockSize[2];
	bool read;

	Config();

	~Config() {
		g_time_zone_unref(tz);
	}

	void write();

	VString getArguments() const;
};

extern Config config;

#endif /* CONFIG_H_ */
