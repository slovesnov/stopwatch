/*
 * EditSettingsFileDialog.h
 *
 *       Created on: 07.05.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#ifndef EDITSETTINGSFILEDIALOG_H_
#define EDITSETTINGSFILEDIALOG_H_

#include "Parameters.h"

class EditSettingsFileDialog: public Parameters {
	GtkWidget *dialog, *view, *info, *button;
	GtkTextBuffer *buffer;
public:
	gint responseCode;

	EditSettingsFileDialog(GtkWidget *parent);
	void updateInfo();
	void buttonClicked(GtkWidget *w);
};

#endif /* EDITSETTINGSFILEDIALOG_H_ */
