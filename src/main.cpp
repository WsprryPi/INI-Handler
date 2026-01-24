/**
 * @file main.cpp
 * @brief Main file for testing the IniFile class
 * @details This file is used to test the IniFile class. It reads an INI file,
 *          performs some read and write operations, and tests exception
 *          handling.
 *
 * This software is distributed under the MIT License. See LICENSE.md for
 * details.
 *
 * Copyright © 2025 - 2026 Lee C. Bussy (@LBussy). All rights reserved.
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
#include <iostream>

//std::string filename = "../test/test.ini";
std::string filename = "/usr/local/etc/wsprrypi.ini";

void test_malformed_entries(IniFile &config)
{
    std::cout << std::endl << "⚠️ Testing Malformed INI Entries:" << std::endl;

    try
    {
        config.set_string_value("Common", "TX Power", "abc"); // Invalid integer
        int txPower = config.get_int_value("Common", "TX Power");
        std::cout << "TX Power after setting invalid value: " << txPower<< std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception for malformed TX Power: " << e.what()<< std::endl;
    }

    try
    {
        config.set_string_value("Extended", "PPM", "xyz"); // Invalid double
        double ppm = config.get_double_value("Extended", "PPM");
        std::cout << "PPM after setting invalid value: " << ppm<< std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception for malformed PPM: " << e.what()<< std::endl;
    }
}

void test_reading(IniFile &config)
{
    std::cout << std::endl << "🔎 Testing Read Operations: on:" << filename << std::endl;

    std::cout << "✅ Control | Transmit Enabled: " << config.get_bool_value("Control", "Transmit") << std::endl;

    std::cout << "✅ Common   | Call Sign: " << config.get_string_value("Common", "Call Sign") << std::endl;
    std::cout << "✅ Common   | Grid Square: " << config.get_string_value("Common", "Grid Square") << std::endl;
    std::cout << "✅ Common   | TX Power: " << config.get_int_value("Common", "TX Power") << std::endl;
    std::cout << "✅ Common   | Frequency: " << config.get_string_value("Common", "Frequency") << std::endl;
    std::cout << "✅ Common   | Transmit Pin: " << config.get_int_value("Common", "Transmit Pin") << std::endl;

    std::cout << "✅ Extended | PPM: " << config.get_double_value("Extended", "PPM") << std::endl;
    std::cout << "✅ Extended | Use NTP: " << config.get_bool_value("Extended", "Use NTP") << std::endl;
    std::cout << "✅ Extended | Offset: " << config.get_bool_value("Extended", "Offset") << std::endl;
    std::cout << "✅ Extended | Use LED: " << config.get_bool_value("Extended", "Use LED") << std::endl;
    std::cout << "✅ Extended | LED Pin: " << config.get_int_value("Extended", "LED Pin") << std::endl;
    std::cout << "✅ Extended | Power Level: " << config.get_int_value("Extended", "Power Level") << std::endl;

    std::cout << "✅ Server   | Web Port: " << config.get_int_value("Server", "Web Port") << std::endl;
    std::cout << "✅ Server   | Socket Port: " << config.get_int_value("Server", "Socket Port") << std::endl;
    std::cout << "✅ Server   | Use Shutdown: " << config.get_bool_value("Server", "Use Shutdown") << std::endl;
    std::cout << "✅ Server   | Shutdown Button: " << config.get_int_value("Server", "Shutdown Button") << std::endl;

    try
    {
        std::cout << "❌ Non-existent Section: " << config.get_string_value("NonExistent", "Key")<< std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "⚠️ Caught Exception: " << e.what()<< std::endl;
    }

    try
    {
        std::cout << "❌ Non-existent Key in Existing Section: "
                  << config.get_string_value("Control", "FakeKey")<< std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "⚠️ Caught Exception: " << e.what()<< std::endl;
    }
}

void test_writing(IniFile &config)
{
    std::cout << std::endl << "📝 Testing Write Operations:" << std::endl;

    config.set_bool_value("Control", "Transmit", true);
    config.set_int_value("Common", "TX Power", 30);
    config.set_double_value("Extended", "PPM", 1.23);
    config.set_string_value("Common", "Call Sign", "TEST123");

    config.set_string_value("NewSection", "NewKey", "NewValue");

    // Will only sav sections that exist
    config.commit_changes();

    std::cout << "✅ Test write complete." << std::endl;
}

void test_exceptions(IniFile &config)
{
    std::cout << std::endl << "🔎 Testing Wsprry Pi INI Exception Processing" << std::endl;
    try
    {
        std::cout << std::endl << "❌ Reading get_string_value() [Section Error]" << std::endl;
        config.get_string_value("Bad Section", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: Caught Exception: " <<  e.what() << std::endl;
    }

    try
    {
        std::cout << std::endl << "❌ Reading get_bool_value() [Section Error]" << std::endl;
        config.get_bool_value("Bad Section", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: Caught Exception: " << e.what() << std::endl;
    }

    try
    {
        std::cout << std::endl << "❌ Reading get_string_value() [Key Error]" << std::endl;
        config.get_string_value("Common", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: Caught Exception: " << e.what() << std::endl;
    }

    try
    {
        std::cout << std::endl << "❌ Reading get_int_value() [Key Error]" << std::endl;
        config.get_int_value("Common", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: Caught Exception: " << e.what() << std::endl;
    }

    try
    {
        std::cout << std::endl << "❌ Reading get_double_value(0 [Key Error]" << std::endl;
        config.get_double_value("Common", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: Caught Exception: " << e.what() << std::endl;
    }

    // Change PPM to "1e500" in the INI to simulate the error
    try
    {
        std::cout << std::endl << "✅ PPM (see comments to force error): " << config.get_double_value("Extended", "PPM")<< std::endl;
    }
    catch (const std::exception &e)
    {
        std::cout << std::endl << "❌ Reading get_double_value() [stod() Error]" << std::endl;
        std::cerr << "ERROR: Caught Exception: " << e.what() << std::endl;
    }
}

int main()
{
    auto &iniFile = IniFile::instance();
    // Set the filename
    iniFile.set_filename(filename);

    test_reading(iniFile);
    // test_writing(iniFile);
    // test_malformed_entries(ini);
    // test_exceptions(ini);

    return 0;
}
