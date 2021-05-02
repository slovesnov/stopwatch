/*
 * Base.cpp
 *
 *       Created on: 12.01.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "Frame.h"
#include "Base.h"
#include <windows.h>

const char* Base::TITLE[] = { "stopwatch", "minute", "time" };

double Base::getVerticalDPI() {
	auto monitor = gdk_display_get_monitor(gdk_display_get_default(), 0);
	GdkRectangle r;
	gdk_monitor_get_geometry(monitor, &r);
	auto h = gdk_monitor_get_height_mm(monitor);
	return r.height * 25.4 / h;
}

void Base::sendMciCommand(const char *s) {
#ifdef NDEBUG
	mciSendString(s, NULL, 0, 0);
#else
	int i = mciSendString(s, NULL, 0, 0);
	if (i) {
		println("Error %d when sending %s", i, s);
	}
#endif
}

void Base::beep() {
	char b[128];

	sendMciCommand("Close All");
	sprintf(b, "Open %s/beep.mp3 Type MPEGVideo Alias theMP3", PROJECT);
	sendMciCommand(b);

	const short v=0x3000;//v is volume from 0 to 0xffff
	waveOutSetVolume(0, v|(v<<16));//low word is left volume, high word is right volume

	//originally was "Play theMP3 Wait". In this case program wait until sound finish play.
	//If use "Play theMP3" command the function will be asynchronous
	sendMciCommand("Play theMP3");
}

VString Base::split(const std::string& subject, const std::string& separator) {
	VString r;
	size_t pos, prev;
	for (prev = 0; (pos = subject.find(separator, prev)) != std::string::npos;
			prev = pos + separator.length()) {
		r.push_back(subject.substr(prev, pos - prev));
	}
	r.push_back(subject.substr(prev, subject.length()));
	return r;
}

std::string Base::format(const char* _format, ...) {
	va_list args;
	va_start(args, _format);
	size_t size = vsnprintf(nullptr, 0, _format, args) + 1;
	std::string s;
	s.resize(size);
	vsnprintf(&s[0], size, _format, args);
	s.resize(size - 1);
	va_end(args);
	return s;
}

std::string Base::shortFileName(const char*file) {
	const char*p = strrchr(file, G_DIR_SEPARATOR);
	return p ? p + 1 : file;
}

bool Base::has(const VPredefinedDateType& v, int i) {
	auto a = std::find_if(v.cbegin(), v.cend(),
			[&] (auto e) {return e.first==i;});
	return a != v.cend();
}

bool Base::comparePredefinedDate(const PredefinedDateType& a,
		const PredefinedDateType& b) {
	auto amd = getMMDD(a.first);
	auto bmd = getMMDD(b.first);
	auto ay = getYYYY(a.first);
	auto by = getYYYY(b.first);

	if (amd == bmd) {
		return ay < by;
	}
	else {
		return amd < bmd;
	}
}

#ifdef BASE_ADDONS

typedef std::vector<std::string> VString;
typedef VString::iterator VStringI;
typedef VString::const_iterator VStringCI;

void Base::exploreAllChildrenRecursive(GtkWidget* w) {
	std::string s,q;
	VString v;
	VStringCI it;
	const char*p;
	GList *children, *iter;
	children=gtk_container_get_children ( GTK_CONTAINER(w) );

	for(iter = children; iter != NULL; iter = g_list_next(iter)) {
		w=GTK_WIDGET(iter->data);
		printinfo

		if(GTK_IS_CONTAINER(w)) {
			printinfo
			GList* ch = gtk_container_get_children(GTK_CONTAINER(w));
			if( g_list_length (ch) > 7 ) {
			}
			g_list_free(ch);
			exploreAllChildrenRecursive(w);
		}
		else {
			s=gtk_widget_path_to_string (gtk_widget_get_path(w));
			println("%s",s.c_str());
			v=split(s," ");
			q="";
			for(it=v.begin();it!=v.end();it++) {
				printinfo
				p=strchr(it->c_str(),':');
				assert(p);
				q+=it->substr(0,p-it->c_str());
				q+=" ";
			}
			if(strstr(s.c_str(),"list")) {
				println("%s", q.c_str());
			}
		}
	}
	g_list_free(children);

}
#endif
