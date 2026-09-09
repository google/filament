// RUN: %dxc -T vs_6_0 -E main %s | FileCheck %s

// CHECK: %[[IN:[^ ]+]] = call float @dx.op.loadInput.f32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %[[IN]])

template<typename T> struct Wrapper;
float get(float x) { return x;}
float get(Wrapper<float> o);

template<typename T> struct Wrapper2;
float get(Wrapper2<float> o);

template<typename T>
struct Wrapper {
    T value;
    void set(float x) {
      // Incomplete target in CanConvert triggered during check whether explicit
      // cast would be allowed for converting to overload candidate 'float
      // get(Wrapper<float>)'.  CanConvert would check spat potential when
      // explicit and source is a basic numeric type.  This would cause crash in
      // IsHLSLCopyableAnnotatableRecord from the incomplete type.
      // Here, we can complete the type: Wrapper<float>, but not Wrapper2<float>
      value = get(x);
    }
};

float get(Wrapper<float> o) { return o.value; }

template<typename T>
struct Wrapper2 : Wrapper<T> {
};

float get(Wrapper2<float> o) { return o.value; }

float main(float x : IN) : OUT {
    Wrapper2<float> w;
    w.set(x);
    return get(w);
}
