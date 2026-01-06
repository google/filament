/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "fg/details/ResourceCreationContext.h"

#include <fg/FrameGraphDummyLink.h>
#include <fg/FrameGraphTexture.h>

#include <utils/StaticString.h>

#include <backend/DriverApiForward.h>

#include <type_traits>
#include <utility>

namespace filament {

// Helper to check if T::create(Args...) exists
template<typename T, typename... Args>
struct check_create {
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U&>().create(std::declval<Args>()...),
        std::true_type{});
    template<typename>
    static std::false_type test(...);
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T, typename... Args>
inline constexpr bool check_create_v = check_create<T, Args...>::value;

// A type-trait to check if a resource is a "backend" resource, i.e. if it has a
// create() method that takes a DriverApi& as its first parameter.
template<typename T>
struct is_backend_resource : std::bool_constant<
            check_create_v<T,
                backend::DriverApi&,
                utils::StaticString,
                typename T::Descriptor const&,
                typename T::Usage> ||
            check_create_v<T,
                backend::DriverApi&,
                utils::StaticString,
                typename T::Descriptor const&> ||
            check_create_v<T,
                backend::DriverApi&,
                typename T::Descriptor const&>> {
};

template<typename T>
inline constexpr bool is_backend_resource_v = is_backend_resource<T>::value;


/**
 * The default ResourceAllocator.
 *
 * This allocator uses SFINAE to automatically handle three types of resources:
 * 1. Backend resources: If `is_backend_resource_v<T>` is true, it calls create/destroy with the
 *    `DriverApi&` from the `ResourceCreationContext`.
 * 2. Generic resources: Otherwise, it calls the simple create/destroy methods.
 * 3. Specialized resources: Types like `FrameGraphTexture` provide their own full specialization
 *    of this struct and are not handled by this default template.
 */
template <typename T>
struct ResourceAllocator {
    static void create(T& resource, ResourceCreationContext const& context,
            utils::StaticString name,
            T::Descriptor const& desc, T::Usage usage) {
        if constexpr (is_backend_resource_v<T>) {
            // This is a backend resource, pass the driver.
            if constexpr (check_create_v<T,
                    backend::DriverApi&,
                    utils::StaticString,
                    typename T::Descriptor const&,
                    typename T::Usage>) {
                resource.create(context.driver, name, desc, usage);
            } else if constexpr (check_create_v<T,
                    backend::DriverApi&,
                    utils::StaticString,
                    typename T::Descriptor const&>) {
                resource.create(context.driver, name, desc);
            } else {
                resource.create(context.driver, desc);
            }
        } else {
            // This is a generic resource, call the simple version.
            if constexpr (check_create_v<T,
                    utils::StaticString,
                    typename T::Descriptor const&,
                    typename T::Usage>) {
                resource.create(name, desc, usage);
            } else if constexpr (check_create_v<T,
                    utils::StaticString,
                    typename T::Descriptor const&>) {
                resource.create(name, desc);
            } else {
                resource.create(desc);
            }
        }
    }

    static void destroy(T& resource, ResourceCreationContext const& context) {
        if constexpr (is_backend_resource_v<T>) {
            // This is a backend resource, pass the driver.
            resource.destroy(context.driver);
        } else {
            // This is a generic resource, call the simple version.
            resource.destroy();
        }
    }
};

/**
 * Specialization of ResourceAllocator for FrameGraphTexture.
 * This resource type is special because it uses the `TextureCacheInterface` for allocation.
 */
template <>
struct ResourceAllocator<FrameGraphTexture> {
    static void create(FrameGraphTexture& resource,
            ResourceCreationContext const& context,
            utils::StaticString name,
            FrameGraphTexture::Descriptor const& desc,
            FrameGraphTexture::Usage usage) {
        if (context.useProtectedMemory) {
            // FIXME: I think we should restrict this to attachments and blit destinations only
            usage |= FrameGraphTexture::Usage::PROTECTED;
        }
        resource.create(context.getTextureCache(), name, desc, usage);
    }

    static void destroy(FrameGraphTexture& resource, ResourceCreationContext const& context) {
        resource.destroy(context.getTextureCache());
    }
};

/**
 * Specialization of ResourceAllocator for FrameGraphDummyLink.
 * This resource type is special because doesn't have state.
 */
template <>
struct ResourceAllocator<FrameGraphDummyLink> {
    static void create(FrameGraphDummyLink&,
            ResourceCreationContext const&,
            utils::StaticString,
            FrameGraphDummyLink::Descriptor const&,
            FrameGraphDummyLink::Usage) {}
    static void destroy(FrameGraphDummyLink&, ResourceCreationContext const&) {}
};

} // namespace filament
