#include "mem.hh"
#include <cassert>
#include <chibi/eval.h>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

using namespace std;

auto init = R"(
(define (string-append . args) (string-concatenate args))
)";

void check(sexp ctx, sexp res) {
  if (sexp_exceptionp(res)) {
    sexp_print_exception(ctx, res, sexp_current_error_port(ctx));
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char* argv[]) {
  sexp_scheme_init();
  sexp context = sexp_make_eval_context(nullptr, nullptr, nullptr, 0, 0);
  shared_ptr<sexp> ctx(&context, [](sexp *c) {
    sexp_destroy_context(*c);
  });
  sexp_load_standard_ports(*ctx, nullptr, stdin, stdout, stderr, 1);

  sexp res = sexp_eval_string(*ctx, init, -1, nullptr);
  check(*ctx, res);

  static size_t rss;
  // tag::native[]
  res = sexp_define_foreign(*ctx, sexp_context_env(*ctx), "read", 0,
                            [](sexp ctx, sexp self, sexp_sint_t n) -> sexp {
                              rss = read_rss();
                              return sexp_c_string(ctx, "world", -1);
                            });
  // end::native[]
  check(*ctx, res);

  auto src = "(define fn(lambda () (string-append \"Hello, \" (read))))";
  res = sexp_eval_string(*ctx, src, -1, nullptr);
  check(*ctx, res);

  res = sexp_eval_string(*ctx, "(fn)", -1, nullptr);
  check(*ctx, res);

  if (sexp_stringp(res)) {
    auto value = string(sexp_string_data(res));
    assert(!value.compare("Hello, world"));
  } else {
    cerr << "Incorrect result type" << endl;
    return EXIT_FAILURE;
  }

  cout << "| Chibi-Scheme" << endl;
  cout << "| " << filesystem::file_size(argv[0]) / 1024 << endl;
  cout << "| " << rss << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
