﻿/*
CuteTorrent BitTorrent Client with dht support, userfriendly interface
and some additional features which make it more convenient.
Copyright (C) 2012 Ruslan Fedoseyenko

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CTD_H
#define CTD_H
#include <QDialog>
#include "ui_CreateTorrentDialog.h"
#include "CustomWindow.h"
#include "ServiceCommon.h"
#include "TorrentCommon.h"
class CreateTorrentDialog;
class StyledProgressBar;

#ifdef Q_OS_WIN 
class QWinTaskbarProgress;
class QWinTaskbarButton;
#endif
#define AUTODETECT_VAL 123

class torrentCreatorThread : public QThread
{
	Q_OBJECT

	QString input_path;
	QString save_path;
	QStringList trackers;
	QStringList url_seeds;
	QString comment;
	bool is_private;
	qint64 piece_size;
	bool abort;
	CreateTorrentDialog* parent;
	QString filter;

public:
	torrentCreatorThread(CreateTorrentDialog* _parent)
	{
		parent = _parent;
	}

	~torrentCreatorThread()
	{
		abort = true;
		wait();
	}

	void create(QString _input_path, QString _save_path, QString _filter, QStringList _trackers, QStringList _url_seeds, QString _comment, bool _is_private, qint64 _piece_size);
	void sendProgressSignal(int progress);


protected:
	void run();

	signals:
	void ShowCreationFailture(QString msg);
	void ShowCreationSucces(QString path);
	void updateProgress(int progress);
};

class CreateTorrentDialog : public BaseWindow<QDialog> , private Ui::CreateTorrentDialog
{
	Q_OBJECT

public:

	CreateTorrentDialog(QWidget* parent = 0, Qt::WindowFlags flags = 0);
	~CreateTorrentDialog();
	void setPath(QString val);
protected:

	QPushButton* getCloseBtn() override;
	QWidget* getTitleBar() override;
	QWidget* centralWidget() override;
	QLabel* getTitleLabel() override;
	QLabel* getTitleIcon() override;
private:
	void setupPieceComboBox();
#ifdef Q_OS_WIN 
	QWinTaskbarProgress* m_pTaskBarProggres;
	QWinTaskbarButton* m_pTaskBarBtn;
#endif
	QString path;
	QFileSystemModel* m_compliterModel;
	quint64 listFolder(QString path);
	torrentCreatorThread* creator;
	TorrentManagerPtr m_pTorrentManager;
	QApplicationSettingsPtr settings;
	StyledProgressBar* progressBar;
	quint64 getPiceSize();
	signals:
	void AbortCreation();
private slots:
	void ShowCreationSucces(QString);
	void ShowCreationFailture(QString);
	void UpdateProgressBar(int);
	void BrowseDir();
	void BrowseFile();
	void BeginCreate();
	void Cancel();
};
#endif

