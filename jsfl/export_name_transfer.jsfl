var document =  fl.getDocumentDOM();
if (document)
{
	var library = document.library

	var items = library.getSelectedItems();
	if (items.length == 0)
	{
		fl.trace("There is no items to copy");
	}

	for (i = 0; items.length > i; i++)
	{
		var item = items[i];
		var item_name = item.name.split("/");
		item_name = item_name[item_name.length - 1];
		item.linkageExportForAS = true;
		item.linkageClassName = item_name;
		fl.trace("Copied: " + item_name);
	}
}

