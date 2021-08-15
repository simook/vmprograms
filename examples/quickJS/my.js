/*
	QuickJS inside Varnish
*/
console.log("Hello QuickJS World");

function my_backend()
{
	varnish.response(
		200,
		"text/html", index_html);
}

var text = "";

function get_storage()
{
	return text;
}

function my_storage(data)
{
	var json = JSON.parse(data);
	text += json["text"] + "\n";
	return text;
}
