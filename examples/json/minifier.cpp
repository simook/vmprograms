#include "../api.h"
#include <simdjson.h>

static
void minifier(const char*, const char* data, size_t length)
{
	auto* minified_buf = new char[length];
	size_t minified_len = 0;
	auto error = simdjson::minify(data, length, minified_buf, minified_len);
	if (error) {
		[[unlikely]];
		const char* ctype = "text/plain";
		const char* cont = "Invalid JSON";
		backend_response(400, ctype, strlen(ctype),
			cont, strlen(cont));
		__builtin_unreachable();
	}

	const char* ctype = "application/json";
	backend_response(200, ctype, strlen(ctype),
		minified_buf, minified_len);
}

int main()
{
	printf("-== SIMD JSON minifier ready ==-\n");
	set_backend_post(minifier);
	wait_for_requests();
}
