/*
	QuickJS inside Varnish

	Write a response back to client and dissolve VM:
	- varnish.response(code, content_type, content)

	Send a resource back to client and dissolve VM:
	- varnish.sendfile(path)

	Call a function in serialized stateful storage with
	data as optional argument, and get a return value back:
	- result = varnish.storage("storage_function")
	- result = varnish.storage("storage_function", data)

	Check if we are currently in storage:
	- result = varnish.is_storage()

	Create a snapshot VM from this and base future requests on it:
	- varnish.vmcommit()

	Fetch content of URL and return as string, or throw:
	- var resp = varnish.fetch(url)
		resp.status = 200
		resp.type   = "text/plain"
		resp.content = "Hello World!"

	Logging and errors will show up in VSL.

	scriptArgs[0] = "vmod_kvm"
	scriptArgs[1] = "mydomain.com"
	scriptArgs[2] = "0" (if request VM) or "1" (if storage VM)
	scriptArgs[3] = "/path/to/state.file"
*/

const state_file = scriptArgs[3]
console.log("Hello QuickJS World");

/* At the start we attempt to load previous text from disk */
var text = std.loadFile(state_file);
if (!text) text = "";

function my_backend(path)
{
	if (path == "/" || path == "/j") {
		varnish.sendfile("/index.html");
	} else if (path == "/j/get") {
		varnish.response(200,
			"text/plain", text);
	}
	varnish.sendfile(path);
}

function my_post_backend(path, data)
{
	/* For demo purposes */
	if (path == "/example") {
		var resp = varnish.fetch("https://example.com");
		varnish.response(resp.status,
			resp.type,
			resp.content);
	}

	/* Make a call into storage @set_storage with data as argument */
	var result = varnish.storage("set_storage", data);

	/* The result is the updated text */
	varnish.response(201,
		"text/plain", result);
}

function set_storage(data)
{
	/* We should be calling this function from storage */
	if (!varnish.is_storage()) {
		console.log("set_storage() should only be called from storage VM");
		return;
	}

	/* Modify text (in storage) */
	var json = JSON.parse(data);
	text += ">> " + json["text"] + "\n";

	/* "Persistence" using the state file from scriptArgs[3] */
	var file = std.open(state_file, "wb");
	file.puts(text);
	file.close();

	/* Clone this VM and make it handle requests */
	varnish.vmcommit();

	/* Finish the current request */
	return text;
}

/* Specially named functions that lets you
   keep the program state during updates. */
function on_live_update()
{
	return text;
}
function on_resume_update(new_text)
{
	text = new_text;
}
