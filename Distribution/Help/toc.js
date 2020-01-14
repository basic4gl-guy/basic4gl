$(function() {
	
	var container = $(".toc")
	var idGen = 0;
	
	// Find headings
	$("h1, h2").each(function() {
		if (!$(this).attr("id"))
			$(this).attr("id", "heading_" + (++idGen));		
		var level = $(this).is("h1") ? 1 : 2;
		var entry = $("<a>");
		entry.text($(this).text());		
		entry.attr("href", "#" + $(this).attr("id"));
		entry.addClass("level" + level);
		container.append(entry);
	});
});
