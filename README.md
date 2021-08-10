# Varnish Edge Compute Demonstration

Demonstration of virtual machines in Varnish that handle backend requests instead of real backends. They can also be live-updated and live-debugged. They're also tiny and run natively on the CPU with KVM.

### About

On each request a VM is picked (or created on demand) from the given tenants program. Then the VM will execute the program in it that will generate the response to the client. When handling a backend request the VMs will have the state of the virtual machine as it was after executing main (the main() function). This means that it is possible to set everything up, like opening and readying a database, for the request handling.

Each VM weighs about 18KB of memory and can use a few KB more during the request. Each request has a maximum amount of memory it allows the VM to use. The memory is made up of around 2kb of the VM itself + the structures inside the kernel. Additionally, 4 pages have to be created which includes the stack page used when calling into the VM. Everything else is memory loaned from the original program, and writable memory is copy-on-write.

The time overhead of the VM should be less than 100 microseconds in production. It is less than 8 microseconds in synthetic benchmarks. These VMs have close to native performance as they are running on bare metal through KVM, easily outperforming WebAssembly.

On my local machine I can generate around 120 000 req/s with these VM backends at ~1ms response time. At 12k req/s the response time is ~600us as seen by the load testing program wrk2.

### Build and upload programs

Example:
```
./build_and_upload.sh xpizza.com examples/png.c
```

Available test tenants:

```
wpizza.com
xpizza.com
ypizza.com
zpizza.com
```

### Writing your own backend

Implement the `my_backend` function as an externally visible C function and make sure you return a backend_response using the API function.

The usual way to expose the function is like this:
```
__attribute__((used))
void my_backend(const char *arg)
{
	/* Here arg happens to be the URL */
}
```
There is no header manipulation API right now, so we simply pass the URL through to the function as the first argument.


### View results

Using browser:

http://141.0.231.216:8080/v

http://141.0.231.216:8080/w

http://141.0.231.216:8080/x

http://141.0.231.216:8080/y

http://141.0.231.216:8080/z


Using curl:
```
curl -D - -H "Host: xpizza.com" http://141.0.231.216:8080/
```

### Infinite loops

Timeouts are not yet implemented, so try to stay away from infinite loops for now. They are next on the list of many things.

Running out of memory or other kinds of things like CPU exceptions are completely fine. Go wild!
