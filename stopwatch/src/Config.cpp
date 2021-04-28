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

Config::Config() {
	unsigned i;
	std::string s;

	timeZone = DEFAULT_TIME_ZONE;
	digitalMode = 0;
	closeWarning = 1;
	maxDigitalClockSize[0]= {800,800,true};
	maxDigitalClockSize[1]= {1800,1800,false};

	int* var[] = {
			&captionsSize.x,
			&captionsSize.y,
			&timeZone,
			&digitalMode,
			&closeWarning,
			&additionalHeight };
	int**p = var;

	std::ifstream f = Base::open(".cfg", false); //Note if do auto f=open... eclipse don't see is_open() method
	read = f.is_open();
	if (read) {
		for (i = 0; i < G_N_ELEMENTS(var); i++, p++) {
			f >> s >> s >> **p;
		}

		f >> s >> s; //read "lastSetTime" & "="
		std::getline(f, s);
		auto v = split(s, " ");
		assert(v.size() > 1);
		v.erase(v.begin()); //remove very 1st element
//		println("%d",v.size())
//		for(auto q:v){
//			println("[%s]",q.c_str())
//		}
		if (v.size() == 1 && v[0].empty()) {
//			printinfo
		}
		else {
//			printinfo
			for (auto a : v) {
				lastSetTime.insert(stoll(a));
			}
		}

		for (auto&d : maxDigitalClockSize) {
			f >> s >> s >> d;
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
	std::ofstream f(Base::fullName(".cfg"));
#define A(s,v) f<<s<<" = "<<v<<std::endl;
#define B(v) A(#v,v)
	B(captionsSize.x)
	B(captionsSize.y)
	A("timeZone", timeZone)
	B(digitalMode)
	B(closeWarning)
	B(additionalHeight)

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
	A("lastSetTime", s)
#undef A
#undef B
	i = 0;
	const std::string NAME[] = { "time", "stopwatch" };
	for (auto&v : maxDigitalClockSize) {
		f << "maxDigitalClockSize(" << NAME[i++] << ") = " << v << std::endl;
	}

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
