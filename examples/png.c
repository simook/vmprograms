#include "api.h"

int main() {
}

EMBED_BINARY(my_data, "examples/inn.png");

__attribute__((used))
void my_backend(const char *arg)
{
	const char ctype[] = "image/png";
	backend_response(ctype, sizeof(ctype)-1, my_data, my_data_size);
}
