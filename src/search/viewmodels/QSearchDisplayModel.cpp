﻿#include <stddef.h>
#include "QSearchDisplayModel.h"
#include "StaticHelpers.h"
#include "filedownloader.h"
#include <gui/Dialogs/OpenTorrentDialog.h>
#include "SearchResult.h"
QSearchDisplayModel::QSearchDisplayModel(QTreeView* pTorrentListView, QSearchFilterModel* pSearchFilterModel, QObject* parent) 
	: QAbstractListModel(parent)
	, m_pTorrentDownloader(FileDownloader::getNewInstance())
	, m_pSearchEngine(SearchEngine::getInstance())
{
	m_pTorrentListView = pTorrentListView;
	m_pSearchFilterModel = pSearchFilterModel;
	connect(m_pSearchEngine.get(), SIGNAL(GotResults()), this, SLOT(OnNewSearchResults()));
	SearchItemsStorragePtr pItems = m_pSearchEngine->GetResults();
	connect(pItems.get(), SIGNAL(reset()), this, SLOT(OnNewSearchResults()));
	connect(m_pTorrentDownloader.get(), SIGNAL(DownloadReady(QUrl, QTemporaryFile*)), SLOT(OnTorrentDownloaded(QUrl, QTemporaryFile*)));
}


QSearchDisplayModel::~QSearchDisplayModel()
{
}

int QSearchDisplayModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	return m_pSearchEngine->GetResults()->length();
}

int QSearchDisplayModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
	return SEARCH_RESULTS_COLUNS_COUNT;
}

QVariant QSearchDisplayModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
	if(index.isValid())
	{
		int row = index.row();
		int col = index.column();
		SearchItemsStorragePtr results = m_pSearchEngine->GetResults();

		if(row > results->length())
		{
			return QVariant();
		}

		SearchResult* res = (*results) [row];

		switch(role)
		{
			case SearchItemRole:
			{
				return qVariantFromValue(res);
			}

			case SearchItemName:
			{
				return res->Name();
			}

			case SearchItemSize:
			{
				return res->Size();
			}

			case SearchItemSeeders:
			{
				return res->Seeders();
			}

			case SearchItemPeers:
			{
				return res->Leechers();
			}
		}
	}

	return QVariant();
}

void QSearchDisplayModel::downloadTorrent()
{
	if (m_pTorrentListView->model() != m_pSearchFilterModel)
	{
		return;
	}

	QModelIndex index = m_pTorrentListView->currentIndex();
	SearchResult* searchResult = index.data(SearchItemRole).value<SearchResult*>();
	m_pTorrentDownloader->download(searchResult->TorrentFileUrl());
}

void QSearchDisplayModel::OnNewSearchResults()
{
	reset();
}

void QSearchDisplayModel::OnTorrentDownloaded(QUrl url, QTemporaryFile* pFile)
{
	boost::scoped_ptr<QTemporaryFile> pSafeFile(pFile);
	boost::scoped_ptr<OpenTorrentDialog> pDialog(new OpenTorrentDialog());
	pDialog->SetData(pSafeFile->fileName());
	pDialog->exec();
}



