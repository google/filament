#include "napi.h"

#if (NAPI_VERSION > 7)

using namespace Napi;

static const napi_type_tag type_tags[5] = {
    {0xdaf987b3cc62481a, 0xb745b0497f299531},
    {0xbb7936c374084d9b, 0xa9548d0762eeedb9},
    {0xa5ed9ce2e4c00c38, 0},
    {0, 0},
    {0xa5ed9ce2e4c00c38, 0xdaf987b3cc62481a},
};

template <TypeTaggable Factory(Env), typename NodeApiClass>
class TestTypeTaggable {
 public:
  static Value TypeTaggedInstance(const CallbackInfo& info) {
    TypeTaggable instance = Factory(info.Env());
    uint32_t type_index = info[0].As<Number>().Int32Value();

    instance.TypeTag(&type_tags[type_index]);

    return instance;
  }

  static Value CheckTypeTag(const CallbackInfo& info) {
    uint32_t type_index = info[0].As<Number>().Int32Value();
    TypeTaggable instance = info[1].As<NodeApiClass>();

    return Boolean::New(info.Env(),
                        instance.CheckTypeTag(&type_tags[type_index]));
  }
};

TypeTaggable ObjectFactory(Env env) {
  return Object::New(env);
}

TypeTaggable ExternalFactory(Env env) {
  // External does not accept a nullptr for its data.
  return External<void>::New(env, reinterpret_cast<void*>(0x1));
}

using TestObject = TestTypeTaggable<ObjectFactory, Object>;
using TestExternal = TestTypeTaggable<ExternalFactory, External<void>>;

Object InitTypeTaggable(Env env) {
  Object exports = Object::New(env);

  Object external = Object::New(env);
  exports["external"] = external;
  external["checkTypeTag"] = Function::New(env, &TestExternal::CheckTypeTag);
  external["typeTaggedInstance"] =
      Function::New(env, &TestExternal::TypeTaggedInstance);

  Object object = Object::New(env);
  exports["object"] = object;
  object["checkTypeTag"] = Function::New(env, &TestObject::CheckTypeTag);
  object["typeTaggedInstance"] =
      Function::New(env, &TestObject::TypeTaggedInstance);

  return exports;
}

#endif
