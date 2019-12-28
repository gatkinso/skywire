//
//   Copyright (C) 2017 Skyloupe, LLC
//   
//   Licensed under the Apache License, Version 2.0 (the "License").
//   You may not use this file except in compliance with the License.
//   A copy of the License is located at
//   
//   https://www.apache.org/licenses/LICENSE-2.0.html
//   
//   or in the "license" file accompanying this file. This file is distributed
//   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
//   express or implied. See the License for the specific language governing
//   permissions and limitations under the License.
//

#include "file_writer.h"
#include "processor.h"

namespace skyloupe
{

bool Processor::process_string(const std::string& input_str, results_t& output_str_map, int& errorcode, bool generate_tests)
{
    errorcode = SW_SUCCESS;
    bool success = false;

    if (input_str.empty())
    {
        errorcode |= SW_BLANKINPUTSTRING;
        std::cerr << "Blank input string" << std::endl;
        return success;
    }

    if (false == msg_stencil_.read_string(input_str))
    {
        errorcode |= SW_STRINGREADERROR;
        std::cerr << "Error reading string: " << input_str << std::endl;
        return success;
    }

    return process(output_str_map, errorcode, generate_tests);
}

bool Processor::process_file(const std::string& pathname, const std::string& outputdir, int& errorcode, bool generate_tests)
{
    errorcode = SW_SUCCESS;
    bool success = false;

    if (pathname.empty())
    {
        errorcode |= SW_BLANKINPUTFILENAME;
        std::cerr << "Blank input file name" << std::endl;
        return success;
    }

    if (false == msg_stencil_.read_file(pathname))
    {
        errorcode |= SW_FILEREADERROR;
        std::cerr << "Error reading file: " << pathname << std::endl;
        return success;
    }

    results_t output_str_map;
    success = process(output_str_map, errorcode, generate_tests);
    if (false == success)
    {
        return success;
    }

    if (false == output_str_map.empty())
    {
        do_file_write(outputdir, output_str_map, "outputheaderfilename", "header", errorcode);  //refactor - confusing
        do_file_write(outputdir, output_str_map, "outputimplfilename", "implementation", errorcode);
        do_file_write(outputdir, output_str_map, "outputtestfilename", "unittest", errorcode);
    }

    return success;
}

bool Processor::process(results_t& output_str_map, int& errorcode, bool generate_tests)
{
    bool success = false;
    std::string lang = msg_stencil_.get_value_string(language_key_);

    std::unique_ptr<skyloupe::Generator> genny;
    std::unique_ptr<skyloupe::Generator> test_genny;
    // temporary - replace with factory, dynamic language definition via json
    //             maybe a plugin for each language?
    if (lang == ada_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::AdaGenerator(msg_stencil_));
        success = genny->generate(errorcode);
    }
    else if (lang == c_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CGenerator(msg_stencil_));
        if (language_file_.empty())
        {
            language_file_.append(msg_stencil_.get_value_string(language_file_key_, "lang.c.json"));
        }

        genny->set_lang_settings(language_file_);
        success = genny->generate(errorcode);
        if (true == generate_tests && success)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CTestGenerator(msg_stencil_));
            test_genny->set_lang_settings(language_file_);
            success = test_genny->generate(errorcode);
        }
    }
    else if (lang == cpp_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CppGenerator(msg_stencil_));
        if (language_file_.empty())
        {
            language_file_.append(msg_stencil_.get_value_string(language_file_key_, "lang.cpp.json"));
        }

        genny->set_lang_settings(language_file_);
        success = genny->generate(errorcode);
        if (true == generate_tests && success)
        {
            test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CppTestGenerator(msg_stencil_));
            test_genny->set_lang_settings(language_file_);
            success = test_genny->generate(errorcode);
        }
    }
    else if (lang == go_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::GoGenerator(msg_stencil_));
        success = genny->generate(errorcode);
    }
    else if (lang == java_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::JavaGenerator(msg_stencil_));
        success = genny->generate(errorcode);
    }
    else if (lang == py_str_)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::PyGenerator(msg_stencil_));
        success = genny->generate(errorcode);
    }
    else
    {
        errorcode |= SW_UNKNOWNLANG;
        std::cerr << "Unsupported language: " << lang << std::endl;
        return success;
    }

    if (success)
    {
        auto t1 = genny->get_results();  // inefficient, refactor
        output_str_map.insert(t1.begin(), t1.end());
        if (true == generate_tests)
        {
            auto t2 = test_genny->get_results();
            output_str_map.insert(t2.begin(), t2.end());
        }
    }
    // end temporary

    return true;
}

void Processor::do_file_write(const std::string& outputdir, results_t& output_str_map, const std::string stencil_key, const std::string value_key, int& errorcode)
{
    std::string filename_str = msg_stencil_.get_value_string(stencil_key);
    auto value = output_str_map.find(value_key);

    if (false == filename_str.empty() && value != output_str_map.end())
    {
        std::string pathname_str = outputdir + "/" + filename_str;

        std::unique_ptr<skyloupe::ResultPresenter> presenter;
        presenter = std::unique_ptr<skyloupe::FileWriter>(new skyloupe::FileWriter(pathname_str, value->second));
        bool success = presenter->present(errorcode);
        if (false == success)
        {
            errorcode |= SW_FILEWRITEERROR;
        }
    }
}

}