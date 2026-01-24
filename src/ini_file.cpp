/**
 * @file ini_file.cpp
 * @brief Implementation of the IniFile class for handling INI file operations.
 * @details This class provides methods for loading, saving, and retrieving
 *          configuration values from an INI file, supporting multiple data types.
 *
 * This software is distributed under the MIT License. See LICENSE.md for
 * details.
 *
 * Copyright © 2023-2026 Lee C. Bussy (@LBussy). All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ini_file.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

/**
 * @brief Tracks whether there are pending changes to be saved.
 */
bool _pendingChanges = false;

/**
 * @brief Returns the singleton IniFile instance.
 *
 * Constructs the static IniFile object upon first invocation and
 * returns a reference to it. Ensures only one instance exists.
 *
 * @return Reference to the single IniFile instance.
 */
IniFile &IniFile::instance()
{
    static IniFile inst;
    return inst;
}

/**
 * @brief Sets the filename and reloads the INI file.
 * @param filename The filename to set.
 */
void IniFile::set_filename(const std::string &filename)
{
    _filename = filename;
    load();
}

/**
 * @brief Loads and parses the INI file into memory.
 *
 * @details
 * This method opens the file specified by `_filename`, reads it line-by-line,
 * and parses it into a section-key-value structure stored in `_data`.
 * It also keeps the original lines in `_lines` and maps keys to line numbers
 * in `_index`, allowing for comment preservation and in-place modification.
 *
 * Empty lines and comments are preserved but ignored in parsing.
 * Inline comments after values (e.g., `key = value ; comment`) are trimmed.
 *
 * @throws std::runtime_error if `_filename` is empty or the file cannot be opened.
 *
 * @return true if the INI file was successfully loaded and parsed.
 */
bool IniFile::load()
{
    if (_filename.empty())
    {
        throw std::runtime_error("Null value filename passed for load.");
    }

    std::ifstream file(_filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open ini file " + _filename + ".");
    }

    // Clear any previously loaded data
    _data.clear();
    _lines.clear();
    _index.clear();

    std::string line;
    std::string current_section;
    size_t line_num = 0;

    // Read each line of the file
    while (std::getline(file, line))
    {
        std::string trimmed = trim(line);
        _lines.push_back(line); // Preserve original formatting

        // Skip empty lines and full-line comments
        if (trimmed.empty() || is_comment(trimmed))
        {
            line_num++;
            continue;
        }

        // Handle section headers like [section]
        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            current_section = trimmed.substr(1, trimmed.size() - 2);
        }
        else
        {
            // Handle key-value pairs like key = value
            size_t pos = trimmed.find('=');
            if (pos != std::string::npos)
            {
                std::string key = trim(trimmed.substr(0, pos));
                std::string value = trim(trimmed.substr(pos + 1));

                // Remove inline comment if present
                size_t comment_pos = value.find_first_of(";#");
                if (comment_pos != std::string::npos)
                {
                    value = trim(value.substr(0, comment_pos));
                }

                if (!key.empty())
                {
                    _data[current_section][key] = value;
                    _index[current_section][key] = line_num;
                }
            }
        }

        line_num++;
    }

    file.close();
    return true;
}

/**
 * @brief Saves the current INI file to disk.
 * @details Writes the stored key-value pairs back to the file while preserving
 *          comments and formatting. If a key exists in the data structure but
 *          not in the original file, it is written as a new entry.
 * @return True if the file was successfully saved, false otherwise.
 * @throws std::runtime_error If the filename is empty or if the file cannot be opened for writing.
 */
bool IniFile::save()
{
    if (_filename.empty())
    {
        throw std::runtime_error("Null value filename passed for save.");
    }

    std::ofstream file(_filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot write to file " + _filename + ".");
    }

    std::string current_section;
    for (size_t i = 0; i < _lines.size(); ++i)
    {
        std::string trimmed = trim(_lines[i]);

        if (trimmed.empty() || is_comment(trimmed))
        {
            file << _lines[i] << "\n";
            continue;
        }

        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            file << _lines[i] << "\n";
            current_section = trimmed.substr(1, trimmed.size() - 2);
            continue;
        }

        size_t pos = trimmed.find('=');
        if (pos != std::string::npos)
        {
            std::string key = trim(trimmed.substr(0, pos));
            if (!key.empty() && _data.count(current_section) && _data.at(current_section).count(key))
            {
                file << key << " = " << _data.at(current_section).at(key) << "\n";
            }
            else
            {
                file << _lines[i] << "\n";
            }
        }
        else
        {
            file << _lines[i] << "\n";
        }
    }

    file.close();
    return true;
}

/**
 * @brief Retrieves a value as a string from the INI file.
 * @param section The section name.
 * @param key The key name.
 * @return The corresponding value as a string.
 * @throws std::runtime_error If the section or key is not found.
 */
std::string IniFile::get_value(const std::string &section, const std::string &key) const
{
    auto sec = _data.find(section);
    if (sec == _data.end())
    {
        throw std::runtime_error("Error retrieving [" + section + "] from '" + _filename + "'.");
    }

    auto val = sec->second.find(key);
    if (val == sec->second.end())
    {
        throw std::runtime_error("Error retrieving '" + key + "' from section [" + section + "].");
    }
    return val->second;
}

/**
 * @brief Retrieves a string value from the INI file.
 * @param section The section name.
 * @param key The key name.
 * @return The string representation of the stored value.
 * @throws std::runtime_error If the section or key is not found.
 */
std::string IniFile::get_string_value(const std::string &section, const std::string &key) const
{
    return get_value(section, key);
}

/**
 * @brief Retrieves an integer value from the INI file.
 * @param section The section name.
 * @param key The key name.
 * @return The integer representation of the stored value.
 * @throws std::runtime_error If the section or key is not found, or if the value cannot be converted to an integer.
 */
int IniFile::get_int_value(const std::string &section, const std::string &key) const
{
    std::string value = get_value(section, key); // Let this throw if needed
    try
    {
        return std::stoi(value);
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Key '" + key + "' in section [" + section + "] is not a valid integer: '" + value + "'");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Key '" + key + "' in section [" + section + "] is out of range for integer: '" + value + "'");
    }
}

/**
 * @brief Converts a string representation of a boolean value to a boolean.
 * @details Performs a case-insensitive comparison to determine if the string
 *          represents a true value. Accepts "true", "1", and variations of
 *          "True".
 * @param value The string to convert.
 * @return True if the string represents a true value, otherwise false.
 */
bool IniFile::string_to_bool(const std::string &value)
{
    std::string lower_value = value;
    std::transform(lower_value.begin(), lower_value.end(), lower_value.begin(), ::tolower);
    return (lower_value == "true" || lower_value == "t" || lower_value == "1");
}

/**
 * @brief Retrieves a double value from the INI file.
 * @param section The section name.
 * @param key The key name.
 * @return The double representation of the stored value.
 * @throws std::runtime_error If the section or key is not found, or if the value
 *         cannot be converted to a double.
 */
double IniFile::get_double_value(const std::string &section, const std::string &key) const
{
    std::string value = get_value(section, key); // Let this throw if needed
    try
    {
        return std::stod(value);
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Key '" + key + "' in section [" + section + "] is not a valid double: '" + value + "'");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Key '" + key + "' in section [" + section + "] is out of range for double: '" + value + "'");
    }
}

/**
 * @brief Retrieves a boolean value from the INI file.
 * @param section The section name.
 * @param key The key name.
 * @return The boolean representation of the stored value.
 * @throws std::runtime_error If the section or key is not found.
 */
bool IniFile::get_bool_value(const std::string &section, const std::string &key) const
{
    return string_to_bool(get_value(section, key));
}

/**
 * @brief Sets a string value in the INI file.
 * @param section The section name.
 * @param key The key name.
 * @param value The string value to set.
 */
// cppcheck-suppress unusedFunction
void IniFile::set_string_value(const std::string &section, const std::string &key, const std::string &value)
{
    _data[section][key] = value;
    _pendingChanges = true;
}

/**
 * @brief Converts a boolean value to a string representation.
 * @param value The boolean value to convert.
 * @return "true" if the value is true, otherwise "false".
 */
std::string IniFile::bool_to_string(bool value)
{
    return value ? "true" : "false";
}

/**
 * @brief Sets a boolean value in the INI file.
 * @param section The section name.
 * @param key The key name.
 * @param value The boolean value to set.
 */
void IniFile::set_bool_value(const std::string &section, const std::string &key, bool value)
{
    _data[section][key] = bool_to_string(value);
    _pendingChanges = true;
}

/**
 * @brief Sets an integer value in the INI file.
 * @param section The section name.
 * @param key The key name.
 * @param value The integer value to set.
 */
void IniFile::set_int_value(const std::string &section, const std::string &key, int value)
{
    _data[section][key] = std::to_string(value);
    _pendingChanges = true;
}

/**
 * @brief Sets a double value in the INI file.
 * @param section The section name.
 * @param key The key name.
 * @param value The double value to set.
 */
void IniFile::set_double_value(const std::string &section, const std::string &key, double value)
{
    _data[section][key] = std::to_string(value);
    _pendingChanges = true;
}

/**
 * @brief Trims leading and trailing whitespace characters from a string.
 * @param str The input string to be trimmed.
 * @return A new string with whitespace removed from both ends.
 */
std::string IniFile::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    size_t last = str.find_last_not_of(" \t\r\n");
    return (first == std::string::npos) ? "" : str.substr(first, (last - first + 1));
}

/**
 * @brief Determines whether a given line is a comment.
 * @param line The line to check.
 * @return True if the line is a comment, false otherwise.
 */
bool IniFile::is_comment(const std::string &line)
{
    return line.empty() || line.front() == ';' || line.front() == '#';
}

/**
 * @brief Commits any pending changes by saving the INI file.
 * @details If there are unsaved changes, this function writes them to the file
 *          and resets the pending changes flag.
 */
// cppcheck-suppress unusedFunction
void IniFile::commit_changes()
{
    if (_pendingChanges)
    {
        save();
        _pendingChanges = false;
    }
}

/**
 * @brief Retrieves the parsed INI data.
 * @return A const reference to the internal data storage.
 */
const std::map<std::string, std::unordered_map<std::string, std::string>> &IniFile::getData() const
{
    return _data;
}

/**
 * @brief Sets the internal data of the INI file.
 *
 * @param data A new mapping of sections to key/value pairs.
 * @return True if the data was set successfully.
 */
// cppcheck-suppress unusedFunction
void IniFile::setData(const std::map<std::string, std::unordered_map<std::string, std::string>> &data)
{
    _data = data;
}
