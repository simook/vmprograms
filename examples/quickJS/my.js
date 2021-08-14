/*
	QuickJS inside Varnish
*/
console.log("Hello QuickJS World");

function my_backend()
{
	varnish.response(
		200,
		"text/plain",
		"Hello Backend World");
}
