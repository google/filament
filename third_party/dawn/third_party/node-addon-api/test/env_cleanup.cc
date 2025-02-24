#include <stdio.h>
#include "napi.h"

using namespace Napi;

#if (NAPI_VERSION > 2)
namespace {

static void cleanup(void* arg) {
  printf("static cleanup(%d)\n", *(int*)(arg));
}
static void cleanupInt(int* arg) {
  printf("static cleanup(%d)\n", *(arg));
}

static void cleanupVoid() {
  printf("static cleanup()\n");
}

static int secret1 = 42;
static int secret2 = 43;

class TestClass {
 public:
  Env::CleanupHook<void (*)(void* arg), int> hook;

  void removeHook(Env env) { hook.Remove(env); }
};

Value AddHooks(const CallbackInfo& info) {
  auto env = info.Env();

  bool shouldRemove = info[0].As<Boolean>().Value();

  // hook: void (*)(void *arg), hint: int
  auto hook1 = env.AddCleanupHook(cleanup, &secret1);
  // test using same hook+arg pair
  auto hook1b = env.AddCleanupHook(cleanup, &secret1);

  // hook: void (*)(int *arg), hint: int
  auto hook2 = env.AddCleanupHook(cleanupInt, &secret2);

  // hook: void (*)(int *arg), hint: void (default)
  auto hook3 = env.AddCleanupHook(cleanupVoid);
  // test using the same hook
  auto hook3b = env.AddCleanupHook(cleanupVoid);

  // hook: lambda []void (int *arg)->void, hint: int
  auto hook4 = env.AddCleanupHook(
      [&](int* arg) { printf("lambda cleanup(%d)\n", *arg); }, &secret1);

  // hook: lambda []void (void *)->void, hint: void
  auto hook5 =
      env.AddCleanupHook([&](void*) { printf("lambda cleanup(void)\n"); },
                         static_cast<void*>(nullptr));

  // hook: lambda []void ()->void, hint: void (default)
  auto hook6 = env.AddCleanupHook([&]() { printf("lambda cleanup()\n"); });

  if (shouldRemove) {
    hook1.Remove(env);
    hook1b.Remove(env);
    hook2.Remove(env);
    hook3.Remove(env);
    hook3b.Remove(env);
    hook4.Remove(env);
    hook5.Remove(env);
    hook6.Remove(env);
  }

  int added = 0;

  added += !hook1.IsEmpty();
  added += !hook1b.IsEmpty();
  added += !hook2.IsEmpty();
  added += !hook3.IsEmpty();
  added += !hook3b.IsEmpty();
  added += !hook4.IsEmpty();
  added += !hook5.IsEmpty();
  added += !hook6.IsEmpty();

  // Test store a hook in a member class variable
  auto myclass = TestClass();
  myclass.hook = env.AddCleanupHook(cleanup, &secret1);
  myclass.removeHook(env);

  return Number::New(env, added);
}

}  // anonymous namespace

Object InitEnvCleanup(Env env) {
  Object exports = Object::New(env);

  exports["addHooks"] = Function::New(env, AddHooks);

  return exports;
}

#endif
