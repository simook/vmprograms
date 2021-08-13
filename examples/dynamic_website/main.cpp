#include "varnish_api.hpp"

#include <cstdio>

extern "C" void retrieve_json(void*, size_t, size_t);
extern "C" void set_json(void*, size_t, size_t);

int main() {
	printf("C++ JSON example main\n");
}

EMBED_BINARY(index_html, "../index.html")

extern "C" __attribute__((used))
void my_backend(const char *arg)
{
	const std::string path {arg};
	if (path == "/" || path == "/w") {
		Backend::response("text/html", index_html, index_html_size);
	} else if (path == "/get") {
		/* Make room for a 1KB result */
		std::vector<uint8_t> result(1024);
		/* Call 'retrieve_json' in storage and retrieve result */
		long len = Storage::call(retrieve_json, result);
		result.resize(len);
		/* Ship the result on the wire, as JSON */
		Backend::response("application/json", result);
	} else {
		Backend::response("text/plain", "Unknown location");
	}
}

extern "C" __attribute__((used))
void my_post_backend(const char* /* arg */, void* data, size_t len)
{
	std::vector<uint8_t> result(1024);

	size_t reslen = Storage::call(set_json, data, len, result);
	result.resize(reslen);

	Backend::response("application/json", result);
}

#include <nlohmann/json.hpp>
using json = nlohmann::json;
static json g_json;

void retrieve_json(void*, size_t, size_t /* reslen */)
{
	Storage::response(g_json.dump(4));
}

void set_json(void* data, size_t len, size_t /* reslen */)
{
	const uint8_t* data_begin = (uint8_t*)data;
	const uint8_t* data_end   = data_begin + len;

	g_json = json::parse(data_begin, data_end);

	Storage::response(g_json.dump(4));
}
