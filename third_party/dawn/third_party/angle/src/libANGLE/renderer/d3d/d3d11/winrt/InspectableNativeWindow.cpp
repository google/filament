//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// InspectableNativeWindow.cpp: NativeWindow base class for managing IInspectable native window
// types.

#include "libANGLE/renderer/d3d/d3d11/winrt/CoreWindowNativeWindow.h"
#include "libANGLE/renderer/d3d/d3d11/winrt/SwapChainPanelNativeWindow.h"

namespace rx
{

bool IsCoreWindow(EGLNativeWindowType window,
                  ComPtr<ABI::Windows::UI::Core::ICoreWindow> *coreWindow)
{
    if (!window)
    {
        return false;
    }

    ComPtr<IInspectable> win = reinterpret_cast<IInspectable *>(window);
    ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWin;
    if (SUCCEEDED(win.As(&coreWin)))
    {
        if (coreWindow != nullptr)
        {
            *coreWindow = coreWin;
        }
        return true;
    }

    return false;
}

bool IsSwapChainPanel(EGLNativeWindowType window, ComPtr<ISwapChainPanel> *swapChainPanel)
{
    if (!window)
    {
        return false;
    }

    ComPtr<IInspectable> win = reinterpret_cast<IInspectable *>(window);
    ComPtr<ISwapChainPanel> panel;
    if (SUCCEEDED(win.As(&panel)))
    {
        if (swapChainPanel != nullptr)
        {
            *swapChainPanel = panel;
        }
        return true;
    }

    return false;
}

bool IsEGLConfiguredPropertySet(EGLNativeWindowType window,
                                ABI::Windows::Foundation::Collections::IPropertySet **propertySet,
                                IInspectable **eglNativeWindow)
{
    if (!window)
    {
        return false;
    }

    ComPtr<IInspectable> props = reinterpret_cast<IInspectable *>(window);
    ComPtr<IPropertySet> propSet;
    ComPtr<IInspectable> nativeWindow;
    ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable *>> propMap;
    boolean hasEglNativeWindowPropertyKey = false;

    HRESULT result = props.As(&propSet);
    if (SUCCEEDED(result))
    {
        result = propSet.As(&propMap);
    }

    // Look for the presence of the EGLNativeWindowType in the property set
    if (SUCCEEDED(result))
    {
        result = propMap->HasKey(HStringReference(EGLNativeWindowTypeProperty).Get(),
                                 &hasEglNativeWindowPropertyKey);
    }

    // If the IPropertySet does not contain the required EglNativeWindowType key, the property set
    // is considered invalid.
    if (SUCCEEDED(result) && !hasEglNativeWindowPropertyKey)
    {
        ERR() << "Could not find EGLNativeWindowTypeProperty in IPropertySet. Valid "
                 "EGLNativeWindowTypeProperty values include ICoreWindow";
        return false;
    }

    // The EglNativeWindowType property exists, so retreive the IInspectable that represents the
    // EGLNativeWindowType
    if (SUCCEEDED(result) && hasEglNativeWindowPropertyKey)
    {
        result =
            propMap->Lookup(HStringReference(EGLNativeWindowTypeProperty).Get(), &nativeWindow);
    }

    if (SUCCEEDED(result))
    {
        if (propertySet != nullptr)
        {
            result = propSet.CopyTo(propertySet);
        }
    }

    if (SUCCEEDED(result))
    {
        if (eglNativeWindow != nullptr)
        {
            result = nativeWindow.CopyTo(eglNativeWindow);
        }
    }

    if (SUCCEEDED(result))
    {
        return true;
    }

    return false;
}

// Retrieve an optional property from a property set
HRESULT GetOptionalPropertyValue(
    const ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable *>> &propertyMap,
    const wchar_t *propertyName,
    boolean *hasKey,
    ComPtr<ABI::Windows::Foundation::IPropertyValue> &propertyValue)
{
    if (!propertyMap || !hasKey)
    {
        return E_INVALIDARG;
    }

    // Assume that the value does not exist
    *hasKey = false;

    HRESULT result = propertyMap->HasKey(HStringReference(propertyName).Get(), hasKey);
    if (SUCCEEDED(result) && !(*hasKey))
    {
        // Value does not exist, so return S_OK and set the exists parameter to false to indicate
        // that a the optional property does not exist.
        return S_OK;
    }

    if (SUCCEEDED(result))
    {
        result = propertyMap->Lookup(HStringReference(propertyName).Get(), &propertyValue);
    }

    return result;
}

// Attempts to read an optional SIZE property value that is assumed to be in the form of
// an ABI::Windows::Foundation::Size.  This function validates the Size value before returning
// it to the caller.
//
// Possible return values are:
// S_OK, valueExists == true - optional SIZE value was successfully retrieved and validated
// S_OK, valueExists == false - optional SIZE value was not found
// E_INVALIDARG, valueExists = false - optional SIZE value was malformed in the property set.
//    * Incorrect property type ( must be PropertyType_Size)
//    * Invalid property value (width/height must be > 0)
// Additional errors may be returned from IMap or IPropertyValue
//
HRESULT GetOptionalSizePropertyValue(
    const ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable *>> &propertyMap,
    const wchar_t *propertyName,
    SIZE *value,
    bool *valueExists)
{
    ComPtr<ABI::Windows::Foundation::IPropertyValue> propertyValue;
    ABI::Windows::Foundation::PropertyType propertyType =
        ABI::Windows::Foundation::PropertyType::PropertyType_Empty;
    Size sizeValue = {0, 0};
    boolean hasKey = false;

    if (!propertyMap || !value || !valueExists)
    {
        return E_INVALIDARG;
    }

    // Assume that the value does not exist
    *valueExists = false;
    *value       = {0, 0};

    HRESULT result = GetOptionalPropertyValue(propertyMap, propertyName, &hasKey, propertyValue);
    if (SUCCEEDED(result) && hasKey)
    {
        result = propertyValue->get_Type(&propertyType);

        // Check if the expected Size property is of PropertyType_Size type.
        if (SUCCEEDED(result) &&
            propertyType == ABI::Windows::Foundation::PropertyType::PropertyType_Size)
        {
            if (SUCCEEDED(propertyValue->GetSize(&sizeValue)) &&
                (sizeValue.Width > 0 && sizeValue.Height > 0))
            {
                // A valid property value exists
                *value = {static_cast<long>(sizeValue.Width), static_cast<long>(sizeValue.Height)};
                *valueExists = true;
                result       = S_OK;
            }
            else
            {
                // An invalid Size property was detected. Width/Height values must > 0
                result = E_INVALIDARG;
            }
        }
        else
        {
            // An invalid property type was detected. Size property must be of PropertyType_Size
            result = E_INVALIDARG;
        }
    }

    return result;
}

// Attempts to read an optional float property value that is assumed to be in the form of
// an ABI::Windows::Foundation::Single.  This function validates the Single value before returning
// it to the caller.
//
// Possible return values are:
// S_OK, valueExists == true - optional Single value was successfully retrieved and validated
// S_OK, valueExists == false - optional Single value was not found
// E_INVALIDARG, valueExists = false - optional Single value was malformed in the property set.
//    * Incorrect property type ( must be PropertyType_Single)
//    * Invalid property value (must be > 0)
// Additional errors may be returned from IMap or IPropertyValue
//
HRESULT GetOptionalSinglePropertyValue(
    const ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable *>> &propertyMap,
    const wchar_t *propertyName,
    float *value,
    bool *valueExists)
{
    ComPtr<ABI::Windows::Foundation::IPropertyValue> propertyValue;
    ABI::Windows::Foundation::PropertyType propertyType =
        ABI::Windows::Foundation::PropertyType::PropertyType_Empty;
    float scaleValue = 0.0f;
    boolean hasKey   = false;

    if (!propertyMap || !value || !valueExists)
    {
        return E_INVALIDARG;
    }

    // Assume that the value does not exist
    *valueExists = false;
    *value       = 0.0f;

    HRESULT result = GetOptionalPropertyValue(propertyMap, propertyName, &hasKey, propertyValue);
    if (SUCCEEDED(result) && hasKey)
    {
        result = propertyValue->get_Type(&propertyType);

        // Check if the expected Scale property is of PropertyType_Single type.
        if (SUCCEEDED(result) &&
            propertyType == ABI::Windows::Foundation::PropertyType::PropertyType_Single)
        {
            if (SUCCEEDED(propertyValue->GetSingle(&scaleValue)) && (scaleValue > 0.0f))
            {
                // A valid property value exists
                *value       = scaleValue;
                *valueExists = true;
                result       = S_OK;
            }
            else
            {
                // An invalid scale was set
                result = E_INVALIDARG;
            }
        }
        else
        {
            // An invalid property type was detected. Size property must be of PropertyType_Single
            result = E_INVALIDARG;
        }
    }

    return result;
}

RECT InspectableNativeWindow::clientRect(const Size &size)
{
    // We don't have to check if a swapchain scale was specified here; the default value is 1.0f
    // which will have no effect.
    return {0, 0, lround(size.Width * mSwapChainScale), lround(size.Height * mSwapChainScale)};
}
}  // namespace rx
