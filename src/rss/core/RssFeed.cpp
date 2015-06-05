#include "RssFeed.h"
#include "RssManager.h"
#include "RssParser.h"
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>
#include <QNetworkCookie>
#include <QNetworkReply>
#include "StaticHelpers.h"
#include "RssItem.h"

RssFeed::RssFeed(QUrl url, QUuid uid) : m_uid(uid), m_url(url), m_ttl(0), m_customTtl(0)
{
	m_pNetManager = new QNetworkAccessManager(this);
	m_pUpdateTimer = new QTimer(this);
	m_elapsedTime.start();
	connect(m_pNetManager, SIGNAL(finished(QNetworkReply*)), SLOT(resourceLoaded(QNetworkReply*)));
	connect(m_pUpdateTimer, SIGNAL(timeout()), SLOT(Update()));

	if (!url.isEmpty())
	{
		Update();
	}
}

QUrl RssFeed::url()
{
	return m_url;
}

QUuid RssFeed::uid()
{
	return m_uid;
}
bool rssItemLessThenByDate(const RssItem* right, const RssItem* left)
{
	QDateTime rightTime = right->pubDate();
	QDateTime leftTime = left->pubDate();
	bool res = rightTime > leftTime;
	return res;
}

void RssFeed::resourceLoaded(QNetworkReply* pReply)
{
	QVariant fromCache = pReply->attribute(QNetworkRequest::SourceIsFromCacheAttribute);

	if (pReply->error() ==  QNetworkReply::NoError)
	{
		RssParserPtr parser = RssParser::getInstance();
		bool ok;
		QString error;
		parser->fillFeed(pReply, this, ok, error);

		if (!ok)
		{
			m_errorString = error;
		}
		else
		{
			m_errorString = "";
		}

		m_isUpdating = false;

		if (m_pUpdateTimer->isActive())
		{
			m_pUpdateTimer->stop();
		}

		m_rssItemsByDate = m_rssItems.values();
		qStableSort(m_rssItemsByDate.begin(), m_rssItemsByDate.end(), rssItemLessThenByDate);

		if (m_rssItemsByDate.length() > 50)
		{
			m_rssItemsByDate.erase(m_rssItemsByDate.begin() + 50, m_rssItemsByDate.end());
			m_rssItems.clear();

			foreach(RssItem* rssItem, m_rssItemsByDate)
			{
				m_rssItems.insert(rssItem->guid(), rssItem);
			}
		}

		UpdateUnreadCount();
	}
	else
	{
		m_errorString = pReply->errorString();
	}

	if (m_ttl == 0)
	{
		m_ttl = 30;
	}

	m_pUpdateTimer->setInterval(ttl() * 60l * 1000l);
	m_pUpdateTimer->start();
	m_elapsedTime.start();
	emit FeedChanged(m_uid);
}


QString RssFeed::title()
{
	return m_title;
}

QList<RssItem*> RssFeed::GetFeedItems()
{
	return m_rssItemsByDate;
}

int RssFeed::ttl()
{
	if (m_customTtl != 0)
	{
		return m_customTtl;
	}

	return m_ttl;
}

void RssFeed::setTll(int value)
{
	m_customTtl = value;
	m_pUpdateTimer->setInterval(ttl() * 60l * 1000l);

	if (m_pUpdateTimer->isActive())
	{
		m_pUpdateTimer->stop();
	}

	m_pUpdateTimer->start();
	m_elapsedTime.restart();
}

QString RssFeed::description() const
{
	return m_description;
}

void RssFeed::Update()
{
	m_elapsedTime.restart();
	QNetworkRequest request(m_url);
	QList<QNetworkCookie> cookies = buildCookies();
	m_pNetManager->cookieJar()->setCookiesFromUrl(cookies, m_url);
	m_pNetManager->get(request);
	m_isUpdating = true;
}

QList<QNetworkCookie> RssFeed::buildCookies() const
{
	QList<QNetworkCookie> res;

	if (m_coookies.size() > 0)
	{
		for (QHash<QString, QString>::const_iterator i = m_coookies.constBegin(); i != m_coookies.constEnd(); ++i)
		{
			res << QNetworkCookie(i.key().toUtf8(), i.value().toUtf8());
		}
	}

	return res;
}

int RssFeed::next_update()
{
	return (m_pUpdateTimer->interval() - m_elapsedTime.elapsed()) / 1000;
}

void RssFeed::setItemUnread(bool unreadValue, QString guid)
{
	if (m_rssItems.contains(guid))
	{
		bool previousValue = m_rssItems[guid]->unread();

		if (previousValue != unreadValue)
		{
			m_rssItems[guid]->setUread(unreadValue);

			if (!unreadValue)
			{
				m_unreadCount--;
			}
		}
	}
}

RssItem* RssFeed::GetFeedItem(QString guid)
{
	if (m_rssItems.contains(guid))
	{
		return m_rssItems[guid];
	}

	return NULL;
}

void RssFeed::UpdateUnreadCount()
{
	m_unreadCount = m_rssItemsByDate.size();

	foreach (RssItem* item, m_rssItemsByDate)
	{
		if (!item->unread())
		{
			m_unreadCount--;
		}
	}
}

QString RssFeed::displayName(bool noUnreadCount)
{
	if (noUnreadCount || m_unreadCount == 0)
	{
		if (m_customDisplayName.isEmpty())
		{
			return QString("%1 - %2").arg(m_title, m_description);
		}
		else
		{
			return m_customDisplayName;
		}
	}
	else
	{
		if (m_customDisplayName.isEmpty())
		{
			return QString("%1 - %2 (%3)").arg(m_title, m_description, QString::number(m_unreadCount));
		}
		else
		{
			return QString("%1 (%2)").arg(m_customDisplayName, QString::number(m_unreadCount));
		}
	}
}

void RssFeed::setDisplayName(QString value)
{
	m_customDisplayName = value;
}

QList<QNetworkCookie> RssFeed::coookies() const
{
	return buildCookies();
}

void RssFeed::setCoookies(const QHash<QString, QString>& coookiesValue)
{
	m_coookies = coookiesValue;
}

QString RssFeed::error() const
{
	return m_errorString;
}

bool RssFeed::isUpdating() const
{
	return m_isUpdating;
}

int RssFeed::unreadCount() const
{
	return m_unreadCount;
}


QDataStream& operator<<(QDataStream& stream, const RssFeed& any)
{
	stream << any.m_uid;
	stream << any.m_url;
	stream << any.m_title;
	stream << any.m_description;
	stream << any.m_customDisplayName;
	stream << any.m_link;
	stream << any.m_customTtl;
	stream << any.m_coookies;
	int size = any.m_rssItems.size();
	stream << size;

	for (int i = 0; i < size; i++)
	{
		stream << *any.m_rssItemsByDate.at(i);
	}

	return stream;
}

QDataStream& operator>>(QDataStream& stream, RssFeed& any)
{
	stream >> any.m_uid;
	stream >> any.m_url;
	stream >> any.m_title;
	stream >> any.m_description;
	stream >> any.m_customDisplayName;
	stream >> any.m_link;
	stream >> any.m_customTtl;
	stream >> any.m_coookies;
	int size;
	stream >> size;
	any.m_rssItems.reserve(size);
	any.m_rssItemsByDate.reserve(size);

	for (int i = 0; i < size; i++)
	{
		RssItem* pItem = new RssItem(&any);
		stream >> *pItem;
		any.m_rssItems.insert(pItem->guid(), pItem);
	}

	any.m_rssItemsByDate = any.m_rssItems.values();
	qStableSort(any.m_rssItemsByDate.begin(), any.m_rssItemsByDate.end(), rssItemLessThenByDate);
	any.UpdateUnreadCount();
	return stream;
}
