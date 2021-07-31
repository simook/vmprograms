import nim/varnish

const innPNG = staticRead("inn.png")

proc backend() {.exportc.} =
    response("image/png", innPNG)
