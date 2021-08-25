#define KVM_API_ALREADY_DEFINED
#include "api.h"
#include <quickjs/cutils.h>
#include <quickjs/quickjs.h>
#include <quickjs/quickjs-libc.h>

JSValue js_http_fetch_call(JSContext *ctx,
	JSValueConst this_val, int argc, JSValueConst *argv)
{
	(void)this_val;
	if (argc >= 1) {
		size_t urllen;
		const char* url =
			JS_ToCStringLen(ctx, &urllen, argv[0]);
		if (!url)
			return JS_EXCEPTION;
		/* Fetch whole thing */
		struct curl_opts opts = {
			.status = 0,
			.ctlen  = 256,
		};
		if (curl_fetch(url, urllen, &opts) == 0) {
			return JS_NewStringLen(
				ctx, opts.content, opts.content_length);
		}
	}
	return JS_EXCEPTION;
}
