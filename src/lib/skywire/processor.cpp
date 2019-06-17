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
#include "generator_factory.h"

namespace skyloupe
{

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

    std::string language = msg_stencil_.get_value_string(language_key_);
    std::string language_file = msg_stencil_.get_value_string(language_file_key_);

    std::unique_ptr<skyloupe::Generator> genny;
    if (false == GeneratorFactory::machine(genny, language, msg_stencil_))
    {
        errorcode = SW_ERROR;
        return false;
    }

    success = do_code(genny, language_file, errorcode);
    if (success && generate_tests)
    {
        std::unique_ptr<skyloupe::Generator> test_genny;
        if (false == GeneratorFactory::machine(test_genny, language, msg_stencil_))
        success = do_tests(test_genny, outputdir, language_file, errorcode);
    }

    if (success)
    {
        success = do_results(genny, outputdir, language, errorcode);
    }

    return success;
}

bool Processor::process_string(const std::string& msg_json, const std::string& outputdir, int& errorcode, bool generate_tests)
{
    errorcode = SW_SUCCESS;
    bool success = false;

    if (msg_json.empty())
    {
        errorcode |= SW_BLANKINPUTSTRING;
        std::cerr << "Blank input string" << std::endl;
        return success;
    }

    if (false == msg_stencil_.read_string(msg_json))
    {
        errorcode |= SW_FILEREADERROR;
        std::cerr << "Error reading input string: " << msg_json << std::endl;
        return success;
    }

    std::string language = msg_stencil_.get_value_string(language_key_);
    std::string language_file = msg_stencil_.get_value_string(language_file_key_);

    std::unique_ptr<skyloupe::Generator> genny;
    if (false == GeneratorFactory::machine(genny, language, msg_stencil_))
    {
        errorcode = SW_ERROR;
        return false;
    }

    success = do_code(genny, language_file, errorcode);
    if (success && generate_tests)
    {
        std::unique_ptr<skyloupe::Generator> test_genny;
        if (false == GeneratorFactory::machine(test_genny, language, msg_stencil_))
        success = do_tests(test_genny, outputdir, language_file, errorcode);
    }

    if (success)
    {
        success = do_results(genny, outputdir, language, errorcode);
    }

    return success;
}

bool Processor::do_tests(std::unique_ptr<skyloupe::Generator>& genny, const std::string& outputdir, const std::string& language_file, int& errorcode)
{
    genny->set_lang_settings(language_file);
    bool success = genny->generate(errorcode);

    if (success)
    {
        Generator::results_t results = genny->get_results();
        if (results.empty() == false)
        {
            std::string filename_str = msg_stencil_.get_value_string("outputtestfilename");
            std::string pathname_str = outputdir + "/" + filename_str;
            const std::string& contents_str = results[0];

            std::unique_ptr<skyloupe::ResultPresenter> ressie;
            ressie = std::unique_ptr<skyloupe::FileWriter>(new skyloupe::FileWriter(pathname_str, contents_str));
            ressie->present(errorcode);
        }
    }

    return success;
}

bool Processor::do_code(std::unique_ptr<skyloupe::Generator>& genny, const std::string& language_file, int& errorcode)
{
    bool success = genny->generate(errorcode);

    return success;     
}

bool Processor::do_results(std::unique_ptr<skyloupe::Generator>& genny, const std::string& outputdir, const std::string& language, int& errorcode)
{
    bool success = true;

    if (language == ada_str_)
    {

    }
    else if (language == c_str_)
    {
        Generator::results_t results = genny->get_results();
        if (results.empty() == false)
        {
            std::string filename_str = msg_stencil_.get_value_string("outputheaderfilename");
            std::string pathname_str = outputdir + "/" + filename_str;
            const std::string& contents_h_str = results[0];
            const std::string& contents_c_str = results[1];

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
    else if (language == cpp_str_)
    {
        Generator::results_t results = genny->get_results();
        if (results.empty() == false)
        {
            std::string filename_str = msg_stencil_.get_value_string("outputheaderfilename");
            std::string pathname_str = outputdir + "/" + filename_str;
            const std::string& contents_str = results[0];

            std::unique_ptr<skyloupe::ResultPresenter> ressie;
            ressie = std::unique_ptr<skyloupe::FileWriter>(new skyloupe::FileWriter(pathname_str, contents_str));
            ressie->present(errorcode);
        }
    }
    else if (language == java_str_)
    {

    }
    else if (language == py_str_)
    {

    }
    else
    {
        errorcode |= SW_UNKNOWNLANG;
        std::cerr << "Unsupported language: " << language << std::endl;
        return false;
    }

    return success;
}

}