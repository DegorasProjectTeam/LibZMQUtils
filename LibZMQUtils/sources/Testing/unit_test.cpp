

// C++ INCLUDES
// =====================================================================================================================
#include <future>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <list>
#include <map>
#include <vector>
// =====================================================================================================================

// ZMQUTILS INCLUDES
// =====================================================================================================================
#include "LibZMQUtils/Testing/unit_test.h"
#include "LibZMQUtils/InternalHelpers/container_helpers.h"
#include "LibZMQUtils/InternalHelpers/string_helpers.h"
// =====================================================================================================================

// ZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace testing{

using namespace zmqutils::internal_helpers;

TestLog::TestLog(const std::string& module, const std::string& test, const std::string& det_ex,
                 bool passed, const utils::TimePointStd& tp, long long elapsed) :
    module_(module),
    test_(test),
    det_ex_(det_ex),
    passed_(passed),
    tp_str_(utils::timePointToIso8601(tp, true, false)),
    elapsed_(elapsed)
{}

std::string TestLog::makeLog(const std::string& storage_path) const
{
    // Stream.
    std::stringstream stream;

    // Update colors.
    stream << (this->passed_ ? "\033[038;2;0;210;0m" : "\033[170;2;0;038;0m");

    // Log the result.
    stream << "[" << this->tp_str_ << "] ";
    stream << this->module_ << " - " << this->test_ << " ";
    std::string aux_str = formatResult();
    size_t dot_w = 50 - this->test_.size() - this->module_.size() - aux_str.size();
    stream << std::left << std::setw(static_cast<int>(dot_w)) << std::setfill('.') << "" << aux_str;

    // Add the elapsed time if any.
    stream << " [ET: " << this->elapsed_ << "us]";

    // Check if we have detailed error.
    if(!this->det_ex_.empty())
        stream << " [Except: " << this->det_ex_ << "]";

    // Restore the default color
    stream << "\x1b[0m";

    // Return the stream.
    return stream.str();
}

std::string TestLog::formatResult() const
{
    std::stringstream result;
    result << std::left << std::setw(4) << std::setfill('.')
           << (this->passed_ ? " PASS" : " FAIL");
    return result.str();
}

const std::string &TestLog::getModuleName() const{return this->module_;}

bool TestLog::getResult() const{return this->passed_;}

void UnitTest::runTests()
{
    // Separator.
    std::string sep = internal_helpers::strings::fillStr("=", 100) + "\n";

    // Log.
    std::cout<<"\033[38;2;255;128;0m"<<sep<<"=                                    ";
    std::cout << "EXECUTING UNIT TEST SESSION                                   =\n";
    std::cout<<"\033[38;2;255;128;0m"<<sep;

    // Iterate over the multimap in order of keys
    for (auto it = this->test_dict_.begin(); it != this->test_dict_.end();)
    {
        // Auxiliar containers.
        std::string c_module = it->first;
        auto range = this->test_dict_.equal_range(c_module);

        // Process all elements with the same key
        for (auto range_it = range.first; range_it != range.second; ++range_it)
        {
            // Auxiliar containers.
            TestBase* test = range_it->second;
            std::string det_ex;
            long long elapsed = 0;
            auto now_t = utils::TimePointStd::clock::now();
            bool result;

            // Log.
            std::cout<<"\033[38;2;255;128;0m"<<"Executing test: "<<"\033[038;2;0;140;255m"
                     <<test->test_name_<<"..."<<std::endl;

            // Async execution.
            std::future<long long> future =
                std::async(std::launch::async,
                           [test, &result]() -> long long
                           {
                               // Start time.
                               auto start = std::chrono::steady_clock::now();
                               // Run the test.
                               test->runTest();
                               // End time.
                               auto stop = std::chrono::steady_clock::now();
                               // Get the elapsed time.
                               auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
                               // Store the result.
                               result = test->result_;
                               return duration.count();
                           });

            // Wait for the asynchronous task to complete.
            future.wait();

            // Get the elapsed time.
            try{elapsed = future.get();}
            catch (const std::exception& e)
            {
                result = false;
                det_ex = e.what();
            }

            // Instantiate the test and store.
            TestLog t_log(c_module, test->test_name_, det_ex, result, now_t, elapsed);
            t_log.makeLog();
            this->summary_.addLog(t_log);
        }

        // Move the iterator to the next unique key
        it = range.second;
    }

    // Log.
    std::cout<<"\033[38;2;255;128;0m"<<"All tests executed!"<<std::endl;
    std::cout<<"\033[38;2;255;128;0m"<<sep<<std::endl;

    // Make the summary.
    this->summary_.makeSummary(true);
}

TestSummary::TestSummary():n_pass_(0),n_fail_(0){}

void TestSummary::setSessionName(const std::string &name){this->session_ = name;}

void TestSummary::addLog(const TestLog &log)
{
    this->test_logs_.insert(std::pair<std::string, TestLog>(log.getModuleName(), log));
    log.getResult() ? this->n_pass_++ : this->n_fail_++;
}

void TestSummary::clear()
{
    this->test_logs_.clear();
}

void TestSummary::makeSummary(bool show, const std::string& storage_path) const
{
    // Auxiliar containers.
    std::vector<std::string> keys = internal_helpers::containers::getMapKeys(this->test_logs_);
    std::stringstream title;
    std::stringstream border;
    std::string s_name = "= Session:  " + this->session_;
    std::string date = "= Datetime: " + utils::currentISO8601Date(true, false, true);
    std::string keys_str = "= Modules:  " + internal_helpers::strings::join(keys, " - ");
    std::string filename = this->session_;
    std::string date_file = utils::timePointToString(utils::TimePointStd::clock::now(),
                                                      "%Y%m%d_%H%M%S", false, false);
    filename = internal_helpers::strings::replaceStr(filename, " ", "");
    filename = internal_helpers::strings::replaceStr(filename, ":", "");
    filename = internal_helpers::strings::replaceStr(filename, "_", "-");
    filename = internal_helpers::strings::toLower(filename) + "_";
    filename += date_file + ".utsum";
    std::string file = "= File:     " + filename;
    std::string sep1 = internal_helpers::strings::fillStr("=", 100) + "\n";
    std::string sep2 = internal_helpers::strings::fillStr("-", 100) + "\n";

    std::stringstream test;
    std::stringstream pass;
    std::stringstream fail;
    std::stringstream all_pass;
    std::string all_pass_color = this->n_fail_ == 0 ? "\033[038;2;0;210;000m" : "\033[170;2;0;038;000m";
    std::string all_pass_str = this->n_fail_ == 0 ? "YES" : "NO ";

    test << "= Tests:    " << "\033[038;2;0;140;255m" << std::to_string(this->test_logs_.size()) << "\033[38;2;255;128;0m";
    pass << "= Passed:   " << "\033[038;2;0;210;000m" << std::to_string(this->n_pass_) << "\033[38;2;255;128;0m";
    fail << "= Failed:   " << "\033[170;2;0;038;000m" << std::to_string(this->n_fail_) << "\033[38;2;255;128;0m";
    all_pass << "= All Pass: " << all_pass_color << all_pass_str << "\033[38;2;255;128;0m";

    std::string pass_str = pass.str();

    // Get the keys.
    keys = internal_helpers::containers::getMapKeys(this->test_logs_);

    // Generate the header.
    title << "\033[38;2;255;128;0m";
    title << sep1;
    title << "=                                COMPLETE UNIT TEST SESSION SUMMARY                                =\n";
    title << sep1;
    title << s_name << std::string(99 - s_name.length(), ' ') << "=\n";
    title << date << std::string(99 - date.length(), ' ') << "=\n";
    title << keys_str << std::string(99 - keys_str.length(), ' ') << "=\n";
    title << file << std::string(99 - file.length(), ' ') << "=\n";
    title << test.str() << std::string(134 - test.str().length(), ' ') << "=\n";
    title << pass.str() << std::string(134 - pass.str().length(), ' ') << "=\n";
    title << fail.str() << std::string(134 - fail.str().length(), ' ') << "=\n";
    title << all_pass.str() << std::string(132 - fail.str().length(), ' ') << "=\n";
    title << sep1;
    title << "\n";

    border.fill('=');
    border.width(80);
    border << "\n";

    // Show the title.
    std::cerr<<title.str();

    // Iterate over the multimap in order of keys
    for (auto it = this->test_logs_.begin(); it != this->test_logs_.end();)
    {
        // Auxiliar containers.
        std::string c_module = it->first;
        auto range = this->test_logs_.equal_range(c_module);
        std::stringstream sep_mod;

        // Prepare the separator string.
        sep_mod << "\033[38;2;255;128;0m";
        sep_mod << sep2;
        sep_mod << "  TEST FOR MODULE: " << c_module << "\n";
        sep_mod << sep2;

        std::cerr << sep_mod.str();

        // Process all elements with the same key.
        for (auto range_it = range.first; range_it != range.second; ++range_it)
        {
            std::cerr << range_it->second.makeLog() << std::endl;
        }

        // Finish the section.
        std::cerr << "\033[38;2;255;128;0m";
        std::cerr << sep2 << "\n";

        // Move the iterator to the next unique key
        it = range.second;
    }

    // Finish and restore the default color.
    std::cerr << sep1;
    std::cerr << "\x1b[0m";
}

TestBase::TestBase(const std::string &name):
    test_name_(name),
    result_(true)
{}

bool TestBase::forceFail()
{
    return false;
}

void TestBase::runTest(){}

TestBase::~TestBase(){}

UnitTest &UnitTest::instance()
{
    static UnitTest uTest;
    return uTest;
}

void UnitTest::setSessionName(std::string &&session)
{
    this->session_ = std::move(session);
    this->summary_.setSessionName(this->session_);
}

void UnitTest::addTest(std::pair<std::string, TestBase *> p)
{
    this->test_dict_.insert(p);
}

void UnitTest::clear()
{
    this->test_dict_.clear();
    this->summary_.clear();
}

// =====================================================================================================================


}} // END NAMESPACES.
// =====================================================================================================================
