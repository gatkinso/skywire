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

#include <regex>
#include <map>
#include <string>
#include "swiftgenerator.h"

std::string basestring = "/// This is a generated Swift file \
\n \
\npublic class __CLASSNAME__ { \
\n    /*PUBLIC_MEMBERS*/ \
\n \
\n    init() { \
\n        /*INITIALIZE_MEMBERS*/ \
\n    } \
\n \
\n    // Serialize method \
\n    public func serialize() -> Data { \
\n        var data = Data() \
\n        /*SERIALIZE_BODY*/ \
\n        return data \
\n    } \
\n \
\n    // Deserialize method \
\n    public func deserialize(_ data: Data) -> Bool { \
\n        /*DESERIALIZE_BODY*/ \
\n        return true \
\n    } \
\n \
\n    // Equals method \
\n    public func Equals(_ other: __CLASSNAME__) -> Bool { \
\n        /*EQUALS_BODY*/ \
\n        return true \
\n    } \
\n \
\n    // Size method \
\n    public func Size() -> UInt64 { \
\n        var size: UInt64 = 0 \
\n        /*SIZE_BODY*/ \
\n        return size \
\n    } \
\n}";

namespace skyloupe
{

bool SwiftGenerator::generate(int& errorcode)
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

bool SwiftGenerator::do_header(int& errorcode)
{
    return true;
}

bool SwiftGenerator::do_implementation(int& errorcode)
{
    std::string& imp_string = results_["implementation"];
    imp_base(errorcode);

    imp_string.append(ss_.str());

    return true;
}

bool SwiftGenerator::imp_base(int& errorcode)
{
    auto s = std::regex_replace(basestring, std::regex("__CLASSNAME__"), classname_);

    ss_ << s;

    return true;
}

}