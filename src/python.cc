#include "proc.hh"
#include <iostream>
#include <memory>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "cython_main.h"

using namespace std;

void verify(const char *src, const char *fn_call) {
  if (PyRun_SimpleString(src)) {
    cerr << "Could define '" << fn_call << "'" << endl;
    exit(EXIT_FAILURE);
  }

  PyObject* main = PyImport_AddModule("__main__");
  PyObject* globals = PyModule_GetDict(main);
  PyObject* locals = PyDict_New();
  PyObject *pstr  = PyRun_String(fn_call, Py_eval_input, globals, locals);
  if (!pstr) {
    cerr << "Couldn't run '" << fn_call << "'" << endl;
    PyErr_Print();
    exit(EXIT_FAILURE);
  }

  char *cstr;
  PyArg_Parse(pstr, "s", &cstr);
  auto value = string(cstr);
  assert(!value.compare("Hello, world"));
  Py_DECREF(pstr);
}

int main(int argc, char *argv[]) {
  setup_py_std();
  shared_ptr<wchar_t> program(Py_DecodeLocale(argv[0], nullptr),
                              [](wchar_t* p) {
                                PyMem_RawFree(p);
                              });
  if (!program) {
    cerr << "Fatal error: cannot decode argv[0], got " <<
      argc << " arguments" << endl;
    return EXIT_FAILURE;
  }

  static size_t rss;
  // tag::native[]
  PyMethodDef user_methods[] = {
    {"read", [](PyObject *self, PyObject *args) -> PyObject * {
      rss = read_rss();
      return Py_BuildValue("s", "world");
    }, METH_VARARGS, nullptr},
    {nullptr, nullptr, 0, nullptr} /* Sentinel */
  };
  static struct PyModuleDef usermod = {
    PyModuleDef_HEAD_INIT, "usermod", nullptr, -1, user_methods
  };
  if (PyImport_AppendInittab("usermod", []() -> PyObject* {
    return PyModule_Create(&usermod);
  }) == -1) {
    cerr << "Error: could not extend in-built modules table" << endl;
    return EXIT_FAILURE;
  }
  // end::native[]

  // tag::cython[]
  if (PyImport_AppendInittab("cython_main", PyInit_cython_main) == -1) {
    cerr << "Error: could not extend in-built modules table" << endl;
    return EXIT_FAILURE;
  }
  // end::cython[]

  Py_SetProgramName(program.get());
  shared_ptr<void> py([]() {
    Py_Initialize();
    return nullptr;
  }(), [](void*) {
    Py_Finalize();
  });

  auto src = "import usermod; fn = lambda: 'Hello, ' + usermod.read()";
  verify(src, "fn()");

  auto cython_src =
    "import cython_main; fn = lambda: 'Hello, ' + cython_main.cy_read()";
  verify(cython_src, "fn()");

  cout << "| Python" << endl;
  cout << "| " << read_size(argv[0]) << endl;
  cout << "| " << rss << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
