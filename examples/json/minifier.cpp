#include "../api.h"
#include <simdjson.h>

static void minifier(const char*, const uint8_t* data, size_t length)
{
	auto* minified_buf = new char[length];
	size_t minified_len = 0;
	if (!simdjson::minify((const char*)data, length, minified_buf, minified_len)) {
		const char* ctype = "application/json";
		backend_response(200, ctype, strlen(ctype), minified_buf, minified_len);
	}

	static const char* const ctype = "text/plain";
	static const char* const cont = "Invalid JSON";
	backend_response(400, ctype, strlen(ctype), cont, strlen(cont));
}

int main()
{
	printf("-== SIMD JSON minifier ready ==-\n");
	set_backend_post(minifier);
	wait_for_requests();
}
