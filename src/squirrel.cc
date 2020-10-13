#include <cassert>
#include <filesystem>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>

#include <squirrel.h>
#include <sqstdaux.h>

#ifdef SQUNICODE
#define scfprintf fwprintf
#define scvprintf vfwprintf
#else
#define scfprintf fprintf
#define scvprintf vfprintf
#endif

using namespace std;

void printfunc(HSQUIRRELVM SQ_UNUSED_ARG(v),const SQChar *s,...) {
  va_list vl;
  va_start(vl, s);
  scvprintf(stdout, s, vl);
  va_end(vl);
}

void errorfunc(HSQUIRRELVM SQ_UNUSED_ARG(v),const SQChar *s,...) {
  va_list vl;
  va_start(vl, s);
  scvprintf(stderr, s, vl);
  va_end(vl);
}

int main(int argc, char* argv[]) {
  HSQUIRRELVM vm = sq_open(1024);
  shared_ptr<HSQUIRRELVM> v(&vm, [](HSQUIRRELVM *v) {
    sq_close(*v);
  });

  sq_setprintfunc(*v, printfunc, errorfunc);
  sqstd_seterrorhandlers(*v);

  // tag::native[]
  sq_pushroottable(*v);
  sq_pushstring(*v, _SC("read"), -1);
  sq_newclosure(*v, [](HSQUIRRELVM v) -> SQInteger {
    sq_pushstring(v, _SC("world"), -1);
    return 1;
  }, 0);
  sq_newslot(*v, -3, SQFalse);
  sq_pop(*v, 1);
  // end::native[]

  SQChar buffer[] = _SC("function fn() { return \"Hello, \" + read(); }");
  if (SQ_SUCCEEDED(sq_compilebuffer(*v,
                                    buffer,
                                    sizeof(buffer)/sizeof(SQChar),
                                    _SC("script"),
                                    SQTrue))) {
    sq_pushroottable(*v);
    if (SQ_FAILED(sq_call(*v, 1, 0, SQTrue))) {
      cerr << "Couldn't run the compiled `fn`" << endl;
      return EXIT_FAILURE;
    }
    sq_pop(*v, 1);

    sq_pushroottable(*v);
    sq_pushstring(*v, _SC("fn"), -1);
    if (SQ_SUCCEEDED(sq_get(*v, -2))) {
      sq_pushroottable(*v);
      if (SQ_SUCCEEDED(sq_call(*v, 1, SQTrue, SQTrue))) {
        const SQChar *s;
        sq_getstring(*v, -1, &s);
        auto value = string(s);
        assert(!value.compare("Hello, world"));
        sq_pop(*v, 2);
      } else {
        cerr << "Couldn't call 'fn'" << endl;
        return EXIT_FAILURE;
      }
    } else {
      cerr << "Couldn't find 'fn'" << endl;
      return EXIT_FAILURE;
    }
  } else {
    cerr << "Couldn't compile 'fn'" << endl;
    return EXIT_FAILURE;
  }

  cout << "| Squirrel" << endl;
  cout << "| " << filesystem::file_size(argv[0]) << endl;
  cout << "| `" << buffer << "`" << endl;

  return EXIT_SUCCESS;
}
