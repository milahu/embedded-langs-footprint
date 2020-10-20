#include "angelscript.h"
#include "mem.hh"
#include <cassert>
#include <filesystem>
#include <iostream>
#include <memory>
#include <scriptstdstring/scriptstdstring.h>
#include <string>

using namespace std;

void messageCallback(const asSMessageInfo *msg, void *param) {
  cerr << msg->section << "(" << msg->row << ", " << msg->col << "): ";
  switch (msg->type) {
  case asMSGTYPE_ERROR: cerr << "ERR"; break;
  case asMSGTYPE_WARNING: cerr << "WARN"; break;
  case asMSGTYPE_INFORMATION: cerr << "INFO"; break;
  }
  cerr << " : " << msg->message << endl;
}

int main(int argc, char* argv[]) {
  shared_ptr<asIScriptEngine> engine(asCreateScriptEngine(),
                                     [](asIScriptEngine *e) {
                                       e->ShutDownAndRelease();
                                     });
  engine->SetEngineProperty(asEP_ALLOW_MULTILINE_STRINGS, true);

  auto r = engine->SetMessageCallback(asFUNCTION(messageCallback),
                                      0, asCALL_CDECL);
  assert(r >= 0);

  RegisterStdString(engine.get());

  static size_t rss;
  // tag::native[]
  r = engine->RegisterGlobalFunction("string read()",
                                     asFUNCTION(*[]() -> string {
                                       rss = read_rss();
                                       return "world";
                                     }),
                                     asCALL_CDECL);
  // end::native[]
  assert(r >= 0);

  auto mod = engine->GetModule("module", asGM_ALWAYS_CREATE);

  auto src = "string fn() { return 'Hello, ' + read(); }";
  mod->AddScriptSection("section", src);

  r = mod->Build();
  assert(r >= 0);

  auto decl = "string fn()";
  auto func = mod->GetFunctionByDecl(decl);
  if (func == nullptr) {
    cerr << "The script must have the function '" << decl << "'."
         << "Please add it and try again." << endl;
    return EXIT_FAILURE;
  }

  shared_ptr<asIScriptContext> ctx(engine->CreateContext(),
                                   [](asIScriptContext *c) {
                                     c->Release();
                                   });
  ctx->Prepare(func);
  r = ctx->Execute();
  if (r != asEXECUTION_FINISHED) {
    cerr << "Couldn't execute." << endl;
    if (r == asEXECUTION_EXCEPTION) {
      cerr << "An exception '" << ctx->GetExceptionString() << "' occurred."
           << "Please correct the code and try again." << endl;
    }
    return EXIT_FAILURE;
  } else {
    auto value = *(string*)ctx->GetReturnObject();
    assert(!value.compare("Hello, world"));
  }

  cout << "| AngelScript" << endl;
  cout << "| " << filesystem::file_size(argv[0]) / 1024 << endl;
  cout << "| " << rss << endl;
  cout << "| `" << src << "`" << endl;

  return EXIT_SUCCESS;
}
