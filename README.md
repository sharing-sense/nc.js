
# Node-CEF (NC.js)

A CEF (Chromium Embedded Framework) extension which provides a Node.js module system for your CEF projects.

![Node-CEF](http://ww2.sinaimg.cn/large/6a6bd9c5gw1f50q3d5xqbj20og0hsq7g.jpg)

It's based on the stable CEF dll-wrapper API, therefore Node-CEF should be compatible with most CEF3 builds without modifying the `V8` engine or compiling a custom `libcef.dll`. Node-CEF is designed to work with general web pages, and it won't crash your render process  like other Node.js-CEF glued projects do when any uncaught errors occur.

**Note:** The native (Java Script) modules are completely compatible with Node.js while the built-in and linked (*.node) modules are not ABI compatible, but you can modify these modules and build them with Node-CEF easily.

## Main Features

- [Node.js](https://nodejs.org/en/) compatible module system for your CEF project.
- Managing and installing packages with [npm](https://www.npmjs.com/).
- Writing and registering your extra built-in modules easily.
- Debugging modules with Chrome DevTools.

### Currently available modules:

- [Assertion Testing](https://nodejs.org/dist/latest-v4.x/docs/api/assert.html)
- [Buffer](https://nodejs.org/dist/latest-v4.x/docs/api/buffer.html)
- [Events](https://nodejs.org/dist/latest-v4.x/docs/api/events.html)
- [File System](https://nodejs.org/dist/latest-v4.x/docs/api/fs.html)
- [Globals](https://nodejs.org/dist/latest-v4.x/docs/api/globals.html)
- [Modules](https://nodejs.org/dist/latest-v4.x/docs/api/modules.html)
- [OS](https://nodejs.org/dist/latest-v4.x/docs/api/os.html)
- [VM](https://nodejs.org/dist/latest-v4.x/docs/api/vm.html)
- [Process](https://nodejs.org/dist/latest-v4.x/docs/api/process.html)
- [Punycode](https://nodejs.org/dist/latest-v4.x/docs/api/punycode.html)
- [Query String](https://nodejs.org/dist/latest-v4.x/docs/api/querystring.html)
- [String Decoder](https://nodejs.org/dist/latest-v4.x/docs/api/string_decoder.html)
- [Timers](https://nodejs.org/dist/latest-v4.x/docs/api/timers.html)
- [URL](https://nodejs.org/dist/latest-v4.x/docs/api/url.html)
- [Utilities](https://nodejs.org/dist/latest-v4.x/docs/api/util.html)

See also [Differences with Node.js](#differences-with-nodejs)

### Security

Any remote accesses to the `ncjs` module are forbidden, currently only `file://` scheme is the valid source to initialize Node-CEF.

### What's next:

- [Child Process](https://nodejs.org/dist/latest-v4.x/docs/api/child_process.html)
- [Stream](https://nodejs.org/dist/latest-v4.x/docs/api/stream.html)
- Supports for remote modules (Sync/Async).

Please create pull requests to help us add supports for the rest modules of Node.js.

## Usages

### Build Node-CEF

Requirement: VS2005 (VC8.0) and above, Python 2.x

1. Open `Node-CEF.sln` with your Visual Studio.
2. Add your CEF path to "Additional Include Directories" of the `libcef_node` project.
3. Build solution.

### Use Node-CEF in your CEF project

Please add these libraries to your project's "Additional Dependencies":

```
libcef_node.lib
libcef_dll_wrapper.lib
libcef.lib
libuv.lib
Psapi.lib
Userenv.lib
Ws2_32.lib
Iphlpapi.lib
```

This sample shows how to integrate Node-CEF in your CEF project:
```cpp
#include <include/cef_app.h>
#include <ncjs/RenderProcessHandler.h>

class MyNodeCefApp : public CefApp, public ncjs::RenderProcessHandler {
public:

    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() { return this; }

    IMPLEMENT_REFCOUNTING(MyNodeCefApp);
};

int APIENTRY wWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine,
                      int       nCmdShow) {
  const CefMainArgs main_args(hInstance);

  return CefExecuteProcess(main_args, new MyNodeCefApp, NULL);
}
```
What we need to do is just changing the inheritance from `CefRenderProcessHandler` to `ncjs::RenderProcessHandler` for your `CefApp` based class.

Now your CEF project owns the Node.js module system, you can test it with:
```js
ncjs.require('os').hostname();
```

### Add your own built-in module

Codes for MyModule.cpp:
```cpp
#include "ncjs/module.h"

namespace ncjs {

class MyModule : public JsObjecT<MyModule> {

    // my_module.foo()
    NCJS_OBJECT_FUNCTION(Foo)(CefRefPtr<CefV8Value> object,
        const CefV8ValueList& args, CefRefPtr<CefV8Value>& retval, CefString& except)
    {
        retval = CefV8Value::CreateString(NCJS_REFTEXT("Hello Node-CEF!"));
    }

    // object factory

    NCJS_BEGIN_OBJECT_FACTORY()
        NCJS_MAP_OBJECT_FUNCTION("foo", Foo)
    NCJS_END_OBJECT_FACTORY()

};
NCJS_DEFINE_BUILTIN_MODULE(my_module, MyModule);

} // ncjs
```
In `RenderProcessHandler::OnNodeCefCreated()` callback:
```cpp
NCJS_DECLARE_BUILTIN_MODULE(MyModule);

void MyNodeCefApp::OnNodeCefCreated(CefCommandLine& args)
{
    ModuleManager::Register(NCJS_BUILTIN_MODULE(MyModule));
}
```
Now you can access `MyModule::Foo()` via Java Script:
```js
ncjs.process.binding('my_module').foo();
```

**Note:** If your render process handler overrides any methods of `CefRenderProcessHandler`, please remember to call the corresponding one of `ncjs::RenderProcessHandler`'s in your implementations, otherwise Node-CEF won't work.

## Differences with Node.js

### Global objects

To accompany general web pages and script, Node-CEF inject a global object identified by `ncjs` to each frame in a sing web page and only will be load when this object get accessed first time, this also improve the performance hit for pages that do not use Node-CEF.

The global `ncjs` object is main module of Node-CEF, all global objects (except the `global` object) defined by Node.js are stored in this object, such as `require`, `process`, etc. You can define aliases for the objects inside `ncjs` for convenience:
```js
var require = ncjs.require;
```
Now you can use `require` directive just like Node.js provides:
```js
var mod = require('a/module');
```

### Search paths for modules

The search paths for `require()` in a web page is NOT relative to the `*.js` file containing `require()` directives but the page itself. This is because all scripts in a web page share a the same main module `ncjs` which loaded by the web page.

Considering this folder structure:
```
./index.html
./path/to/general/java_script/code.js
./path/to/general/java_script/node_modules/my_module.js
./node_modules/my_module.js
```
while code.js:
```js
var my_module = ncjs.require('my_module');
```
When `index.html` loads code.js, the module located at `./node_modules/my_module.js` will be loaded but the `./path/to/general/java_script/node_modules/my_module.js` will be invisible to Node-CEF. Try `test/require` for a completed test.

**Note:** All these changes above only apply to **web pages**, a Node.js module has its `require`, `exports`, etc, as well as search paths, nothing is changed for modules.

### Modules Implementation

#### Buffer
Because of the missing `ArrayBuffer` and `Uint8Array` supports from CEF, the subscripting operator `buf[]` is not supported, use `buf.get()` and `buf.set()` to access buffer data.

#### Process
- Event: `beforeExit`, `rejectionHandled` and `unhandledRejection` are not emitted.
- Event: `uncaughtException` is emitted if `CefSettings::uncaught_exception_stack_size` > 0.


## Compatibilities

|     CEF 3     | Chromium | Linux | Mac | Windows |
|:-------------:|:--------:|:-----:|:---:|:-------:|
|  Branch 3396  |   67.0   |   ?   |  ?  |    ?    |


Please feedback us whether Node-CEF works with your CEF.

Reference / Porting API: [Node.js v4.3.2](https://github.com/nodejs/node/tree/v4.3.2)


## License

[Node-CEF](https://github.com/GPBeta/nc.js) is licensed under the MIT license.

> Joshua ([Studio GPBeta](http://www.gpbeta.com/)) @2016
