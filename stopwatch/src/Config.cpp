/*
 * Config.cpp
 *
 *       Created on: 24.11.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "Frame.h"
#include "Config.h"

const std::string CONFIG_TAGS[] = { "captionsSize", "timeZone", "digitalMode",
		"closeWarning", "additionalHeight", "lastSetTime",
		"maxDigitalClockSize(time)", "maxDigitalClockSize(stopwatch)","volume" };

Config::Config() {
	timeZone=3;

	/* set timezone in config file because got invalid time for
	 * in summer for Moscow (actually no saving daylight)
	 * For Moscow use "3" which means utc+3 without daylight savings
	 */
	if (timeZone == DEFAULT_TIME_ZONE) {
		tz = g_time_zone_new_local();
	}
	else {
		//3 is full width with sign
		std::string s = format("%+03d", timeZone);
		tz = g_time_zone_new_identifier(s.c_str());
	}
}

void Config::init(){
	int i,j,k;
	int64_t ll;
	std::string s;

	timeZone = DEFAULT_TIME_ZONE;
	digitalMode = 0;
	closeWarning = 1;
	maxDigitalClockSize[0]= {800,800,true};
	maxDigitalClockSize[1]= {1800,1800,false};
	soundVolume=0x3000;

	int* var[] = {
			nullptr,
			&timeZone,
			&digitalMode,
			&closeWarning,
			&additionalHeight,
			&soundVolume};
	const int sz=SIZEI(var);

	//c:\Users\noteboot\AppData\Local\stopwatch
	MapStringString m;
	MapStringString::iterator it;
	read=loadConfig(m);
	if (read) {
		for(auto a:m){
			i=INDEX_OF(a.first,CONFIG_TAGS);
			j=i-sz;
			s=a.second;
			if(i==0){
				captionsSize.fromString(s);
			}
			else if(i<sz){
				if(parseString(s, k)){
					*var[i]=k;
				}
			}
			else if(i==sz){
				if(!s.empty()){//s.empty() if was stopwatch mode
					auto v = split(s, " ");
					for (auto a : v) {
						if(parseString(a, ll)){
							lastSetTime.insert(ll);
						}
					}
				}
			}
			else{
				assert(j<SIZEI(maxDigitalClockSize));
				maxDigitalClockSize[j].fromString(s);
			}
		}
	}

	/* set timezone in config file because got invalid time for
	 * in summer for Moscow (actually no saving daylight)
	 * For Moscow use "3" which means utc+3 without daylight savings
	 */
	if (timeZone == DEFAULT_TIME_ZONE) {
		tz = g_time_zone_new_local();
	}
	else {
		//3 is full width with sign
		std::string s = format("%+03d", timeZone);
		tz = g_time_zone_new_identifier(s.c_str());
	}
}

void Config::write() {
	std::string s;
	int i;
	if (frame.isTime() && !frame.beepTime.empty()) {
		i = 0;
		for (auto v : frame.beepTime) {
			if (i++) {
				s += " ";
			}
			s += std::to_string(v);
		}
	}
	else {
		s = "";
	}

	WRITE_CONFIG(CONFIG_TAGS,
			captionsSize,
			timeZone,
			digitalMode,
			closeWarning,
			additionalHeight,
			s,
			maxDigitalClockSize[0],
			maxDigitalClockSize[1],
			soundVolume
	);
}

VString Config::getArguments() const {
	VString r;

	if (!read) {
		return r;
	}

	//diffDays
	DateTime dt;
	BeepTimeType now = DateTime().toBeepTime();
	for (auto v : config.lastSetTime) {
		if (now < v) {
			dt.fromBeepTime(v);
			int d = dt.diffDays(now);
			auto s = std::to_string(getHHMM(v));
			if (d > 0) {
				s += "+";
				if (d > 1) {
					s += std::to_string(d);
				}
			}
			r.push_back(s);
		}
	}

	if (!r.empty()) {
		r.insert(r.begin(), "tm");
	}

	return r;

}

