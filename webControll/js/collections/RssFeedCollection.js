window.RssFeedCollection = Backbone.Collection.extend({
    model: Torrent,
    url:'/rss/api',
    reset:true,
	search : function(letters){
		if(letters == "") return this;
		return _(this.filter(function(data) {
		  	return data.get("name").toLowerCase().indexOf(letters.toLowerCase()) >= 0;
		}));
	}
});