/*
 * Dialog.h
 *
 *       Created on: 03.01.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#ifndef DIALOG_H_
#define DIALOG_H_

#include "Parameters.h"

class Dialog: public Parameters {
	GtkWidget *dialog, *entry, *comboMode, *lError, *lInfo, *mPredefined;
	GtkWidget *minimizeCheck, *digitalModeCheck, *closeWarningCheck, *volume;
	GtkWidget *okButton, *copyButton, *upcomingButton, *helpButton,
			*reloadButton, *upcomingAllButton, *editButton, *testSoundButton;
	bool signals = true;

	void updateParametersChanges(bool ok = true);
	void updateEntryChanges(bool ok);
	void updateReload();
	void fillPredefinedSet();
	void updateLabelInfo();
	void updateLoadPredefined();

public:
	gboolean digitalMode;
	gboolean closeWarning;
	double soundVolume;

	Dialog(DialogType dt, const std::string message = "");
	void buttonClicked(GtkWidget *w, int n=-1);
	void entryActivated();
	void entryChanged();
	void checkChanged(GtkToggleButton *w);
	void comboChanged(GtkComboBox *w);
	void modifyMinimizeCheck();
	void soundVolumeChanged(double v);
};

#endif /* DIALOG_H_ */
