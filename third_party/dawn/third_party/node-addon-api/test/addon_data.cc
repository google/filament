#if (NAPI_VERSION > 5)
#include <stdio.h>
#include "napi.h"
#include "test_helper.h"

// An overly elaborate way to get/set a boolean stored in the instance data:
// 0. The constructor for JS `VerboseIndicator` instances, which have a private
//    member named "verbose", is stored in the instance data.
// 1. Add a property named "verbose" onto exports served by a getter/setter.
// 2. The getter returns an object of type VerboseIndicator, which itself has a
//    property named "verbose", also served by a getter/setter:
//    * The getter returns a boolean, indicating whether "verbose" is set.
//    * The setter sets "verbose" on the instance data.
// 3. The setter sets "verbose" on the instance data.

class Addon {
 public:
  class VerboseIndicator : public Napi::ObjectWrap<VerboseIndicator> {
   public:
    VerboseIndicator(const Napi::CallbackInfo& info)
        : Napi::ObjectWrap<VerboseIndicator>(info) {
      info.This().As<Napi::Object>()["verbose"] = Napi::Boolean::New(
          info.Env(), info.Env().GetInstanceData<Addon>()->verbose);
    }

    Napi::Value Getter(const Napi::CallbackInfo& info) {
      return Napi::Boolean::New(info.Env(),
                                info.Env().GetInstanceData<Addon>()->verbose);
    }

    void Setter(const Napi::CallbackInfo& info, const Napi::Value& val) {
      info.Env().GetInstanceData<Addon>()->verbose = val.As<Napi::Boolean>();
    }

    static Napi::FunctionReference Init(Napi::Env env) {
      return Napi::Persistent(DefineClass(
          env,
          "VerboseIndicator",
          {InstanceAccessor<&VerboseIndicator::Getter,
                            &VerboseIndicator::Setter>("verbose")}));
    }
  };

  static Napi::Value Getter(const Napi::CallbackInfo& info) {
    return MaybeUnwrap(
        info.Env().GetInstanceData<Addon>()->VerboseIndicator.New({}));
  }

  static void Setter(const Napi::CallbackInfo& info) {
    info.Env().GetInstanceData<Addon>()->verbose = info[0].As<Napi::Boolean>();
  }

  Addon(Napi::Env env) : VerboseIndicator(VerboseIndicator::Init(env)) {}
  ~Addon() {
    if (verbose) {
      fprintf(stderr, "addon_data: Addon::~Addon\n");
    }
  }

  static void DeleteAddon(Napi::Env, Addon* addon, uint32_t* hint) {
    delete addon;
    fprintf(stderr, "hint: %u\n", *hint);
    delete hint;
  }

  static Napi::Object Init(Napi::Env env, Napi::Value jshint) {
    if (!jshint.IsNumber()) {
      NAPI_THROW(Napi::Error::New(env, "Expected number"), Napi::Object());
    }
    uint32_t hint = jshint.As<Napi::Number>();
    if (hint == 0)
      env.SetInstanceData(new Addon(env));
    else
      env.SetInstanceData<Addon, uint32_t, DeleteAddon>(new Addon(env),
                                                        new uint32_t(hint));
    Napi::Object result = Napi::Object::New(env);
    result.DefineProperties({
        Napi::PropertyDescriptor::Accessor<Getter, Setter>("verbose"),
    });

    return result;
  }

 private:
  bool verbose = false;
  Napi::FunctionReference VerboseIndicator;
};

// We use an addon factory so we can cover both the case where there is an
// instance data hint and the case where there isn't.
static Napi::Value AddonFactory(const Napi::CallbackInfo& info) {
  return Addon::Init(info.Env(), info[0]);
}

Napi::Object InitAddonData(Napi::Env env) {
  return Napi::Function::New(env, AddonFactory);
}
#endif  // (NAPI_VERSION > 5)
