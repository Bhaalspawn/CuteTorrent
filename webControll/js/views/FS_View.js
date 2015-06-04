/**
 * Created with IntelliJ IDEA.
 * User: rfedos
 * Date: 7/24/13
 * Time: 1:54 PM
 * To change this template use File | Settings | File Templates.
 */
_.templateSettings = {
    evaluate: /\{\{(.+?)\}\}/g,
    interpolate: /\{\{=(.+?)\}\}/g
};
String.prototype.endsWith = function(suffix) {
	return this.indexOf(suffix, this.length - suffix.length) !== -1;
};
var gloablInstanceCount=0;
window.FileSystemView = Backbone.View.extend({
    initialize: function (options) {
        this.template = $.templates("#tree-view-item");
		this.currentPath = '';
		this.folderCounter = 0;
		this.rootEL = options.rootEL;
		this.pathEl = options.pathEl;
		this.instanceId = gloablInstanceCount++;
    },
	fetchData: function(currentEL) {
		currentEL = typeof currentEL !== 'undefined' ? currentEL : this.rootEL;
		if (currentEL.children('li').length > 0) {
			return;
		}
		var view = this;
		currentEL.html('<h3><img src="/img/loader.gif">Loading...</h3>');
		$.ajax({
				url: '/fs/api',
				type: "GET",
				async:true,
				cache: false,
				data : {
					"currentPath": view.currentPath
				},
				success: function(msg){
					if (msg.length > 0)
					{
						currentEL.html('');
						view.render(msg, currentEL);	
					}
					else
					{
						var treeItem = currentEL.parent('li.tree-item');
						var input = treeItem.children('input');
						input.removeAttr('checked');
						input.remove();
						treeItem.children('ol').remove();
					}
					view = null;
					
					
				}
		});
	},
	updateCurrentPath: function(el)	{
		
		var pathItems = el.parents('li.tree-item');
		this.currentPath = '';
		for(var i=pathItems.length - 1; i>= 0; i--)
		{
			pathItem = $(pathItems[i]);
			var itemText = pathItem.children('label.name').text();
			if (!itemText.endsWith('/'))
			{
				itemText += '/';
			}
			this.currentPath += itemText;
		}
		var itemText = el.children('label.name').text();
		if (!itemText.endsWith('/'))
		{
			itemText += '/';
		}
		this.currentPath += itemText;
		this.pathEl.val(this.currentPath);
	},
	onFetchMore: function(e)
	{
		console.log(e);
		var treeItem = $(e.target).parent('.tree-item');
		this.updateCurrentPath(treeItem);
		this.fetchData(treeItem.find('ol.sublist'));
	},
    render: function (fsItems, el) {
		var view = this;
		fsItems.forEach (function (fsItem){
			fsItem['folderCnt'] = view.instanceId.toString() + view.folderCounter++;
			el.append( view.template.render(fsItem));
		});	
		el.find('input').click($.proxy(view.onFetchMore, view));
		view = null;
		return this;
    }




});