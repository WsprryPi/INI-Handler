/*
 * main.cpp
 *
 * Standalone test harness for the IniFile class. This file intentionally uses
 * regular implementation comments rather than API Doxygen because the public
 * interface is documented in ini_file.hpp.
 */

#include "ini_file.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

namespace
{
    const std::string kSystemIni = "/usr/local/etc/wsprrypi.ini";
    const std::string kFallbackIni = "../test/test.ini";
    const std::string kTestDir = "../test";
    const std::string kBaselineIni = "../test/wsprrypi.baseline.ini";
    const std::string kBaselineStockIni = "../test/wsprrypi.baseline.ini.stock";
    const std::string kTestIni = "../test/wsprrypi.test.ini";
    const std::string kTestStockIni = "../test/wsprrypi.test.ini.stock";
    const std::string kSemanticVersion = "9.9.9-test";

    void remove_if_exists(const fs::path &path)
    {
        std::error_code ec;
        if (fs::exists(path, ec) && !fs::remove(path, ec))
        {
            throw std::runtime_error("Cannot remove file " + path.string() +
                                     ".");
        }
    }

    void copy_file_or_throw(const fs::path &source, const fs::path &destination)
    {
        std::error_code ec;
        fs::copy_file(source,
                      destination,
                      fs::copy_options::overwrite_existing,
                      ec);
        if (ec)
        {
            throw std::runtime_error("Cannot copy file from " +
                                     source.string() + " to " +
                                     destination.string() + ".");
        }
    }

    void require_file_contains(const fs::path &path, const std::string &text)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("Cannot open file " + path.string() +
                                     " for verification.");
        }

        std::string contents((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());

        if (contents.find(text) == std::string::npos)
        {
            throw std::runtime_error("Expected text not found in " +
                                     path.string() + ".");
        }
    }

    fs::path get_source_ini()
    {
        const fs::path system_ini(kSystemIni);
        const fs::path fallback_ini(kFallbackIni);

        if (fs::exists(system_ini))
        {
            return system_ini;
        }

        if (fs::exists(fallback_ini))
        {
            return fallback_ini;
        }

        throw std::runtime_error("No source INI file found at " +
                                 system_ini.string() + " or " +
                                 fallback_ini.string() + ".");
    }

    fs::path get_source_stock_ini(const fs::path &source_ini)
    {
        const fs::path system_stock_ini = fs::path(kSystemIni + ".stock");
        const fs::path fallback_stock_ini = fs::path(kFallbackIni + ".stock");

        if (fs::exists(system_stock_ini))
        {
            return system_stock_ini;
        }

        if (fs::exists(fallback_stock_ini))
        {
            return fallback_stock_ini;
        }

        return source_ini;
    }

    void prepare_test_environment()
    {
        const fs::path test_dir(kTestDir);
        const fs::path baseline_ini(kBaselineIni);
        const fs::path baseline_stock_ini(kBaselineStockIni);
        const fs::path test_ini(kTestIni);
        const fs::path test_stock_ini(kTestStockIni);

        std::error_code ec;
        fs::create_directories(test_dir, ec);
        if (ec)
        {
            throw std::runtime_error("Cannot create test directory " +
                                     test_dir.string() + ".");
        }

        remove_if_exists(baseline_ini);
        remove_if_exists(baseline_stock_ini);
        remove_if_exists(test_ini);
        remove_if_exists(test_stock_ini);

        const fs::path source_ini = get_source_ini();
        const fs::path source_stock_ini = get_source_stock_ini(source_ini);

        copy_file_or_throw(source_ini, baseline_ini);
        copy_file_or_throw(source_stock_ini, baseline_stock_ini);
        copy_file_or_throw(baseline_ini, test_ini);
        copy_file_or_throw(baseline_stock_ini, test_stock_ini);
    }

    void restore_known_good_state(IniFile &config)
    {
        const fs::path baseline_ini(kBaselineIni);
        const fs::path baseline_stock_ini(kBaselineStockIni);
        const fs::path test_ini(kTestIni);
        const fs::path test_stock_ini(kTestStockIni);

        copy_file_or_throw(baseline_ini, test_ini);
        copy_file_or_throw(baseline_stock_ini, test_stock_ini);
        config.set_filename(test_ini.string());
    }

    void cleanup_test_environment()
    {
        remove_if_exists(fs::path(kTestIni));
        remove_if_exists(fs::path(kTestStockIni));
        remove_if_exists(fs::path(kBaselineIni));
        remove_if_exists(fs::path(kBaselineStockIni));
    }

    class TestFileGuard
    {
    public:
        TestFileGuard() = default;

        ~TestFileGuard()
        {
            try
            {
                cleanup_test_environment();
            }
            catch (const std::exception &e)
            {
                std::cerr << "Cleanup warning: " << e.what() << std::endl;
            }
        }

        TestFileGuard(const TestFileGuard &) = delete;
        TestFileGuard &operator=(const TestFileGuard &) = delete;
    };

    void print_header(const std::string &title)
    {
        std::cout << std::endl
                  << title << std::endl;
    }
}

void test_malformed_entries(IniFile &config)
{
    print_header("Testing malformed INI entries.");

    try
    {
        config.set_string_value("Common", "TX Power", "abc");
        const int tx_power = config.get_int_value("Common", "TX Power");
        std::cout << "TX Power after setting invalid value: " << tx_power
                  << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception for malformed TX Power: "
                  << e.what() << std::endl;
    }

    try
    {
        config.set_string_value("Extended", "PPM", "xyz");
        const double ppm = config.get_double_value("Extended", "PPM");
        std::cout << "PPM after setting invalid value: " << ppm << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception for malformed PPM: "
                  << e.what() << std::endl;
    }
}

void test_reading(IniFile &config, const std::string &filename)
{
    print_header("Testing read operations on: " + filename);

    std::cout << std::boolalpha;
    std::cout << "Control  | Transmit: "
              << config.get_bool_value("Control", "Transmit") << std::endl;

    std::cout << "Common   | Call Sign: "
              << config.get_string_value("Common", "Call Sign") << std::endl;
    std::cout << "Common   | Grid Square: "
              << config.get_string_value("Common", "Grid Square")
              << std::endl;
    std::cout << "Common   | TX Power: "
              << config.get_int_value("Common", "TX Power") << std::endl;
    std::cout << "Common   | Frequency: "
              << config.get_string_value("Common", "Frequency") << std::endl;
    std::cout << "Common   | Transmit Pin: "
              << config.get_int_value("Common", "Transmit Pin") << std::endl;

    std::cout << "Extended | PPM: "
              << config.get_double_value("Extended", "PPM") << std::endl;
    std::cout << "Extended | Use NTP: "
              << config.get_bool_value("Extended", "Use NTP") << std::endl;
    std::cout << "Extended | Offset: "
              << config.get_bool_value("Extended", "Offset") << std::endl;
    std::cout << "Extended | Use LED: "
              << config.get_bool_value("Extended", "Use LED") << std::endl;
    std::cout << "Extended | LED Pin: "
              << config.get_int_value("Extended", "LED Pin") << std::endl;
    std::cout << "Extended | Power Level: "
              << config.get_int_value("Extended", "Power Level") << std::endl;

    std::cout << "Server   | Web Port: "
              << config.get_int_value("Server", "Web Port") << std::endl;
    std::cout << "Server   | Socket Port: "
              << config.get_int_value("Server", "Socket Port") << std::endl;
    std::cout << "Server   | Use Shutdown: "
              << config.get_bool_value("Server", "Use Shutdown")
              << std::endl;
    std::cout << "Server   | Shutdown Button: "
              << config.get_int_value("Server", "Shutdown Button")
              << std::endl;

    try
    {
        std::cout << "Reading missing section." << std::endl;
        config.get_string_value("NonExistent", "Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
    }

    try
    {
        std::cout << "Reading missing key in existing section." << std::endl;
        config.get_string_value("Control", "FakeKey");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
    }
}

void test_writing(IniFile &config, const std::string &filename)
{
    print_header("Testing write operations.");

    config.set_bool_value("Control", "Transmit", true);
    config.set_int_value("Common", "TX Power", 30);
    config.set_double_value("Extended", "PPM", 1.23);
    config.set_string_value("Common", "Call Sign", "TEST123");
    config.set_string_value("NewSection", "NewKey", "NewValue");

    config.commit_changes();

    std::cout << "Write test complete." << std::endl;

    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Cannot open file for verification: " +
                                 filename + ".");
    }

    std::string contents((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    if (contents.find("[NewSection]") != std::string::npos ||
        contents.find("NewKey") != std::string::npos)
    {
        throw std::runtime_error("NewSection/NewKey was written but should "
                                 "not have been.");
    }

    std::cout << "Verified: NewSection/NewKey was not written."
              << std::endl;
}

void test_exceptions(IniFile &config)
{
    print_header("Testing INI exception processing.");

    try
    {
        std::cout << "Reading get_string_value() with bad section."
                  << std::endl;
        config.get_string_value("Bad Section", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }

    try
    {
        std::cout << "Reading get_bool_value() with bad section."
                  << std::endl;
        config.get_bool_value("Bad Section", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }

    try
    {
        std::cout << "Reading get_string_value() with bad key."
                  << std::endl;
        config.get_string_value("Common", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }

    try
    {
        std::cout << "Reading get_int_value() with bad key." << std::endl;
        config.get_int_value("Common", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }

    try
    {
        std::cout << "Reading get_double_value() with bad key."
                  << std::endl;
        config.get_double_value("Common", "Bad Key");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }

    try
    {
        std::cout << "Reading get_double_value() for PPM." << std::endl;
        std::cout << "PPM: "
                  << config.get_double_value("Extended", "PPM")
                  << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
}

void test_reset_to_stock(IniFile &config, const std::string &filename)
{
    print_header("Testing reset_to_stock().");

    config.set_string_value("Common", "Call Sign", "RESETME");
    config.set_int_value("Common", "TX Power", 42);
    config.commit_changes();

    std::cout << "Modified values before reset." << std::endl;
    std::cout << "Common   | Call Sign: "
              << config.get_string_value("Common", "Call Sign") << std::endl;
    std::cout << "Common   | TX Power: "
              << config.get_int_value("Common", "TX Power") << std::endl;

    config.reset_to_stock(kSemanticVersion);

    std::cout << "Values after reset." << std::endl;
    std::cout << "Common   | Call Sign: "
              << config.get_string_value("Common", "Call Sign") << std::endl;
    std::cout << "Common   | TX Power: "
              << config.get_int_value("Common", "TX Power") << std::endl;

    require_file_contains(filename, kSemanticVersion);
    std::cout << "Semantic version token replacement verified."
              << std::endl;

    const std::string reset_call_sign =
        config.get_string_value("Common", "Call Sign");
    if (reset_call_sign == "RESETME")
    {
        throw std::runtime_error("reset_to_stock() did not restore Call Sign.");
    }

    const int reset_tx_power = config.get_int_value("Common", "TX Power");
    if (reset_tx_power == 42)
    {
        throw std::runtime_error("reset_to_stock() did not restore TX Power.");
    }

    std::cout << "reset_to_stock() verification passed." << std::endl;
}

void test_repair_from_stock(IniFile &config, const std::string &filename)
{
    print_header("Testing repair_from_stock().");

    config.set_string_value("Common", "Call Sign", "REPAIRME");
    config.set_int_value("Common", "TX Power", 33);
    config.set_double_value("Extended", "PPM", 2.5);
    config.set_bool_value("Control", "Transmit", true);
    config.commit_changes();

    std::ofstream damaged_file(filename, std::ios::trunc);
    if (!damaged_file.is_open())
    {
        throw std::runtime_error("Cannot open damaged test ini file " +
                                 filename + ".");
    }

    damaged_file
        << "; Created for WsprryPi version broken\n"
        << "[Control]\n"
        << "Transmit = True\n"
        << "\n"
        << "[Common]\n"
        << "Call Sign = REPAIRME\n"
        << "TX Power = 33\n"
        << "\n"
        << "[Extended]\n"
        << "PPM = 2.5\n";
    damaged_file.close();

    config.set_filename(filename);

    config.repair_from_stock(kSemanticVersion);

    std::cout << "Values after repair." << std::endl;
    std::cout << "Control  | Transmit: "
              << config.get_bool_value("Control", "Transmit") << std::endl;
    std::cout << "Common   | Call Sign: "
              << config.get_string_value("Common", "Call Sign") << std::endl;
    std::cout << "Common   | TX Power: "
              << config.get_int_value("Common", "TX Power") << std::endl;
    std::cout << "Common   | Grid Square: "
              << config.get_string_value("Common", "Grid Square")
              << std::endl;
    std::cout << "Extended | PPM: "
              << config.get_double_value("Extended", "PPM") << std::endl;
    std::cout << "Server   | Web Port: "
              << config.get_int_value("Server", "Web Port") << std::endl;
    std::cout << "Server   | Socket Port: "
              << config.get_int_value("Server", "Socket Port") << std::endl;

    require_file_contains(filename, kSemanticVersion);
    std::cout << "Semantic version token replacement verified."
              << std::endl;

    if (config.get_string_value("Common", "Call Sign") != "REPAIRME")
    {
        throw std::runtime_error("repair_from_stock() did not preserve "
                                 "Call Sign.");
    }

    if (config.get_int_value("Common", "TX Power") != 33)
    {
        throw std::runtime_error("repair_from_stock() did not preserve "
                                 "TX Power.");
    }

    if (config.get_double_value("Extended", "PPM") != 2.5)
    {
        throw std::runtime_error("repair_from_stock() did not preserve PPM.");
    }

    if (!config.get_bool_value("Control", "Transmit"))
    {
        throw std::runtime_error("repair_from_stock() did not preserve "
                                 "Transmit.");
    }

    if (config.get_string_value("Common", "Grid Square") != "ZZ99")
    {
        throw std::runtime_error("repair_from_stock() did not restore "
                                 "Grid Square from stock.");
    }

    if (config.get_int_value("Server", "Web Port") != 31415)
    {
        throw std::runtime_error("repair_from_stock() did not restore "
                                 "Web Port.");
    }

    if (config.get_int_value("Server", "Socket Port") != 31416)
    {
        throw std::runtime_error("repair_from_stock() did not restore "
                                 "Socket Port.");
    }

    std::cout << "repair_from_stock() verification passed." << std::endl;
}

template <typename TestFunc>
void run_test(IniFile &config,
              const std::string &test_name,
              TestFunc test_func)
{
    std::cout << std::endl
              << "Preparing known-good state for " << test_name << "."
              << std::endl;
    restore_known_good_state(config);
    test_func();
}

int main()
{
    try
    {
        const TestFileGuard guard;
        prepare_test_environment();

        auto &ini_file = IniFile::instance();
        ini_file.set_filename(kTestIni);

        run_test(ini_file,
                 "test_reading()",
                 [&]()
                 {
                     test_reading(ini_file, kTestIni);
                 });

        run_test(ini_file,
                 "test_writing()",
                 [&]()
                 {
                     test_writing(ini_file, kTestIni);
                 });

        run_test(ini_file,
                 "test_malformed_entries()",
                 [&]()
                 {
                     test_malformed_entries(ini_file);
                 });

        run_test(ini_file,
                 "test_exceptions()",
                 [&]()
                 {
                     test_exceptions(ini_file);
                 });

        run_test(ini_file,
                 "test_reset_to_stock()",
                 [&]()
                 {
                     test_reset_to_stock(ini_file, kTestIni);
                 });

        run_test(ini_file,
                 "test_repair_from_stock()",
                 [&]()
                 {
                     test_repair_from_stock(ini_file, kTestIni);
                 });

        std::cout << std::endl
                  << "All tests passed." << std::endl;

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Test harness failed: " << e.what() << std::endl;
        return 1;
    }
}
