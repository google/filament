#include "napi.h"
#include "test_helper.h"

using namespace Napi;

// Native wrappers for testing Object::Get()
Value GetPropertyWithUint32(const CallbackInfo& info);
Value GetPropertyWithNapiValue(const CallbackInfo& info);
Value GetPropertyWithNapiWrapperValue(const CallbackInfo& info);
Value GetPropertyWithCStyleString(const CallbackInfo& info);
Value GetPropertyWithCppStyleString(const CallbackInfo& info);

// Native wrappers for testing Object::Set()
Value SetPropertyWithUint32(const CallbackInfo& info);
Value SetPropertyWithNapiValue(const CallbackInfo& info);
Value SetPropertyWithNapiWrapperValue(const CallbackInfo& info);
Value SetPropertyWithCStyleString(const CallbackInfo& info);
Value SetPropertyWithCppStyleString(const CallbackInfo& info);

// Native wrappers for testing Object::Delete()
Value DeletePropertyWithUint32(const CallbackInfo& info);
Value DeletePropertyWithNapiValue(const CallbackInfo& info);
Value DeletePropertyWithNapiWrapperValue(const CallbackInfo& info);
Value DeletePropertyWithCStyleString(const CallbackInfo& info);
Value DeletePropertyWithCppStyleString(const CallbackInfo& info);

// Native wrappers for testing Object::HasOwnProperty()
Value HasOwnPropertyWithNapiValue(const CallbackInfo& info);
Value HasOwnPropertyWithNapiWrapperValue(const CallbackInfo& info);
Value HasOwnPropertyWithCStyleString(const CallbackInfo& info);
Value HasOwnPropertyWithCppStyleString(const CallbackInfo& info);

// Native wrappers for testing Object::Has()
Value HasPropertyWithUint32(const CallbackInfo& info);
Value HasPropertyWithNapiValue(const CallbackInfo& info);
Value HasPropertyWithNapiWrapperValue(const CallbackInfo& info);
Value HasPropertyWithCStyleString(const CallbackInfo& info);
Value HasPropertyWithCppStyleString(const CallbackInfo& info);

// Native wrappers for testing Object::AddFinalizer()
Value AddFinalizer(const CallbackInfo& info);
Value AddFinalizerWithHint(const CallbackInfo& info);

// Native wrappers for testing Object::operator []
Value SubscriptGetWithCStyleString(const CallbackInfo& info);
Value SubscriptGetWithCppStyleString(const CallbackInfo& info);
Value SubscriptGetAtIndex(const CallbackInfo& info);
void SubscriptSetWithCStyleString(const CallbackInfo& info);
void SubscriptSetWithCppStyleString(const CallbackInfo& info);
void SubscriptSetAtIndex(const CallbackInfo& info);

static bool testValue = true;
// Used to test void* Data() integrity
struct UserDataHolder {
  int32_t value;
};

Value TestGetter(const CallbackInfo& info) {
  return Boolean::New(info.Env(), testValue);
}

void TestSetter(const CallbackInfo& info) {
  testValue = info[0].As<Boolean>();
}

Value TestGetterWithUserData(const CallbackInfo& info) {
  const UserDataHolder* holder = reinterpret_cast<UserDataHolder*>(info.Data());
  return Number::New(info.Env(), holder->value);
}

void TestSetterWithUserData(const CallbackInfo& info) {
  UserDataHolder* holder = reinterpret_cast<UserDataHolder*>(info.Data());
  holder->value = info[0].As<Number>().Int32Value();
}

Value TestFunction(const CallbackInfo& info) {
  return Boolean::New(info.Env(), true);
}

Value TestFunctionWithUserData(const CallbackInfo& info) {
  UserDataHolder* holder = reinterpret_cast<UserDataHolder*>(info.Data());
  return Number::New(info.Env(), holder->value);
}

Value EmptyConstructor(const CallbackInfo& info) {
  auto env = info.Env();
  bool isEmpty = info[0].As<Boolean>();
  Object object = isEmpty ? Object() : Object(env, Object::New(env));
  return Boolean::New(env, object.IsEmpty());
}

Value ConstructorFromObject(const CallbackInfo& info) {
  auto env = info.Env();
  Object object = info[0].As<Object>();
  return Object(env, object);
}

Array GetPropertyNames(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Array arr = MaybeUnwrap(obj.GetPropertyNames());
  return arr;
}

void DefineProperties(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  String nameType = info[1].As<String>();
  Env env = info.Env();

  Boolean trueValue = Boolean::New(env, true);
  UserDataHolder* holder = new UserDataHolder();
  holder->value = 1234;

  if (nameType.Utf8Value() == "literal") {
    obj.DefineProperties({
        PropertyDescriptor::Accessor(env, obj, "readonlyAccessor", TestGetter),
        PropertyDescriptor::Accessor(
            env, obj, "readwriteAccessor", TestGetter, TestSetter),
        PropertyDescriptor::Accessor(env,
                                     obj,
                                     "readonlyAccessorWithUserData",
                                     TestGetterWithUserData,
                                     napi_property_attributes::napi_default,
                                     reinterpret_cast<void*>(holder)),
        PropertyDescriptor::Accessor(env,
                                     obj,
                                     "readwriteAccessorWithUserData",
                                     TestGetterWithUserData,
                                     TestSetterWithUserData,
                                     napi_property_attributes::napi_default,
                                     reinterpret_cast<void*>(holder)),

        PropertyDescriptor::Accessor<TestGetter>("readonlyAccessorT"),
        PropertyDescriptor::Accessor<TestGetter, TestSetter>(
            "readwriteAccessorT"),
        PropertyDescriptor::Accessor<TestGetterWithUserData>(
            "readonlyAccessorWithUserDataT",
            napi_property_attributes::napi_default,
            reinterpret_cast<void*>(holder)),
        PropertyDescriptor::Accessor<TestGetterWithUserData,
                                     TestSetterWithUserData>(
            "readwriteAccessorWithUserDataT",
            napi_property_attributes::napi_default,
            reinterpret_cast<void*>(holder)),

        PropertyDescriptor::Value("readonlyValue", trueValue),
        PropertyDescriptor::Value("readwriteValue", trueValue, napi_writable),
        PropertyDescriptor::Value(
            "enumerableValue", trueValue, napi_enumerable),
        PropertyDescriptor::Value(
            "configurableValue", trueValue, napi_configurable),
        PropertyDescriptor::Function(env, obj, "function", TestFunction),
        PropertyDescriptor::Function(env,
                                     obj,
                                     "functionWithUserData",
                                     TestFunctionWithUserData,
                                     napi_property_attributes::napi_default,
                                     reinterpret_cast<void*>(holder)),
    });
  } else if (nameType.Utf8Value() == "string") {
    // VS2013 has lifetime issues when passing temporary objects into the
    // constructor of another object. It generates code to destruct the object
    // as soon as the constructor call returns. Since this isn't a common case
    // for using std::string objects, I'm refactoring the test to work around
    // the issue.
    std::string str1("readonlyAccessor");
    std::string str2("readwriteAccessor");
    std::string str1a("readonlyAccessorWithUserData");
    std::string str2a("readwriteAccessorWithUserData");

    std::string str1t("readonlyAccessorT");
    std::string str2t("readwriteAccessorT");
    std::string str1at("readonlyAccessorWithUserDataT");
    std::string str2at("readwriteAccessorWithUserDataT");

    std::string str3("readonlyValue");
    std::string str4("readwriteValue");
    std::string str5("enumerableValue");
    std::string str6("configurableValue");
    std::string str7("function");
    std::string str8("functionWithUserData");

    obj.DefineProperties({
        PropertyDescriptor::Accessor(env, obj, str1, TestGetter),
        PropertyDescriptor::Accessor(env, obj, str2, TestGetter, TestSetter),
        PropertyDescriptor::Accessor(env,
                                     obj,
                                     str1a,
                                     TestGetterWithUserData,
                                     napi_property_attributes::napi_default,
                                     reinterpret_cast<void*>(holder)),
        PropertyDescriptor::Accessor(env,
                                     obj,
                                     str2a,
                                     TestGetterWithUserData,
                                     TestSetterWithUserData,
                                     napi_property_attributes::napi_default,
                                     reinterpret_cast<void*>(holder)),

        PropertyDescriptor::Accessor<TestGetter>(str1t),
        PropertyDescriptor::Accessor<TestGetter, TestSetter>(str2t),
        PropertyDescriptor::Accessor<TestGetterWithUserData>(
            str1at,
            napi_property_attributes::napi_default,
            reinterpret_cast<void*>(holder)),
        PropertyDescriptor::Accessor<TestGetterWithUserData,
                                     TestSetterWithUserData>(
            str2at,
            napi_property_attributes::napi_default,
            reinterpret_cast<void*>(holder)),

        PropertyDescriptor::Value(str3, trueValue),
        PropertyDescriptor::Value(str4, trueValue, napi_writable),
        PropertyDescriptor::Value(str5, trueValue, napi_enumerable),
        PropertyDescriptor::Value(str6, trueValue, napi_configurable),
        PropertyDescriptor::Function(env, obj, str7, TestFunction),
        PropertyDescriptor::Function(env,
                                     obj,
                                     str8,
                                     TestFunctionWithUserData,
                                     napi_property_attributes::napi_default,
                                     reinterpret_cast<void*>(holder)),
    });
  } else if (nameType.Utf8Value() == "value") {
    obj.DefineProperties({
        PropertyDescriptor::Accessor(
            env, obj, Napi::String::New(env, "readonlyAccessor"), TestGetter),
        PropertyDescriptor::Accessor(
            env,
            obj,
            Napi::String::New(env, "readwriteAccessor"),
            TestGetter,
            TestSetter),
        PropertyDescriptor::Accessor(
            env,
            obj,
            Napi::String::New(env, "readonlyAccessorWithUserData"),
            TestGetterWithUserData,
            napi_property_attributes::napi_default,
            reinterpret_cast<void*>(holder)),
        PropertyDescriptor::Accessor(
            env,
            obj,
            Napi::String::New(env, "readwriteAccessorWithUserData"),
            TestGetterWithUserData,
            TestSetterWithUserData,
            napi_property_attributes::napi_default,
            reinterpret_cast<void*>(holder)),

        PropertyDescriptor::Accessor<TestGetter>(
            Napi::String::New(env, "readonlyAccessorT")),
        PropertyDescriptor::Accessor<TestGetter, TestSetter>(
            Napi::String::New(env, "readwriteAccessorT")),
        PropertyDescriptor::Accessor<TestGetterWithUserData>(
            Napi::String::New(env, "readonlyAccessorWithUserDataT"),
            napi_property_attributes::napi_default,
            reinterpret_cast<void*>(holder)),
        PropertyDescriptor::Accessor<TestGetterWithUserData,
                                     TestSetterWithUserData>(
            Napi::String::New(env, "readwriteAccessorWithUserDataT"),
            napi_property_attributes::napi_default,
            reinterpret_cast<void*>(holder)),

        PropertyDescriptor::Value(Napi::String::New(env, "readonlyValue"),
                                  trueValue),
        PropertyDescriptor::Value(
            Napi::String::New(env, "readwriteValue"), trueValue, napi_writable),
        PropertyDescriptor::Value(Napi::String::New(env, "enumerableValue"),
                                  trueValue,
                                  napi_enumerable),
        PropertyDescriptor::Value(Napi::String::New(env, "configurableValue"),
                                  trueValue,
                                  napi_configurable),
        PropertyDescriptor::Function(
            env, obj, Napi::String::New(env, "function"), TestFunction),
        PropertyDescriptor::Function(
            env,
            obj,
            Napi::String::New(env, "functionWithUserData"),
            TestFunctionWithUserData,
            napi_property_attributes::napi_default,
            reinterpret_cast<void*>(holder)),
    });
  }
}

void DefineValueProperty(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Name name = info[1].As<Name>();
  Value value = info[2];

  obj.DefineProperty(PropertyDescriptor::Value(name, value));
}

Value CreateObjectUsingMagic(const CallbackInfo& info) {
  Env env = info.Env();
  Object obj = Object::New(env);
  obj["cp_false"] = false;
  obj["cp_true"] = true;
  obj[std::string("s_true")] = true;
  obj[std::string("s_false")] = false;
  obj["0"] = 0;
  obj[(uint32_t)42] = 120;
  obj["0.0f"] = 0.0f;
  obj["0.0"] = 0.0;
  obj["-1"] = -1;
  obj["foo2"] = u"foo";
  obj[std::string("foo4")] = NAPI_WIDE_TEXT("foo");
  obj[std::string("foo5")] = "foo";
  obj[std::string("foo6")] = std::u16string(NAPI_WIDE_TEXT("foo"));
  obj[std::string("foo7")] = std::string("foo");
  obj[std::string("circular")] = obj;
  obj["circular2"] = obj;
  return obj;
}

#ifdef NAPI_CPP_EXCEPTIONS
Value Sum(const CallbackInfo& info) {
  Object object = info[0].As<Object>();
  int64_t sum = 0;

  for (const auto& e : object) {
    sum += static_cast<Value>(e.second).As<Number>().Int64Value();
  }

  return Number::New(info.Env(), sum);
}

void Increment(const CallbackInfo& info) {
  Env env = info.Env();
  Object object = info[0].As<Object>();

  for (auto e : object) {
    int64_t value = static_cast<Value>(e.second).As<Number>().Int64Value();
    ++value;
    e.second = Napi::Number::New(env, value);
  }
}
#endif  // NAPI_CPP_EXCEPTIONS

Value InstanceOf(const CallbackInfo& info) {
  Object obj = info[0].As<Object>();
  Function constructor = info[1].As<Function>();
  return Boolean::New(info.Env(), MaybeUnwrap(obj.InstanceOf(constructor)));
}

Object InitObject(Env env) {
  Object exports = Object::New(env);

  exports["emptyConstructor"] = Function::New(env, EmptyConstructor);
  exports["constructorFromObject"] = Function::New(env, ConstructorFromObject);

  exports["GetPropertyNames"] = Function::New(env, GetPropertyNames);
  exports["defineProperties"] = Function::New(env, DefineProperties);
  exports["defineValueProperty"] = Function::New(env, DefineValueProperty);

  exports["getPropertyWithUint32"] = Function::New(env, GetPropertyWithUint32);
  exports["getPropertyWithNapiValue"] =
      Function::New(env, GetPropertyWithNapiValue);
  exports["getPropertyWithNapiWrapperValue"] =
      Function::New(env, GetPropertyWithNapiWrapperValue);
  exports["getPropertyWithCStyleString"] =
      Function::New(env, GetPropertyWithCStyleString);
  exports["getPropertyWithCppStyleString"] =
      Function::New(env, GetPropertyWithCppStyleString);

  exports["setPropertyWithUint32"] = Function::New(env, SetPropertyWithUint32);
  exports["setPropertyWithNapiValue"] =
      Function::New(env, SetPropertyWithNapiValue);
  exports["setPropertyWithNapiWrapperValue"] =
      Function::New(env, SetPropertyWithNapiWrapperValue);
  exports["setPropertyWithCStyleString"] =
      Function::New(env, SetPropertyWithCStyleString);
  exports["setPropertyWithCppStyleString"] =
      Function::New(env, SetPropertyWithCppStyleString);

  exports["deletePropertyWithUint32"] =
      Function::New(env, DeletePropertyWithUint32);
  exports["deletePropertyWithNapiValue"] =
      Function::New(env, DeletePropertyWithNapiValue);
  exports["deletePropertyWithNapiWrapperValue"] =
      Function::New(env, DeletePropertyWithNapiWrapperValue);
  exports["deletePropertyWithCStyleString"] =
      Function::New(env, DeletePropertyWithCStyleString);
  exports["deletePropertyWithCppStyleString"] =
      Function::New(env, DeletePropertyWithCppStyleString);

  exports["hasOwnPropertyWithNapiValue"] =
      Function::New(env, HasOwnPropertyWithNapiValue);
  exports["hasOwnPropertyWithNapiWrapperValue"] =
      Function::New(env, HasOwnPropertyWithNapiWrapperValue);
  exports["hasOwnPropertyWithCStyleString"] =
      Function::New(env, HasOwnPropertyWithCStyleString);
  exports["hasOwnPropertyWithCppStyleString"] =
      Function::New(env, HasOwnPropertyWithCppStyleString);

  exports["hasPropertyWithUint32"] = Function::New(env, HasPropertyWithUint32);
  exports["hasPropertyWithNapiValue"] =
      Function::New(env, HasPropertyWithNapiValue);
  exports["hasPropertyWithNapiWrapperValue"] =
      Function::New(env, HasPropertyWithNapiWrapperValue);
  exports["hasPropertyWithCStyleString"] =
      Function::New(env, HasPropertyWithCStyleString);
  exports["hasPropertyWithCppStyleString"] =
      Function::New(env, HasPropertyWithCppStyleString);

  exports["createObjectUsingMagic"] =
      Function::New(env, CreateObjectUsingMagic);
#ifdef NAPI_CPP_EXCEPTIONS
  exports["sum"] = Function::New(env, Sum);
  exports["increment"] = Function::New(env, Increment);
#endif  // NAPI_CPP_EXCEPTIONS

  exports["addFinalizer"] = Function::New(env, AddFinalizer);
  exports["addFinalizerWithHint"] = Function::New(env, AddFinalizerWithHint);

  exports["instanceOf"] = Function::New(env, InstanceOf);

  exports["subscriptGetWithCStyleString"] =
      Function::New(env, SubscriptGetWithCStyleString);
  exports["subscriptGetWithCppStyleString"] =
      Function::New(env, SubscriptGetWithCppStyleString);
  exports["subscriptGetAtIndex"] = Function::New(env, SubscriptGetAtIndex);
  exports["subscriptSetWithCStyleString"] =
      Function::New(env, SubscriptSetWithCStyleString);
  exports["subscriptSetWithCppStyleString"] =
      Function::New(env, SubscriptSetWithCppStyleString);
  exports["subscriptSetAtIndex"] = Function::New(env, SubscriptSetAtIndex);

  return exports;
}
