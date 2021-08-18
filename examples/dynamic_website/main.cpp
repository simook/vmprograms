#include "varnish_api.hpp"

#include <cstdio>
extern "C" {
	static void retrieve_json(size_t, virtbuffer[], size_t);
	static void set_json(size_t, virtbuffer[], size_t);
}
EMBED_BINARY(index_html, "../index.html")

int main() {
	printf("C++ JSON example main\n");
}
/* Make pre-allocated 4KB result */
static std::vector<uint8_t> result(4096);

extern "C" __attribute__((used))
void my_backend(const char *arg)
{
	const std::string path {arg};
	if (path == "/" || path == "/w") {
		Backend::response(200, "text/html", index_html, index_html_size);
	} else if (path == "/w/get") {
		/* Call 'retrieve_json' in storage and retrieve result */
		long len = Storage::call(retrieve_json, result);
		result.resize(len);
		/* Ship the result on the wire */
		Backend::response(200, "text/plain", result);
	}
	Backend::response(404, "text/plain", "Unknown location");
}

extern "C" __attribute__((used))
void my_post_backend(const char* /* arg */, void* data, size_t len)
{
	size_t reslen = Storage::call(set_json, data, len, result);
	result.resize(reslen);

	Backend::response(201, "text/plain", result);
}

#include <nlohmann/json.hpp>
using json = nlohmann::json;
std::string text;

void retrieve_json(size_t n, virtbuffer buffers[n], size_t)
{
	Storage::response(text);
}

void set_json(size_t n, virtbuffer buffers[n], size_t)
{
	const uint8_t* data_begin = (uint8_t*)buffers[0].data;
	const uint8_t* data_end   = data_begin + buffers[0].len;

	json j = json::parse(data_begin, data_end);
	text += j["text"].get<std::string>() + "\n";

	Storage::response(text);
}
