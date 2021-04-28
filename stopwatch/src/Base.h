/*
 * Base.h
 *
 *       Created on: 12.01.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef BASE_H_
#define BASE_H_

#include <ctime>
#include <vector>
#include <cassert>
#include <fstream>
#include <gtk/gtk.h>
#include "Point.h"

//#define BASE_ADDONS

#ifdef NDEBUG

#define println(f, ...)  ((void)0);
#define printinfo ((void)0);

#define printlog(f, ...)  ((void)0);
#define printloginfo ((void)0);

#else
const char LOG_FILE_NAME[] = "stopwatchLog.txt";

#define println(f, ...) g_print("%-40s %s:%d %s\n",\
		Base::format(f,##__VA_ARGS__).c_str(),Base::shortFileName(__FILE__).c_str()\
		,__LINE__,__PRETTY_FUNCTION__);

#define printinfo println("")//just for debug output

#define printlog(f, ...)  {\
	char _s[1024],_b[127];\
	sprintf(_b,__FILE__);\
	char*_p=strrchr(_b,G_DIR_SEPARATOR);\
	sprintf(_s,f,##__VA_ARGS__);\
	time_t _t=time(NULL);\
	tm* _q=localtime(&_t);\
	FILE*_f=fopen(LOG_FILE_NAME,"a");\
	fprintf(_f,"%s %s:%d %s() %02d:%02d:%02d %02d.%02d.%d\n",_s,_p==NULL?_b:_p+1,__LINE__,__func__,_q->tm_hour,_q->tm_min,_q->tm_sec,_q->tm_mday,_q->tm_mon+1,_q->tm_year+1900);\
	fclose(_f);\
}

#define printloginfo printlog(" ")

#endif

enum class Mode {
	STOPWATCH, MINUTE, TIME, SIZE
};

enum class DialogType {
	PARAMETERS, ERROR, MESSASE, SIZE
};

typedef std::vector<std::string> VString;
typedef int64_t BeepTimeType;
typedef std::pair<int, std::string> PredefinedDateType;
typedef std::vector<PredefinedDateType> VPredefinedDateType;

const char PROJECT[] = "stopwatch";
const char UPCOMING[] = "upcoming";
const int BTDIVISOR = 10000;
const int MMDDDIVISOR = 10000;
const int MMDIVISOR = 100;

class Base {
public:
	static const char* TITLE[int(Mode::SIZE)];

	static double secondsSince(clock_t t) {
		return double(clock() - t) / CLOCKS_PER_SEC;
	}

	static double getVerticalDPI();
	static void sendMciCommand(const char *s);
	static void beep();
	static void addClass(GtkWidget*w, const gchar* s) {
		GtkStyleContext *context = gtk_widget_get_style_context(w);
		gtk_style_context_add_class(context, s);
	}

	static void removeClass(GtkWidget*w, const gchar* s) {
		GtkStyleContext *context = gtk_widget_get_style_context(w);
		gtk_style_context_remove_class(context, s);
	}

	static VString split(const std::string& subject,
			const std::string& separator);

	static std::string format(const char* _format, ...);

	static std::string shortFileName(const char*file);

	static std::string fullName(const char*p) {
		return PROJECT + std::string(p);
	}

	static std::ifstream open(const char*p, bool check = true) {
		std::ifstream f(fullName(p));
#ifndef NDEBUG
		if (check) {
			assert(f.is_open());
		}
#endif
		return f;
	}

	static int abs(int i) {
		return i >= 0 ? i : -i;
	}

#ifdef BASE_ADDONS
	void exploreAllChildrenRecursive(GtkWidget* w);
#endif

	static int getYYYYMMDD(BeepTimeType v) {
		return v / BTDIVISOR;
	}

	static int getHHMM(BeepTimeType v) {
		return v % BTDIVISOR;
	}

	static bool has(VPredefinedDateType const& v, int i);
	static bool comparePredefinedDate(const PredefinedDateType& a,
			const PredefinedDateType& b);
	static int getYYYY(int v) {
		return v / MMDDDIVISOR;
	}
	static int getMM(int v) {
		return getMMDD(v) / MMDIVISOR;
	}
	static int getDD(int v) {
		return getMMDD(v) % MMDIVISOR;
	}
	static int getMMDD(int v) {
		return v % MMDDDIVISOR;
	}

	static bool equal(Point const& a, Point const& b) {
		return a.x == b.x && a.y == b.y;
	}

	static bool notEqual(Point const& a, Point const& b) {
		return !equal(a, b);
	}
};

#endif /* BASE_H_ */
