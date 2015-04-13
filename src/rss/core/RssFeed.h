#ifndef _RSS_FEED_INCLUDED_
#define _RSS_FEED_INCLUDED_
#include <QUuid>
#include <Qurl>
#include <QTime>
#include <QMetaType>
class RssParser;
class QNetworkAccessManager;
class NetworkDiskCache;
class QNetworkReply;
class QTimer;
class QDataStream;
class RssItem;

class RssFeed : public QObject
{
	Q_OBJECT
	
	friend RssParser;
	friend QDataStream& operator <<(QDataStream& out, const RssFeed& any);
	friend QDataStream& operator >>(QDataStream& out, RssFeed& any);
	
	QTimer* m_pUpdateTimer;
	QUuid m_uid;
	QUrl m_url;
	QNetworkAccessManager* m_pNetManager;
	QTime m_elapsedTime;
	QString m_title, m_link, m_description, m_errorString, m_customDisplayName;
	bool m_isUpdating;
	QList<RssItem*> m_rssItemsByDate;
	QHash<QString, RssItem*> m_rssItems;
	int m_ttl, m_unreadCount, m_customTtl;
signals:
	void FeedChanged(QUuid);
public:
	RssFeed(QUrl url, QUuid uid);
	void UpdateUnreadCount();
	QString displayName(bool noUnreadCount = false);
	void setDisplayName(QString value);
	QUrl url();
	QUuid uid();
	QString title();
	QString description() const;
	int ttl();
	void setTll(int value);
	int unreadCount() const;
	bool isUpdating() const;
	QString error() const;
	int next_update();
	QList<RssItem*> GetFeedItems();
	void setItemUnread(bool unreadValue, QString guid);
	RssItem* GetFeedItem(QString guid);
public slots:
	void Update();
private slots:
	void resourceLoaded(QNetworkReply*);

};
Q_DECLARE_METATYPE(RssFeed*)
#endif