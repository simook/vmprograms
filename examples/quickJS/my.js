/*
	QuickJS inside Varnish

	Write a response back to client and dissolve VM:
	- varnish.response(code, content_type, content)

	Call a function in serialized stateful storage with
	data as optional argument, and get a return value back:
	- result = varnish.storage("storage_function")
	- result = varnish.storage("storage_function", data)

	Logging and errors will show up in VSL.
*/
console.log("Hello QuickJS World");

function my_backend(path)
{
	if (path == "/" || path == "/j") {
		varnish.response(200,
			"text/html", index_html);
	} else if (path == "/j/get") {
		varnish.response(200,
			"text/plain", varnish.storage("get_storage"));
	}
}

function my_post_backend(path, data)
{
	varnish.response(200,
		"text/plain", varnish.storage("set_storage", data));
}

var text = "";

function get_storage()
{
	return text;
}

function set_storage(data)
{
	var json = JSON.parse(data);
	text += json["text"] + "\n";
	return text;
}
