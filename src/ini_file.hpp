/**
 * @file ini_file.hpp
 * @brief INI file handling class definition.
 * @details This class provides functionality to load, save, and retrieve
 *          configuration values from an INI file format. Supports string,
 *          boolean, integer, and double values with optional default values.
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

#ifndef INI_FILE_HPP
#define INI_FILE_HPP

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @class IniFile
 * @brief Handles reading and writing INI-style configuration files.
 * @details This class loads an INI file into an internal data structure,
 *          allowing retrieval and modification of values with optional
 *          default fallbacks.
 */
class IniFile
{
public:
    /**
     * @brief Returns the singleton IniFile instance.
     *
     * Constructs the static IniFile object upon first invocation and
     * returns a reference to it. Ensures only one instance exists.
     *
     * @return Reference to the single IniFile instance.
     */
    static IniFile &instance();

    /**
     * @brief Sets the filename for loading and saving operations.
     * @param filename Path to the INI file.
     */
    void set_filename(const std::string &filename);

    /**
     * @brief Loads the INI file into memory.
     * @return True if the file was successfully loaded, false otherwise.
     */
    bool load();

    /**
     * @brief Saves the current data to the INI file.
     * @return True if the file was successfully saved, false otherwise.
     */
    bool save();

    /**
     * @brief Retrieves a string value from the INI file.
     * @param section The section name.
     * @param key The key name.
     * @return The corresponding value as a string.
     * @throws std::runtime_error if the section or key is not found.
     */
    std::string get_value(const std::string &section, const std::string &key) const;

    /**
     * @brief Retrieves a string value with an optional default.
     */
    std::string get_string_value(const std::string &section, const std::string &key) const;

    /**
     * @brief Retrieves a boolean value with an optional default.
     */
    bool get_bool_value(const std::string &section, const std::string &key) const;

    /**
     * @brief Retrieves an integer value with an optional default.
     */
    int get_int_value(const std::string &section, const std::string &key) const;

    /**
     * @brief Retrieves a double value with an optional default.
     */
    double get_double_value(const std::string &section, const std::string &key) const;

    /**
     * @brief Sets a string value in the INI file.
     *
     * Updates the internal data map with the given section/key and
     * marks the file as having pending changes.
     *
     * @param section  The section under which to store the value.
     * @param key      The key within the section.
     * @param value    The string value to assign to the key.
     */
    void set_string_value(const std::string &section,
                          const std::string &key,
                          const std::string &value);

    /**
     * @brief Sets a boolean value in the INI file.
     *
     * Converts the boolean to "true"/"false", updates the internal data,
     * and marks the file as having pending changes.
     *
     * @param section  The section under which to store the value.
     * @param key      The key within the section.
     * @param value    The boolean value to assign to the key.
     */
    void set_bool_value(const std::string &section,
                        const std::string &key,
                        bool value);

    /**
     * @brief Sets an integer value in the INI file.
     *
     * Converts the integer to its string representation, updates the internal
     * data, and marks the file as having pending changes.
     *
     * @param section  The section under which to store the value.
     * @param key      The key within the section.
     * @param value    The integer value to assign to the key.
     */
    void set_int_value(const std::string &section,
                       const std::string &key,
                       int value);

    /**
     * @brief Sets a double value in the INI file.
     *
     * Converts the double to its string representation, updates the internal
     * data, and marks the file as having pending changes.
     *
     * @param section  The section under which to store the value.
     * @param key      The key within the section.
     * @param value    The double value to assign to the key.
     */
    void set_double_value(const std::string &section,
                          const std::string &key,
                          double value);

    /**
     * @brief Commits any pending changes to the INI file.
     */
    void commit_changes();

    /**
     * @brief Retrieves the parsed INI data.
     * @return A const reference to the internal data storage.
     */
    const std::map<std::string, std::unordered_map<std::string, std::string>> &getData() const;

    /**
     * @brief Sets the internal data of the INI file.
     *
     * @param data A new mapping of sections to key/value pairs.
     * @return True if the data was set successfully.
     */
    void setData(const std::map<std::string, std::unordered_map<std::string, std::string>> &data);

private:
    /**
     * @brief Default constructor.
     *
     * Constructs an IniFile with no filename set. Use set_filename() before use.
     */
    IniFile() = default;

    /**
     * @brief Default destructor.
     *
     * Tears down an IniFile.
     */
    ~IniFile() = default;

    /**
     * @brief Deleted copy constructor.
     *
     * Prevents copying of the singleton instance.
     */
    IniFile(const IniFile &) = delete;

    /**
     * @brief Deleted move constructor.
     *
     * Prevents moving of the singleton instance.
     */
    IniFile(IniFile &&) = delete;

    /**
     * @brief Deleted copy-assignment operator.
     *
     * Prevents assigning from another IniFile, preserving singleton uniqueness.
     *
     * @return Reference to this instance.
     */
    IniFile &operator=(const IniFile &) = delete;

    /**
     * @brief Deleted move-assignment operator.
     *
     * Prevents move-assigning from another IniFile, preserving singleton uniqueness.
     *
     * @return Reference to this instance.
     */
    IniFile &operator=(IniFile &&) = delete;

    /**
     * @brief Path to the INI configuration file.
     *
     * This filename is used by load() and save() to access the file on disk.
     */
    std::string _filename;

    /**
     * @brief Internal data storage.
     *
     * Maps each section name to a map of key/value string pairs.
     */
    std::map<std::string, std::unordered_map<std::string, std::string>> _data;

    /**
     * @brief Original file lines.
     *
     * Stores every line from the INI file, including comments and blank lines,
     * to allow preservation of formatting on save().
     */
    std::vector<std::string> _lines;

    /**
     * @brief Fast lookup index.
     *
     * Maps section/key pairs to their line number in _lines for in-place edits.
     */
    std::map<std::string, std::map<std::string, size_t>> _index;

    /**
     * @brief Trims whitespace from a string.
     * @param str The string to trim.
     * @return Trimmed string.
     */
    static std::string trim(const std::string &str);

    /**
     * @brief Determines if a line is a comment.
     * @param line The line to check.
     * @return True if the line is a comment, false otherwise.
     */
    static bool is_comment(const std::string &line);

    /**
     * @brief Converts a boolean value to string.
     * @param value The boolean value.
     * @return "true" or "false".
     */
    static std::string bool_to_string(bool value);

    /**
     * @brief Converts a string to boolean.
     * @param value The string to convert.
     * @return True if the string represents a true value, false otherwise.
     */
    static bool string_to_bool(const std::string &value);
};

#endif // INI_FILE_HPP
