#include <cstdio>
#include "../api.h"

extern "C" {
	struct facedetect_result {
		const uint8_t* resdata;
		size_t         reslen;
	};
	DYNAMIC_CALL(facedetect, 0x5B5BAE2, const uint8_t*, size_t, facedetect_result*);
}

static void facedetector(const char* /*url*/, const uint8_t* data, size_t len)
{
	facedetect_result result;
	facedetect(data, len, &result);

	const char* ctype = "image/jpeg";
	const size_t ctlen = strlen(ctype);
	backend_response(200, ctype, ctlen, result.resdata, result.reslen);
}

int main()
{
	printf("Facedetect Loaded\n");
	set_backend_post(facedetector);
	wait_for_requests();
}
