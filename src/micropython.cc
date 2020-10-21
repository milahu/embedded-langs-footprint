#include "proc.hh"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include "py/compile.h"
#include "py/gc.h"
#include "py/runtime.h"
#include "py/stackctrl.h"
}

using namespace std;

static char heap[16384];

mp_obj_t exec(const string &str, mp_parse_input_kind_t parse_input_kind) {
  auto lex = mp_lexer_new_from_str_len(MP_QSTR__lt_string_gt_,
                                       str.c_str(),
                                       str.length(),
                                       0);
  return mp_parse_compile_execute(lex,
                                  parse_input_kind,
                                  mp_globals_get(),
                                  mp_locals_get());
}

int main(int argc, char *argv[]) {
  mp_stack_set_limit(40000 * (BYTES_PER_WORD / 4));
  gc_init(heap, heap + sizeof(heap));
  mp_init();

  static size_t rss;
  // tag::native[]
  // C code should use STATIC MP_DEFINE_CONST_FUN_OBJ_0
  // (that is not compatible with C++)
  STATIC const mp_obj_fun_builtin_fixed_t read_obj = {
    {&mp_type_fun_builtin_0},.fun={._0 = []() -> mp_obj_t {
      const char str[] = "world";
      rss = read_rss();
      return mp_obj_new_str(str, sizeof(str));
    }}};

  // Uses already generated QSTR, production code
  // should use proper MicroPython string interning
  mp_store_attr(mp_obj_new_module(MP_QSTR_builtins),
                MP_QSTR_read,
                (mp_obj_t)MP_ROM_PTR(&read_obj));
  // end::native[]

  auto src = "import builtins; fn = lambda: 'Hello, ' + builtins.read()";
  exec(src, MP_PARSE_FILE_INPUT);

  auto mp_obj = exec("fn()", MP_PARSE_EVAL_INPUT);
  auto value = string(mp_obj_str_get_str(mp_obj));
  assert(!value.compare("Hello, world"));

  cout << "| MicroPython" << endl;
  cout << "| " << read_size(argv[0]) << endl;
  cout << "| " << rss << endl;
  cout << "| `" << src << "`" << endl;
}

mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

void nlr_jump_fail(void *val) {
    mp_obj_print_exception(&mp_plat_print, MP_OBJ_FROM_PTR(val));
    exit(EXIT_FAILURE);
}
