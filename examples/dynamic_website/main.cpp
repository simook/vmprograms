#include "../api.h"
#include <cstdio>
#include <nlohmann/json.hpp>
#include <vector>
using json = nlohmann::json;

int main() {
	printf("C++ JSON example main\n");
}

__attribute__((noreturn))
inline void response(const std::string& ctype, const std::string& content)
{
	backend_response(ctype.c_str(), ctype.size(), content.c_str(), content.size());
	__builtin_unreachable();
}

static json gj;

extern "C" void
retrieve_json(void*, size_t len, size_t /* reslen */)
{
	auto result = gj.dump(4);
	storage_return(result.c_str(), result.size());
}

extern "C" __attribute__((used))
void my_backend(const char *arg)
{
	char result[1024];
	int len = storage_call(retrieve_json, nullptr, 0, result, sizeof(result));
	backend_response("text/plain", 10, result, len);
}

extern "C" void
set_json(void* data, size_t len, size_t /* reslen */)
{
	const uint8_t* data_begin = (uint8_t*)data;
	const uint8_t* data_end   = data_begin + len;

	gj = json::parse(data_begin, data_end);

	auto result = gj.dump(4);
	storage_return(result.c_str(), result.size());
}

extern "C" __attribute__((used))
void my_post_backend(const char* /* arg */, void* data, size_t len)
{
	char result[1024];
	int reslen =
		storage_call(set_json, data, len, result, sizeof(result));

	backend_response("text/plain", 10, result, reslen);
}
