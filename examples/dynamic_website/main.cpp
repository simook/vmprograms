#include "varnish_api.hpp"

#include <cstdio>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main() {
	printf("C++ JSON example main\n");
}

static json gj;

extern "C" void
retrieve_json(void*, size_t len, size_t /* reslen */)
{
	Storage::response(gj.dump(4));
}

extern "C" __attribute__((used))
void my_backend(const char *arg)
{
	std::vector<uint8_t> result(1024);

	long len = Storage::call(retrieve_json, result);
	result.resize(len);

	Backend::response("application/json", result);
}

extern "C" void
set_json(void* data, size_t len, size_t /* reslen */)
{
	const uint8_t* data_begin = (uint8_t*)data;
	const uint8_t* data_end   = data_begin + len;

	gj = json::parse(data_begin, data_end);

	Storage::response(gj.dump(4));
}

extern "C" __attribute__((used))
void my_post_backend(const char* /* arg */, void* data, size_t len)
{
	std::vector<uint8_t> result(1024);

	size_t reslen = Storage::call(set_json, data, len, result);
	result.resize(reslen);

	Backend::response("application/json", result);
}
