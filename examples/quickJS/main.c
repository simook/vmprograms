#include "../api.h"
#include <stdio.h>
#include "quickjs/quickjs.h"
#define COUNTOF(a)  (sizeof(a) / sizeof(*(a)))

static JSValue js_varnish_backend_response(JSContext*, JSValueConst, int, JSValueConst*);

static const JSCFunctionListEntry js_varnish_funcs[] = {
    JS_CFUNC_DEF("backend_response", 4, js_varnish_backend_response ),
};

static int js_varnish_init(JSContext *ctx, JSModuleDef *m)
{
    return JS_SetModuleExportList(ctx, m, js_varnish_funcs,
                                  COUNTOF(js_varnish_funcs));
}

#define JS_INIT_MODULE js_init_module_varnish

JSModuleDef *JS_INIT_MODULE(JSContext *ctx, const char *module_name)
{
    JSModuleDef *m = JS_NewCModule(ctx, module_name, js_varnish_init);
    if (!m)
        return NULL;
    JS_AddModuleExportList(ctx, m, js_varnish_funcs, COUNTOF(js_varnish_funcs));
    return m;
}

int main()
{
	printf("Hello QuickJS main\n");
}

JSValue js_varnish_backend_response(JSContext *ctx,
	JSValueConst this_val, int argc, JSValueConst *argv)
{
	JSValue obj;
	return obj;
}
