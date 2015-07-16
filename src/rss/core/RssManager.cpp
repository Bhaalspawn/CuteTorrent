#include "RssManager.h"
#include <QNetworkRequest>
#include <QNetworkDiskCache>
#include <QDesktopServices>
#include "QApplicationSettings.h"
#include "RssFeed.h"
#include "StaticHelpers.h"
#include "filedownloader.h"
#include "NotificationSystem.h"
#include "TorrentManager.h"
#include "MetaDataDownloadWaiter.h"
#include "RssItem.h"
#include "EmailNotifier.h"
RssManager::RssManager(QObject* parent) : QObject(parent)
{
	m_pTorrentDownloader = FileDownloader::getInstance();
	m_pNotificationSystem = NotificationSystem::getInstance();
	m_pSettings = QApplicationSettings::getInstance();
	QTimer::singleShot(500, this, SLOT(LoadFeeds()));
	QTimer::singleShot(500, this, SLOT(LoadDownloadRules()));
	connect(this, SIGNAL(Notify(int, QString, QVariant)), m_pNotificationSystem.get(), SLOT(OnNewNotification(int, QString, QVariant)));;
	connect(m_pTorrentDownloader.get(), SIGNAL(DownloadReady(QUrl, QTemporaryFile*)), SLOT(onTorrentDownloaded(QUrl, QTemporaryFile*)));
}

RssFeed* RssManager::addFeed(QUrl url, bool& isNew)
{
	isNew = true;

	foreach (RssFeed* pFeed, m_pFeeds)
	{
		if (pFeed->url() == url)
		{
			isNew = false;
			return pFeed;
		}
	}

	RssFeed* pFeed = new RssFeed(url, QUuid::createUuid());
	m_pFeeds.append(pFeed);
	connect(pFeed, SIGNAL(FeedChanged(QUuid)), SIGNAL(FeedChanged(QUuid)));
	emit FeedChanged(pFeed->uid());
	return pFeed;
}

RssManager::~RssManager()
{
}

QList<RssFeed*> RssManager::feeds()
{
	return m_pFeeds;
}

void RssManager::SaveFeeds()
{
	QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QString feedsPath = StaticHelpers::CombinePathes(dataDir, "feeds.dat");
	QFile feedsDat(feedsPath);

	if (feedsDat.open(QIODevice::WriteOnly))
	{
		QDataStream dataStream(&feedsDat);
		dataStream.setVersion(QDataStream::Qt_4_8);
		dataStream << m_pFeeds.size();

		for(int i = 0; i < m_pFeeds.size(); i++)
		{
			dataStream << *m_pFeeds[i];
		}

		feedsDat.flush();
		feedsDat.close();
	}
}

void RssManager::LoadFeeds()
{
	QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QString feedsPath = StaticHelpers::CombinePathes(dataDir, "feeds.dat");
	QFile feedsDat(feedsPath);

	if (feedsDat.open(QIODevice::ReadOnly))
	{
		QDataStream dataStream(&feedsDat);
		dataStream.setVersion(QDataStream::Qt_4_8);
		int numFeeds;
		dataStream >> numFeeds;
		QUuid tempUid = QUuid::createUuid();

		for (int i = 0; i < numFeeds; i++)
		{
			RssFeed* tempFeed = new RssFeed(QUrl(""), tempUid);
			QUuid uid = tempFeed->uid();
			dataStream >> *tempFeed;
			int index = -1;
			foreach(RssFeed* pFeed, m_pFeeds)
			{
				if (pFeed->uid() == uid)
				{
					index = m_pFeeds.indexOf(pFeed);
					break;
				}
			}
			if (index > -1)
			{
				delete tempFeed;
			}
			else
			{
				m_pFeeds.append(tempFeed);
				tempFeed->Update();
				connect(tempFeed, SIGNAL(FeedChanged(QUuid)), SLOT(onFeedChanged(QUuid)));
			}
			
		}

		feedsDat.close();
	}
}

void RssManager::removeFeed(const QUuid& uid)
{
	int index = 0;

	foreach(RssFeed* pFeed, m_pFeeds)
	{
		if (pFeed->uid() == uid)
		{
			break;
		}

		index++;
	}

	RssFeed* pFeed2Remove = m_pFeeds.at(index);
	m_pFeeds.removeAt(index);
	disconnect(pFeed2Remove, SIGNAL(FeedChanged(QUuid)), this, SLOT(onFeedChanged(QUuid)));
	pFeed2Remove->deleteLater();
	emit FeedChanged(uid);
}

void RssManager::addDownloadRule(RssDownloadRule* rule)
{
	bool ok;
	rule->validate(ok);

	if (ok)
	{
		m_downloadRules.insert(rule->Uid(), rule);

		foreach(RssFeed* pFeed, m_pFeeds)
		{
			onFeedChanged(pFeed->uid());
		}
	}
}

void RssManager::removeDownloadRule(const QUuid& uid)
{
	m_downloadRules.remove(uid);
}

RssDownloadRule* RssManager::findDownloadRule(const QUuid& uid)
{
	foreach(RssDownloadRule* rule, m_downloadRules.values())
	{
		if (rule->Uid() == uid)
		{
			return rule;
		}
	}

	return NULL;
}

void RssManager::updateDownloadRule(RssDownloadRule* rule)
{
	removeDownloadRule(rule->Uid());
	addDownloadRule(rule);
}

QList<RssDownloadRule*> RssManager::downloadRules()
{
	return m_downloadRules.values();
}

void RssManager::LoadDownloadRules()
{
	QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QString rssDownloadRulesPath = StaticHelpers::CombinePathes(dataDir, "rssDownloadRules.dat");
	QFile rssDownloadRulesDat(rssDownloadRulesPath);

	if (rssDownloadRulesDat.open(QIODevice::ReadOnly))
	{
		QDataStream dataStream(&rssDownloadRulesDat);
		dataStream.setVersion(QDataStream::Qt_4_8);
		int numDownloadRules = 0;
		dataStream >> numDownloadRules;
		m_downloadRules.reserve(numDownloadRules);

		for (int i = 0; i < numDownloadRules; i++)
		{
			RssDownloadRule* rule = new RssDownloadRule();
			dataStream >> *rule;
			m_downloadRules.insert(rule->Uid(), rule);;
		}

		rssDownloadRulesDat.close();
	}
}

QMap<QString, quint8> RssManager::getFilePriorities(TorrentDownloadInfo downloadInfo, file_storage fileStorage)
{
	QMap<QString, quint8> result;

	foreach(RssDownloadRule* pRule, m_downloadRules)
	{
		if (pRule->RuleType() == RssDownloadRule::SELECT_FILE_RULE && pRule->MatchFeed(downloadInfo.rssFeedId))
		{
			for (int i = 0; i < fileStorage.num_files(); i++)
			{
				QString fileName = QString::fromUtf8(fileStorage.file_path(i).c_str());

				if (pRule->MatchFile(fileName))
				{
					result.insert(fileName, 1);
				}
				else
				{
					result.insert(fileName, 0);
				}
			}
		}
	}

	return result;
}


void RssManager::SaveDownloadRules()
{
	QString dataDir = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
	QString rssDownloadRulesPath = StaticHelpers::CombinePathes(dataDir, "rssDownloadRules.dat");
	QFile rssDownloadRulesDat(rssDownloadRulesPath);

	if (rssDownloadRulesDat.open(QIODevice::WriteOnly))
	{
		QDataStream dataStream(&rssDownloadRulesDat);
		dataStream.setVersion(QDataStream::Qt_4_8);
		dataStream << m_downloadRules.size();

		foreach(RssDownloadRule* rule, m_downloadRules.values())
		{
			dataStream << *rule;
		}

		rssDownloadRulesDat.flush();
		rssDownloadRulesDat.close();
	}
}

void RssManager::onFeedChanged(QUuid uid)
{
	emit FeedChanged(uid);

	foreach(RssDownloadRule* rule, m_downloadRules.values())
	{
		if (rule->MatchFeed(uid))
		{
			RssFeed* pFeed = findFeed(uid);

			if (pFeed != NULL)
			{
				QList<RssItem*> feedItems = pFeed->GetFeedItems();

				foreach(RssItem* rssItem, feedItems)
				{
					if (rssItem->downloadingTorrent().isEmpty() && rule->Match(rssItem))
					{
						downloadRssItem(rssItem, pFeed, rule);
					}
				}
			}
		}
	}
}

RssFeed* RssManager::findFeed(const QUuid& uid)
{
	foreach(RssFeed* pFeed, m_pFeeds)
	{
		if (pFeed->uid() == uid)
		{
			return pFeed;
		}
	}

	return NULL;
}

void RssManager::downloadRssItem(RssItem* rssItem, RssFeed* pFeed, RssDownloadRule* rule)
{
	if (!rssItem->torrentUrl().isEmpty())
	{
		QString torrentUrl = rssItem->torrentUrl();
		if (!m_activeTorrentDownloads.contains(torrentUrl))
		{
			TorrentDownloadInfo info;
			info.downloadRule = rule;
			info.torrentUrl = torrentUrl;
			info.rssFeedId = pFeed->uid();
			info.rssItemId = rssItem->guid();
			m_activeTorrentDownloads.insert(torrentUrl, info);
			m_pTorrentDownloader->download(torrentUrl, pFeed->coookies());
		}
		
	}
	else if (!rssItem->magnetUrl().isEmpty())
	{
		QString magnetUrl = QUrl::fromPercentEncoding(rssItem->magnetUrl().toUtf8());
		if (!m_activeTorrentDownloads.contains(magnetUrl))
		{
			MetaDataDownloadWaiter* magnetWaiter = new MetaDataDownloadWaiter(magnetUrl, this);
			connect(magnetWaiter, SIGNAL(DownloadCompleted(openmagnet_info)), this, SLOT(onDownloadMetadataCompleted(openmagnet_info)));
			connect(magnetWaiter, SIGNAL(ErrorOccured(QString)), this, SLOT(onMagnetError(QString)));
			connect(magnetWaiter, SIGNAL(finished()), magnetWaiter, SLOT(deleteLater()));
			magnetWaiter->start(QThread::HighPriority);
			TorrentDownloadInfo info;
			info.downloadRule = rule;
			info.torrentUrl = magnetUrl;
			info.rssFeedId = pFeed->uid();
			info.rssItemId = rssItem->guid();
			m_activeTorrentDownloads.insert(magnetUrl, info);
		}
	}
	else
	{
		emit Notify(NotificationSystem::RSS_ERROR, tr("RSS_ITEM_UNABLE_TO_DOWNLOAD %1 NO_URL").arg(rssItem->title()), QVariant());
	}
}

void RssManager::onTorrentDownloaded(QUrl url, QTemporaryFile* pUnsafeFile)
{
	TorrentDownloadInfo info = m_activeTorrentDownloads[url];
	m_activeTorrentDownloads.remove(url);
	boost::scoped_ptr<QTemporaryFile> pFile(pUnsafeFile);
	QString torrentFilePath = pFile->fileName();
	TorrentManagerPtr pTorrentManager = TorrentManager::getInstance();
	error_code ec;
	RssFeed* pFeed = findFeed(info.rssFeedId);
	RssItem* feedItem = pFeed->GetFeedItem(info.rssItemId);
	boost::scoped_ptr<opentorrent_info> pTorrentInfo(pTorrentManager->GetTorrentInfo(torrentFilePath, ec));

	if (ec.category() == get_libtorrent_category())
	{
		if (ec.value() == errors::duplicate_torrent)
		{
			feedItem->setInfoHash(pTorrentInfo->infoHash);
			return;
		}
	}

	if (ec)
	{
		emit Notify(NotificationSystem::RSS_ERROR, tr("ERROR_DURING_AUTOMATED_RSS_DOWNLOAD: %1 %2").arg(StaticHelpers::translateLibTorrentError(ec), feedItem->title()), QVariant());
		return;
	}

	QString savePath = gessSavePath(info.downloadRule, pTorrentInfo->baseSuffix);
	QMap<QString, quint8> filePriorities = getFilePriorities(info, pTorrentInfo->files);
	pTorrentManager->AddTorrent(torrentFilePath, savePath, "", ec, filePriorities);

	if (ec)
	{
		emit Notify(NotificationSystem::RSS_ERROR, tr("ERROR_DURING_AUTOMATED_RSS_DOWNLOAD: %1 %2").arg(StaticHelpers::translateLibTorrentError(ec), feedItem->title()), QVariant());
	}
	else
	{
		emit Notify(NotificationSystem::TORRENT_INFO, tr("AUTOMATED_RSS_DOWNLOAD_START_DOWNLOAD: %1 %2").arg(pTorrentInfo->name, feedItem->title()), QVariant());

		if (m_pSettings->valueBool("rss", "auto_download_emeail_notify"))
		{
			EmailNotifierPtr emailNotifier(new EmailNotifier());
			QString to = m_pSettings->valueString("rss", "rss_send_to");
			emailNotifier->SendEmail(to, tr("STARTED_AUTOMETED_RSS_DOWNLOAD"), tr("%1 STARTED_DOWNLOADING.<br/> <a href=\"%3\">DESCRIBTION</a><br/> %2").arg(feedItem->title(), feedItem->description(),
			                         feedItem->describtionLink()));
		}

		RssFeed* rssFeed = findFeed(info.rssFeedId);
		RssItem* rssItem = rssFeed->GetFeedItem(info.rssItemId);
		rssItem->setDownloadingTorrent(pTorrentInfo->infoHash);
	}
}

void RssManager::onMagnetError(QString error)
{
	emit Notify(NotificationSystem::RSS_ERROR, tr("ERROR_DURING_AUTOMATED_RSS_DOWNLOAD_MAGNET: %1").arg(error), QVariant());
}

void RssManager::onDownloadMetadataCompleted(openmagnet_info magnetInfo)
{
	TorrentManagerPtr pTorrentManager = TorrentManager::getInstance();

	if (m_activeTorrentDownloads.contains(magnetInfo.link))
	{
		TorrentDownloadInfo info = m_activeTorrentDownloads[magnetInfo.link];
		QString savePath = gessSavePath(info.downloadRule, magnetInfo.baseSuffix);
		pTorrentManager->AddMagnet(magnetInfo.handle, savePath);
		m_activeTorrentDownloads.remove(magnetInfo.link);
		emit Notify(NotificationSystem::TORRENT_INFO, tr("AUTOMATED_RSS_DOWNLOAD_START_DOWNLOAD: %1").arg(magnetInfo.name), QVariant());
	}
}

QString RssManager::gessSavePath(RssDownloadRule* downloadRule, QString base_suffix)
{
	QApplicationSettingsPtr pSettings = QApplicationSettings::getInstance();

	if (downloadRule->UseStaticSavePath())
	{
		return downloadRule->StaticSavePath();
	}
	else
	{
		QList<GroupForFileFiltering> filters = pSettings->GetFileFilterGroups();

		foreach(GroupForFileFiltering filter, filters)
		{
			if (filter.Contains(base_suffix))
			{
				return filter.SavePath();
			}
		}
	}

	QString lastSaveDir = pSettings->valueString("System", "LastSaveTorrentDir");
	return lastSaveDir;
}

