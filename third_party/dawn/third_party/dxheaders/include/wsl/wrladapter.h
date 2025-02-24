// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "winadapter.h"

// defined by winadapter.h and needed by some windows headers, but conflicts
// with some libc++ implementation headers
#ifdef __in
#undef __in
#endif
#ifdef __out
#undef __out
#endif

#include <type_traits>
#include <atomic>
#include <memory>
#include <new>
#include <climits>
#include <cassert>

namespace Microsoft
{
namespace WRL
{
    namespace Details
    {
        struct BoolStruct { int Member; };
        typedef int BoolStruct::* BoolType;

        template <typename T> // T should be the ComPtr<T> or a derived type of it, not just the interface
        class ComPtrRefBase
        {
        public:
            typedef typename T::InterfaceType InterfaceType;

            operator IUnknown**() const throw()
            {
                static_assert(__is_base_of(IUnknown, InterfaceType), "Invalid cast: InterfaceType does not derive from IUnknown");
                return reinterpret_cast<IUnknown**>(ptr_->ReleaseAndGetAddressOf());
            }

        protected:
            T* ptr_;
        };

        template <typename T>
        class ComPtrRef : public Details::ComPtrRefBase<T> // T should be the ComPtr<T> or a derived type of it, not just the interface
        {
            using Super = Details::ComPtrRefBase<T>;
            using InterfaceType = typename Super::InterfaceType;
        public:
            ComPtrRef(_In_opt_ T* ptr) throw()
            {
                this->ptr_ = ptr;
            }

            // Conversion operators
            operator void**() const throw()
            {
                return reinterpret_cast<void**>(this->ptr_->ReleaseAndGetAddressOf());
            }

            // This is our operator ComPtr<U> (or the latest derived class from ComPtr (e.g. WeakRef))
            operator T*() throw()
            {
                *this->ptr_ = nullptr;
                return this->ptr_;
            }

            // We define operator InterfaceType**() here instead of on ComPtrRefBase<T>, since
            // if InterfaceType is IUnknown or IInspectable, having it on the base will collide.
            operator InterfaceType**() throw()
            {
                return this->ptr_->ReleaseAndGetAddressOf();
            }

            // This is used for IID_PPV_ARGS in order to do __uuidof(**(ppType)).
            // It does not need to clear  ptr_ at this point, it is done at IID_PPV_ARGS_Helper(ComPtrRef&) later in this file.
            InterfaceType* operator *() throw()
            {
                return this->ptr_->Get();
            }

            // Explicit functions
            InterfaceType* const * GetAddressOf() const throw()
            {
                return this->ptr_->GetAddressOf();
            }

            InterfaceType** ReleaseAndGetAddressOf() throw()
            {
                return this->ptr_->ReleaseAndGetAddressOf();
            }
        };
    }

    template <typename T>
    class ComPtr
    {
    public:
        typedef T InterfaceType;

    protected:
        InterfaceType *ptr_;
        template<class U> friend class ComPtr;

        void InternalAddRef() const throw()
        {
            if (ptr_ != nullptr)
            {
                ptr_->AddRef();
            }
        }

        unsigned long InternalRelease() throw()
        {
            unsigned long ref = 0;
            T* temp = ptr_;

            if (temp != nullptr)
            {
                ptr_ = nullptr;
                ref = temp->Release();
            }

            return ref;
        }

    public:
        ComPtr() throw() : ptr_(nullptr)
        {
        }

        ComPtr(decltype(nullptr)) throw() : ptr_(nullptr)
        {
        }

        template<class U>
        ComPtr(_In_opt_ U *other) throw() : ptr_(other)
        {
            InternalAddRef();
        }

        ComPtr(const ComPtr& other) throw() : ptr_(other.ptr_)
        {
            InternalAddRef();
        }

        // copy constructor that allows to instantiate class when U* is convertible to T*
        template<class U>
        ComPtr(const ComPtr<U> &other, typename std::enable_if<std::is_convertible<U*, T*>::value, void *>::type * = 0) throw() :
            ptr_(other.ptr_)
        {
            InternalAddRef();
        }

        ComPtr(_Inout_ ComPtr &&other) throw() : ptr_(nullptr)
        {
            if (this != reinterpret_cast<ComPtr*>(&reinterpret_cast<unsigned char&>(other)))
            {
                Swap(other);
            }
        }

        // Move constructor that allows instantiation of a class when U* is convertible to T*
        template<class U>
        ComPtr(_Inout_ ComPtr<U>&& other, typename std::enable_if<std::is_convertible<U*, T*>::value, void *>::type * = 0) throw() :
            ptr_(other.ptr_)
        {
            other.ptr_ = nullptr;
        }

        ~ComPtr() throw()
        {
            InternalRelease();
        }

        ComPtr& operator=(decltype(nullptr)) throw()
        {
            InternalRelease();
            return *this;
        }

        ComPtr& operator=(_In_opt_ T *other) throw()
        {
            if (ptr_ != other)
            {
                ComPtr(other).Swap(*this);
            }
            return *this;
        }

        template <typename U>
        ComPtr& operator=(_In_opt_ U *other) throw()
        {
            ComPtr(other).Swap(*this);
            return *this;
        }

        ComPtr& operator=(const ComPtr &other) throw()
        {
            if (ptr_ != other.ptr_)
            {
                ComPtr(other).Swap(*this);
            }
            return *this;
        }

        template<class U>
        ComPtr& operator=(const ComPtr<U>& other) throw()
        {
            ComPtr(other).Swap(*this);
            return *this;
        }

        ComPtr& operator=(_Inout_ ComPtr &&other) throw()
        {
            ComPtr(static_cast<ComPtr&&>(other)).Swap(*this);
            return *this;
        }

        template<class U>
        ComPtr& operator=(_Inout_ ComPtr<U>&& other) throw()
        {
            ComPtr(static_cast<ComPtr<U>&&>(other)).Swap(*this);
            return *this;
        }

        void Swap(_Inout_ ComPtr&& r) throw()
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        void Swap(_Inout_ ComPtr& r) throw()
        {
            T* tmp = ptr_;
            ptr_ = r.ptr_;
            r.ptr_ = tmp;
        }

        operator Details::BoolType() const throw()
        {
            return Get() != nullptr ? &Details::BoolStruct::Member : nullptr;
        }

        T* Get() const throw()
        {
            return ptr_;
        }

        InterfaceType* operator->() const throw()
        {
            return ptr_;
        }

        Details::ComPtrRef<ComPtr<T>> operator&() throw()
        {
            return Details::ComPtrRef<ComPtr<T>>(this);
        }

        const Details::ComPtrRef<const ComPtr<T>> operator&() const throw()
        {
            return Details::ComPtrRef<const ComPtr<T>>(this);
        }

        T* const* GetAddressOf() const throw()
        {
            return &ptr_;
        }

        T** GetAddressOf() throw()
        {
            return &ptr_;
        }

        T** ReleaseAndGetAddressOf() throw()
        {
            InternalRelease();
            return &ptr_;
        }

        T* Detach() throw()
        {
            T* ptr = ptr_;
            ptr_ = nullptr;
            return ptr;
        }

        void Attach(_In_opt_ InterfaceType* other) throw()
        {
            if (ptr_ != nullptr)
            {
                auto ref = ptr_->Release();
                // DBG_UNREFERENCED_LOCAL_VARIABLE(ref);
                // Attaching to the same object only works if duplicate references are being coalesced. Otherwise
                // re-attaching will cause the pointer to be released and may cause a crash on a subsequent dereference.
                assert(ref != 0 || ptr_ != other);
            }

            ptr_ = other;
        }

        unsigned long Reset()
        {
            return InternalRelease();
        }

        // Previously, unsafe behavior could be triggered when 'this' is ComPtr<IInspectable> or ComPtr<IUnknown> and CopyTo is used to copy to another type U. 
        // The user will use operator& to convert the destination into a ComPtrRef, which can then implicit cast to IInspectable** and IUnknown**. 
        // If this overload of CopyTo is not present, it will implicitly cast to IInspectable or IUnknown and match CopyTo(InterfaceType**) instead.
        // A valid polymoprhic downcast requires run-time type checking via QueryInterface, so CopyTo(InterfaceType**) will break type safety.
        // This overload matches ComPtrRef before the implicit cast takes place, preventing the unsafe downcast.
        template <typename U>
        HRESULT CopyTo(Details::ComPtrRef<ComPtr<U>> ptr, typename std::enable_if<
            (std::is_same<T, IUnknown>::value)
            && !std::is_same<U*, T*>::value, void *>::type * = 0) const throw()
        {
            return ptr_->QueryInterface(uuidof<U>(), ptr);
        }

        HRESULT CopyTo(_Outptr_result_maybenull_ InterfaceType** ptr) const throw()
        {
            InternalAddRef();
            *ptr = ptr_;
            return S_OK;
        }

        HRESULT CopyTo(REFIID riid, _Outptr_result_nullonfailure_ void** ptr) const throw()
        {
            return ptr_->QueryInterface(riid, ptr);
        }

        template<typename U>
        HRESULT CopyTo(_Outptr_result_nullonfailure_ U** ptr) const throw()
        {
            return ptr_->QueryInterface(uuidof<U>(), reinterpret_cast<void**>(ptr));
        }

        // query for U interface
        template<typename U>
        HRESULT As(_Inout_ Details::ComPtrRef<ComPtr<U>> p) const throw()
        {
            return ptr_->QueryInterface(uuidof<U>(), p);
        }

        // query for U interface
        template<typename U>
        HRESULT As(_Out_ ComPtr<U>* p) const throw()
        {
            return ptr_->QueryInterface(uuidof<U>(), reinterpret_cast<void**>(p->ReleaseAndGetAddressOf()));
        }

        // query for riid interface and return as IUnknown
        HRESULT AsIID(REFIID riid, _Out_ ComPtr<IUnknown>* p) const throw()
        {
            return ptr_->QueryInterface(riid, reinterpret_cast<void**>(p->ReleaseAndGetAddressOf()));
        }

    };    // ComPtr


    namespace Details
    {
        // Empty struct used as default template parameter
        class Nil
        {
        };

        // Empty struct used for validating template parameter types in Implements
        struct ImplementsBase
        {
        };

        class RuntimeClassBase
        {
        protected:
            template<typename T>
            static HRESULT AsIID(_In_ T* implements, REFIID riid, _Outptr_result_nullonfailure_ void **ppvObject) noexcept
            {
                *ppvObject = nullptr;
                bool isRefDelegated = false;
                // Prefer InlineIsEqualGUID over other forms since it's better perf on 4-byte aligned data, which is almost always the case.
                if (InlineIsEqualGUID(riid, uuidof<IUnknown>()))
                {
                    *ppvObject = implements->CastToUnknown();
                    static_cast<IUnknown*>(*ppvObject)->AddRef();
                    return S_OK;
                }

                HRESULT hr = implements->CanCastTo(riid, ppvObject, &isRefDelegated);
                if (SUCCEEDED(hr) && !isRefDelegated)
                {
                    static_cast<IUnknown*>(*ppvObject)->AddRef();
                }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 6102) // '*ppvObject' is used but may not be initialized
#endif
                _Analysis_assume_(SUCCEEDED(hr) || (*ppvObject == nullptr));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
                return hr;
            }

        public:
            HRESULT RuntimeClassInitialize() noexcept
            {
                return S_OK;
            }
        };

        // Interface traits provides casting and filling iids methods helpers
        template<typename I0>
        struct InterfaceTraits
        {
            typedef I0 Base;

            template<typename T>
            static Base* CastToBase(_In_ T* ptr) noexcept
            {
                return static_cast<Base*>(ptr);
            }

            template<typename T>
            static IUnknown* CastToUnknown(_In_ T* ptr) noexcept
            {
                return static_cast<IUnknown*>(static_cast<Base*>(ptr));
            }

            template <typename T>
            _Success_(return == true)
                static bool CanCastTo(_In_ T* ptr, REFIID riid, _Outptr_ void **ppv) noexcept
            {
                // Prefer InlineIsEqualGUID over other forms since it's better perf on 4-byte aligned data, which is almost always the case.
                if (InlineIsEqualGUID(riid, uuidof<Base>()))
                {
                    *ppv = static_cast<Base*>(ptr);
                    return true;
                }

                return false;
            }
        };

        // Specialization for Nil parameter
        template<>
        struct InterfaceTraits<Nil>
        {
            typedef Nil Base;

            template <typename T>
            _Success_(return == true)
                static bool CanCastTo(_In_ T*, REFIID, _Outptr_ void **) noexcept
            {
                return false;
            }
        };

        // ChainInterfaces - template allows specifying a derived COM interface along with its class hierarchy to allow QI for the base interfaces
        template <typename I0, typename I1, typename I2 = Nil, typename I3 = Nil,
            typename I4 = Nil, typename I5 = Nil, typename I6 = Nil,
            typename I7 = Nil, typename I8 = Nil, typename I9 = Nil>
            struct ChainInterfaces : I0
        {
        protected:
            HRESULT CanCastTo(REFIID riid, _Outptr_ void **ppv) throw()
            {
                typename InterfaceTraits<I0>::Base* ptr = InterfaceTraits<I0>::CastToBase(this);

                return (InterfaceTraits<I0>::CanCastTo(this, riid, ppv) ||
                    InterfaceTraits<I1>::CanCastTo(ptr, riid, ppv) ||
                    InterfaceTraits<I2>::CanCastTo(ptr, riid, ppv) ||
                    InterfaceTraits<I3>::CanCastTo(ptr, riid, ppv) ||
                    InterfaceTraits<I4>::CanCastTo(ptr, riid, ppv) ||
                    InterfaceTraits<I5>::CanCastTo(ptr, riid, ppv) ||
                    InterfaceTraits<I6>::CanCastTo(ptr, riid, ppv) ||
                    InterfaceTraits<I7>::CanCastTo(ptr, riid, ppv) ||
                    InterfaceTraits<I8>::CanCastTo(ptr, riid, ppv) ||
                    InterfaceTraits<I9>::CanCastTo(ptr, riid, ppv)) ? S_OK : E_NOINTERFACE;
            }

            IUnknown* CastToUnknown() throw()
            {
                return InterfaceTraits<I0>::CastToUnknown(this);
            }
        };

        // Helper template used by Implements. This template traverses a list of interfaces and adds them as base class and information
        // to enable QI.
        template <typename ...TInterfaces>
        struct ImplementsHelper;

        template <typename T>
        struct ImplementsMarker
        {};

        template <typename I0, bool isImplements>
        struct MarkImplements;

        template <typename I0>
        struct MarkImplements<I0, false>
        {
            typedef I0 Type;
        };

        template <typename I0>
        struct MarkImplements<I0, true>
        {
            typedef ImplementsMarker<I0> Type;
        };

        // AdjustImplements pre-processes the type list for more efficient builds.
        template <typename ...Bases>
        struct AdjustImplements;

        template <typename I0, typename ...Bases>
        struct AdjustImplements<I0, Bases...>
        {
            typedef ImplementsHelper<typename MarkImplements<I0, std::is_base_of<ImplementsBase, I0>::value>::Type, Bases...> Type;
        };

        // Use AdjustImplements to remove instances of "Nil" from the type list.
        template <typename ...Bases>
        struct AdjustImplements<Nil, Bases...>
        {
            typedef typename AdjustImplements<Bases...>::Type Type;
        };

        template <>
        struct AdjustImplements<>
        {
            typedef ImplementsHelper<> Type;
        };

        // Specialization handles unadorned interfaces
        template <typename I0, typename ...TInterfaces>
        struct ImplementsHelper<I0, TInterfaces...> :
            I0,
            AdjustImplements<TInterfaces...>::Type
        {
            template <typename ...> friend struct ImplementsHelper;
            friend class RuntimeClassBase;

        protected:

            HRESULT CanCastTo(REFIID riid, _Outptr_ void **ppv, bool *pRefDelegated = nullptr) noexcept
            {
                // Prefer InlineIsEqualGUID over other forms since it's better perf on 4-byte aligned data, which is almost always the case.
                if (InlineIsEqualGUID(riid, uuidof<I0>()))
                {
                    *ppv = reinterpret_cast<I0*>(reinterpret_cast<void*>(this));
                    return S_OK;
                }
                return AdjustImplements<TInterfaces...>::Type::CanCastTo(riid, ppv, pRefDelegated);
            }

            IUnknown* CastToUnknown() noexcept
            {
                return reinterpret_cast<I0*>(reinterpret_cast<void*>(this));
            }
        };


        // Selector is used to "tag" base interfaces to be used in casting, since a runtime class may indirectly derive from 
        // the same interface or Implements<> template multiple times
        template <typename base, typename disciminator>
        struct  Selector : public base
        {
        };

        // Specialization handles types that derive from ImplementsHelper (e.g. nested Implements).
        template <typename I0, typename ...TInterfaces>
        struct ImplementsHelper<ImplementsMarker<I0>, TInterfaces...> :
            Selector<I0, ImplementsHelper<ImplementsMarker<I0>, TInterfaces...>>,
            Selector<typename AdjustImplements<TInterfaces...>::Type, ImplementsHelper<ImplementsMarker<I0>, TInterfaces...>>
        {
            template <typename ...> friend struct ImplementsHelper;
            friend class RuntimeClassBase;

        protected:
            typedef Selector<I0, ImplementsHelper<ImplementsMarker<I0>, TInterfaces...>> CurrentType;
            typedef Selector<typename AdjustImplements<TInterfaces...>::Type, ImplementsHelper<ImplementsMarker<I0>, TInterfaces...>> BaseType;

            HRESULT CanCastTo(REFIID riid, _Outptr_ void **ppv, bool *pRefDelegated = nullptr) noexcept
            {
                HRESULT hr = CurrentType::CanCastTo(riid, ppv);
                if (hr == E_NOINTERFACE)
                {
                    hr = BaseType::CanCastTo(riid, ppv, pRefDelegated);
                }
                return hr;
            }

            IUnknown* CastToUnknown() noexcept
            {
                // First in list wins.
                return CurrentType::CastToUnknown();
            }
        };

        // terminal case specialization.
        template <>
        struct ImplementsHelper<>
        {
            template <typename ...> friend struct ImplementsHelper;
            friend class RuntimeClassBase;

        protected:
            HRESULT CanCastTo(_In_ REFIID /*riid*/, _Outptr_ void ** /*ppv*/, bool * /*pRefDelegated*/ = nullptr) noexcept
            {
                return E_NOINTERFACE;
            }

            // IUnknown* CastToUnknown() noexcept; // not defined for terminal case.
        };

        // Specialization handles chaining interfaces
        template <typename C0, typename C1, typename C2, typename C3, typename C4, typename C5, typename C6, typename C7, typename C8, typename C9, typename ...TInterfaces>
        struct ImplementsHelper<ChainInterfaces<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9>, TInterfaces...> :
            ChainInterfaces<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9>,
            AdjustImplements<TInterfaces...>::Type
        {
            template <typename ...> friend struct ImplementsHelper;
            friend class RuntimeClassBase;

        protected:
            typedef typename AdjustImplements<TInterfaces...>::Type BaseType;

            HRESULT CanCastTo(REFIID riid, _Outptr_ void **ppv, bool *pRefDelegated = nullptr) noexcept
            {
                HRESULT hr = ChainInterfaces<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9>::CanCastTo(riid, ppv);
                if (FAILED(hr))
                {
                    hr = BaseType::CanCastTo(riid, ppv, pRefDelegated);
                }

                return hr;
            }

            IUnknown* CastToUnknown() noexcept
            {
                return ChainInterfaces<C0, C1, C2, C3, C4, C5, C6, C7, C8, C9>::CastToUnknown();
            }
        };

        // Implements - template implementing QI using the information provided through its template parameters
        // Each template parameter has to be one of the following:
        // * COM Interface
        // * A class that implements one or more COM interfaces
        // * ChainInterfaces template
        template <typename I0, typename ...TInterfaces>
        struct Implements :
            AdjustImplements<I0, TInterfaces...>::Type,
            ImplementsBase
        {
        public:
            typedef I0 FirstInterface;
        protected:
            typedef typename AdjustImplements<I0, TInterfaces...>::Type BaseType;
            template <typename ...> friend struct ImplementsHelper;
            friend class RuntimeClassBase;

            HRESULT CanCastTo(REFIID riid, _Outptr_ void **ppv) noexcept
            {
                return BaseType::CanCastTo(riid, ppv);
            }

            IUnknown* CastToUnknown() noexcept
            {
                return BaseType::CastToUnknown();
            }
        };

        // Used on RuntimeClass to protect it from being constructed with new
        class DontUseNewUseMake
        {
        private:
            void* operator new(size_t) noexcept
            {
                assert(false);
                return 0;
            }

        public:
            void* operator new(size_t, _In_ void* placement) noexcept
            {
                return placement;
            }
        };

        template <typename ...TInterfaces>
        class RuntimeClassImpl :
            public AdjustImplements<TInterfaces...>::Type,
            public RuntimeClassBase,
            public DontUseNewUseMake
        {
        public:
            STDMETHOD(QueryInterface)(REFIID riid, _Outptr_result_nullonfailure_ void **ppvObject)
            {
                return Super::AsIID(this, riid, ppvObject);
            }

            STDMETHOD_(ULONG, AddRef)()
            {
                return InternalAddRef();
            }

            STDMETHOD_(ULONG, Release)()
            {
                ULONG ref = InternalRelease();
                if (ref == 0)
                {
                    this->~RuntimeClassImpl();
                    delete[] reinterpret_cast<char*>(this);
                }

                return ref;
            }

        protected:
            using Super = RuntimeClassBase;
            static const LONG c_lProtectDestruction = -(LONG_MAX / 2);

            RuntimeClassImpl() noexcept = default;

            virtual ~RuntimeClassImpl() noexcept
            {
                // Set refcount_ to -(LONG_MAX/2) to protect destruction and
                // also catch mismatched Release in debug builds
                refcount_ = static_cast<ULONG>(c_lProtectDestruction);
            }

            ULONG InternalAddRef() noexcept
            {
                return ++refcount_;
            }

            ULONG InternalRelease() noexcept
            {
                return --refcount_;
            }

            unsigned long GetRefCount() const noexcept
            {
                return refcount_;
            }

            std::atomic<ULONG> refcount_{1};
        };
    }

    template <typename ...TInterfaces>
    class Base : public Details::RuntimeClassImpl<TInterfaces...>
    {
        Base(const Base&) = delete;
        Base& operator=(const Base&) = delete;

    protected:
        HRESULT CustomQueryInterface(REFIID /*riid*/, _Outptr_result_nullonfailure_ void** /*ppvObject*/, _Out_ bool *handled)
        {
            *handled = false;
            return S_OK;
        }

    public:
        Base() throw() = default;
        typedef Base RuntimeClassT;
    };

    // Creates a Nano-COM object wrapped in a smart pointer.
    template <typename T, typename ...TArgs>
    ComPtr<T> Make(TArgs&&... args)
    {
        std::unique_ptr<char[]> buffer(new(std::nothrow) char[sizeof(T)]);
        ComPtr<T> object;

        if (buffer)
        {
            T* ptr = new (buffer.get())T(std::forward<TArgs>(args)...);
            object.Attach(ptr);
            buffer.release();
        }

        return object;
    }

    using Details::ChainInterfaces;
}
}

// Overloaded global function to provide to IID_PPV_ARGS that support Details::ComPtrRef
template<typename T>
void** IID_PPV_ARGS_Helper(Microsoft::WRL::Details::ComPtrRef<T> pp) throw()
{
    return pp;
}
