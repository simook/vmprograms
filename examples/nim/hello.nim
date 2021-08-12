import varnish

const innPNG = staticRead("../inn.png")

proc on_request(arg: cstring) {.cdecl.} =
    response("image/png", innPNG)

proc post_backend(arg: string, buffer: string): (string, string) {.cdecl.} =
    ("image/png", innPNG)

set_request_handler(on_request)
set_post_handler(post_backend)
