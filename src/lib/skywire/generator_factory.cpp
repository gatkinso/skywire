//
//   Copyright (C) 2019 Skyloupe, LLC
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

#include "generator_factory.h"
#include "generators.h"

namespace skyloupe
{

bool GeneratorFactory::machine(std::unique_ptr<Generator>& gen_ptr, 
                               const std::string type_str, skyloupe::Stencil& msg_stencil)
{
    std::string msglang = msg_stencil.get_value_string("language");

    auto lang = Generator::GeneratorTypes.find(msglang);

    if (lang == Generator::GeneratorTypes.end())
    {
        std::cerr << "Cound not determine language language: " << msglang << std::endl;
        return false;        
    }

    if (lang->second == "ada")
    {
        gen_ptr = std::unique_ptr<skyloupe::Generator>(new skyloupe::AdaGenerator(msg_stencil));
    }
    else if (lang->second == "c")
    {
        gen_ptr = std::unique_ptr<skyloupe::Generator>(new skyloupe::CGenerator(msg_stencil));
    }
    else if (lang->second == "c++")
    {
        gen_ptr = std::unique_ptr<skyloupe::Generator>(new skyloupe::CppGenerator(msg_stencil));
    }
    else if (lang->second == "java")
    {
        gen_ptr = std::unique_ptr<skyloupe::Generator>(new skyloupe::JavaGenerator(msg_stencil));
    }
    else if (lang->second == "python")
    {
        gen_ptr = std::unique_ptr<skyloupe::Generator>(new skyloupe::PyGenerator(msg_stencil));
    }
    else
    {
        std::cerr << "Unknown language: " << lang->second << std::endl;
        return false;
    }

    return true;
}

bool GeneratorFactory::tests_machine(std::unique_ptr<Generator>& gen_ptr, 
                                    const std::string type_str, skyloupe::Stencil& msg_stencil)
{
    std::string msglang = msg_stencil.get_value_string("language");

    auto lang = Generator::GeneratorTypes.find(msglang);

    if (lang == Generator::GeneratorTypes.end())
    {
        std::cerr << "Cound not determine language language: " << msglang << std::endl;
        return false;        
    }

    if (lang->second == "ada")
    {
        std::cerr << "Not yet implemented" << std::endl;
        return false;
    }
    else if (lang->second == "c")
    {
        gen_ptr = std::unique_ptr<skyloupe::Generator>(new skyloupe::CTestGenerator(msg_stencil));
    }
    else if (lang->second == "c++")
    {
        gen_ptr = std::unique_ptr<skyloupe::Generator>(new skyloupe::CppTestGenerator(msg_stencil));
    }
    else if (lang->second == "java")
    {
        std::cerr << "Not yet implemented" << std::endl;
        return false;
    }
    else if (lang->second == "python")
    {
        std::cerr << "Not yet implemented" << std::endl;
        return false;
    }
    else
    {
        std::cerr << "Unknown language: " << lang->second << std::endl;
        return false;
    }

    return true;
}

}