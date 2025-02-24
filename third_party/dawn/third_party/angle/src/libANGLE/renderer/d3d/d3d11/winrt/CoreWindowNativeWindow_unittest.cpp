//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

// WINAPI_FAMILY is required to be defined as WINAPI_FAMILY_PC_APP
// to ensure that the proper defines are set when including additional
// headers which rely on Windows Store specific configuration.
// This would normally be defined already but this unittest exe is compiled
// as a desktop application which results in WINAPI_FAMILY being
// set to WINAPI_FAMILY_DESKTOP_APP
#undef WINAPI_FAMILY
#define WINAPI_FAMILY WINAPI_FAMILY_PC_APP
#include <angle_windowsstore.h>
#include "libANGLE/renderer/d3d/d3d11/NativeWindow.h"

using namespace rx;
using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::System;

namespace
{

// Mock Generic interfaces which are NOT ICoreWindow or
// IPropertySet
MIDL_INTERFACE("A4D67D0A-0E7F-4DF7-918B-7A1395413AF2")
IAmInspectable : public IInspectable{};

class GenericIInspectable
    : public RuntimeClass<RuntimeClassFlags<WinRtClassicComMix>, IAmInspectable>
{};

MIDL_INTERFACE("3CBCFE7A-E000-4094-B2D2-B7C9A4D67A2C")
IAmUnknown : public IUnknown{};

class GenericIUnknown : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IAmUnknown>
{};

// Mock ICoreWindow
class MockCoreWindow : public ABI::Windows::UI::Core::ICoreWindow
{
  public:
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject)
    {
        *ppvObject = nullptr;

        if (IsEqualIID(IID_IUnknown, riid))
        {
            *ppvObject = static_cast<IUnknown *>(this);
            return S_OK;
        }

        if (IsEqualIID(IID_IInspectable, riid))
        {
            *ppvObject = static_cast<IInspectable *>(this);
            return S_OK;
        }

        if (IsEqualIID(IID_ICoreWindow, riid))
        {
            *ppvObject = static_cast<ABI::Windows::UI::Core::ICoreWindow *>(this);
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    STDMETHOD_(ULONG, AddRef)() { return 1; }
    STDMETHOD_(ULONG, Release)() { return 1; }

    // IInspectable
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE, GetIids, HRESULT(ULONG *, IID **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, GetRuntimeClassName, HRESULT(HSTRING *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, GetTrustLevel, HRESULT(TrustLevel *));

    // ICoreWindow
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               get_AutomationHostProvider,
                               HRESULT(IInspectable **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Bounds, HRESULT(Rect *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_CustomProperties, HRESULT(IPropertySet **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Dispatcher, HRESULT(ICoreDispatcher **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               get_FlowDirection,
                               HRESULT(CoreWindowFlowDirection *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               put_FlowDirection,
                               HRESULT(CoreWindowFlowDirection));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_IsInputEnabled, HRESULT(boolean *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_IsInputEnabled, HRESULT(boolean));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_PointerCursor, HRESULT(ICoreCursor **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_PointerCursor, HRESULT(ICoreCursor *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_PointerPosition, HRESULT(Point *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Visible, HRESULT(boolean *));
    MOCK_METHOD0_WITH_CALLTYPE(STDMETHODCALLTYPE, Activate, HRESULT());
    MOCK_METHOD0_WITH_CALLTYPE(STDMETHODCALLTYPE, Close, HRESULT());
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               GetAsyncKeyState,
                               HRESULT(ABI::Windows::System::VirtualKey,
                                       ABI::Windows::UI::Core::CoreVirtualKeyStates *));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               GetKeyState,
                               HRESULT(ABI::Windows::System::VirtualKey,
                                       ABI::Windows::UI::Core::CoreVirtualKeyStates *));
    MOCK_METHOD0_WITH_CALLTYPE(STDMETHODCALLTYPE, ReleasePointerCapture, HRESULT());
    MOCK_METHOD0_WITH_CALLTYPE(STDMETHODCALLTYPE, SetPointerCapture, HRESULT());
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_Activated,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CWindowActivatedEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_Activated,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_AutomationProviderRequested,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CAutomationProviderRequestedEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_AutomationProviderRequested,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_CharacterReceived,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CCharacterReceivedEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_CharacterReceived,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_Closed,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CCoreWindowEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_Closed, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_InputEnabled,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CInputEnabledEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_InputEnabled,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_KeyDown,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CKeyEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_KeyDown, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_KeyUp,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CKeyEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_KeyUp, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_PointerCaptureLost,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerCaptureLost,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_PointerEntered,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerEntered,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_PointerExited,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerExited,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_PointerMoved,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerMoved,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_PointerPressed,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerPressed,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_PointerReleased,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerReleased,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_TouchHitTesting,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CTouchHitTestingEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_TouchHitTesting,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_PointerWheelChanged,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerWheelChanged,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_SizeChanged,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CWindowSizeChangedEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_SizeChanged,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_VisibilityChanged,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CVisibilityChangedEventArgs
                *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_VisibilityChanged,
                               HRESULT(EventRegistrationToken));
};

HRESULT CreatePropertyMap(IMap<HSTRING, IInspectable *> **propertyMap)
{
    HRESULT result = S_OK;
    ComPtr<IPropertySet> propertySet;
    ComPtr<IActivationFactory> propertySetFactory;
    result = GetActivationFactory(
        HStringReference(RuntimeClass_Windows_Foundation_Collections_PropertySet).Get(),
        &propertySetFactory);
    EXPECT_HRESULT_SUCCEEDED(result);

    result = propertySetFactory->ActivateInstance(&propertySet);
    EXPECT_HRESULT_SUCCEEDED(result);

    result = propertySet.CopyTo(propertyMap);
    EXPECT_HRESULT_SUCCEEDED(result);

    return result;
}

HRESULT CreatePropertyValueStatics(IPropertyValueStatics **propertyStatics)
{
    ComPtr<IPropertyValueStatics> propertyValueStatics;
    HRESULT result =
        GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get(),
                             &propertyValueStatics);
    EXPECT_HRESULT_SUCCEEDED(result);

    result = propertyValueStatics.CopyTo(propertyStatics);
    EXPECT_HRESULT_SUCCEEDED(result);

    return result;
}

HRESULT SetInspectablePropertyValue(const ComPtr<IMap<HSTRING, IInspectable *>> &propertyMap,
                                    const wchar_t *propertyName,
                                    IInspectable *inspectable)
{
    boolean propertyReplaced = false;
    return propertyMap->Insert(HStringReference(propertyName).Get(), inspectable,
                               &propertyReplaced);
}

void expectNativeWindowInitCalls(MockCoreWindow &coreWindow, bool expectBounds)
{
    if (expectBounds)
    {
        EXPECT_CALL(coreWindow, get_Bounds(testing::_)).Times(1);
    }

    EXPECT_CALL(coreWindow, add_SizeChanged(testing::_, testing::_)).Times(1);
    EXPECT_CALL(coreWindow, remove_SizeChanged(testing::_)).Times(1);
}

TEST(NativeWindowTest, NativeWindowNull)
{
    NativeWindow nativeWindow(nullptr);
    EXPECT_FALSE(nativeWindow.initialize());
}

TEST(NativeWindowTest, NativeWindowBadInspectable)
{
    ComPtr<IAmInspectable> notCoreWindow = Make<GenericIInspectable>();
    NativeWindow nativeWindow(notCoreWindow.Get());
    EXPECT_FALSE(nativeWindow.initialize());
}

TEST(NativeWindowTest, NativeWindowNotInspectable)
{
    ComPtr<IAmUnknown> notIInspectable = Make<GenericIUnknown>();
    NativeWindow nativeWindow((IInspectable *)notIInspectable.Get());
    EXPECT_FALSE(nativeWindow.initialize());
}

TEST(NativeWindowTest, NativeWindowValidCoreWindow)
{
    MockCoreWindow mockCoreWindow;
    expectNativeWindowInitCalls(mockCoreWindow, true);
    NativeWindow nativeWindow(&mockCoreWindow);
    EXPECT_TRUE(nativeWindow.initialize());
}

TEST(NativeWindowTest, NativeWindowValidCoreWindowInPropertySet)
{
    // COM is required to be initialized for creation of the property set
    EXPECT_HRESULT_SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    {
        MockCoreWindow mockCoreWindow;
        ComPtr<IMap<HSTRING, IInspectable *>> propertySet;

        // Add the CoreWindow to the property set
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyMap(&propertySet));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLNativeWindowTypeProperty, &mockCoreWindow));

        expectNativeWindowInitCalls(mockCoreWindow, true);
        NativeWindow nativeWindow(propertySet.Get());
        EXPECT_TRUE(nativeWindow.initialize());
    }
    CoUninitialize();
}

TEST(NativeWindowTest, NativeWindowMissingCoreWindowInPropertySet)
{
    // COM is required to be initialized for creation of the property set
    EXPECT_HRESULT_SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    {
        MockCoreWindow mockCoreWindow;
        ComPtr<IMap<HSTRING, IInspectable *>> propertySet;

        EXPECT_HRESULT_SUCCEEDED(CreatePropertyMap(&propertySet));

        NativeWindow nativeWindow(propertySet.Get());
        EXPECT_FALSE(nativeWindow.initialize());
    }
    CoUninitialize();
}

// Tests that the scale property works as expected in a property set with a SwapChainPanel
class CoreWindowScaleTest : public testing::TestWithParam<std::pair<float, bool>>
{};

TEST_P(CoreWindowScaleTest, ValidateScale)
{
    float scale         = GetParam().first;
    bool expectedResult = GetParam().second;

    // COM is required to be initialized for creation of the property set
    EXPECT_HRESULT_SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    {
        MockCoreWindow mockCoreWindow;
        ComPtr<IMap<HSTRING, IInspectable *>> propertySet;
        ComPtr<IPropertyValueStatics> propertyValueStatics;
        ComPtr<IPropertyValue> singleValue;

        // Create a simple property set
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyMap(&propertySet));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLNativeWindowTypeProperty,
                                        reinterpret_cast<IInspectable *>(&mockCoreWindow)));

        // Add a valid scale factor to the property set
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyValueStatics(propertyValueStatics.GetAddressOf()));
        propertyValueStatics->CreateSingle(
            scale, reinterpret_cast<IInspectable **>(singleValue.GetAddressOf()));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLRenderResolutionScaleProperty,
                                        reinterpret_cast<IInspectable *>(singleValue.Get())));

        // Check native window init status and calls to the mock swapchainpanel
        NativeWindow nativeWindow(propertySet.Get());
        if (expectedResult)
        {
            expectNativeWindowInitCalls(mockCoreWindow, true);
        }

        EXPECT_EQ(nativeWindow.initialize(), expectedResult);
    }
    CoUninitialize();
}

typedef std::pair<float, bool> scaleValidPair;
static const scaleValidPair scales[] = {scaleValidPair(1.0f, true), scaleValidPair(0.5f, true),
                                        scaleValidPair(0.0f, false), scaleValidPair(0.01f, true),
                                        scaleValidPair(2.00f, true)};

INSTANTIATE_TEST_SUITE_P(NativeWindowTest, CoreWindowScaleTest, testing::ValuesIn(scales));

// Tests that the size property works as expected in a property set with a SwapChainPanel
class CoreWindowSizeTest : public testing::TestWithParam<std::tuple<float, float, bool>>
{};

TEST_P(CoreWindowSizeTest, ValidateSize)
{
    Size renderSize     = {std::get<0>(GetParam()), std::get<1>(GetParam())};
    bool expectedResult = std::get<2>(GetParam());

    // COM is required to be initialized for creation of the property set
    EXPECT_HRESULT_SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    {
        MockCoreWindow mockCoreWindow;
        ComPtr<IMap<HSTRING, IInspectable *>> propertySet;
        ComPtr<IPropertyValueStatics> propertyValueStatics;
        ComPtr<IPropertyValue> sizeValue;

        // Create a simple property set
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyMap(&propertySet));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLNativeWindowTypeProperty,
                                        reinterpret_cast<IInspectable *>(&mockCoreWindow)));

        // Add a valid size to the property set
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyValueStatics(propertyValueStatics.GetAddressOf()));
        propertyValueStatics->CreateSize(
            renderSize, reinterpret_cast<IInspectable **>(sizeValue.GetAddressOf()));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLRenderSurfaceSizeProperty,
                                        reinterpret_cast<IInspectable *>(sizeValue.Get())));

        // Check native window init status and calls to the mock swapchainpanel
        NativeWindow nativeWindow(propertySet.Get());
        if (expectedResult)
        {
            expectNativeWindowInitCalls(mockCoreWindow, false);
        }

        EXPECT_EQ(nativeWindow.initialize(), expectedResult);
    }
    CoUninitialize();
}

typedef std::tuple<float, float, bool> sizeValidPair;
static const sizeValidPair sizes[] = {sizeValidPair(800, 480, true), sizeValidPair(0, 480, false),
                                      sizeValidPair(800, 0, false), sizeValidPair(0, 0, false)};

INSTANTIATE_TEST_SUITE_P(NativeWindowTest, CoreWindowSizeTest, testing::ValuesIn(sizes));

}  // namespace
