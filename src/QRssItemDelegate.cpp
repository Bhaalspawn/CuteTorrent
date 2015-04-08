﻿#include "QRssItemDelegate.h"
#include "RssFeed.h"
#include "RssFeedTreeItem.h"
#include "RssFeedItemTreeItem.h"
#include "StaticHelpers.h"
#include "FaviconDownloader.h"
enum
{
	GUI_PAD = 6
};

namespace
{
	int MAX3(int a, int b, int c)
	{
		const int ab(a > b ? a : b);
		return ab > c ? ab : c;
	}
}

QRssItemDelegate::QRssItemDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
	m_pFaviconDownloader = new FaviconDownloader();
}


QRssItemDelegate::~QRssItemDelegate()
{
	delete m_pFaviconDownloader;
}

QSize QRssItemDelegate::margin(const QStyle& style) const
{
	Q_UNUSED(style);
	return QSize(4, 4);
}

void QRssItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	try
	{
		RssBaseTreeItem* baseItem = static_cast<RssBaseTreeItem*>(index.internalPointer());
		qDebug() << "BaseItemType" << baseItem->GetType();
		switch (baseItem->GetType())
		{
			case RssBaseTreeItem::Feed:
			{
				RssFeedTreeItem* pFeed = reinterpret_cast<RssFeedTreeItem*>(baseItem);
				drawFeed(painter, option, index, pFeed);
				break;
			}
			case RssBaseTreeItem::FeedItem:
			{
				RssFeedItemTreeItem* pFeedItem = reinterpret_cast<RssFeedItemTreeItem*>(baseItem);
				drawFeedItem(painter, option, index, pFeedItem);
				break;
			}
		}
	}
	catch (...)
	{
		
	}
	
}

QSize QRssItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	try
	{

		RssBaseTreeItem* baseItem = static_cast<RssBaseTreeItem*>(index.internalPointer());
		switch (baseItem->GetType())
		{
			case RssBaseTreeItem::Feed:
			{
				RssFeedTreeItem* pFeed = reinterpret_cast<RssFeedTreeItem*>(baseItem);
				return feedSizeHint(option, pFeed);
			}
			case RssBaseTreeItem::FeedItem:
			{
				RssFeedItemTreeItem* pFeedItem = reinterpret_cast<RssFeedItemTreeItem*>(baseItem);
				return feedItemSizeHint(option, pFeedItem);
			}
			default:
			{
				return QSize();
			}
		}
	}
	catch (...)
	{
		
	}
	return QSize();
}

void QRssItemDelegate::drawFeed(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex &index, RssFeedTreeItem* pFeed) const
{
	QStyleOptionViewItemV4 opt = option;
	initStyleOption(&opt, index);
	QStyle* style;
	RssFeed* pRssFeed = pFeed->GetFeed();
	if (opt.widget != NULL)
	{
		style = opt.widget->style();
	}
	else
	{
		style = QApplication::style();
	}
	const int iconSize(style->pixelMetric(QStyle::PM_SmallIconSize));
	QFont nameFont(option.font);
	nameFont.setWeight(QFont::Bold);
	const QFontMetrics nameFM(nameFont);
	const QPixmap favicon = m_pFaviconDownloader->getFavicon(pRssFeed->url().toString());

	QString nameStr = pRssFeed->displayName();
	QSize nameSize(nameFM.size(0, nameStr));

	QFont statusFont(option.font);
	statusFont.setPointSize(int(option.font.pointSize() * 0.9));
	const QFontMetrics statusFM(statusFont);
	QString statusStr;
	QString errorStr = pRssFeed->error();
	if (errorStr.isEmpty())
	{
		if (pRssFeed->isUpdating())
		{
			statusStr = tr("STATUS_UPDATING");
		}
		else
		{
			statusStr = tr("STATUS_OK");
		}
	}
	else
	{
		statusStr = errorStr;
	}

	QFont updateTimeFont(statusFont);
	const QFontMetrics updateTimeFM(updateTimeFont);
	const QString updateTimeStr(StaticHelpers::toTimeString(pRssFeed->next_update()));

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
	QIcon::Mode im = QIcon::Normal;
	QIcon::State qs = QIcon::On;
	QPalette::ColorGroup cg = QPalette::Normal;
	QPalette::ColorRole cr;

	cr = QPalette::Text;
	// layout
	const QSize m(margin(*style));
	QRect fillArea(option.rect);
	fillArea.adjust(m.width(), m.height(), -m.width(), -m.height());
	QRect iconArea(fillArea.x(), fillArea.y() + (fillArea.height() - iconSize) / 2, iconSize, iconSize);
	QRect nameArea(iconArea.x() + iconArea.width() + GUI_PAD, fillArea.y(),
		fillArea.width() - GUI_PAD - iconArea.width(), nameSize.height());
	QRect updateTimeArea(nameArea);
	updateTimeArea.moveTop(nameArea.y() + statusFM.lineSpacing() + GUI_PAD / 2);
	updateTimeArea.setWidth(nameArea.width() / 2);
	QRect statusArea(nameArea);
	statusArea.moveTop(nameArea.y() + statusFM.lineSpacing() + GUI_PAD / 2);
	statusArea.moveLeft(nameArea.width() / 2 + iconArea.width() + GUI_PAD);
	statusArea.setWidth(nameArea.width() / 2);
	statusArea.setHeight(nameSize.height());

	painter->setPen(opt.palette.color(cg, cr));
	//favicon.paint(painter, iconArea, Qt::AlignCenter, im, qs);
	painter->drawPixmap(iconArea, favicon);
	painter->setFont(nameFont);
	nameStr = nameFM.elidedText(nameStr, Qt::ElideRight, nameArea.width());
	style->drawItemText(painter, nameArea, Qt::AlignLeft, opt.palette, option.state & QStyle::State_Enabled, nameStr);
	painter->setFont(statusFont);
	style->drawItemText(painter, statusArea, Qt::AlignRight, opt.palette, option.state & QStyle::State_Enabled, statusStr);
	painter->setFont(updateTimeFont);
	style->drawItemText(painter, updateTimeArea, Qt::AlignLeft, opt.palette, option.state & QStyle::State_Enabled, updateTimeStr);
	painter->restore();

}

void QRssItemDelegate::drawFeedItem(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex &index, RssFeedItemTreeItem* pFeedTreeItem) const
{
	QStyleOptionViewItemV4 opt = option;
	initStyleOption(&opt, index);
	QStyle* style;

	if (opt.widget != NULL)
	{
		style = opt.widget->style();
	}
	else
	{
		style = QApplication::style();
	}
	RssItem pFeedItem = pFeedTreeItem->FeedItem();

	QFont nameFont(option.font);
	if (pFeedItem["unread"].toBool() == true)
	{
		nameFont.setWeight(QFont::Bold);
	}
	const QFontMetrics nameFM(nameFont);
	QString nameStr(pFeedItem["title"].toString());
	QSize nameSize(nameFM.size(0, nameStr));

	QFont sizeFont(option.font);
	if (pFeedItem["unread"].toBool() == true)
	{
		sizeFont.setWeight(QFont::Bold);
	}
	const QFontMetrics sizeFM(sizeFont);
	qint64 size = pFeedItem["size"].toULongLong();
	QString sizeStr;
	if (size > 0)
	{
		sizeStr = StaticHelpers::toKbMbGb(size);
	}
	else
	{
		sizeStr = tr("- Mb");
	}
	QSize sizeSize(sizeFM.size(0, sizeStr));

	QFont categoryFont(option.font);
	if (pFeedItem["unread"].toBool() == true)
	{
		categoryFont.setWeight(QFont::Bold);
	}
	const QFontMetrics categoryFM(categoryFont);
	QString categoryStr(pFeedItem["category"].toString());
	if (categoryStr.isEmpty())
	{
		categoryStr = "-";
	}
	QSize categorySize(categoryFM.size(0, categoryStr));

	const QSize m(margin(*style));
	QRect fillArea(option.rect);
	fillArea.adjust(m.width(), m.height(), -m.width(), -m.height());

	QRect nameArea(fillArea.x(), fillArea.y(), fillArea.width(), nameSize.height());

	QRect sizeArea(fillArea.x(), fillArea.y() + sizeFM.lineSpacing() + GUI_PAD / 2, fillArea.width() / 2, nameSize.height());
	
	QRect categoryArea(nameArea);
	categoryArea.moveTop(nameArea.y() + categoryFM.lineSpacing() + GUI_PAD / 2);

	QPalette::ColorGroup cg = QPalette::Normal;
	QPalette::ColorRole cr = QPalette::Text;
	

	painter->save();
	painter->setRenderHint(QPainter::Antialiasing);
	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
	painter->setPen(opt.palette.color(cg, cr));
	
	painter->setFont(nameFont);
	nameStr = nameFM.elidedText(nameStr, Qt::ElideRight, nameArea.width());
	style->drawItemText(painter, nameArea, Qt::AlignLeft, opt.palette, option.state & QStyle::State_Enabled, nameStr);
	painter->setFont(sizeFont);
	style->drawItemText(painter, sizeArea, Qt::AlignLeft, opt.palette, option.state & QStyle::State_Enabled, sizeStr);
	painter->setFont(categoryFont);
	style->drawItemText(painter, categoryArea, Qt::AlignRight, opt.palette, option.state & QStyle::State_Enabled, categoryStr);
	painter->restore();

}

QSize QRssItemDelegate::feedSizeHint(const QStyleOptionViewItem & option, RssFeedTreeItem* pFeed) const
{
	RssFeed* pRssFeed = pFeed->GetFeed();
	const QStyle* style(QApplication::style());
	const int iconSize(style->pixelMetric(QStyle::PM_ToolBarIconSize));
	QFont nameFont(option.font);
	nameFont.setWeight(QFont::Bold);
	const QFontMetrics nameFM(nameFont);
	const QString nameStr(pRssFeed->title());
	int nameWidth = nameFM.width(nameStr);

	

	QFont statusFont(option.font);
	statusFont.setPointSize(int(option.font.pointSize() * 0.9));
	const QFontMetrics statusFM(statusFont);
	QString statusStr;
	QString errorStr = pRssFeed->error();
	if (errorStr.isEmpty())
	{
		if (pRssFeed->isUpdating())
		{
			statusStr = tr("STATUS_UPDATING");
		}
		else
		{
			statusStr = tr("STATUS_OK");
		}
	}
	else
	{
		statusStr = errorStr;
	}
	int statusWidth = statusFM.width(statusStr);
	QFont updateTimeFont(statusFont);
	const QFontMetrics updateTimeFM(updateTimeFont);
	const QString updateTimeStr(StaticHelpers::toTimeString(pRssFeed->next_update()));
	const int updateTimeWidth = updateTimeFM.width(updateTimeStr);
	const QSize m(margin(*style));
	return QSize(m.width() + iconSize + MAX3(nameWidth, statusWidth, updateTimeWidth),
		m.height()*2 + nameFM.lineSpacing() + updateTimeFM.lineSpacing());
}

QSize QRssItemDelegate::feedItemSizeHint(const QStyleOptionViewItemV4 & opt, RssFeedItemTreeItem* pFeedTreeItem) const
{
	QStyle* style;

	if (opt.widget != NULL)
	{
		style = opt.widget->style();
	}
	else
	{
		style = QApplication::style();
	}
	RssItem pFeedItem = pFeedTreeItem->FeedItem();

	QFont nameFont(opt.font);
	nameFont.setWeight(QFont::Bold);
	const QFontMetrics nameFM(nameFont);
	QString nameStr(pFeedItem["title"].toString());
	QSize nameSize(nameFM.size(0, nameStr));

	QFont sizeFont(opt.font);
	sizeFont.setWeight(QFont::Bold);
	const QFontMetrics sizeFM(sizeFont);
	QString sizeStr(StaticHelpers::toKbMbGb(pFeedItem["size"].toULongLong()));
	QSize sizeSize(sizeFM.size(0, sizeStr));

	QFont categoryFont(opt.font);
	categoryFont.setWeight(QFont::Bold);
	const QFontMetrics categoryFM(categoryFont);
	QString categoryStr(pFeedItem["category"].toString());
	QSize categorySize(categoryFM.size(0, categoryStr));

	const QSize m(margin(*style));
	QRect fillArea(opt.rect);
	fillArea.adjust(m.width(), m.height(), -m.width(), -m.height());

	QRect nameArea(fillArea.x(), fillArea.y(), fillArea.width(), nameSize.height());

	QRect sizeArea(nameArea);
	sizeArea.setWidth(nameArea.width() / 2);
	sizeArea.moveRight(nameArea.width() / 2);

	QRect categoryArea(nameArea);
	categoryArea.moveTop(nameArea.y() + categoryFM.lineSpacing() + GUI_PAD / 2);

	return QSize(m.width() + nameSize.width(), m.height()*2 + nameSize.height() + categorySize.height());
}


