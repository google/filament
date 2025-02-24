// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#include "Configurator.hpp"
#include "Debug.hpp"

#include <algorithm>
#include <fstream>
#include <istream>

namespace {
inline std::string trimSpaces(const std::string &str)
{
	std::string trimmed = str;
	trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(), [](unsigned char c) {
		              return !std::isspace(c);
	              }));
	trimmed.erase(std::find_if(trimmed.rbegin(), trimmed.rend(), [](unsigned char c) {
		              return !std::isspace(c);
	              }).base(),
	              trimmed.end());
	return trimmed;
}
}  // namespace

namespace sw {

Configurator::Configurator(const std::string &filePath)
{
	std::fstream file(filePath, std::ios::in);
	if(file.fail())
	{
		return;
	}
	readConfiguration(file);
	file.close();
}

Configurator::Configurator(std::istream &str)
{
	readConfiguration(str);
}

bool Configurator::readConfiguration(std::istream &str)
{
	std::string line;
	std::string sectionName;

	int lineNumber = 0;
	while(getline(str, line))
	{
		++lineNumber;
		if(line.length())
		{
			if(line[line.length() - 1] == '\r')
			{
				line = line.substr(0, line.length() - 1);
			}

			if(!isprint(line[0]))
			{
				sw::warn("Cannot parse line %d of configuration, skipping line\n", lineNumber);
				return false;
			}

			std::string::size_type pLeft = line.find_first_of(";#[=");

			if(pLeft != std::string::npos)
			{
				switch(line[pLeft])
				{
				case '[':
					{
						std::string::size_type pRight = line.find_last_of("]");

						if(pRight != std::string::npos && pRight > pLeft)
						{
							sectionName = trimSpaces(line.substr(pLeft + 1, pRight - pLeft - 1));
							if(!sectionName.length())
							{
								sw::warn("Found empty section name at line %d of configuration\n", lineNumber);
							}
						}
					}
					break;
				case '=':
					{
						std::string key = trimSpaces(line.substr(0, pLeft));
						std::string value = trimSpaces(line.substr(pLeft + 1));
						if(!key.length() || !value.length())
						{
							sw::warn("Cannot parse key-value pair at line %d of configuration (key or value is empty), skipping key-value pair\n", lineNumber);
						}
						else
						{
							sections[sectionName].keyValuePairs[key] = value;
						}
					}
					break;
				case ';':
				case '#':
					// Ignore comments
					break;
				}
			}
		}
	}

	return sections.size() > 0;
}

void Configurator::writeFile(const std::string &filePath, const std::string &title)
{
	std::fstream file(filePath, std::ios::out);
	if(file.fail())
	{
		return;
	}

	file << "; " << title << std::endl
	     << std::endl;

	for(const auto &[sectionName, section] : sections)
	{
		file << "[" << sectionName << "]" << std::endl;
		for(const auto &[key, value] : section.keyValuePairs)
		{
			file << key << "=" << value << std::endl;
		}
		file << std::endl;
	}

	file.close();
}

std::optional<std::string> Configurator::getValueIfExists(const std::string &sectionName, const std::string &keyName) const
{
	const auto section = sections.find(sectionName);
	if(section == sections.end())
	{
		return std::nullopt;
	}

	const auto keyValue = section->second.keyValuePairs.find(keyName);
	if(keyValue == section->second.keyValuePairs.end())
	{
		return std::nullopt;
	}

	return keyValue->second;
}

std::string Configurator::getValue(const std::string &sectionName, const std::string &keyName, const std::string &defaultValue) const
{
	const auto value = getValueIfExists(sectionName, keyName);
	if(value)
	{
		return *value;
	}
	return defaultValue;
}

void Configurator::addValue(const std::string &sectionName, const std::string &keyName, const std::string &value)
{
	sections[sectionName].keyValuePairs[keyName] = value;
}

bool Configurator::getBoolean(const std::string &sectionName, const std::string &keyName, bool defaultValue) const
{
	auto strValue = getValueIfExists(sectionName, keyName);
	if(!strValue)
	{
		return defaultValue;
	}

	std::stringstream ss{ *strValue };

	bool val;
	ss >> val;
	if(ss.fail())
	{
		// Accept "true" and "false" as well.
		ss.clear();
		ss >> std::boolalpha >> val;
	}
	return val;
}

double Configurator::getFloat(const std::string &sectionName, const std::string &keyName, double defaultValue) const
{
	auto strValue = getValueIfExists(sectionName, keyName);
	if(!strValue)
	{
		return defaultValue;
	}

	std::stringstream ss{ *strValue };

	double val = 0;
	ss >> val;
	return val;
}
}  // namespace sw
