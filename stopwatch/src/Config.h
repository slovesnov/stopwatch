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
#include "DigitalFontParameters.h"
#include "help.h"

const int DEFAULT_TIME_ZONE = 100;

class Config {
public:

	GTimeZone *tz;
	Point captionsSize;
	int timeZone;
	int digitalMode;
	int closeWarning;
	int additionalHeight;
	std::set<BeepTimeType> lastSetTime;
	DigitalFontParameters maxDigitalClockSize[2];
	/* soundVolume 0 - 0xffff=65535
	 * for new notebook soundVolume=0x3000=12288 18.75%
	 * for old notebook soundVolume=0xffff=65535 100%
	 */
	int soundVolume;
	bool read;

	Config();
	~Config() {
		g_time_zone_unref(tz);
	}

	void init();
	void write();

	VString getArguments() const;

	static const int MAX_VOLUME=0xffff;
	double getSoundVolume() const {
		return soundVolume / double(MAX_VOLUME);
	}

	static int getSoundVolumeValue(double volume){
		return int(volume*MAX_VOLUME);
	}

	void setSoundVolume(double v){
		soundVolume=getSoundVolumeValue(v);
	}
};

extern Config config;

#endif /* CONFIG_H_ */
