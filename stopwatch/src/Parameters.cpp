/*
 * Parameters.cpp
 *
 *       Created on: 03.01.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#include "Parameters.h"
#include <cassert>
#include <sstream>

const char MINIMIZE_CHAR = 'm';
const char PREDEFINED_FILE_NAME[] = "Predefined.txt";
std::set<int> Parameters::predefinedTime[8];
VString Parameters::predefinedSet;
VPredefinedDateType Parameters::predefinedDate;
std::string Parameters::infoString;
const char *Parameters::TITLE[] = { "stopwatch", "minute", "time" };

bool Parameters::parse(int argc, char *argv[]) {
	arg.clear(); //may be not empty from previous errors
	for (int i = 1; i < argc; i++) { //don't need very first string
		arg.push_back(argv[i]);
	}
	return parse();
}

#define ASSERT_FULL(c,m,a,t) if(!(c)){message=m;condition=#c;line=__LINE__;\
	file=getFileInfo(__FILE__, FILEINFO::NAME);errorType=t;argument=a;return false;}
#define ASSERT(c,m,a) ASSERT_FULL(c,m,a,0)
#define ASSERT1(c,m) ASSERT(c,m,-1)
#define ASSERT_TIME_FULL(t,a,type) \
	ASSERT_FULL( (t%100)>=0&&(t%100)<60, "MINUTE should be from 0 to 59", a,type)\
	ASSERT_FULL( (t/100)>=0&&(t/100)<24, "HOUR should be from 0 to 23", a,type)
#define ASSERT_TIME(t,a) ASSERT_TIME_FULL(t,a,0)

bool Parameters::parse(int combo, int minimize, const char *p) {
	ASSERT1(combo == int(Mode::STOPWATCH) || strlen(p) != 0, "no arguments")
	std::string s(1, *TITLE[combo]);
	if (minimize) {
		s += MINIMIZE_CHAR;
	}
	s += " ";
	s += p;
	return parse(s);
}

bool Parameters::parse(const std::string s, bool predefined) {
	arg.clear(); //may be not empty from previous errors

	for (auto &a : split(s, " ")) {
		/* if p="10  20" -> split={"10","","20"} so remove empty values
		 * for command line if call "stopwatch.exe 10  20" then argv={"10","20"}, so don't need check for main()
		 */
		if (!a.empty()) { //if p="10  20" -> split={"10","","20"} so remove empty values
			arg.push_back(a);
		}
	}
	return parse(predefined);
}

bool Parameters::parse(bool predefined) {
	/* use lastInserted because we use set, and "1505 -30 -10"
	 * cause invalid recognition because we cann't use *beepTime.rbegin()
	 * for third parameter "-10"
	 */
	int i, j, k = 0, l, addDays;
	BeepTimeType lastInserted = 0;
	size_t lastSize;
	const char *p;
	char *e;
	bool repeat;
	DateTime d;

	ASSERT1(arg.size() > 0, "too few arguments")

	p = arg[0].c_str();
	ASSERT(!arg[0].empty(), "empty argument", 0)
	i = 0;
	for (auto a : TITLE) {
		if (a[0] == *p) {
			break;
		}
		i++;
	}
	mode = Mode(i);
	ASSERT(mode != Mode::MODE_SIZE, "unknown mode", 0)

	ASSERT1(isStopwatch() || arg.size() > 1, "too few arguments")

	//after mode is set
	i = strlen(p);
	ASSERT(i <= 2, "length<=2", 0)
	_minimize = false;
	for (p = arg[0].c_str() + 1, j = 1; j < i; j++, p++) {
		if (*p == MINIMIZE_CHAR) {
			_minimize = true;
		} else {
			ASSERT(0, "invalid char", 0)
		}
	}

	beepTime.clear();
	for (i = 1; i < int(arg.size()); i++) {
		addDays = 0;
		lastSize = beepTime.size();
		p = arg[i].c_str();
		j = strtol(p, &e, 10);
		if (p + strlen(p) == e) {
			k = 1; //repeat
			repeat = false;
		} else {
			if (isTime() && e > p && *e == '+') {
				repeat = false;
				e++;
				if (*e == 0) {
					addDays = 1;
					ASSERT(*e == 0, "not all string recognized", i)
				} else {
					//"730++1" invalid string
					ASSERT(isdigit(*e), "not all string recognized", i)
					addDays = strtol(e, &e, 10);
					ASSERT(addDays > 0, "invalid add days", i)
					ASSERT(*e == 0, "not all string recognized", i)
				}
			} else {
				ASSERT(e > p && *e == ',', "not all string recognized", i)
				//For example "t ab"
				e++;
				ASSERT(*e != 0, "counter is not set use +20,6", i)

				k = strtol(e, &e, 10);
				ASSERT(k > 0, "counter after comma should be positive", i)
				ASSERT(p + strlen(p) == e, "not all string recognized", i)
				repeat = true;
			}

		}

		if (isStopwatch()) {
			ASSERT(j > 0, "arguments should be positive integers", i)
			if (j < 10) {
				j *= 60;
			} else if (j >= 100) {
				l = j % 100;
				ASSERT(l <= 59, "seconds should be from 0 to 59", i)
				j = (j / 100) * 60 + l;
			}
			if (repeat || *p == '+') {
				for (; k > 0; k--) {
					lastInserted = (beepTime.empty() ? 0 : lastInserted) + j;
					beepTime.insert(lastInserted);
				}
			} else {
				lastInserted = j;
				beepTime.insert(lastInserted);
			}

		} else { // time or minute mode
			if (repeat || strchr("+-", *p)) {
				ASSERT(j != 0, "invalid increment or decrement value", i)
				if (isTime()) {
					if (abs(j) > 99) { //+130 means +(1 hour and 30 minutes)
						ASSERT_TIME(abs(j), i)
						j = (j / 100) * 60 + (j % 100);
					}
				} else {
					ASSERT(abs(j) < 60, "invalid increment or decrement value",
							i)
				}

				if (isTime()) {
					DateTime a;
					if (!beepTime.empty()) {
						a.fromBeepTime(lastInserted);
					}

					for (l = j; k > 0; k--, l += j) {
						DateTime v(a, l);
						if (v.toBeepTime() <= d.toBeepTime()) { //if signal earlier than now
							if (j < 0) {
								break;
							} else { //lastInserted is incremented so just continue
								continue;
							}
						}
						lastInserted = v.toBeepTime();
						beepTime.insert(lastInserted);
					}
				} else {
					if (beepTime.empty()) {
						l = d.toInt(false);
					} else {
						l = lastInserted;
					}

					for (; k > 0; k--) {
						l += j;
						if (l < 0 || l >= 60) {
							break;
						}
						lastInserted = l;
						beepTime.insert(lastInserted);
					}
				}
			} else {
				ASSERT(isdigit(*p),
						"argument should start from digit or + or -", i)
				ASSERT(p + strlen(p) == e, "not all string recognized", i)
				if (isTime()) {
					ASSERT_TIME(j, i)
					DateTime q(j, addDays);
					if (!predefined) {
						ASSERT(q.toBeepTime() > d.toBeepTime(),
								"no new value inserted", i)
					}
					lastInserted = q.toBeepTime();
					beepTime.insert(lastInserted);
				} else {
					lastInserted = j;
					ASSERT(j >= 0 && j < 60, "arguments should be from 0 to 59",
							i)
					beepTime.insert(lastInserted);
				}
			}
		}

		if (!predefined) {
			ASSERT(beepTime.size() != lastSize, "no new values inserted", i)
		}
	}

	//for sureness
	ASSERT1(isStopwatch() || !beepTime.empty(), "beep time is empty")

	lastBeepTime = 0;
	setupTime.setNow();
	return true;
}

std::string Parameters::what(bool forLabel) const {
	const char *separator = forLabel ? " " : "\n";
	std::string s;
	if (forLabel) {
		s += message;
	} else {
		s += "condition " + condition + separator + "message " + message;
	}
	if (errorType == 0) {
		if (argument != -1) {
			s += separator
					+ format("argument %d", argument + (forLabel ? 0 : 1));
		}
	} else {
		s += separator + std::string("file ") + getPredefinedFileName()
				+ format(" line %d", argument);
	}
	s += separator + std::string("file ") + file + format(" line %d", line);
	return s;
}

std::string Parameters::toString(StringType t) const {
	int i = 0;
	std::string s;
	if (t == StringType::DIALOG) {
		s = "recognized";
	} else if (t == StringType::FRAME) {
		i = 0;
		for (auto a : arg) {
			s += (i++ ? " " : "") + a;
		}
	} else {
		i = 0;
		for (auto a : arg) {
			if (i != 0) { //need increment i anyway
				if (i != 1) {
					s += " ";
				}
				s += a;
			}
			i++;
		}
		return s;
	}

	i = 0;
	s += " [";
	for (auto a : beepTime) {
		if (i)
			s += " ";
		else
			i = 1;
		s += beepTimeFormat(a, false);
	}
	s += "]";

	if (t == StringType::FRAME && isTime()) {
		DateTime dt;
		assert(!beepTime.empty());
		dt.fromBeepTime(*beepTime.begin());
		s += " " + dt.sformat();
	}

	return s;
}

void Parameters::operator=(const Parameters &p) {
	mode = p.mode;
	_minimize = p._minimize;
	arg = p.arg;
	beepTime = p.beepTime;
	lastBeepTime = 0;
}

void Parameters::addDays(Parameters const &p, int days) {
	/* relative time time with + or - at the beginning
	 *
	 * days=1 & "908+ -30" -> "908 -30"
	 * days=1 & "908+ 1506" -> "908"
	 * days=1 & "+15" -> ""
	 *
	 */

	assert(p.isTime());
	assert(days > 0);
	mode = p.mode;
	_minimize = p._minimize;
	int i;
	std::string s;
	for (auto a : p.arg) {
		if (!isdigit(a[0])) { //relative
			arg.push_back(a);
			continue;
		}
		auto f = a.find('+');
		bool b = f + 1 == a.length();
		if (f != std::string::npos || b) {

			i = b ? 1 : atoi(a.substr(f + 1).c_str());

			if (i >= days) {
				s = a.substr(0, f);
				if (i > days) {
					s += format("+%d", i - days);
				}
				arg.push_back(s);
			}
		}
	}

	bool ok = true;
	if (arg.empty()) { //"908+" add two days
		ok = false;
	} else {
		ok = parse(true);
	}

	auto b = beepTime;
	auto pb = p.beepTime;

	if (ok) {
		/* p={1908+ 1506} add 1 day, need to remove 1506 from p
		 * because *this={1908}
		 */

		//remove all events until now
		auto now = DateTime().toBeepTime();
		removeUntilTime(b, now);
		removeUntilTime(pb, now);

		bool eq = b.size() == b.size()
				&& std::equal(b.begin(), b.end(), pb.begin());
		if (!eq) {
			ok = false;
		}
	}

	if (!ok) {
		*this = p; //couldn't add days just copy
	}

}

std::string Parameters::beepTimeFormat(BeepTimeType v, bool icon) const {
	int i;
	std::string s;
	if (isStopwatch()) {
		i = v;
		if (icon || i / 60 != 0) {
			s = format("%d", i / 60);
		}
		return s + format("%02d", i % 60); //output m[m]ss
	}

	s = format("%d", getHHMM(v));

	if (isTime()) {
		i = -DateTime().diffDays(v);
		if (i == 0) {
			//no + after
		} else if (i > 0) {
			if (icon) {
				s += "'";
			} else {
				s += "+";
				if (i > 1) {
					s += format("%d", i);
				}
			}
		} else {
			assert(0);
		}
	}
	return s;
}

#define ASSERT_LINE(c,m) ASSERT_FULL(c,m,fileline,1)
#define ASSERT_TIME_LINE(t) ASSERT_TIME_FULL(t,fileline,1)

std::string Parameters::getPredefinedFileContents() {
	return writableFileGetContents(getPredefinedFileName());
}

void Parameters::setPredefinedFileContents(std::string const &s) {
	writableFileSetContents(getPredefinedFileName(), s);
}

bool Parameters::loadPredefined(std::string const &data) {
	int i, j, d[3];
	//line already defined as member of class Parameters.h so use another name instead of line
	int fileline;
	const char *p;
	char *e;
	std::string s, s1, s2;
	std::size_t pos, p1;
	//try to parse into pTime, pSet, pDate, to not corrupt class members in case of error parsing predefined file
	std::set<int> pTime[8];
	VString pSet;
	VPredefinedDateType pDate;
	DateTime dt;
	std::vector<int> vi;
	std::vector<int> vskiplines;
	const int ANY_MONTH = 0;
	VString vo;
	VString vd = split(data, "\n");
	fileline = 0;
	for (auto ss : vd) {
		fileline++;
		//replace tabs to spaces
		s = ss;
		std::replace(s.begin(), s.end(), '\t', ' ');

		//removes spaces from the beginning
		if ((pos = s.find_first_not_of(' ')) == std::string::npos) {
			s = ""; //only tabs & spaces
		} else {
			s = s.substr(pos);
		}

		const bool predefinedDate = s.find('.') != std::string::npos;

		//remove inline comment in string, leave for predefined date
		if (!predefinedDate && (pos = s.find('#')) != std::string::npos) {
			s = s.substr(0, pos);
		}

		//removes spaces at the end of the string
		if ((pos = s.find_last_not_of(' ')) == std::string::npos) {
			s = "";
		} else {
			s = s.substr(0, pos + 1);
		}

		if (s.empty()) {
			continue;
		}

		if (predefinedDate) {
			//special date
			for (p1 = s.find('.') + 1;
					isdigit(s[p1]) || s[p1] == '.' || s[p1] == '*'; p1++)
				;
			//tabs changes to spaces
			ASSERT_LINE(s[p1] == ' ' || s[p1] == 0,
					"should be space or tab after predefined date");

			s1 = s.substr(0, p1);
			if (s[p1] == ' ') {
				p1++;
			}
			s2 = s.substr(p1);
			//println("%d %d[%s]",int(s[p1]),s2.length(),s2.c_str());

			i = 0;
			auto vv = split(s1, ".");
			//allow 13.3 or 13.3.2020 or 15.*
			ASSERT_LINE(vv.size() >= 2 && vv.size() <= 3,
					"invalid predefined day");
			d[2] = 0;
			for (auto v : vv) {
				p = v.c_str();
				if (i == 1 && v == "*") {
					d[i] = ANY_MONTH;
					continue;
				}
				d[i] = strtol(p, &e, 10);
				ASSERT_LINE(p + strlen(p) == e,
						"not all string recognized [" + s + "]");
				if (i == 2 && d[2] < 100) {
					//allow 19 instead of 2019
					d[2] += (dt.year() / 100) * 100;
				}
				i++;
			}

			ASSERT_LINE(d[0] >= 1, "invalid predefined day");
			ASSERT_LINE(d[1] == ANY_MONTH || (d[1] >= 1 && d[1] <= 12),
					"invalid predefined month [" + s1
							+ "]. month out of bounds");

			i = d[2]; //i==0 means every year
			const int DMAX[] = { 31,
					i == 0 ?
							29 :
							28 + (i % 400 == 0 || (i % 100 != 0 && i % 4 == 0)),
					31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
			if (d[1] != ANY_MONTH) {
				ASSERT_LINE(d[0] <= DMAX[d[1] - 1],
						"invalid predefined date. day out of bound");
			}

			i = d[0] + (d[1] + d[2] * 100) * 100;
			j = getYYYYMMDD(dt.toBeepTime());
			if (d[1] != ANY_MONTH && d[2] != 0 && i < j) { //special year is set
				if (yesNoDialog(
						"set date which already past skip it?\n" + ss
								+ "\nIf you skip all past dates then file will be rewritten.")) {
					vskiplines.push_back(fileline);
					continue;
				} else {
					ASSERT_LINE(i >= j, "set date which already past");
				}
			}
			ASSERT_LINE(!has(pDate, i),
					"repeat predefined parameter [" + s + "]");
			pDate.push_back( { i, s2 });
			continue;
		}

		/*this file should output invalid parsing [tm 2929]
		 #program reads this file automatically
		 tm 845
		 tm 2929
		 */
		auto v = split(s, " ");
		i = 0;
		for (auto &a : TITLE) {
			if (v[0][0] == a[0]) {
				if (!parse(s, true)) { //don't call ASSERT_LINE store error message
					argument = fileline;
					errorType = 1;
					message += " try to parse [" + s + "]";
					return false;
				}

				ASSERT_LINE(
						std::find(pSet.begin(), pSet.end(), s) == pSet.end(),
						"repeat predefined parameter [" + s + "]");
				pSet.push_back(s);
				i = 1;
			}
		}

		if (i) {
			continue;
		}

		ASSERT_LINE(v.size() > 1, "too short string");

		vi.clear();
		s = v[0];
		//1-4,5 1707
		for (auto &t : split(s, ",")) {
			ASSERT_LINE(t.length() == 1 || t.length() == 3,
					"invalid week day string");
			i = t[0] - '0';
			ASSERT_LINE(i > 0 && i < 8, "invalid week day found");
			if (t.length() == 1) {
				vi.push_back(i);
			} else {
				j = t[2] - '0';
				ASSERT_LINE(t[1] == '-', "invalid second symbol in range");
				ASSERT_LINE(i < j, "invalid range lower bound >= upper bound");
				ASSERT_LINE(j > 0 && j < 8, "invalid week day found");
				for (; i <= j; i++) {
					vi.push_back(i);
				}
			}
		}

		for (auto &t : vi) {
			auto &set = pTime[t];

			i = 0;
			for (auto a : v) {
				if (i != 0) {
					p = a.c_str();
					//1-7 "1706 " split={"1706",""} skip empty string
					if (strlen(p) == 0) {
						continue;
					}
					j = strtol(p, &e, 10);
					ASSERT_LINE(p + strlen(p) == e,
							"not all string recognized [" + a
									+ "] from predefined file");
					ASSERT_TIME_LINE(j);
					ASSERT_LINE(set.insert(j).second,
							format("repeated time %d for ", j)
									+ DateTime::weekdayName(t));
				}

				i++;
			}
		}
	}

	if (!vskiplines.empty()) {
		//overwrite file
		fileline = 0;
		for (auto ss : vd) {
			fileline++;
			if (!oneOf(fileline, vskiplines)) {
				vo.push_back(ss);
			} else {
//				printl(ss);
			}
		}
		setPredefinedFileContents(joinV(vo, '\n'));
	}

	i = 0;
	for (auto &a : pTime) {
		predefinedTime[i++] = a;
	}
	predefinedSet = pSet;

	std::sort(pDate.begin(), pDate.end(), comparePredefinedDate);
	predefinedDate = pDate;

	return true;
}

std::set<int> const& Parameters::getPredefinedTime(int wday) {
	return predefinedTime[wday];
}

BeepTimeType Parameters::getNextBeepTime() {
	assert(!isStopwatch());
	DateTime d;
	BeepTimeType v = isTime() ? d.toBeepTime() : d.toInt(false);

	for (auto a : beepTime) {
		if (v < a) {
			return a;
		}
	}

	return isTime() ? getUpcoming().toBeepTime() : *beepTime.begin();
}

DateTime Parameters::getUpcoming() {
	int t;
	DateTime d;
	int v = d.toInt();
	int wday = d.day_of_week();
	auto &s = getPredefinedTime(wday);
	auto it = std::find_if(s.cbegin(), s.cend(), [&](auto e) {
		return v < e;
	});

	bool tomorrow = it == s.cend();
	if (tomorrow) { //example 2300 means 1st item of next day
		t = *getPredefinedTime(wday == 7 ? 1 : wday + 1).begin();
	} else {
		t = *it;
	}

	return DateTime(t, tomorrow);

}

std::string Parameters::getPredefinedTimeString(DialogType dt) {
	std::string s = "predefined time";
	if (dt == DialogType::MESSASE) {
		s = "<i>" + s + "</i>";
	}
	std::string v[8];
	bool b[8];
	int i, j;
	for (i = 1; i < 8; i++) {
		v[i] = getPredefinedTimeString(i);
		b[i] = false;
	}

	for (i = 1; i < 8; i++) {
		if (b[i]) {
			continue;
		}
		s += format("\n%d", i);
		for (j = i + 1; j < 8; j++) {
			if (v[i] == v[j]) {
				b[j] = true;
				s += format(",%d", j);
			}
		}
		s += v[i];
	}
	return s;
}

std::string Parameters::getPredefinedTimeString(int wday) {
	auto &a = predefinedTime[wday];
	if (a.empty()) {
		return "";
	}
	std::string s;
	for (auto b : a) {
		s += format(" %d", b);
	}
	return s;

}

void Parameters::staticInit(const char *file) {
	// d.format("%b")-short month name doesn't work if not make putenv
	putenv("LANG=C.UTF-8");
	DateTime d;
	auto s = d.format("start %d %b %Y %H:%M:%S\n");

	GFile *gf = g_file_new_for_path(file);
	GError *error = 0;
	auto fi = g_file_query_info(gf, G_FILE_ATTRIBUTE_TIME_MODIFIED,
			G_FILE_QUERY_INFO_NONE, NULL, &error);
	g_assert_no_error(error);
#ifndef NDEBUG
	GFileAttributeType at = g_file_info_get_attribute_type(fi,
	G_FILE_ATTRIBUTE_TIME_MODIFIED);
	assert(at == G_FILE_ATTRIBUTE_TYPE_UINT64);
#endif
	guint64 v = g_file_info_get_attribute_uint64(fi,
	G_FILE_ATTRIBUTE_TIME_MODIFIED);

	g_object_unref(fi);
	g_object_unref(gf);

	/* Note use last modified time in buildString
	 * old method based on __DATE__ and  __TIME__ constants
	 * not always working, because this file sometimes isn't changed
	 * */
	d.initTimeZone(v);
	infoString = getVersionString(false) + "\n" + s
			+ d.format("build %d %b %Y %H:%M:%S");

}

void Parameters::upcoming() {
	DateTime t = getUpcoming();
	parse(
			format("%c%c %d%s", *TITLE[int(Mode::TIME)], MINIMIZE_CHAR,
					t.toInt(),
					DateTime().day_of_week() == t.day_of_week() ? "" : "+"));
}

void Parameters::upcomingAll() {
	DateTime u = getUpcoming();
	DateTime d;

	int wday = d.day_of_week();
	if (wday != u.day_of_week()) {
		upcoming();
		return;
	}
	int t = d.toInt();
	auto &p = getPredefinedTime(wday);
	auto it = std::find_if(p.cbegin(), p.cend(), [&](auto e) {
		return t < e;
	});

	assert(it != p.cend());
	std::string s = format("%c%c", *TITLE[int(Mode::TIME)], MINIMIZE_CHAR);
	for (; it != p.cend(); it++) {
		s += format(" %d", *it);
	}

	parse(s);
}

std::string Parameters::getPredefinedDateString() {
	std::string s = "<i>predefined date</i>\n";
	int i, j = 0, k;
	std::string s1;
	for (auto p : predefinedDate) {
		i = p.first;
		DateTime dt(1, (i / 100) % 100, 2019);
		auto a = dt.format("%B", true);
		if (j) {
			s += "\n";
		}
		k = getYYYY(i);
		s += format("%d ", i % 100) + a + " "
				+ (k == 0 ? "any year" : format("%d", k));
		s1 = p.second;
		if (!s1.empty()) {
			s += " " + s1;
		}
		j++;
	}
	return s;
}

std::string Parameters::getPredefinedSetString() {
	std::string s = "<i>predefined set</i>\n";
	int j = 0;
	for (auto a : predefinedSet) {
		if (j) {
			s += "\n";
		}
		s += a;
		j++;
	}
	return s;
}

bool Parameters::isPredefinedDate() {
	DateTime dt;
	int md = dt.monthDayToInt();
	int ymd = dt.yearMonthDayToInt();
	int d;
	for (auto a : predefinedDate) {
		d = a.first;
		if (getYYYY(d) == 0) { //any year
			if (getMM(d) == 0) { //any month
				if (getDD(d) == getDD(md)) {
					return true;
				}
			} else {
				if (d == md) {
					return true;
				}
			}
		} else {
			if (d == ymd) {
				return true;
			}
		}
	}
	return false;
}

void Parameters::copyAddDays(const Parameters &p) {
	setupTime.setNow();
	int i = setupTime.diffDays(p.setupTime);
	if (!p.isTime() || i == 0) {
		*this = p;
		return;
	}

	assert(i > 0);
	addDays(p, i);
}

void Parameters::removeUntilTime(std::set<BeepTimeType> &b,
		BeepTimeType const &t) {
	auto it = std::find_if(b.cbegin(), b.cend(), [&](auto e) {
		return t < e;
	});
	b.erase(b.begin(), it);
}

std::string Parameters::getPredefinedFileName() {
	return getApplicationName() + PREDEFINED_FILE_NAME;
}
