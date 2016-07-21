#include "QTorrentFilterProxyModel.h"
#include "QTorrentDisplayModel.h"
#include <QtCore>
#include <float.h>

QTorrentFilterProxyModel::QTorrentFilterProxyModel(QObject* parent)
	: QSortFilterProxyModel(parent)
	  , m_pUpdateLocker(new QMutex())
	  , m_currentFilterType(TORRENT)
	  , m_torrentFilter(EMPTY)
	  , m_pTorrentGroupsManager(TorrentGroupsManager::getInstance())
{
	setDynamicSortFilter(true);
}

bool QTorrentFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

	if (index.isValid())
	{
		Torrent* pTorrent = index.data(QTorrentDisplayModel::TorrentRole).value<Torrent*>();

		if (pTorrent != NULL)
		{
			bool searchFilterAccept = m_torrentSearchFilter.indexIn(pTorrent->GetName()) > -1;
			TorrentGroup* torrentGroup;
			QUuid torrentUid;
			switch (m_currentFilterType)
			{
				case GROUP:
					if (m_groupFilter.isNull())
					{
						return searchFilterAccept;
					}
					torrentGroup = m_pTorrentGroupsManager->GetGroup(m_groupFilter);
					if (torrentGroup == NULL)
					{
						return searchFilterAccept;
					}
					torrentUid = pTorrent->GetGroupUid();
					return (torrentGroup->uid() == torrentUid || torrentGroup->contains(torrentUid)) && searchFilterAccept;

				case TORRENT:
				{
					switch (m_torrentFilter)
					{
						case ACTIVE:
							return pTorrent->isActive() && searchFilterAccept;

						case NOT_ACTIVE:
							return !pTorrent->isActive() && searchFilterAccept;

						case SEEDING:
							return pTorrent->isSeeding() && searchFilterAccept;

						case DOWNLOADING:
							return pTorrent->isDownloading() && searchFilterAccept;

						case COMPLETED:
							return 100.0f - pTorrent->GetProgress() < FLT_EPSILON && searchFilterAccept;

						case EMPTY:
							return searchFilterAccept;

						default:
							break;
					}

					break;
				}

				default:
					break;
			}
		}
	}

	return false;
}

bool QTorrentFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
	if (left.isValid() && right.isValid())
	{
		Torrent* rightTorrent = right.data(QTorrentDisplayModel::TorrentRole).value<Torrent*>();
		Torrent* leftTorrent = left.data(QTorrentDisplayModel::TorrentRole).value<Torrent*>();

		if (rightTorrent != NULL && leftTorrent != NULL)
		{
			QTorrentDisplayModel::Role torrentSortRole = QTorrentDisplayModel::Role(sortRole());
			qDebug() << "Sort role while sorting " << torrentSortRole;
			QVariant leftData = left.data(torrentSortRole);
			QVariant rightData = right.data(torrentSortRole);

			if (leftData == rightData)
			{
				return leftTorrent->GetName().compare(rightTorrent->GetName()) > 0;
			}

			return QSortFilterProxyModel::lessThan(left, right);
		}
	}

	return false;
}

void QTorrentFilterProxyModel::setGroupFilter(QUuid groupName)
{
	QMutexLocker lockMutex(m_pUpdateLocker);
	m_currentFilterType = GROUP;
	m_groupFilter = groupName;
	invalidateFilter();
}

void QTorrentFilterProxyModel::setTorrentFilter(TorrentFilterType activityFilter)
{
	QMutexLocker lockMutex(m_pUpdateLocker);
	m_currentFilterType = TORRENT;
	m_torrentFilter = activityFilter;
	invalidateFilter();
}

void QTorrentFilterProxyModel::setTorrentSearchFilter(QString filter)
{
	QMutexLocker lockMutex(m_pUpdateLocker);
	m_torrentSearchFilter = QRegExp(filter, Qt::CaseInsensitive, QRegExp::Wildcard);
	invalidateFilter();
}

