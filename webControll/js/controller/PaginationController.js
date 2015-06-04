
window.PaginationController = Backbone.Router.extend({
    routes: {
        'torrent/page/:page': 'changeTorrentPage',
		'rss/page/:page': 'changeRssPage'
    },
	initialize: function(options){
		this.torrentsView=options.torrentsView;
		this.rssView=options.rssView;
	},	
	changeTorrentPage: function(page){
			this.torrentsView.paginationView.page_active=parseInt(page);
			this.torrentsView.paginationView.render();
			this.torrentsView.changePage(parseInt(page));
	},
	changeRssPage: function(page){
			this.rssView.paginationView.page_active=parseInt(page);
			this.rssView.paginationView.render();
			this.rssView.changePage(parseInt(page));
	}


});
