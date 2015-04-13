#ifndef _RSS_MANAGER_INCLUDED_
#define _RSS_MANAGER_INCLUDED_

#include <QHash>
#include <QUrl>
#include <QUuid>
#include <QRunnable>
#include <QQueue>
#include <boost/weak_ptr.hpp>
#include "RssCommon.h"
#include "RssDownloadRule.h"
#include "ServiceCommon.h"
#include "defs.h"
class QApplicationSettings;
class RssItem;
class RssManager : public QObject
{
	Q_OBJECT
public:

	QList<RssFeed*> feeds();
	RssFeed* addFeed(QUrl url, bool& isNew);
	RssFeed* findFeed(const QUuid& uid);
	void removeFeed(const QUuid& uid);
	void addDownloadRule(RssDownloadRule* rule);
	void removeDownloadRule(const QUuid& uid);
	RssDownloadRule* findDownloadRule(const QUuid& uid);
	void updateDownloadRule(RssDownloadRule* rule);
	QList<RssDownloadRule*> downloadRules();
	~RssManager();
	void SaveFeeds();
	void SaveDownloadRules();
	static RssManagerPtr getInstance();
private:
	struct TorrentDownloadInfo 
	{
		QUrl torrentUrl;
		RssDownloadRule* downloadRule;
		QUuid rssFeedId;
		QString rssItemId;
	};
	static boost::weak_ptr<RssManager> m_sInstrance;
	RssManager(QObject* parent = nullptr);
	TorrentDownloaderPtr m_pTorrentDownloader;
	NotificationSystemPtr m_pNotificationSystem;
	QList<RssFeed*> m_pFeeds;
	QHash<QUrl,TorrentDownloadInfo> m_activeTorrentDownloads;
	QHash<QUuid, RssDownloadRule*> m_downloadRules;
	QApplicationSettings* m_pSettings;
	void downloadRssItem(RssItem* rssItem,RssFeed* pFeed, RssDownloadRule* rule);
	QString gessSavePath(RssDownloadRule* downloadRule, QString base_suffix);
	QMap<QString, quint8> getFilePriorities(TorrentDownloadInfo downloadInfo, file_storage fileStorage);
private slots:
	void onFeedChanged(QUuid);
	void LoadFeeds();
	void LoadDownloadRules();
	void onTorrentDownloaded(QUrl url, QTemporaryFile* pFile);
	void onMagnetError(QString);
	void onDownloadMetadataCompleted(openmagnet_info);
signals:
	void Notify(int, QString, QVariant);
	void FeedChanged(QUuid);
	void FeedRemoved(QUuid);
	
};

#endif

