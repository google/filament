/*!
\brief Contains the CommandLine class.
\file PVRCore/commandline/CommandLine.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <vector>
#include <sstream>
#include <string>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#if !defined(__APPLE__)
#include <malloc.h>
#endif

#if !defined(_WIN32)
#define _stricmp strcasecmp
#endif

namespace pvr {
namespace platform {

/// <summary>This class parses, abstracts, stores and handles command line options passed on application launch.</summary>
class CommandLineParser
{
public:
	/// <summary>This class provides access to the command line arguments of a CommandLineParser. Its lifecycle is tied
	/// to the commandLineParser.</summary>
	class ParsedCommandLine
	{
	public:
		/// <summary>Default constructor for a command line parser.</summary>
		ParsedCommandLine() = default;
		/// <summary>Constructor for a command line parser.</summary>
		/// <param name="argc">The number of arguments to parse</param>
		/// <param name="argv">A list of argc arguments to parse</param>
		ParsedCommandLine(int argc, char** argv)
		{
			CommandLineParser parser(argc, argv);
			*this = parser.getParsedCommandLine();
		}

		/// <summary>A c-style std::string name-value pair that represents command line argument (arg: name, val: value)</summary>
		struct Option
		{
			const char* arg; //!< Argument name (i.e. -Width)
			const char* val; //!< Argument value (i.e. 640)
			/// <summary>Equality</summary>
			/// <param name="rhs">Right hand side of the operator</param>
			/// <returns>True if left and right side are equal, otherwise false</returns>
			bool operator==(const Option& rhs) const { return strcmp(arg, rhs.arg) == 0; }
			/// <summary>Equality to c-string</summary>
			/// <param name="rhs">Right hand side of the operator</param>
			/// <returns>True if left and right side are equal, otherwise false</returns>
			bool operator==(const char* rhs) const { return strcmp(arg, rhs) == 0; }
		};
		typedef std::vector<Option> Options; //!< List of all options passed

		/// <summary>Get all command line options as a list of name/value pairs (c-strings)</summary>
		/// <returns>The command line options</returns>
		const Options& getOptionsList() const { return _options; }

		/// <summary>Query if a specific argument name exists (regardless of the presence of a value or not). For
		/// example, if the command line was "myapp.exe -fps", the query hasOption("fps") will return true.</summary>
		/// <param name="name">The argument name to test</param>
		/// <returns>True if the argument name was passed through the command line , otherwise false.</returns>
		bool hasOption(const char* name) const { return std::find(_options.begin(), _options.end(), name) != _options.end(); }

		/// <summary>Get an argument as a std::string value. Returns false and leaves the value unchanged if the value is not
		/// present, allowing very easy use of default arguments.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">The value passed with the argument (verbatim). If the name was not present, it remains
		/// unchanged</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getStringOption(const char* name, std::string& outValue) const
		{
			auto it = std::find(_options.begin(), _options.end(), name);
			if (it == _options.end()) { return false; }
			outValue = it->val;
			return true;
		}

		/// <summary>Get an argument assuming it is a comma-separated list of string values. Returns false and leaves the value
		/// unchanged if the value is not present, allowing easy use of default arguments. Otherwise, tokenizes and adds the values
		/// to the provided list of strings.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValues">A std::vector of strings where all values will be appended/</param>
		/// <returns>True if the argument <paramRef name="name"/> was present, false otherwise</returns>
		bool getStringOptionList(const char* name, std::vector<std::string>& outValues) const
		{
			std::string tmp;
			if (!getStringOption(name, tmp)) { return false; }

			if (!tmp.empty())
			{
				std::istringstream iss(tmp);
				std::string output;
				while (std::getline(iss, output, ',')) { outValues.emplace_back(output); }
			}
			return true;
		}

		/// <summary>Get an argument's value as a float value. Returns false and leaves the value unchanged if the value
		/// is not present, allowing very easy use of default arguments.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">The value passed with the argument interpreted as a float. If the name was not present, or no value was passed with the name,it remains
		/// unchanged. If it was not representing a float, it silently returns zero (0.0).</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getFloatOption(const char* name, float& outValue) const
		{
			auto it = std::find(_options.begin(), _options.end(), name);
			if (it == _options.end() || it->val == NULL) { return false; }
			outValue = static_cast<float>(atof(it->val));
			return true;
		}

		/// <summary>Get an argument's value as an integer value. Returns false and leaves the value unchanged if the
		/// value is not present, allowing very easy use of default arguments.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">The value passed with the argument interpreted as an integer. If the name was not
		/// present, or no value was passed with the name, it remains unchanged. If it was not representing a float, it silently returns zero (0).</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getIntOption(const char* name, int32_t& outValue) const
		{
			auto it = std::find(_options.begin(), _options.end(), name);
			if (it == _options.end() || it->val == NULL) { return false; }
			outValue = atoi(it->val);
			return true;
		}

		/// <summary>If a specific argument was present, set outValue to True.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">True if the argument "name" was present, otherwise unchanged</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getBoolOptionSetTrueIfPresent(const char* name, bool& outValue) const
		{
			auto it = std::find(_options.begin(), _options.end(), name);
			if (it == _options.end()) { return false; }
			outValue = true;
			return true;
		}

		/// <summary>If a specific argument was present, set outValue to False.</summary>
		/// <param name="name">The command line argument (e.g. "-captureFrames")</param>
		/// <param name="outValue">False if the argument "name" was present, otherwise unchanged</param>
		/// <returns>True if the argument "name" was present, false otherwise</returns>
		bool getBoolOptionSetFalseIfPresent(const char* name, bool& outValue) const
		{
			auto it = std::find(_options.begin(), _options.end(), name);
			if (it == _options.end()) { return false; }
			outValue = false;
			return true;
		}

	private:
		friend class CommandLineParser;
		Options _options;
	};

	/// <summary>Constructor.</summary>
	CommandLineParser() : _data(0) {}

	/// <summary>Constructor from C++ main arguments</summary>
	/// <param name="argc">The number of arguments to parse</param>
	/// <param name="argv">A list of argc arguments to parse</param>
	CommandLineParser(int argc, char** argv) : _data(0) { set(argc, argv); }

	/// <summary>Get a ParsedCommandLine option to inspect and use the command line arguments.</summary>
	/// <returns>The processed command line object.</returns>
	const ParsedCommandLine& getParsedCommandLine() const { return _commandLine; }

	/// <summary>Set the command line to a new std::string (wide).</summary>
	/// <param name="cmdLine">The new (wide) std::string to set the command line to</param>
	void set(const wchar_t* cmdLine)
	{
		if (cmdLine == nullptr) { return; }

		size_t length = wcslen(cmdLine) + 1;

		std::vector<char> tmp;
		tmp.resize(length);

		while (length != 0u)
		{
			--length;
			tmp[length] = static_cast<char>(cmdLine[length]);
		}

		parseCmdLine(tmp.data());
	}

	/// <summary>Set the command line to a new list of arguments.</summary>
	/// <param name="argc">The number of arguments</param>
	/// <param name="argv">The list of arguments</param>
	void set(int argc, char** argv)
	{
		if (argc < 0) { return; }

		_commandLine._options.clear();

		{
			size_t length = 0;

			for (int i = 0; i < argc; ++i) { length += strlen(argv[i]) + 1; }

			_data.resize(length);
		}

		size_t offset = 0;

		for (int i = 0; i < argc; ++i)
		{
			// length
			const size_t length = strlen(argv[i]) + 1;

			memcpy(&_data[offset], argv[i], length);

			// split into var/arg
			parseArgV(&_data[offset]);

			offset += length;
		}
	}

	/// <summary>Set the command line from a new std::string.</summary>
	/// <param name="cmdLine">The new std::string to set the command line to</param>
	void set(const char* cmdLine) { parseCmdLine(cmdLine); }

	/// <summary>Set the command line from another command line.</summary>
	/// <param name="cmdLine">Copy the data from another command line</param>
	void set(const CommandLineParser& cmdLine) { *this = cmdLine; }

	/// <summary>Prepend data to the command line.</summary>
	/// <param name="cmdLine">A std::string containing the data to prepend to this command line</param>
	void prefix(const wchar_t* cmdLine)
	{
		if (!_commandLine._options.empty() != 0u)
		{
			CommandLineParser tmp;
			tmp.set(cmdLine);
			prefix(tmp);
		}
		else
		{
			set(cmdLine);
		}
	}

	/// <summary>Prepend a new list of arguments to the command line.</summary>
	/// <param name="argc">The number of arguments</param>
	/// <param name="argv">The list of arguments</param>
	void prefix(int argc, char** argv)
	{
		if (!_commandLine._options.empty() != 0u)
		{
			CommandLineParser tmp;
			tmp.set(argc, argv);
			prefix(tmp);
		}
		else
		{
			set(argc, argv);
		}
	}

	/// <summary>Prepend data from a std::string to the command line.</summary>
	/// <param name="cmdLine">The std::string whose data to prepend to the command line</param>
	void prefix(const char* cmdLine)
	{
		if (!_commandLine._options.empty() != 0u)
		{
			CommandLineParser tmp;
			tmp.set(cmdLine);
			prefix(tmp);
		}
		else
		{
			return set(cmdLine);
		}
	}

	/// <summary>Prepend the data from another command line.</summary>
	/// <param name="cmdLine">The command line from which to prepend the data</param>
	void prefix(const CommandLineParser& cmdLine)
	{
		if (cmdLine._commandLine._options.empty()) { return; }

		std::vector<char> newData;
		newData.resize(_data.size() + cmdLine._data.size());

		std::vector<ParsedCommandLine::Option> newOptions;
		newOptions.resize(_commandLine._options.size() + cmdLine._commandLine._options.size());

		// copy the data
		memcpy(newData.data(), cmdLine._data.data(), cmdLine._data.size());
		memcpy(&newData[cmdLine._data.size()], _data.data(), _data.size());

		// Initialize the options
		for (uint32_t i = 0; i < cmdLine._commandLine._options.size(); ++i)
		{
			newOptions[i].arg = (const char*)((size_t)cmdLine._commandLine._options[i].arg - (size_t)cmdLine._data.data()) + (size_t)newData.data();
			newOptions[i].val = (const char*)((size_t)cmdLine._commandLine._options[i].val - (size_t)cmdLine._data.data()) + (size_t)newData.data();
		}

		for (uint32_t i = 0; i < _commandLine._options.size(); ++i)
		{
			newOptions[cmdLine._commandLine._options.size() + i].arg =
				(const char*)((size_t)_commandLine._options[i].arg - (size_t)_data.data()) + (size_t)newData.data() + cmdLine._data.size();
			newOptions[cmdLine._commandLine._options.size() + i].val =
				(const char*)((size_t)_commandLine._options[i].val - (size_t)_data.data()) + (size_t)newData.data() + cmdLine._data.size();
		}

		// Set the variables
		_data = newData;

		_commandLine._options = newOptions;
	}

	/// <summary>Append data to the command line.</summary>
	/// <param name="cmdLine">A std::string containing the data to append to this command line</param>
	void append(const wchar_t* cmdLine)
	{
		if (!_commandLine._options.empty() != 0u)
		{
			CommandLineParser tmp;
			tmp.set(cmdLine);
			append(tmp);
		}
		else
		{
			set(cmdLine);
		}
	}

	/// <summary>Append a new list of arguments to the command line.</summary>
	/// <param name="argc">The number of arguments</param>
	/// <param name="argv">The list of arguments</param>
	void append(int argc, char** argv)
	{
		if (!_commandLine._options.empty() != 0u)
		{
			CommandLineParser tmp;
			tmp.set(argc, argv);
			append(tmp);
		}
		else
		{
			set(argc, argv);
		}
	}

	/// <summary>Append data from a std::string to the command line.</summary>
	/// <param name="cmdLine">The std::string whose data to append to the command line</param>
	void append(const char* cmdLine)
	{
		if (!_commandLine._options.empty() != 0u)
		{
			CommandLineParser tmp;
			tmp.set(cmdLine);
			append(tmp);
		}
		else
		{
			set(cmdLine);
		}
	}

	/// <summary>Append data from from another command line.</summary>
	/// <param name="cmdLine">The command line from which to append the data</param>
	void append(const CommandLineParser& cmdLine)
	{
		if (cmdLine._commandLine._options.empty()) { return; }

		std::vector<char> newData;
		newData.resize(_data.size() + cmdLine._data.size());

		std::vector<ParsedCommandLine::Option> newOptions;
		newOptions.resize(_commandLine._options.size() + cmdLine._commandLine._options.size());

		// copy the data
		memcpy(newData.data(), _data.data(), _data.size());
		memcpy(&newData[_data.size()], cmdLine._data.data(), cmdLine._data.size());

		// Initialize the options
		for (uint32_t i = 0; i < _commandLine._options.size(); ++i)
		{
			newOptions[i].arg = (const char*)((size_t)_commandLine._options[i].arg - (size_t)_data.data()) + (size_t)newData.data();
			newOptions[i].val = (const char*)((size_t)_commandLine._options[i].val - (size_t)_data.data()) + (size_t)newData.data();
		}

		for (uint32_t i = 0; i < cmdLine._commandLine._options.size(); ++i)
		{
			newOptions[_commandLine._options.size() + i].arg =
				(const char*)((size_t)cmdLine._commandLine._options[i].arg - (size_t)cmdLine._data.data()) + (size_t)newData.data() + _data.size();
			newOptions[_commandLine._options.size() + i].val =
				(const char*)((size_t)cmdLine._commandLine._options[i].val - (size_t)cmdLine._data.data()) + (size_t)newData.data() + _data.size();
		}

		// Set the variables
		_data = newData;
		_commandLine._options = newOptions;
	}

protected:
	/// <summary>Parse an entire std::string for command line data.</summary>
	/// <param name="cmdLine">The std::string containing the command line data</param>
	void parseCmdLine(const char* const cmdLine)
	{
		size_t len;
		uint32_t nIn, nOut;
		bool bInQuotes;
		ParsedCommandLine::Option opt;

		if (cmdLine == nullptr) { return; }

		// Take a copy of the original
		len = strlen(cmdLine) + 1;

		// Take a copy to be edited
		_data.resize(len);

		// Break the command line into options
		bInQuotes = false;
		opt.arg = nullptr;
		opt.val = nullptr;
		nIn = static_cast<uint32_t>(-1);
		nOut = 0;
		do
		{
			++nIn;
			if (cmdLine[nIn] == '"') { bInQuotes = !bInQuotes; }
			else
			{
				if (bInQuotes && cmdLine[nIn] != 0)
				{
					if (opt.arg == nullptr) { opt.arg = &_data[nOut]; }

					_data[nOut++] = cmdLine[nIn];
				}
				else
				{
					switch (cmdLine[nIn])
					{
					case '=':
						_data[nOut++] = 0;
						opt.val = &_data[nOut];
						break;

					case ' ':
					case '\t':
					case '\0':
						_data[nOut++] = 0;
						if ((opt.arg != nullptr) || (opt.val != nullptr))
						{
							// Add option to list
							_commandLine._options.emplace_back(opt);

							opt.arg = nullptr;
							opt.val = nullptr;
						}
						break;

					default:
						if (opt.arg == nullptr) { opt.arg = &_data[nOut]; }

						_data[nOut++] = cmdLine[nIn];
						break;
					}
				}
			}
		} while (cmdLine[nIn] != 0);
	}

	/// <summary>Parse a single argument as passed by the C/C++ style argc/argv command line format.</summary>
	/// <param name="arg">A single element of the "argv" array of char*</param>
	void parseArgV(char* arg)
	{
		ParsedCommandLine::Option opt;
		size_t j;

		// Hunt for an = symbol
		for (j = 0; (arg[j] != 0) && arg[j] != '='; ++j) { ; }

		opt.arg = arg;
		if (arg[j] != 0)
		{
			// terminate the arg std::string, set value std::string
			arg[j] = 0;
			opt.val = &arg[j + 1];
		}
		else
		{
			// no value specified
			opt.val = nullptr;
		}

		// Add option to list
		_commandLine._options.emplace_back(opt);
	}

private:
	uint32_t findArg(const char* arg) const
	{
		uint32_t i;

		/*
			Find an argument, case insensitive. Returns the index of the option
			if found, or the number of options if not.
		*/
		for (i = 0; i < _commandLine._options.size(); ++i)
		{
			if (_stricmp(_commandLine._options[i].arg, arg) == 0) { break; }
		}

		return i;
	}
	bool readFlag(const char* arg, bool& bVal) const
	{
		uint32_t nIdx = findArg(arg);

		if (nIdx == _commandLine._options.size()) { return false; }
		// a flag must have no value
		bVal = _commandLine._options[nIdx].val != nullptr ? false : true;
		return true;
	}
	bool readUint(const char* arg, uint32_t& val) const
	{
		uint32_t nIdx = findArg(arg);

		if (nIdx == _commandLine._options.size()) { return false; }
		val = static_cast<uint32_t>(atoi(_commandLine._options[nIdx].val));
		return true;
	}
	bool readFloat(const char* arg, float& val) const
	{
		uint32_t nIdx = findArg(arg);

		if (nIdx == _commandLine._options.size()) { return false; }
		val = static_cast<float>(atof(_commandLine._options[nIdx].val));
		return true;
	}
	std::vector<char> _data;
	ParsedCommandLine _commandLine;
};
} // namespace platform
///< summary> Typedef of the CommandLine into the pvr namespace</summary>
typedef platform::CommandLineParser::ParsedCommandLine CommandLine;
} // namespace pvr
