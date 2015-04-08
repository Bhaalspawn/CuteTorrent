﻿#include "QRssDisplayModel.h"
#include "RssFeed.h"
#include "RssFeedTreeItem.h"
#include "RssFeedTreeItem.h"
#include "RssFeedItemTreeItem.h"
#include "RssManager.h"
#include "messagebox.h"
#include "StyleEngene.h"
#include "OpenTorrentDialog.h"
#include "torrentdownloader.h"
#include <QInputDialog>

QRssDisplayModel::QRssDisplayModel(QTreeView* pItemsView, QObject* parrent) : QAbstractItemModel(parrent), m_pRssManager(RssManager::getInstance()), m_pTorrentDownloader(TorrentDownloader::getInstance())
{
	m_pItemsView = pItemsView;
	m_pUdpateTimer = new QTimer(this);
	setupFeedMenu();
	setupItemMenu();
	UpdateModel();
	QObject::connect(m_pRssManager.get(), SIGNAL(FeedChanged(QUuid)), this, SLOT(UpdateModel()));
	QObject::connect(m_pTorrentDownloader.get(), SIGNAL(TorrentReady(QUrl, QTemporaryFile*)), SLOT(onTorrentDownloaded(QUrl, QTemporaryFile*)));
	/*QObject::connect(m_pTorrentsManager, SIGNAL(OnNewFeed()), this, SLOT(UpdateModel()));
	QObject::connect(m_pTorrentsManager, SIGNAL(OnFeedDeleted()), this, SLOT(UpdateModel()));
	QObject::connect(m_pTorrentsManager, SIGNAL(OnNewFeedItem()), this, SLOT(UpdateModel()));
	QObject::connect(m_pTorrentsManager, SIGNAL(OnFeedChanged()), this, SLOT(UpdateModel()));*/
	QObject::connect(m_pUdpateTimer, SIGNAL(timeout()), this, SLOT(UpdateVisibleData()));
	m_pUdpateTimer->start(1000);
	
}

int QRssDisplayModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	return 1;
}

QVariant QRssDisplayModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
	if (role == RssFeedItemRole && index.row() < m_pRssManager->feeds().count())
	{
		return qVariantFromValue(m_pRssManager->feeds().at(index.row()));
	}
	return QVariant();
}

int QRssDisplayModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
	if (!parent.isValid())
	{
		qDebug() << "rowCount root_items" << m_rootItems.count();
		return m_rootItems.count();
	}
	RssBaseTreeItem* parentBaseItem = static_cast<RssBaseTreeItem*>(parent.internalPointer());
	if (parentBaseItem != NULL && parentBaseItem->GetType() == RssBaseTreeItem::Feed)
	{
		RssFeedTreeItem* pTreeFeedItem = reinterpret_cast<RssFeedTreeItem*>(parentBaseItem);
		qDebug() << "rowCount children_items" << pTreeFeedItem->Children().count();
		return pTreeFeedItem->Children().count();
	}
	return 0;
}




QModelIndex QRssDisplayModel::index(int row, int column, const QModelIndex &parent /*= QModelIndex()*/) const
{
	if (!hasIndex(row, column, parent))
	{
		return QModelIndex();
	}

	RssFeedTreeItem* parentItem;

	if (!parent.isValid())
	{
		parentItem = m_rootItems.at(row);
		return createIndex(row, column, parentItem);
	}
	else
	{
		RssBaseTreeItem* parentBaseItem = static_cast<RssBaseTreeItem*>(parent.internalPointer());
		if (parentBaseItem != NULL && parentBaseItem->GetType() == RssBaseTreeItem::Feed)
		{
			parentItem = static_cast<RssFeedTreeItem*>(parent.internalPointer());
		}
		else
		{
			return QModelIndex();
		}
	}

	RssFeedItemTreeItem* childItem = parentItem->Children().at(row);

	if (childItem != NULL)
	{
		return createIndex(row, column, childItem);
	}
	else
	{
		return QModelIndex();
	}
}

QModelIndex QRssDisplayModel::parent(const QModelIndex &child) const
{
	if (!child.isValid())
	{
		return QModelIndex();
	}

	RssBaseTreeItem* pBaseChildItem = static_cast<RssBaseTreeItem*>(child.internalPointer());
	if (pBaseChildItem->GetType() != RssBaseTreeItem::FeedItem)
	{
		return QModelIndex();
	}
	RssFeedItemTreeItem* pChildItem = reinterpret_cast<RssFeedItemTreeItem*>(pBaseChildItem);
	RssFeedTreeItem* pParentItem = pChildItem->Parent();
	int row = m_rootItems.indexOf(pParentItem);

	return createIndex(row, 0, pParentItem);
}

void QRssDisplayModel::UpdateModel()
{
	qDebug() << Q_FUNC_INFO;
	qDeleteAll(m_rootItems);
	m_rootItems.clear();
	QList<RssFeed*> feeds = m_pRssManager->feeds();
	qDebug() << "m_feeds" << feeds.count();
	for each (RssFeed* var in feeds)
	{
		m_rootItems.append(new RssFeedTreeItem(var));
	}
	reset();
}

void QRssDisplayModel::UpdateVisibleData()
{
	QModelIndex top(index(0, 0)), bot(index(m_rootItems.count() - 1, 0));
	emit dataChanged(top, bot);
}

QList<RssFeed*> QRssDisplayModel::SelectedFeeds()
{
	QSet<RssFeed*> res;
	QModelIndexList selectedIndexes = m_pItemsView->selectionModel()->selectedIndexes();
	for each (QModelIndex selectedIndex in selectedIndexes)
	{
		if (selectedIndex.isValid())
		{
			RssBaseTreeItem* pBaseItem = static_cast<RssBaseTreeItem*>(selectedIndex.internalPointer());
			switch (pBaseItem->GetType())
			{
			case RssBaseTreeItem::Feed:
			{
				RssFeedTreeItem* pFeedTreeItem = reinterpret_cast<RssFeedTreeItem*>(pBaseItem);
				res.insert(pFeedTreeItem->GetFeed());
				break;
			}
			case RssBaseTreeItem::FeedItem:
			{
				RssFeedItemTreeItem* pFeedTreeItem = reinterpret_cast<RssFeedItemTreeItem*>(pBaseItem);
				res.insert(pFeedTreeItem->Parent()->GetFeed());
				break;
			}
			default:
			{
				qDebug() << "Unknown item type";
				break;
			}

			}
		}
	}
	return res.toList();
}
RssItem QRssDisplayModel::SelectedFeedItem()
{
	RssItem res;
	QModelIndex selectedIndex = m_pItemsView->selectionModel()->currentIndex();
	if (selectedIndex.isValid())
	{
		RssBaseTreeItem* pBaseItem = static_cast<RssBaseTreeItem*>(selectedIndex.internalPointer());
		switch (pBaseItem->GetType())
		{
			case RssBaseTreeItem::FeedItem:
			{
				RssFeedItemTreeItem* pFeedTreeItem = reinterpret_cast<RssFeedItemTreeItem*>(pBaseItem);
				res = pFeedTreeItem->FeedItem();
				pFeedTreeItem->Parent()->GetFeed()->setItemUnread(false, res["guid"].toString());
				break;
			}
			default:
			{
				qDebug() << "Unknown item type";
				break;
			}
		}
	}
	return res;
}
RssFeed* QRssDisplayModel::SelectedFeed()
{
	RssFeed* res = NULL;
	QModelIndex selectedIndex = m_pItemsView->selectionModel()->currentIndex();
	if (selectedIndex.isValid())
	{
		RssBaseTreeItem* pBaseItem = static_cast<RssBaseTreeItem*>(selectedIndex.internalPointer());
		switch (pBaseItem->GetType())
		{
			case RssBaseTreeItem::Feed:
			{
				RssFeedTreeItem* pFeedTreeItem = reinterpret_cast<RssFeedTreeItem*>(pBaseItem);
				res = pFeedTreeItem->GetFeed();
				break;
			}
			case RssBaseTreeItem::FeedItem:
			{
				RssFeedItemTreeItem* pFeedTreeItem = reinterpret_cast<RssFeedItemTreeItem*>(pBaseItem);
				res = pFeedTreeItem->Parent()->GetFeed();
				break;
			}
			default:
			{
				qDebug() << "Unknown item type";
				break;
			}
		}
	}
	return res;
}

QRssDisplayModel::~QRssDisplayModel()
{
	qDeleteAll(m_rootItems);
	m_rootItems.clear();
}

void QRssDisplayModel::retranslate()
{
	updateAction->setText(tr("UPDATE_FEED_ITEM"));
	renameAction->setText(tr("RENAME_FEED_ITEM"));
	removeAction->setText(tr("REMOVE_FEED_ITEM"));
}

void QRssDisplayModel::contextualMenu(const QPoint& point)
{
	if (m_pItemsView->model() != this)
	{
		return;
	}
	QModelIndex index = m_pItemsView->indexAt(point);
	if (index.isValid())
	{
		RssBaseTreeItem* pBaseItem = static_cast<RssBaseTreeItem*>(index.internalPointer());
		RssBaseTreeItem::ItemType type = pBaseItem->GetType();
		if (type == RssBaseTreeItem::Feed)
		{
			m_pFeedMenu->exec(m_pItemsView->mapToGlobal(point));
		}
		else if (type == RssBaseTreeItem::FeedItem)
		{
			RssFeedItemTreeItem* pFeedItemTreeItem = reinterpret_cast<RssFeedItemTreeItem*>(pBaseItem);
			RssItem item = pFeedItemTreeItem->FeedItem();
			downloadTorrent->setEnabled(item.contains("torrent_url"));
			openLink->setEnabled(item.contains("link"));
			m_pItemMenu->exec(m_pItemsView->mapToGlobal(point));
		}

	}
}

void QRssDisplayModel::setupFeedMenu()
{
	StyleEngene* pStyleEngine = StyleEngene::getInstance();
	m_pFeedMenu = new QMenu(m_pItemsView);
	updateAction = new QAction(pStyleEngine->getIcon("update_trackers"), tr("UPDATE_FEED_ITEM"), m_pFeedMenu);
	updateAction->setObjectName("ACTION_RSSLIST_UPDATE");
	updateAction->setShortcut(Qt::Key_F5);
	QObject::connect(updateAction, SIGNAL(triggered()), SLOT(onFeedUpdate()));
	renameAction = new QAction(pStyleEngine->getIcon("move_folder"), tr("RENAME_FEED_ITEM"), m_pFeedMenu);
	renameAction->setObjectName("ACTION_RSSLIST_RENAME");
	renameAction->setShortcut(Qt::Key_F2);
	QObject::connect(renameAction, SIGNAL(triggered()), SLOT(onFeedRename()));
	removeAction = new QAction(pStyleEngine->getIcon("delete"), tr("REMOVE_FEED_ITEM"), m_pFeedMenu);
	removeAction->setObjectName("ACTION_RSSLIST_REMOVE");
	removeAction->setShortcut(Qt::Key_Delete);
	QObject::connect(removeAction, SIGNAL(triggered()), SLOT(onFeedRemove()));
	m_pFeedMenu->addAction(updateAction);
	m_pFeedMenu->addAction(renameAction);
	m_pFeedMenu->addAction(removeAction);
}

void QRssDisplayModel::onFeedUpdate()
{
	if (m_pItemsView->model() == this)
	{
		RssFeed* current = SelectedFeed();
		if (current != NULL)
		{
			current->Update();
		}
	}
	
}

void QRssDisplayModel::onFeedRename()
{
	if (m_pItemsView->model() == this)
	{
		RssFeed* current = SelectedFeed();
		if (current != NULL)
		{
			bool ok;
			QString newFeedDisplayName = QInputDialog::getText(m_pItemsView, tr("RSS_FEED_RENAME"), tr("RSS_FEED_NAME:"), QLineEdit::Normal, current->displayName(true), &ok);
			if (ok && !newFeedDisplayName.isEmpty())
			{
				current->setDisplayName(newFeedDisplayName);
			}
		}
	}
}

void QRssDisplayModel::onFeedRemove()
{
	if (m_pItemsView->model() == this)
	{
		QList<RssFeed*> selectedItems = SelectedFeeds();
		if (!selectedItems.isEmpty())
		{
			//beginResetModel();
			RssManagerPtr pRssManager = RssManager::getInstance();
			bool yesToAll = false;
			foreach(RssFeed* pFeed, selectedItems)
			{
				QMessageBox::StandardButton button;
				if (!yesToAll)
				{
					QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No;
					if (selectedItems.length() > 1)
					{
						buttons |= QMessageBox::YesToAll;
						buttons |= QMessageBox::NoToAll;
					}
					button = MyMessageBox::warning(m_pItemsView, tr("RSS_FEED_DELETE"), tr("RSS_FEED_DELETE_MSG").arg(pFeed->displayName()), buttons);
					if (button == QMessageBox::YesToAll)
					{
						yesToAll = true;
					}
					else if (button == QMessageBox::NoToAll)
					{
						break;
					}
				}
				else
				{
					button = QMessageBox::YesToAll;
				}

				if (QMessageBox::No != button || yesToAll)
				{
					pRssManager->removeFeed(pFeed->uid());
				}

			}
			//endResetModel();
		}
	}
}

void QRssDisplayModel::setupItemMenu()
{
	StyleEngene* pStyleEngine = StyleEngene::getInstance();
	m_pItemMenu = new QMenu(m_pItemsView);
	downloadTorrent = new QAction(pStyleEngine->getIcon("toolbar_download"), tr("DOWNLOAD_FEED_ITEM"), m_pFeedMenu);
	downloadTorrent->setObjectName("ACTION_RSSLIST_DOWNLOAD");
	QObject::connect(downloadTorrent, SIGNAL(triggered()), SLOT(onItemDownload()));
	openLink = new QAction(pStyleEngine->getIcon("open_link"), tr("OPEN_DESC_FEED_ITEM"), m_pFeedMenu);
	openLink->setObjectName("ACTION_RSSLIST_OPEN_DESC");
	QObject::connect(openLink, SIGNAL(triggered()), SLOT(onItemOpenDesc()));
	m_pItemMenu->addAction(openLink);
	m_pItemMenu->addAction(downloadTorrent);
}

void QRssDisplayModel::onItemDownload()
{
	if (m_pItemsView->model() == this)
	{
		RssItem current = SelectedFeedItem();
		QString torrentUrl = current["torrent_url"].toString();
		if (torrentUrl.startsWith("magnet"))
		{
			boost::scoped_ptr<OpenTorrentDialog> pDlg(new OpenTorrentDialog(m_pItemsView));
			pDlg->SetData(torrentUrl);
			pDlg->execConditional();
		}
		else
		{
			m_pTorrentDownloader->downloadTorrent(torrentUrl);
		}
		
	}
}

void QRssDisplayModel::onItemOpenDesc()
{
	if (m_pItemsView->model() == this)
	{
		RssItem current = SelectedFeedItem();
		QDesktopServices::openUrl(current["link"].toString());
	}
}

void QRssDisplayModel::onTorrentDownloaded(QUrl url, QTemporaryFile* pfile)
{
	qDebug() << "Temp FileName is: " << pfile->fileName();
	boost::scoped_ptr<OpenTorrentDialog> pDlg (new OpenTorrentDialog(m_pItemsView));
	pDlg->SetData(pfile->fileName());
	pDlg->execConditional();
	pfile->setAutoRemove(true);
	delete pfile;
}

