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
#include <windows.ui.xaml.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include "libANGLE/renderer/d3d/d3d11/NativeWindow.h"

using namespace rx;
using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::System;
using namespace ABI::Windows::UI::Xaml::Controls;
using namespace ABI::Windows::UI::Xaml::Data;
using namespace ABI::Windows::UI::Xaml::Media;
using namespace ABI::Windows::UI::Xaml::Input;
using namespace ABI::Windows::UI::Xaml;

namespace
{

// Mock ISwapChainPanel
class MockSwapChainPanel : public ISwapChainPanel,
                           IFrameworkElement,
                           IUIElement,
                           ISwapChainPanelNative
{
  public:
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject)
    {
        *ppvObject = nullptr;

        if (IsEqualIID(IID_IUnknown, riid))
        {
            *ppvObject = reinterpret_cast<IUnknown *>(this);
            return S_OK;
        }

        if (IsEqualIID(IID_IInspectable, riid))
        {
            *ppvObject = reinterpret_cast<IInspectable *>(this);
            return S_OK;
        }

        if (IsEqualIID(IID_ISwapChainPanel, riid))
        {
            *ppvObject = static_cast<ISwapChainPanel *>(this);
            return S_OK;
        }

        if (IsEqualIID(IID_IFrameworkElement, riid))
        {
            *ppvObject = static_cast<IFrameworkElement *>(this);
            return S_OK;
        }

        if (IsEqualIID(IID_IUIElement, riid))
        {
            *ppvObject = static_cast<IUIElement *>(this);
            return S_OK;
        }

        if (IsEqualIID(__uuidof(ISwapChainPanelNative), riid))
        {
            *ppvObject = static_cast<ISwapChainPanelNative *>(this);
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

    // ISwapChainPanelNative
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, SetSwapChain, HRESULT(IDXGISwapChain *));

    // ISwapChainPanel
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_CompositionScaleX, HRESULT(FLOAT *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_CompositionScaleY, HRESULT(FLOAT *));
    MOCK_METHOD2_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        add_CompositionScaleChanged,
        HRESULT(
            __FITypedEventHandler_2_Windows__CUI__CXaml__CControls__CSwapChainPanel_IInspectable *,
            EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_CompositionScaleChanged,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               CreateCoreIndependentInputSource,
                               HRESULT(CoreInputDeviceTypes, ICoreInputSourceBase **));

    // IFrameworkElement
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               get_Triggers,
                               HRESULT(__FIVector_1_Windows__CUI__CXaml__CTriggerBase **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Resources, HRESULT(IResourceDictionary **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Resources, HRESULT(IResourceDictionary *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Tag, HRESULT(IInspectable **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Tag, HRESULT(IInspectable *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Language, HRESULT(HSTRING *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Language, HRESULT(HSTRING));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_ActualWidth, HRESULT(DOUBLE *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_ActualHeight, HRESULT(DOUBLE *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Width, HRESULT(DOUBLE *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Width, HRESULT(DOUBLE));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Height, HRESULT(DOUBLE *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Height, HRESULT(DOUBLE));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_MinWidth, HRESULT(DOUBLE *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_MinWidth, HRESULT(DOUBLE));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_MaxWidth, HRESULT(DOUBLE *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_MaxWidth, HRESULT(DOUBLE));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_MinHeight, HRESULT(DOUBLE *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_MinHeight, HRESULT(DOUBLE));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_MaxHeight, HRESULT(DOUBLE *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_MaxHeight, HRESULT(DOUBLE));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               get_HorizontalAlignment,
                               HRESULT(HorizontalAlignment *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               put_HorizontalAlignment,
                               HRESULT(HorizontalAlignment));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               get_VerticalAlignment,
                               HRESULT(VerticalAlignment *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               put_VerticalAlignment,
                               HRESULT(VerticalAlignment));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Margin, HRESULT(Thickness *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Margin, HRESULT(Thickness));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Name, HRESULT(HSTRING *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Name, HRESULT(HSTRING));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_BaseUri, HRESULT(IUriRuntimeClass **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_DataContext, HRESULT(IInspectable **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_DataContext, HRESULT(IInspectable *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Style, HRESULT(IStyle **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Style, HRESULT(IStyle *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Parent, HRESULT(IDependencyObject **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_FlowDirection, HRESULT(FlowDirection *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_FlowDirection, HRESULT(FlowDirection));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_Loaded,
                               HRESULT(IRoutedEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_Loaded, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_Unloaded,
                               HRESULT(IRoutedEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_Unloaded, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_SizeChanged,
                               HRESULT(ISizeChangedEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_SizeChanged,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_LayoutUpdated,
                               HRESULT(__FIEventHandler_1_IInspectable *,
                                       EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_LayoutUpdated,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE, FindName, HRESULT(HSTRING, IInspectable **));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               SetBinding,
                               HRESULT(IDependencyProperty *, IBindingBase *));

    // IUIElement
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               get_DesiredSize,
                               HRESULT(ABI::Windows::Foundation::Size *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_AllowDrop, HRESULT(boolean *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_AllowDrop, HRESULT(boolean));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Opacity, HRESULT(DOUBLE *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Opacity, HRESULT(DOUBLE));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Clip, HRESULT(IRectangleGeometry **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Clip, HRESULT(IRectangleGeometry *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_RenderTransform, HRESULT(ITransform **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_RenderTransform, HRESULT(ITransform *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Projection, HRESULT(IProjection **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Projection, HRESULT(IProjection *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_RenderTransformOrigin, HRESULT(Point *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_RenderTransformOrigin, HRESULT(Point));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_IsHitTestVisible, HRESULT(boolean *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_IsHitTestVisible, HRESULT(boolean));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_Visibility, HRESULT(Visibility *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_Visibility, HRESULT(Visibility));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_RenderSize, HRESULT(Size *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_UseLayoutRounding, HRESULT(boolean *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_UseLayoutRounding, HRESULT(boolean));
    MOCK_METHOD1_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        get_Transitions,
        HRESULT(__FIVector_1_Windows__CUI__CXaml__CMedia__CAnimation__CTransition **));
    MOCK_METHOD1_WITH_CALLTYPE(
        STDMETHODCALLTYPE,
        put_Transitions,
        HRESULT(__FIVector_1_Windows__CUI__CXaml__CMedia__CAnimation__CTransition *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_CacheMode, HRESULT(ICacheMode **));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_CacheMode, HRESULT(ICacheMode *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_IsTapEnabled, HRESULT(boolean *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_IsTapEnabled, HRESULT(boolean));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_IsDoubleTapEnabled, HRESULT(boolean *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_IsDoubleTapEnabled, HRESULT(boolean));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_IsRightTapEnabled, HRESULT(boolean *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_IsRightTapEnabled, HRESULT(boolean));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, get_IsHoldingEnabled, HRESULT(boolean *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_IsHoldingEnabled, HRESULT(boolean));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               get_ManipulationMode,
                               HRESULT(ManipulationModes *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, put_ManipulationMode, HRESULT(ManipulationModes));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               get_PointerCaptures,
                               HRESULT(__FIVectorView_1_Windows__CUI__CXaml__CInput__CPointer **));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_KeyUp,
                               HRESULT(IKeyEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_KeyUp, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_KeyDown,
                               HRESULT(IKeyEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_KeyDown, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_GotFocus,
                               HRESULT(IRoutedEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_GotFocus, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_LostFocus,
                               HRESULT(IRoutedEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_LostFocus,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_DragEnter,
                               HRESULT(IDragEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_DragEnter,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_DragLeave,
                               HRESULT(IDragEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_DragLeave,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_DragOver,
                               HRESULT(IDragEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_DragOver, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_Drop,
                               HRESULT(IDragEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_Drop, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_PointerPressed,
                               HRESULT(IPointerEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerPressed,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_PointerMoved,
                               HRESULT(IPointerEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerMoved,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_PointerReleased,
                               HRESULT(IPointerEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerReleased,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_PointerExited,
                               HRESULT(IPointerEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerExited,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_PointerCaptureLost,
                               HRESULT(IPointerEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerCaptureLost,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_PointerCanceled,
                               HRESULT(IPointerEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerCanceled,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_PointerWheelChanged,
                               HRESULT(IPointerEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerWheelChanged,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_PointerEntered,
                               HRESULT(IPointerEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_PointerEntered,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_Tapped,
                               HRESULT(ITappedEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_Tapped, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_DoubleTapped,
                               HRESULT(IDoubleTappedEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_DoubleTapped,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_Holding,
                               HRESULT(IHoldingEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, remove_Holding, HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_RightTapped,
                               HRESULT(IRightTappedEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_RightTapped,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_ManipulationStarting,
                               HRESULT(IManipulationStartingEventHandler *,
                                       EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_ManipulationStarting,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_ManipulationInertiaStarting,
                               HRESULT(IManipulationInertiaStartingEventHandler *,
                                       EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_ManipulationInertiaStarting,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_ManipulationStarted,
                               HRESULT(IManipulationStartedEventHandler *,
                                       EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_ManipulationStarted,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_ManipulationDelta,
                               HRESULT(IManipulationDeltaEventHandler *, EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_ManipulationDelta,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               add_ManipulationCompleted,
                               HRESULT(IManipulationCompletedEventHandler *,
                                       EventRegistrationToken *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               remove_ManipulationCompleted,
                               HRESULT(EventRegistrationToken));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, Measure, HRESULT(Size));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, Arrange, HRESULT(Rect));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE, CapturePointer, HRESULT(IPointer *, boolean *));
    MOCK_METHOD1_WITH_CALLTYPE(STDMETHODCALLTYPE, ReleasePointerCapture, HRESULT(IPointer *));
    MOCK_METHOD0_WITH_CALLTYPE(STDMETHODCALLTYPE, ReleasePointerCaptures, HRESULT());
    MOCK_METHOD3_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               AddHandler,
                               HRESULT(IRoutedEvent *, IInspectable *, boolean));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               RemoveHandler,
                               HRESULT(IRoutedEvent *, IInspectable *));
    MOCK_METHOD2_WITH_CALLTYPE(STDMETHODCALLTYPE,
                               TransformToVisual,
                               HRESULT(IUIElement *, IGeneralTransform **));
    MOCK_METHOD0_WITH_CALLTYPE(STDMETHODCALLTYPE, InvalidateMeasure, HRESULT());
    MOCK_METHOD0_WITH_CALLTYPE(STDMETHODCALLTYPE, InvalidateArrange, HRESULT());
    MOCK_METHOD0_WITH_CALLTYPE(STDMETHODCALLTYPE, UpdateLayout, HRESULT());
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

void expectNativeWindowInitCalls(MockSwapChainPanel &panel, bool expectRenderSize)
{
    if (expectRenderSize)
    {
        EXPECT_CALL(panel, get_RenderSize(testing::_)).Times(1);
    }

    EXPECT_CALL(panel, add_SizeChanged(testing::_, testing::_)).Times(1);
    EXPECT_CALL(panel, remove_SizeChanged(testing::_)).Times(1);
}

TEST(NativeWindowTest, SwapChainPanelByItself)
{
    MockSwapChainPanel mockSwapChainPanel;
    NativeWindow nativeWindow(reinterpret_cast<IInspectable *>(&mockSwapChainPanel));
    expectNativeWindowInitCalls(mockSwapChainPanel, true);
    EXPECT_TRUE(nativeWindow.initialize());
}

TEST(NativeWindowTest, SwapChainPanelInPropertySet)
{
    // COM is required to be initialized for creation of the property set
    EXPECT_HRESULT_SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    {
        MockSwapChainPanel mockSwapChainPanel;
        ComPtr<IMap<HSTRING, IInspectable *>> propertySet;

        // Create a simple property set with the swapchainpanel
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyMap(&propertySet));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLNativeWindowTypeProperty,
                                        reinterpret_cast<IInspectable *>(&mockSwapChainPanel)));

        // Check native window init calls
        NativeWindow nativeWindow(propertySet.Get());
        expectNativeWindowInitCalls(mockSwapChainPanel, true);
        EXPECT_TRUE(nativeWindow.initialize());
    }
    CoUninitialize();
}

TEST(NativeWindowTest, SwapChainPanelInPropertySetWithSizeAndScale)
{
    // COM is required to be initialized for creation of the property set
    EXPECT_HRESULT_SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    {
        MockSwapChainPanel mockSwapChainPanel;
        ComPtr<IMap<HSTRING, IInspectable *>> propertySet;
        ComPtr<IPropertyValueStatics> propertyValueStatics;
        ComPtr<IPropertyValue> singleValue;
        ComPtr<IPropertyValue> sizeValue;

        // Create a simple property set with the swapchainpanel
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyMap(&propertySet));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLNativeWindowTypeProperty,
                                        reinterpret_cast<IInspectable *>(&mockSwapChainPanel)));

        // Add a valid scale factor to the property set
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyValueStatics(propertyValueStatics.GetAddressOf()));
        propertyValueStatics->CreateSingle(
            0.5f, reinterpret_cast<IInspectable **>(singleValue.GetAddressOf()));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLRenderResolutionScaleProperty,
                                        reinterpret_cast<IInspectable *>(singleValue.Get())));

        // Add a valid size to the property set
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyValueStatics(propertyValueStatics.GetAddressOf()));
        propertyValueStatics->CreateSize(
            {480, 800}, reinterpret_cast<IInspectable **>(sizeValue.GetAddressOf()));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLRenderSurfaceSizeProperty,
                                        reinterpret_cast<IInspectable *>(sizeValue.Get())));

        // Check native window init fails, since we shouldn't be able to set a size and a scale
        // together
        NativeWindow nativeWindow(propertySet.Get());
        EXPECT_FALSE(nativeWindow.initialize());
    }
    CoUninitialize();
}

// Tests that the scale property works as expected in a property set with a SwapChainPanel
class SwapChainPanelScaleTest : public testing::TestWithParam<std::pair<float, bool>>
{};

TEST_P(SwapChainPanelScaleTest, ValidateScale)
{
    float scale         = GetParam().first;
    bool expectedResult = GetParam().second;

    // COM is required to be initialized for creation of the property set
    EXPECT_HRESULT_SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    {
        MockSwapChainPanel mockSwapChainPanel;
        ComPtr<IMap<HSTRING, IInspectable *>> propertySet;
        ComPtr<IPropertyValueStatics> propertyValueStatics;
        ComPtr<IPropertyValue> singleValue;

        // Create a simple property set
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyMap(&propertySet));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLNativeWindowTypeProperty,
                                        reinterpret_cast<IInspectable *>(&mockSwapChainPanel)));

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
            expectNativeWindowInitCalls(mockSwapChainPanel, true);
        }

        EXPECT_EQ(nativeWindow.initialize(), expectedResult);
    }
    CoUninitialize();
}

typedef std::pair<float, bool> scaleValidPair;
static const scaleValidPair scales[] = {scaleValidPair(1.0f, true), scaleValidPair(0.5f, true),
                                        scaleValidPair(0.0f, false), scaleValidPair(0.01f, true),
                                        scaleValidPair(2.00f, true)};

INSTANTIATE_TEST_SUITE_P(NativeWindowTest, SwapChainPanelScaleTest, testing::ValuesIn(scales));

// Tests that the size property works as expected in a property set with a SwapChainPanel
class SwapChainPanelSizeTest : public testing::TestWithParam<std::tuple<float, float, bool>>
{};

TEST_P(SwapChainPanelSizeTest, ValidateSize)
{
    Size renderSize     = {std::get<0>(GetParam()), std::get<1>(GetParam())};
    bool expectedResult = std::get<2>(GetParam());

    // COM is required to be initialized for creation of the property set
    EXPECT_HRESULT_SUCCEEDED(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    {
        MockSwapChainPanel mockSwapChainPanel;
        ComPtr<IMap<HSTRING, IInspectable *>> propertySet;
        ComPtr<IPropertyValueStatics> propertyValueStatics;
        ComPtr<IPropertyValue> sizeValue;

        // Create a simple property set
        EXPECT_HRESULT_SUCCEEDED(CreatePropertyMap(&propertySet));
        EXPECT_HRESULT_SUCCEEDED(
            SetInspectablePropertyValue(propertySet, EGLNativeWindowTypeProperty,
                                        reinterpret_cast<IInspectable *>(&mockSwapChainPanel)));

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
            expectNativeWindowInitCalls(mockSwapChainPanel, false);
        }

        EXPECT_EQ(nativeWindow.initialize(), expectedResult);
    }
    CoUninitialize();
}

typedef std::tuple<float, float, bool> sizeValidPair;
static const sizeValidPair sizes[] = {sizeValidPair(800, 480, true), sizeValidPair(0, 480, false),
                                      sizeValidPair(800, 0, false), sizeValidPair(0, 0, false)};

INSTANTIATE_TEST_SUITE_P(NativeWindowTest, SwapChainPanelSizeTest, testing::ValuesIn(sizes));

}  // namespace
