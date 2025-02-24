/*
 * Copyright (c) 2019 The Khronos Group Inc.
 * Copyright (c) 2019 Valve Corporation
 * Copyright (c) 2019 LunarG, Inc.
 * Copyright (c) 2023 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */

#pragma once

#include <iomanip>
#include <iostream>
#include <ostream>
#include <stack>
#include <sstream>
#include <string>

#include <assert.h>

#include "vulkaninfo.h"

std::string insert_quotes(std::string s) { return "\"" + s + "\""; }

std::string to_string(const std::array<uint8_t, 16> &uid) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) ss << '-';
        ss << std::setw(2) << static_cast<unsigned>(uid[i]);
    }
    return ss.str();
}
std::string to_string(const std::array<uint8_t, 8> &uid) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 8; ++i) {
        if (i == 4) ss << '-';
        ss << std::setw(2) << static_cast<unsigned>(uid[i]);
    }
    return ss.str();
}

std::ostream &operator<<(std::ostream &out, const VkConformanceVersion &v) {
    return out << static_cast<unsigned>(v.major) << "." << static_cast<unsigned>(v.minor) << "."
               << static_cast<unsigned>(v.subminor) << "." << static_cast<unsigned>(v.patch);
}

enum class OutputType { text, html, json, vkconfig_output };

struct PrinterCreateDetails {
    OutputType output_type = OutputType::text;
    bool print_to_file = false;
    std::string file_name = APP_SHORT_NAME ".txt";
    std::string start_string = "";
};

class Printer {
  public:
    Printer(const PrinterCreateDetails &details, std::ostream &out, const APIVersion vulkan_version)
        : output_type(details.output_type), out(out) {
        StackNode node{};
        node.is_first_item = false;
        node.indents = 0;
        switch (output_type) {
            case (OutputType::text):
                out << std::string(strlen(APP_UPPER_CASE_NAME), '=') << "\n";
                out << APP_UPPER_CASE_NAME "\n";
                out << std::string(strlen(APP_UPPER_CASE_NAME), '=') << "\n\n";
                out << API_NAME " Instance Version: " << APIVersion(vulkan_version) << "\n\n\n";
                break;
            case (OutputType::html):
                out << "<!doctype html>\n";
                out << "<html lang='en'>\n";
                out << "\t<head>\n";
                out << "\t\t<title>" APP_SHORT_NAME "</title>\n";
                out << "\t\t<style>\n";
                out << "\t\thtml {\n";
                out << "\t\t\tbackground-color: #0b1e48;\n";
                out << "\t\t\tbackground-image: url(\"https://vulkan.lunarg.com/img/bg-starfield.jpg\");\n";
                out << "\t\t\tbackground-position: center;\n";
                out << "\t\t\t-webkit-background-size: cover;\n";
                out << "\t\t\t-moz-background-size: cover;\n";
                out << "\t\t\t-o-background-size: cover;\n";
                out << "\t\t\tbackground-size: cover;\n";
                out << "\t\t\tbackground-attachment: fixed;\n";
                out << "\t\t\tbackground-repeat: no-repeat;\n";
                out << "\t\t\theight: 100%;\n";
                out << "\t\t}\n";
                out << "\t\t#header {\n";
                out << "\t\t\tz-index: -1;\n";
                out << "\t\t}\n";
                out << "\t\t#header>img {\n";
                out << "\t\t\tposition: absolute;\n";
                out << "\t\t\twidth: 160px;\n";
                out << "\t\t\tmargin-left: -280px;\n";
                out << "\t\t\ttop: -10px;\n";
                out << "\t\t\tleft: 50%;\n";
                out << "\t\t}\n";
                out << "\t\t#header>h1 {\n";
                out << "\t\t\tfont-family: Arial, \"Helvetica Neue\", Helvetica, sans-serif;\n";
                out << "\t\t\tfont-size: 44px;\n";
                out << "\t\t\tfont-weight: 200;\n";
                out << "\t\t\ttext-shadow: 4px 4px 5px #000;\n";
                out << "\t\t\tcolor: #eee;\n";
                out << "\t\t\tposition: absolute;\n";
                out << "\t\t\twidth: 400px;\n";
                out << "\t\t\tmargin-left: -80px;\n";
                out << "\t\t\ttop: 8px;\n";
                out << "\t\t\tleft: 50%;\n";
                out << "\t\t}\n";
                out << "\t\tbody {\n";
                out << "\t\t\tfont-family: Consolas, monaco, monospace;\n";
                out << "\t\t\tfont-size: 14px;\n";
                out << "\t\t\tline-height: 20px;\n";
                out << "\t\t\tcolor: #eee;\n";
                out << "\t\t\theight: 100%;\n";
                out << "\t\t\tmargin: 0;\n";
                out << "\t\t\toverflow: hidden;\n";
                out << "\t\t}\n";
                out << "\t\t#wrapper {\n";
                out << "\t\t\tbackground-color: rgba(0, 0, 0, 0.7);\n";
                out << "\t\t\tborder: 1px solid #446;\n";
                out << "\t\t\tbox-shadow: 0px 0px 10px #000;\n";
                out << "\t\t\tpadding: 8px 12px;\n\n";
                out << "\t\t\tdisplay: inline-block;\n";
                out << "\t\t\tposition: absolute;\n";
                out << "\t\t\ttop: 80px;\n";
                out << "\t\t\tbottom: 25px;\n";
                out << "\t\t\tleft: 50px;\n";
                out << "\t\t\tright: 50px;\n";
                out << "\t\t\toverflow: auto;\n";
                out << "\t\t}\n";
                out << "\t\tdetails>details {\n";
                out << "\t\t\tmargin-left: 22px;\n";
                out << "\t\t}\n";
                out << "\t\tdetails>summary:only-child::-webkit-details-marker {\n";
                out << "\t\t\tdisplay: none;\n";
                out << "\t\t}\n";
                out << "\t\t.var, .type, .val {\n";
                out << "\t\t\tdisplay: inline;\n";
                out << "\t\t}\n";
                out << "\t\t.var {\n";
                out << "\t\t}\n";
                out << "\t\t.type {\n";
                out << "\t\t\tcolor: #acf;\n";
                out << "\t\t\tmargin: 0 12px;\n";
                out << "\t\t}\n";
                out << "\t\t.val {\n";
                out << "\t\t\tcolor: #afa;\n";
                out << "\t\t\tbackground: #222;\n";
                out << "\t\t\ttext-align: right;\n";
                out << "\t\t}\n";
                out << "\t\t</style>\n";
                out << "\t</head>\n";
                out << "\t<body>\n";
                out << "\t\t<div id='header'>\n";
                out << "\t\t\t<h1>" APP_SHORT_NAME "</h1>\n";
                out << "\t\t</div>\n";
                out << "\t\t<div id='wrapper'>\n";
                out << "\t\t\t<details><summary>" API_NAME " Instance Version: <span class='val'>" << APIVersion(vulkan_version)
                    << "</span></summary></details>\n\t\t\t<br />\n";
                node.indents = 3;
                break;
            case (OutputType::json):
            case (OutputType::vkconfig_output):
                out << details.start_string;
                node.indents = 1;
                break;
            default:
                break;
        }
        object_stack.push(node);
    }
    ~Printer() {
        switch (output_type) {
            case (OutputType::text):
                break;
            case (OutputType::html):
                out << "\t\t</div>\n";
                out << "\t</body>\n";
                out << "</html>\n";
                break;
            case (OutputType::json):
            case (OutputType::vkconfig_output):
                out << "\n}\n";
                break;
        }
        assert(!object_stack.empty() && "mismatched number of ObjectStart/ObjectEnd or ArrayStart/ArrayEnd's");
        object_stack.pop();
        assert(object_stack.empty() && "indents must be zero at program end");
    };

    Printer(const Printer &) = delete;
    const Printer &operator=(const Printer &) = delete;

    OutputType Type() { return output_type; }

    // When an error occurs, call this to create a valid output file. Needed for json/html
    void FinishOutput() {
        while (!object_stack.empty()) {
            switch (output_type) {
                case (OutputType::text):
                    break;
                case (OutputType::html):
                    while (get_top().indents > 3) {
                        out << "</details>\n";
                    }
                    break;
                case (OutputType::json):
                case (OutputType::vkconfig_output):
                    out << "\n" << std::string(static_cast<size_t>(get_top().indents), '\t');
                    if (get_top().is_array) {
                        out << "]";
                    } else {
                        out << "}";
                    }
                    break;
                default:
                    break;
            }
            object_stack.pop();
        }
    }

    // Custom Formatting
    // use by prepending with p.SetXXX().ObjectStart/ArrayStart

    Printer &SetHeader() {
        get_top().set_next_header = true;
        return *this;
    }

    Printer &SetSubHeader() {
        get_top().set_next_subheader = true;
        return *this;
    }

    Printer &SetOpenDetails() {
        get_top().set_details_open = true;
        return *this;
    }

    Printer &SetAlwaysOpenDetails(bool value = true) {
        get_top().should_always_open = value;
        return *this;
    }

    Printer &SetTitleAsType() {
        get_top().set_object_name_as_type = true;
        return *this;
    }

    Printer &SetAsType() {
        get_top().set_as_type = true;
        return *this;
    }

    Printer &SetIgnoreMinWidthInChild() {
        get_top().ignore_min_width_parameter_in_child = true;
        return *this;
    }

    Printer &SetMinKeyWidth(size_t min_key_width) {
        get_top().min_key_width = min_key_width;
        return *this;
    }

    Printer &SetElementIndex(int index) {
        assert(index >= 0 && "cannot set element index to a negative value");
        get_top().element_index = index;
        return *this;
    }
    Printer &SetValueDescription(std::string str) {
        value_description = str;
        return *this;
    }

    void ObjectStart(std::string object_name, int32_t count_subobjects = -1) {
        switch (output_type) {
            case (OutputType::text): {
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << object_name;
                if (!value_description.empty()) {
                    out << " (" << value_description << ")";
                    value_description = {};
                }
                if (get_top().element_index != -1) {
                    out << "[" << get_top().element_index << "]";
                }
                out << ":";
                if (count_subobjects >= 0) {
                    out << " count = " << count_subobjects;
                }
                out << "\n";
                size_t headersize = object_name.size() + 1;
                if (count_subobjects >= 0) {
                    headersize += 9 + std::to_string(count_subobjects).size();
                }
                if (get_top().element_index != -1) {
                    headersize += 2 + std::to_string(get_top().element_index).size();
                    get_top().element_index = -1;
                }
                PrintHeaderUnderlines(headersize);
                break;
            }
            case (OutputType::html):
                out << std::string(static_cast<size_t>(get_top().indents), '\t');
                if (get_top().set_details_open || get_top().should_always_open) {
                    out << "<details open>";
                    get_top().set_details_open = false;
                } else {
                    out << "<details>";
                }
                out << "<summary>";
                if (get_top().set_object_name_as_type) {
                    out << "<span class='type'>" << object_name << "</span>";
                    get_top().set_object_name_as_type = false;
                } else {
                    out << object_name;
                }
                if (!value_description.empty()) {
                    out << " (" << value_description << ")";
                    value_description = {};
                }
                if (get_top().element_index != -1) {
                    out << "[<span class='val'>" << get_top().element_index << "</span>]";
                    get_top().element_index = -1;
                }
                if (count_subobjects >= 0) {
                    out << ": count = <span class='val'>" << std::to_string(count_subobjects) << "</span>";
                }
                out << "</summary>\n";
                break;
            case (OutputType::json):
                if (!get_top().is_first_item) {
                    out << ",\n";
                } else {
                    get_top().is_first_item = false;
                }
                out << std::string(static_cast<size_t>(get_top().indents), '\t');
                // Objects with no name are elements in an array of objects
                if (object_name == "" || get_top().element_index != -1) {
                    out << "{\n";
                    get_top().element_index = -1;
                } else {
                    out << "\"" << object_name << "\": {\n";
                }
                if (!value_description.empty()) {
                    value_description = {};
                }
                break;
            case (OutputType::vkconfig_output):
                if (!get_top().is_first_item) {
                    out << ",\n";
                } else {
                    get_top().is_first_item = false;
                }
                out << std::string(static_cast<size_t>(get_top().indents), '\t');

                if (get_top().element_index != -1) {
                    out << "\"" << object_name << "[" << get_top().element_index << "]\": {\n";
                    get_top().element_index = -1;
                } else {
                    out << "\"" << object_name << "\": {\n";
                }
                if (!value_description.empty()) {
                    value_description = {};
                }
                break;
            default:
                break;
        }
        push_node(false);
    }
    void ObjectEnd() {
        assert(get_top().is_array == false && "cannot call ObjectEnd while inside an Array");
        object_stack.pop();
        assert(get_top().indents >= 0 && "indents cannot go below zero");
        switch (output_type) {
            case (OutputType::text):

                break;
            case (OutputType::html):
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << "</details>\n";
                break;
            case (OutputType::json):
            case (OutputType::vkconfig_output):
                out << "\n" << std::string(static_cast<size_t>(get_top().indents), '\t') << "}";
                break;
            default:
                break;
        }
    }
    void ArrayStart(std::string array_name, size_t element_count = 0) {
        switch (output_type) {
            case (OutputType::text): {
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << array_name << ":";
                size_t underline_count = array_name.size() + 1;
                if (element_count > 0) {
                    out << " count = " << element_count;
                    underline_count += 9 + std::to_string(element_count).size();
                }
                out << "\n";
                PrintHeaderUnderlines(underline_count);
                break;
            }
            case (OutputType::html):
                out << std::string(static_cast<size_t>(get_top().indents), '\t');
                if (get_top().set_details_open || get_top().should_always_open) {
                    out << "<details open>";
                    get_top().set_details_open = false;
                } else {
                    out << "<details>";
                }
                out << "<summary>" << array_name;
                if (element_count > 0) {
                    out << ": count = <span class='val'>" << element_count << "</span>";
                }
                out << "</summary>\n";
                break;
            case (OutputType::json):
            case (OutputType::vkconfig_output):
                if (!get_top().is_first_item) {
                    out << ",\n";
                } else {
                    get_top().is_first_item = false;
                }
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << "\"" << array_name << "\": " << "[\n";
                assert(get_top().is_array == false &&
                       "Cant start an array object inside another array, must be enclosed in an object");
                break;
            default:
                break;
        }
        push_node(true);
    }
    void ArrayEnd() {
        assert(get_top().is_array == true && "cannot call ArrayEnd while inside an Object");
        object_stack.pop();
        assert(get_top().indents >= 0 && "indents cannot go below zero");
        switch (output_type) {
            case (OutputType::text):
                break;
            case (OutputType::html):
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << "</details>\n";
                break;
            case (OutputType::json):
            case (OutputType::vkconfig_output):
                out << "\n" << std::string(static_cast<size_t>(get_top().indents), '\t') << "]";
                break;
            default:
                break;
        }
    }

    // For printing key-value pairs.
    // value_description is for reference information and is displayed inside parenthesis after the value
    template <typename T>
    void PrintKeyValue(std::string key, T value) {
        // If we are inside of an array, Print the value as an element
        if (get_top().is_array) {
            PrintElement(value);
            return;
        }

        switch (output_type) {
            case (OutputType::text):
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << key;
                if (get_top().min_key_width > key.size() && !get_top().ignore_min_width_parameter) {
                    out << std::string(get_top().min_key_width - key.size(), ' ');
                }
                out << " = " << value;
                if (value_description != "") {
                    out << " (" << value_description << ")";
                    value_description = {};
                }
                out << "\n";
                break;
            case (OutputType::html):
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << "<details><summary>" << key;
                if (get_top().min_key_width > key.size()) {
                    out << std::string(get_top().min_key_width - key.size(), ' ');
                }
                if (get_top().set_as_type) {
                    get_top().set_as_type = false;
                    out << " = <span class='type'>" << value << "</span>";
                } else {
                    out << " = <span class='val'>" << value << "</span>";
                }
                if (!value_description.empty()) {
                    out << " (<span class='val'>" << value_description << "</span>)";
                    value_description = {};
                }
                out << "</summary></details>\n";
                break;
            case (OutputType::json):
            case (OutputType::vkconfig_output):
                if (!get_top().is_first_item) {
                    out << ",\n";
                } else {
                    get_top().is_first_item = false;
                }
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << "\"" << key << "\": ";
                if (!value_description.empty()) {
                    out << "\"" << value << " (" << value_description << ")\"";
                    value_description = {};
                } else {
                    out << value;
                }
            default:
                break;
        }
    }

    // Need a specialization to handle C style arrays since they are implicitly converted to pointers
    template <size_t N>
    void PrintKeyValue(std::string key, const uint8_t (&values)[N]) {
        switch (output_type) {
            case (OutputType::json): {
                ArrayStart(key, N);
                for (uint32_t i = 0; i < N; i++) {
                    PrintElement(static_cast<uint32_t>(values[i]));
                }
                ArrayEnd();
                break;
            }
            default: {
                std::array<uint8_t, N> arr{};
                std::copy(std::begin(values), std::end(values), std::begin(arr));
                PrintKeyString(key, to_string(arr));
                break;
            }
        }
    }

    // For printing key - string pairs (necessary because of json)
    void PrintKeyString(std::string key, std::string value) {
        switch (output_type) {
            case (OutputType::text):
            case (OutputType::html):
                PrintKeyValue(key, value);
                break;
            case (OutputType::json):
            case (OutputType::vkconfig_output):
                if (!value_description.empty()) {
                    // PrintKeyValue adds the necessary quotes when printing with a value description set
                    PrintKeyValue(key, EscapeJSONCString(value));
                } else {
                    PrintKeyValue(key, std::string("\"") + EscapeJSONCString(value) + "\"");
                }
                break;
            default:
                break;
        }
    }

    // For printing key - string pairs (necessary because of json)
    void PrintKeyBool(std::string key, bool value) { PrintKeyValue(key, value ? "true" : "false"); }

    // print inside array
    template <typename T>
    void PrintElement(T element) {
        // If we are inside of an object, just use an empty string as the key
        if (!get_top().is_array) {
            PrintKeyValue("placeholder", element);
            return;
        }
        switch (output_type) {
            case (OutputType::text):
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << element << "\n";
                break;
            case (OutputType::html):
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << "<details><summary>";
                if (get_top().set_as_type) {
                    get_top().set_as_type = false;
                    out << "<span class='type'>" << element << "</span>";
                } else {
                    out << "<span class='val'>" << element << "</span>";
                }
                out << "</summary></details>\n";
                break;
            case (OutputType::json):
            case (OutputType::vkconfig_output):
                if (!get_top().is_first_item) {
                    out << ",\n";
                } else {
                    get_top().is_first_item = false;
                }
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << element;
                break;
            default:
                break;
        }
    }
    void PrintString(std::string string) {
        switch (output_type) {
            case (OutputType::text):
            case (OutputType::html):
                PrintElement(string);
                break;
            case (OutputType::json):
            case (OutputType::vkconfig_output):
                PrintElement("\"" + EscapeJSONCString(string) + "\"");
            default:
                break;
        }
    }
    void PrintExtension(std::string ext_name, uint32_t revision, size_t min_width = 0) {
        switch (output_type) {
            case (OutputType::text):
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << ext_name
                    << std::string(min_width - ext_name.size(), ' ') << " : extension revision " << revision << "\n";
                break;
            case (OutputType::html):
                out << std::string(static_cast<size_t>(get_top().indents), '\t') << "<details><summary>" << DecorateAsType(ext_name)
                    << std::string(min_width - ext_name.size(), ' ') << " : extension revision "
                    << DecorateAsValue(std::to_string(revision)) << "</summary></details>\n";
                break;
            case (OutputType::json):
                PrintKeyValue(ext_name, revision);
                break;
            case (OutputType::vkconfig_output):
                ObjectStart(ext_name);
                PrintKeyValue("specVersion", revision);
                ObjectEnd();
                break;
            default:
                break;
        }
    }
    void AddNewline() {
        if (output_type == OutputType::text) {
            out << "\n";
        }
    }
    void IndentIncrease() {
        if (output_type == OutputType::text) {
            get_top().indents++;
        }
    }
    void IndentDecrease() {
        if (output_type == OutputType::text) {
            get_top().indents--;
            assert(get_top().indents >= 0 && "indents cannot go below zero");
        }
    }

    std::string DecorateAsType(const std::string &input) {
        if (output_type == OutputType::html)
            return "<span class='type'>" + input + "</span>";
        else
            return input;
    }

    std::string DecorateAsValue(const std::string &input) {
        if (output_type == OutputType::html)
            return "<span class='val'>" + input + "</span>";
        else
            return input;
    }

  protected:
    OutputType output_type;
    std::ostream &out;

    struct StackNode {
        int indents = 0;

        // header, subheader
        bool set_next_header = false;
        bool set_next_subheader = false;

        // html coloring
        bool set_as_type = false;

        // open <details>
        bool set_details_open = false;

        // always open <details>
        bool should_always_open = false;

        // make object titles the color of types
        bool set_object_name_as_type = false;

        // ignores the min_width parameter
        bool ignore_min_width_parameter = false;

        // sets the next created object/child to ignore the min_width parameter
        bool ignore_min_width_parameter_in_child = false;

        // keep track of the spacing between names and values
        size_t min_key_width = 0;

        // objects which are in an array
        int element_index = -1;  // negative one is the sentinel value

        // json
        bool is_first_item;  // for json: for adding a comma in between objects
        bool is_array;       // for json: match pairs of {}'s and []'s
    };

    // Contains the details about the current 'object' in the tree
    std::stack<StackNode> object_stack;

    // Helper to get the current top of the object_stack
    StackNode &get_top() { return object_stack.top(); }

    // Optional 'description' for values
    // Must be set right before the Print() function is called
    std::string value_description;

    // can only be called after a node was manually pushed onto the stack in the constructor
    void push_node(bool array) {
        StackNode node{};
        node.indents = get_top().indents + 1;
        node.is_array = array;
        node.is_first_item = true;
        node.ignore_min_width_parameter = get_top().ignore_min_width_parameter_in_child;
        object_stack.push(node);
    }

    // utility
    void PrintHeaderUnderlines(size_t length) {
        assert(get_top().indents >= 0 && "indents must not be negative");
        assert(length <= 10000 && "length shouldn't be unreasonably large");
        if (get_top().set_next_header) {
            out << std::string(static_cast<size_t>(get_top().indents), '\t') << std::string(length, '=') << "\n";
            get_top().set_next_header = false;
        } else if (get_top().set_next_subheader) {
            out << std::string(static_cast<size_t>(get_top().indents), '\t') << std::string(length, '-') << "\n";
            get_top().set_next_subheader = false;
        }
    }

    // Replace special characters in strings with their escaped versions.
    // <https://www.json.org/json-en.html>
    std::string EscapeJSONCString(std::string string) {
        if (output_type == OutputType::text || output_type == OutputType::html) return string;
        std::string out{};
        for (size_t i = 0; i < string.size(); i++) {
            char c = string[i];
            char out_c = c;
            switch (c) {
                case '\"':
                case '\\':
                    out.push_back('\\');
                    break;
                case '\b':
                    out.push_back('\\');
                    out_c = 'b';
                    break;
                case '\f':
                    out.push_back('\\');
                    out_c = 'f';
                    break;
                case '\n':
                    out.push_back('\\');
                    out_c = 'n';
                    break;
                case '\r':
                    out.push_back('\\');
                    out_c = 'r';
                    break;
                case '\t':
                    out.push_back('\\');
                    out_c = 't';
                    break;
            }
            out.push_back(out_c);
        }

        return out;
    }
};
// Purpose: When a Printer starts an object or array it will automatically indent the output. This isn't
// always desired, requiring a manual decrease of indention. This wrapper facilitates that while also
// automatically re-indenting the output to the previous indent level on scope exit.
class IndentWrapper {
  public:
    IndentWrapper(Printer &p) : p(p) {
        if (p.Type() == OutputType::text) p.IndentDecrease();
    }
    ~IndentWrapper() {
        if (p.Type() == OutputType::text) p.IndentIncrease();
    }

  private:
    Printer &p;
};

class ObjectWrapper {
  public:
    ObjectWrapper(Printer &p, std::string object_name) : p(p) { p.ObjectStart(object_name); }
    ObjectWrapper(Printer &p, std::string object_name, size_t count_subobjects) : p(p) {
        p.ObjectStart(object_name, static_cast<int32_t>(count_subobjects));
    }
    ~ObjectWrapper() { p.ObjectEnd(); }

  private:
    Printer &p;
};

class ArrayWrapper {
  public:
    ArrayWrapper(Printer &p, std::string array_name, size_t element_count = 0) : p(p) { p.ArrayStart(array_name, element_count); }
    ~ArrayWrapper() { p.ArrayEnd(); }

  private:
    Printer &p;
};
