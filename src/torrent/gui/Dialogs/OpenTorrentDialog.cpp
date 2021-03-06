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
#include <stddef.h>
#include <QDir>
#include <QMap>
#include <QMouseEvent>
#include <QMovie>
#include <QPainter>
#include <QTextCodec>
#include "messagebox.h"
#include "FileTreeModel.h"
#include "OpenTorrentDialog.h"
#include "QApplicationSettings.h"
#include "TorrentManager.h"
#include "qstorageinfo.h"
#include <viewModel/itemDelegate/FileSizeItemDelegate.h>
#include <viewModel/FileTreeSortProxyModel.h>
#include "MetaDataDownloadWaiter.h"
#include "StaticHelpers.h"
#include "defs.h"
#include <TorrentGroupsManager.h>
#include "FiltersViewModel.h"
#include "AddRssDwonloadRuleDialog.h"
#include <libtorrent/torrent_info.hpp>
#include "TorrentStorrage.h"
#include "Torrent.h"
#include <libtorrent/announce_entry.hpp>
#include <libtorrent/link.hpp>
#include <libtorrent/magnet_uri.hpp>

OpenTorrentDialog::OpenTorrentDialog(QWidget* parent, Qt::WindowFlags flags)
	: BaseWindow(BaseWindow::OnlyCloseButton, BaseWindow::NoResize, parent)
	  , m_size(0)
	  , m_bUseGroup(false)
	  , m_pFileTreeModel(NULL)
	  , m_pTerminationToken(new TerminationToken)
{
	setupUi(this);
	setupCustomWindow();
	setupWindowIcons();
	setAttribute(Qt::WA_DeleteOnClose, false);
	GroupComboBox = new TreeBox(this);
	gridLayout->addWidget(GroupComboBox, 1, 0, 1, 1);
	m_pGroupsModel = new FiltersViewModel(FiltersViewModel::Groups, this);
	GroupComboBox->setModel(m_pGroupsModel);
	GroupComboBox->setRootModelIndex(m_pGroupsModel->index(0, 0, QModelIndex()));
	GroupComboBox->setCurrentIndex(-1);
	qRegisterMetaType<error_code>("errro_code");
	connect(GroupComboBox, SIGNAL(currentIndexChanged(int)), SLOT(ChangeGroup()));
	m_pTorrentManager = TorrentManager::getInstance();
	QCompleter* pathComplitter = new QCompleter(this);
	m_compliterModel = new QFileSystemModel(pathComplitter);
	m_compliterModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
	m_compliterModel->setRootPath("");
	pathComplitter->setModel(m_compliterModel);
	pathEdit->setCompleter(pathComplitter);
	validTorrent = true;
	torrentFilesTreeView->setItemDelegateForColumn(1, new FileSizeItemDelegate(this));
}

void OpenTorrentDialog::reject()
{
	if (m_torrentFilename.startsWith("magnet"))
	{
		m_pTorrentManager->CancelMagnetLink(m_torrentFilename);

		if (m_pMetaDataWaiter != nullptr && m_pMetaDataWaiter->isRunning())
		{
			m_pTerminationToken->IsTerminationRequested = true;
			m_pMetaDataWaiter->wait();
		}
	}

	QDialog::reject();
}

int OpenTorrentDialog::exec()
{
	return validTorrent ? QDialog::exec() : Rejected;
}

OpenTorrentDialog::~OpenTorrentDialog()
{
	if (m_pFileTreeModel != NULL)
	{
		delete m_pFileTreeModel;
	}
}


void OpenTorrentDialog::FillData(opentorrent_info* info)
{
	labelNameData->setText(info->name);
	QFontMetrics metrics(labelComentData->font());
	QString elidedText = metrics.elidedText(info->describtion, Qt::ElideRight, labelComentData->width());
	labelComentData->setText(elidedText);
	m_size = info->size;
	labelSizeData->setText(StaticHelpers::toKbMbGb(info->size));
	m_pFileTreeModel = new FileTreeModel();
	FileTreeSortProxyModel* sortModel = new FileTreeSortProxyModel(this);
	file_storage files = info->torrentInfo->files();
	for (int i = 0; i < files.num_files(); i++)
	{
		m_pFileTreeModel->addPath(QString::fromUtf8(files.file_path(i).c_str()), files.file_size(i));
	}

	sortModel->setSourceModel(m_pFileTreeModel);
	torrentFilesTreeView->setModel(sortModel);
	torrentFilesTreeView->expandToDepth(0);
	torrentFilesTreeView->setColumnWidth(0, 300);
	torrentFilesTreeView->setColumnWidth(1, 60);
	torrentFilesTreeView->header()->setSortIndicator(0, Qt::AscendingOrder);

	if (!info->baseSuffix.isEmpty())
	{
		TorrentGroup* pTorrentGroup = TorrentGroupsManager::getInstance()->GetGroupByExtentions(info->baseSuffix);
		if (pTorrentGroup != NULL)
		{
			InternalySetPath(pTorrentGroup->savePath());
			QModelIndex groupIndex = m_pGroupsModel->index(pTorrentGroup->uid());
			if (groupIndex.isValid())
			{
				GroupComboBox->treeView()->expand(groupIndex);
				GroupComboBox->setCurrentIndex(groupIndex);
			}
			else
			{
				qDebug() << "Invalid QModelIndex recived for group" << pTorrentGroup->name();
			}
		}
		else
		{
			qDebug() << "No group found for ext. " << info->baseSuffix;
		}
	}
}

void OpenTorrentDialog::InternalySetPath(QString path)
{
	m_isInternalPathChange = true;
	pathEdit->setText(path);
	m_isInternalPathChange = false;
}

void OpenTorrentDialog::SetData(QString filename)
{
	m_torrentFilename = filename;
	QApplicationSettingsPtr settings = QApplicationSettings::getInstance();
	QString lastSaveDir = settings->valueString("System", "LastSaveTorrentDir", QDir::homePath());
	InternalySetPath(lastSaveDir);
	qDebug() << "lastSaveDir" << lastSaveDir;
	if (filename.startsWith("magnet"))
	{
		QMovie* movie = new QMovie(":/images/loader.gif");
		loaderGifLabel->setMovie(movie);
		movie->start();
		qRegisterMetaType<openmagnet_info>("openmagnet_info");
		m_pTerminationToken->IsTerminationRequested = false;
		m_pMetaDataWaiter = new MetaDataDownloadWaiter(filename, m_pTerminationToken, this);
		connect(m_pMetaDataWaiter, SIGNAL(DownloadCompleted(openmagnet_info)), this, SLOT(DownloadMetadataCompleted(openmagnet_info)));
		connect(m_pMetaDataWaiter, SIGNAL(ErrorOccured(error_code)), this, SLOT(OnError(error_code)));
		connect(m_pMetaDataWaiter, SIGNAL(finished()), m_pMetaDataWaiter, SLOT(deleteLater()));
		m_pMetaDataWaiter->start(QThread::HighPriority);
		yesButton->setEnabled(false);
		setSeedModeCheckBox->setEnabled(false);
		setSeedModeCheckBox->setToolTip(tr("MAGNET_LINKS_DONT_SUPORT_SEED_MODE"));
	}
	else
	{
		loaderGifLabel->hide();
		loaderTextLabel->hide();
		error_code ec;
		boost::scoped_ptr<opentorrent_info> info(m_pTorrentManager->GetTorrentInfo(m_torrentFilename, ec));

		if (info != NULL && ec == 0)
		{
			FillData(info.get());
		}
		else
		{
			OnError(ec);
		}
	}
}


void OpenTorrentDialog::BrowseButton()
{
	QApplicationSettingsPtr settings = QApplicationSettings::getInstance();
	QString lastDir = pathEdit->text();
	QString dir = QFileDialog::getExistingDirectory(this, tr("DIALOG_OPEN_DIR"),
	                                                lastDir,
	                                                QFileDialog::ShowDirsOnly
	                                                | QFileDialog::DontResolveSymlinks);

	if (!dir.isEmpty())
	{
		dir.append(QDir::separator());
		dir = QDir::toNativeSeparators(dir);
		settings->setValue("System", "LastSaveTorrentDir", dir);
		GroupComboBox->setCurrentIndex(-1);
		m_compliterModel->setRootPath(dir);
		pathEdit->setText(dir);
		m_bUseGroup = false;
	}
}


bool OpenTorrentDialog::AccepTorrent()
{
	if (validTorrent)
	{
		QMap<QString, quint8> filePriorities = m_pFileTreeModel->getFilePiorites();
		QList<quint8> values = filePriorities.values();
		qDebug() << filePriorities;
		if (values.count(0) == values.size())
		{
			CustomMessageBox::critical("Adding torrent Error", tr("SELECT_AT_LEAST_ONE_FILE"));
			return false;
		}

		error_code ec;
		QModelIndex groupIndex = GroupComboBox->currentIndex();
		QUuid groupUid = groupIndex.data(FiltersViewModel::FilterRole).value<QUuid>();
		TorrentGroup* group = TorrentGroupsManager::getInstance()->GetGroup(groupUid);
		TorrentManager::AddTorrentFlags flags = static_cast<TorrentManager::AddTorrentFlags>(BuildFlags());
		QString savePath = pathEdit->displayText();

		if (m_torrentFilename.startsWith("magnet"))
		{
			m_pTorrentManager->AddMagnet(m_info.handle, savePath, group, filePriorities, flags);
		}
		else
		{
			m_pTorrentManager->AddTorrent(m_torrentFilename, savePath, ec, labelNameData->text(), filePriorities, group, flags);
		}

		if (ec)
		{
			CustomMessageBox::critical("Adding torrent Error", StaticHelpers::translateLibTorrentError(ec));
			return false;
		}
	}


	return true;
}

void OpenTorrentDialog::ChangeGroup()
{
	QModelIndex index = GroupComboBox->view()->currentIndex();

	if (index.isValid())
	{
		QUuid groupUid = index.data(FiltersViewModel::FilterRole).value<QUuid>();
		if (!groupUid.isNull())
		{
			TorrentGroup* pGroup = TorrentGroupsManager::getInstance()->GetGroup(groupUid);
			if (pGroup != NULL)
			{
				pathEdit->setText(pGroup->savePath());
			}
		}
	}
}

void OpenTorrentDialog::DownloadMetadataCompleted(openmagnet_info info)
{
	yesButton->setEnabled(true);
	loaderGifLabel->hide();
	loaderTextLabel->hide();
	m_info = info;
	FillData(&info);
}


void OpenTorrentDialog::OnPathChanged(QString path)
{
	if (m_isInternalPathChange)
		return;

	QStorageInfo storageInfo(path);
	labelSizeData->setText(tr("%1 (AVAILABLE: %2)").arg(StaticHelpers::toKbMbGb(m_size), StaticHelpers::toKbMbGb(storageInfo.bytesAvailable())));
	if (m_size + 20l * 1024l * 1024l > storageInfo.bytesAvailable())
	{
		if (CustomMessageBox::critical(tr("NOT_ENOUGH_SPACE"), tr("NOT_ENOGH_STORRAGE_SPACE_TO_STORE_TORRENT"), CustomMessageBox::YesNo) == CustomMessageBox::No)
		{
			reject();
		}
	}
}

void OpenTorrentDialog::setCheckedValue(bool val)
{
	QAbstractItemModel* itemModel = torrentFilesTreeView->model();

	if (itemModel != NULL)
	{
		int rowCount = itemModel->rowCount();

		for (int i = 0; i < rowCount; i++)
		{
			itemModel->setData(itemModel->index(i, 0), val ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
		}
	}
}

void OpenTorrentDialog::OnCheckAll()
{
	setCheckedValue(true);
}

void OpenTorrentDialog::OnUncheckAll()
{
	setCheckedValue(false);
}

void OpenTorrentDialog::changeEvent(QEvent* event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		retranslateUi(this);
	}
}


QPushButton* OpenTorrentDialog::getCloseBtn()
{
	return pbClose;
}

QWidget* OpenTorrentDialog::getTitleBar()
{
	return titleBar;
}

QLabel* OpenTorrentDialog::getTitleLabel()
{
	return LTitle;
}

QLabel* OpenTorrentDialog::getTitleIcon()
{
	return tbMenu;
}

int OpenTorrentDialog::BuildFlags()
{
	TorrentManager::AddTorrentFlags res = 0;

	if (setSequntialCheckBox->isChecked())
	{
		res |= TorrentManager::SEQUENTIAL_MODE;
	}

	if (setSeedModeCheckBox->isChecked())
	{
		res |= TorrentManager::SEED_MODE;
	}

	if (setPausedCheckBox->isChecked())
	{
		res |= TorrentManager::PAUSED_MODE;
	}

	if (setSuperSeedCheckBox->isChecked())
	{
		res |= TorrentManager::SUPER_SEED_MODE;
	}

	if (skipNameInPathCheckBox->isChecked())
	{
		res |= TorrentManager::SKIP_ROOT_DIR;
	}
	return res;
}

void OpenTorrentDialog::done(int r)
{
	if (Accepted == r)
	{
		if (AccepTorrent())
		{
			QDialog::done(r);
		}
	}
	else if (Rejected == r)
	{
		QDialog::done(r);
	}
}

QWidget* OpenTorrentDialog::centralWidget()
{
	return m_centralWidget;
}

void OpenTorrentDialog::OnError(error_code ec)
{
	validTorrent = false;
	QString error_msg = StaticHelpers::translateLibTorrentError(ec);
	if (ec == errors::duplicate_torrent)
	{
		if (CustomMessageBox::question(tr("Confirm merge trackers"), error_msg + tr(" Add new Trackers from this torrent?")) == CustomMessageBox::Yes)
		{
			TorrentStorragePtr torrentStorrage = TorrentStorrage::getInstance();
			if (m_torrentFilename.startsWith("magnet"))
			{
				add_torrent_params add_info;
				parse_magnet_uri(m_torrentFilename.toStdString(), add_info, ec);
				Torrent* torrent = torrentStorrage->getTorrent(QString::fromStdString(add_info.info_hash.to_string()));
				if (torrent != nullptr)
				{
					std::vector<announce_entry> torrentTrackers = torrent->GetTrackerInfo();
					
					for (int i = 0; i < torrentTrackers.size(); i++)
					{
						int trackerToRemove = -1;
						for (int j = 0; j < add_info.trackers.size(); j++)
						{
							if (torrentTrackers[i].url == add_info.trackers[j])
							{
								trackerToRemove = j;
								break;
							}

						}
						if (trackerToRemove > -1)
						{
							add_info.trackers.erase(add_info.trackers.begin() + trackerToRemove);
						}
					}

					for (int j = 0; j < add_info.trackers.size(); j++)
					{
						torrent->AddTracker(QString::fromUtf8(add_info.trackers[j].c_str()));
					}
					if (add_info.trackers.size() > 0)
					{
						CustomMessageBox::information("Trackers added", tr("Added %1 trackers").arg(QString::number(add_info.trackers.size())));
					}
					else
					{
						CustomMessageBox::information("No trackers added", tr("No trackers added"));
					}
				}
			}
			else
			{
				
				boost::scoped_ptr<opentorrent_info> info(m_pTorrentManager->GetTorrentInfo(m_torrentFilename, ec));
				std::vector<announce_entry> trackers = info->torrentInfo->trackers();
				Torrent* torrent = torrentStorrage->getTorrent(info->infoHash);
				if (torrent != nullptr)
				{
					std::vector<announce_entry> torrentTrackers = torrent->GetTrackerInfo();
					//Remove Duplicates
					for (int i = 0; i < torrentTrackers.size(); i++)
					{
						int trackerToRemove = -1;
						for(int j=0; j < trackers.size(); j++)
						{
							if (torrentTrackers[i].url == trackers[j].url)
							{
								trackerToRemove = j;
								break;
							}
						}

						if (trackerToRemove > -1)
						{
							trackers.erase(trackers.begin() + trackerToRemove);
						}
					}
					for (int j = 0; j < trackers.size(); j++)
					{
						torrent->AddTracker(QString::fromUtf8(trackers[j].url.c_str()));
					}
					if (trackers.size() > 0)
					{
						CustomMessageBox::information("Trackers added", tr("Added %1 trackers").arg(QString::number(trackers.size())));
					}
					else
					{
						CustomMessageBox::information("No trackers added", tr("No trackers added"));
					}
				}
			}

			return;
		}
	}
	CustomMessageBox::critical("Error", error_msg);
	reject();
}
