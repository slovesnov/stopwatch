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
#include "help.h"
#include "Config.h"

#include <windows.h>//ERROR has in windows & in DialogType, so include after help.h

void sendMciCommand(const char *s) {
#ifdef NDEBUG
	mciSendString(s, NULL, 0, 0);
#else
	int i = mciSendString(s, NULL, 0, 0);
	if (i) {
		println("Error %d when sending %s", i, s);
	}
#endif
}

//volume from 0 to 0xffff
void beep(short volume) {
	std::string s;

	sendMciCommand("Close All");
	s= "Open "+getResourcePath("beep.mp3")+" Type MPEGVideo Alias theMP3";
	sendMciCommand(s.c_str());

	waveOutSetVolume(0, volume|(volume<<16));//low word is left volume, high word is right volume

	//originally was "Play theMP3 Wait". In this case program wait until sound finish play.
	//If use "Play theMP3" command the function will be asynchronous
	sendMciCommand("Play theMP3");
}

void beep() {
	beep(config.soundVolume);
}

bool has(const VPredefinedDateType& v, int i) {
	auto a = std::find_if(v.cbegin(), v.cend(),
			[&] (auto e) {return e.first==i;});
	return a != v.cend();
}

bool comparePredefinedDate(const PredefinedDateType& a,
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

typedef VString::iterator VStringI;
typedef VString::const_iterator VStringCI;

void exploreAllChildrenRecursive(GtkWidget* w) {
	std::string s,q;
	VString v;
	VStringCI it;
	const char*p;
	GList *children, *iter;
	children=gtk_container_get_children ( GTK_CONTAINER(w) );

	for(iter = children; iter != NULL; iter = g_list_next(iter)) {
		w=GTK_WIDGET(iter->data);
		printi

		if(GTK_IS_CONTAINER(w)) {
			printi
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
				printi
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

double secondsSince(clock_t t) {
	return double(clock() - t) / CLOCKS_PER_SEC;
}

int getYYYYMMDD(BeepTimeType v) {
	return v / BTDIVISOR;
}

int getHHMM(BeepTimeType v) {
	return v % BTDIVISOR;
}

int getYYYY(int v) {
	return v / MMDDDIVISOR;
}

int getMM(int v) {
	return getMMDD(v) / MMDIVISOR;
}

int getDD(int v) {
	return getMMDD(v) % MMDIVISOR;
}

int getMMDD(int v) {
	return v % MMDDDIVISOR;
}

bool yesNoDialog(std::string text) {
	auto dialog = gtk_message_dialog_new(GTK_WINDOW(frame.window),
			GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO, text.c_str());
	gtk_window_set_title(GTK_WINDOW(dialog), "Question");
	//GTK_RESPONSE_DELETE_EVENT, GTK_RESPONSE_NO, GTK_RESPONSE_YES
	bool b = gtk_dialog_run(GTK_DIALOG(dialog))==GTK_RESPONSE_YES;
	gtk_widget_destroy(dialog);
	return b;
}
