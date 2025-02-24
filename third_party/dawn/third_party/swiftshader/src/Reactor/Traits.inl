// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef rr_Traits_inl
#define rr_Traits_inl

namespace rr {

// Non-specialized implementation of CToReactorPtr::cast() defaults to
// returning a ConstantPointer for v.
template<typename T, typename ENABLE>
Pointer<Byte> CToReactorPtr<T, ENABLE>::cast(const T *v)
{
	return ConstantPointer(v);
}

// CToReactorPtr specialization for T types that have a CToReactorT<>
// specialization.
template<typename T>
Pointer<CToReactorT<T>>
CToReactorPtr<T, std::enable_if_t<HasReactorType<T>::value>>::cast(const T *v)
{
	return type(v);
}

// CToReactorPtr specialization for void*.
Pointer<Byte> CToReactorPtr<void, void>::cast(const void *v)
{
	return ConstantPointer(v);
}

// CToReactorPtrT specialization for function pointer types.
template<typename T>
Pointer<Byte>
CToReactorPtr<T, std::enable_if_t<std::is_function<T>::value>>::cast(T *v)
{
	return ConstantPointer(v);
}

// CToReactor specialization for pointer types.
template<typename T>
CToReactorPtrT<typename std::remove_pointer<T>::type>
CToReactor<T, std::enable_if_t<std::is_pointer<T>::value>>::cast(T v)
{
	return CToReactorPtr<elem>::cast(v);
}

// CToReactor specialization for enum types.
template<typename T>
CToReactorT<typename std::underlying_type<T>::type>
CToReactor<T, std::enable_if_t<std::is_enum<T>::value>>::cast(T v)
{
	return CToReactor<underlying>::cast(v);
}

}  // namespace rr

#endif  // rr_Traits_inl
