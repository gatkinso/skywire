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

bool Processor::process_string(const std::string& input_str, std::map<std::string, std::string>& output_str_map, int& errorcode, bool generate_tests)
{
    return false;
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

    std::string lang = msg_stencil_.get_value_string(language_key_);

    std::unique_ptr<skyloupe::Generator> genny;
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

        if (success)
        {
            std::map<std::string, std::string> results = genny->get_results();
            if (results.empty() == false)
            {
                std::string filename_str = msg_stencil_.get_value_string("outputheaderfilename");
                std::string pathname_str = outputdir + "/" + filename_str;
                const std::string& contents_h_str = results["header"];
                const std::string& contents_c_str = results["implementation"];

                std::unique_ptr<skyloupe::ResultPresenter> ressie_h;
                ressie_h = std::unique_ptr<skyloupe::FileWriter>(new skyloupe::FileWriter(pathname_str, contents_h_str));
                ressie_h->present(errorcode);

                filename_str = msg_stencil_.get_value_string("outputcfilename");
                pathname_str = outputdir + "/" + filename_str;

                std::unique_ptr<skyloupe::ResultPresenter> ressie_c;
                ressie_c = std::unique_ptr<skyloupe::FileWriter>(new skyloupe::FileWriter(pathname_str, contents_c_str));
                ressie_c->present(errorcode);
            }
        }

        if (true == generate_tests)
        {
            std::unique_ptr<skyloupe::Generator> test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CTestGenerator(msg_stencil_));
            success = do_tests_file(test_genny, outputdir, language_file_, errorcode);
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

        if (success)
        {
            std::map<std::string, std::string> results = genny->get_results();
            if (results.empty() == false)
            {
                std::string filename_str = msg_stencil_.get_value_string("outputheaderfilename");
                std::string pathname_str = outputdir + "/" + filename_str;
                const std::string& contents_str = results["header"];

                std::unique_ptr<skyloupe::ResultPresenter> ressie;
                ressie = std::unique_ptr<skyloupe::FileWriter>(new skyloupe::FileWriter(pathname_str, contents_str));
                ressie->present(errorcode);
            }
        }

        if (true == generate_tests)
        {
            std::unique_ptr<skyloupe::Generator> test_genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CppTestGenerator(msg_stencil_));
            success = do_tests_file(test_genny, outputdir, language_file_, errorcode);
        }
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
    // end temporary

    return success;
}

bool Processor::do_tests_file(std::unique_ptr<skyloupe::Generator>& genny, const std::string& outputdir, const std::string& langfile, int& errorcode)
{
    if (language_file_.empty())
    {
        language_file_.append(msg_stencil_.get_value_string(language_file_key_, langfile));
    }

    genny->set_lang_settings(language_file_);
    bool success = genny->generate(errorcode);

    if (success)
    {
        std::map<std::string, std::string> results = genny->get_results();
        if (results.empty() == false)
        {
            std::string filename_str = msg_stencil_.get_value_string("outputtestfilename");
            std::string pathname_str = outputdir + "/" + filename_str;
            const std::string& contents_str = results["unittest"];

            std::unique_ptr<skyloupe::ResultPresenter> ressie;
            ressie = std::unique_ptr<skyloupe::FileWriter>(new skyloupe::FileWriter(pathname_str, contents_str));
            ressie->present(errorcode);
        }
    }

    return success;
}

bool Processor::do_tests_string(std::unique_ptr<skyloupe::Generator>& genny, std::map<std::string, std::string>& output_str_map, const std::string& langfile, int& errorcode)
{
    return false;
}

}