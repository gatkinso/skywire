//
//   Copyright (C) 2018 Skyloupe, LLC
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

#include <regex>
#include "javagenerator.h"

std::string basestringy ="\
\nimport java.nio.ByteBuffer; \
\nimport java.util.Arrays; \
\n \
\npublic class __CLASSNAME__ { \
\n    /*PUBLIC_MEMBERS*/ \
\n \
\n    public __CLASSNAME__() { \
\n         /*INITIALIZE_MEMBERS*/ \
\n    } \
\n \
\n    /*GETTERS_SETTERS*/ \
\n \
\n    // Serialize to byte array (little-endian) \
\n    public byte[] serialize() { \
\n        ByteBuffer buffer = ByteBuffer.allocate(8); \
\n        /*SERIALIZE_BODY*/ \
\n        return buffer.array(); \
\n    } \
\n \
\n    // Deserialize from byte array (little-endian) \
\n    public boolean deserialize(byte[] data) { \
\n        if (data == null) return false; \
\n        /*DESERIALIZE_BODY*/ \
\n        return true; \
\n    } \
\n \
\n    // Equals method \
\n    public boolean equals(__CLASSNAME__ other) { \
\n        if (other == null) return false; \
\n        /*EQUALS_BODY*/ \
\n.       return true; \
\n    } \
\n \
\n    // Size in bytes \
\n    public int size() { \
\n        int size = 0; \
\n        /*SIZE_BODY*/ \
\n        return size; \
\n    } \
\n}";


namespace skyloupe
{

bool JavaGenerator::generate(int& errorcode)
{
    errorcode = 0;

    results_.clear();

    classname_ = msg_spec_.get_value_string("classname", "@NOTFOUND");

    if (false == do_header(errorcode))
    {
        errorcode |= SW_FILE_HEADER;
        return false;
    }

    ss_.str("");

    if (false == do_implementation(errorcode))
    {
        errorcode |= SW_FILE_IMP;
        return false;
    }

    return true;
}

bool JavaGenerator::do_header(int& errorcode)
{
    return true;
}

bool JavaGenerator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];
    imp_base(errorcode);

    imp_string.append(ss_.str());

    return true;
}

bool JavaGenerator::imp_base(int& errorcode)
{
    auto s = std::regex_replace(basestringy, std::regex("__CLASSNAME__"), classname_);

    ss_ << s;

    return true;
}

}