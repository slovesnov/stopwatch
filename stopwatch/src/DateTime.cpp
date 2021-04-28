/*
 * DateTime.cpp
 *
 *       Created on: 20.02.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "DateTime.h"
#include "Config.h"
#include <cmath>

DateTime::DateTime(int time, int addDays) :
		d(0) {
	if (addDays == 0) {
		setNow();
	}
	else {
		init(g_date_time_add_days(DateTime().d, addDays));
	}
	init(format("%FT") + Base::format("%04d00", time));
}

int DateTime::diffDays(BeepTimeType v) {
	DateTime da(day_of_month(), month(), year());
	int i = getYYYYMMDD(v); //clear time of v
	int d = i % 100;
	i /= 100;
	int m = i % 100;
	i /= 100;
	DateTime db(d, m, i);
	GTimeSpan ts = g_date_time_difference(da.d, db.d);
	assert(ts%G_TIME_SPAN_HOUR==0);

	i = ts / G_TIME_SPAN_HOUR;
	/* use lround because ts%G_TIME_SPAN_DAY!=0 if daylight savings
	 * for example from 29mar to 1apr
	 */
	return lround(i / 24.);
}

void DateTime::initTimeZone(gint64 v) {
	init(g_date_time_new_from_unix_local(v)); //set "d" member
	init(g_date_time_to_timezone(d, config.tz)); //adjust daylight save
}

void DateTime::init(std::string const& iso) {
	init(g_date_time_new_from_iso8601(iso.c_str(), config.tz));
}

void DateTime::setNow() {
	/* init(g_date_time_new_now_local()); get invalid time for Moscow time
	 * because of saving daylight, so use use timezone see Config.h
	 */
	init(g_date_time_new_now(config.tz));
}
