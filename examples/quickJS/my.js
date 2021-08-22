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

var text = "";

function my_backend(path)
{
	if (path == "/" || path == "/j") {
		varnish.response(200,
			"text/html", index_html);
	} else if (path == "/j/get") {
		varnish.response(200,
			"text/plain", text);
	}
}

function my_post_backend(path, data)
{
	varnish.response(201,
		"text/plain", varnish.storage("set_storage", data));
}

function get_storage()
{
	return text;
}

function set_storage(data)
{
	var json = JSON.parse(data);
	/* Modify text (in storage) */
	text += ">> " + json["text"] + "\n";
	/* Clone this VM and make it handle requests */
	varnish.vmcommit();
	/* Finish the current request */
	return text;
}

/* Keep the program state during updates */
function on_live_update()
{
	return text;
}
function on_resume_update(new_text)
{
	text = new_text;
}
