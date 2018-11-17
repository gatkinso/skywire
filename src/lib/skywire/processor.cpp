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

bool Processor::process_file(const std::string& pathname, const std::string& outputdir, int& errorcode)
{
    bool success = false;
    if (false == msg_stencil_.read_file(pathname))
    {
        errorcode = 1;
        std::cerr << "Error reading file: " << pathname << std::endl;
        return success;
    }

    std::string lang = msg_stencil_.get_value_string(language_key);

    std::unique_ptr<skyloupe::Generator> genny;
    // temporary - replace with factory, dynamic language definition via json
    //             maybe a plugin for each language?
    if (lang == ada_str)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::AdaGenerator(msg_stencil_));
        success = genny->generate(errorcode);
    }
    else if (lang == c_str)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CGenerator(msg_stencil_));
        success = genny->generate(errorcode);
    }
    else if (lang == cpp_str)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::CppGenerator(msg_stencil_));
        genny->set_lang_settings("lang.cpp.json");
        success = genny->generate(errorcode);

        if (success)
        {
            Generator::results_t results = genny->get_results();
            if (results.empty() == false)
            {
                std::string filename_str = msg_stencil_.get_value_string("outputfilename");
                std::string pathname_str = outputdir + "/" + filename_str;
                const std::string& contents_str = results[0];

                std::unique_ptr<skyloupe::ResultPresenter> ressie;
                ressie = std::unique_ptr<skyloupe::FileWriter>(new skyloupe::FileWriter(pathname_str, contents_str));
                ressie->present(errorcode);
            }
        }
    }
    else if (lang == java_str)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::JavaGenerator(msg_stencil_));
        success = genny->generate(errorcode);
    }
    else if (lang == py_str)
    {
        genny = std::unique_ptr<skyloupe::Generator>(new skyloupe::PyGenerator(msg_stencil_));
        success = genny->generate(errorcode);
    }
    else
    {
        errorcode = 2;
        std::cerr << "Unsupported language: " << lang << std::endl;
        return success;
    }
    // end temporary



    return success;
}

}