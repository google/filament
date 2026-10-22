/*
 * Copyright 2025 The Android Open Source Project
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
@file:JvmName("Exceptions")

package {{ kotlin_package }}
{% set ns = namespace() %}
{% for enum in by_category["enum"] %}
    {% if enum.name.get() == "error type" %}{% set ns.error = enum %}{% endif %}
    {% if enum.name.get() == "device lost reason" %}{% set ns.device_lost_reason = enum %}{% endif %}
{% endfor %}
{% for obj in by_category["object"] if obj.name.get() == "device" %}
    {% set ns.device = obj %}
{% endfor %}

/**
 * Exception for errors originating from the Dawn WebGPU library that do not fit
 * into more specific WebGPU error categories.
 *
 * @param message A detailed message explaining the error.
 */
public class DawnException(message: String) : Exception(message)

/**
 * Exception thrown when a [{{kotlin_name(ns.device)}}] is lost and can no longer be used.
 *
 * @param device The [{{kotlin_name(ns.device)}}] that was lost.
 * @param reason The reason code indicating why the device was lost.
 * @param message A human-readable message describing the device loss.
 */
public class DeviceLostException(
  public val device: {{kotlin_name(ns.device)}},
  @{{kotlin_name(ns.device_lost_reason)}}.Type public val reason: Int,
  message: String
) : Exception(message)

/**
 * Base class for exceptions that can happen at runtime.
 */
public open class WebGpuRuntimeException(message: String): Exception(message) {
    public companion object {
        /**
         * Create the exception for the appropriate error type.
         * @param type The [{{ kotlin_name(ns.error) }}].
         * @param message A human-readable message describing the error.
         */
        @JvmStatic
        public fun create(@{{ kotlin_name(ns.error) }}.Type type: Int, message: String): WebGpuRuntimeException =
            when (type) {
                {% for value in ns.error.values if value.name.get() != "no error" %}
                    {{ kotlin_name(ns.error) }}.{{value.name.CamelCase()}} -> {{value.name.CamelCase()}}Exception(message)
                {% endfor %}
                else -> UnknownException(message)
            }
    }
}

{% for value in ns.error.values if value.name.get() != "no error" %}
    /**
     * Exception for {{value.name.CamelCase()}} type errors.
     *
     * @param message A message explaining the error.
     */
    public class {{value.name.CamelCase()}}Exception(message: String) : WebGpuRuntimeException(message);

{% endfor %}

//* Generate a custom exception for every enum ending 'status'.
//* 'status' is renamed 'web gpu status'.
{% for enum in by_category['enum'] if include_enum(enum) and enum.name.chunks[-1] == 'status' %}
    {% set success = enum.values[0] %}  //* 'Success' is conventionally the first enum.
    {% set exception_name = (enum.name.chunks[:-1] if len(enum.name.chunks) > 1 else ['web', 'gpu']) | map('title') | join + 'Exception' %}
    public class {{ exception_name }} (
        public val reason: String = "",
        @{{ enum.name.CamelCase() }}.Type public val status: Int = {{ enum.name.CamelCase() }}.{{ success.name.CamelCase() }}) : Exception(
            (if (status != {{ enum.name.CamelCase() }}.{{ success.name.CamelCase() }}) "${ {{ enum.name.CamelCase() }}.toString(status)}: " else "") + reason) {
    }

{% endfor %}
