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

#include <fstream>
#include <iostream>
#include "file_writer.h"
#include "errorcodes.h"

namespace skyloupe
{

FileWriter::FileWriter(const std::string& pathname, const std::string& result_string)
: pathname_(pathname)
, result_string_(result_string) { }


bool FileWriter::present(int& errorcode)
{
    return write_file(errorcode);
}

bool FileWriter::write_file(int& errorcode)
{
    std::ofstream out_file(pathname_, std::ofstream::out | std::ofstream::trunc);

    if (false == out_file.is_open())
    {
        errorcode |= SW_FILEWRITEERROR;
        std::cerr << "Cannot open " << pathname_ << " for writing" << std::endl;
        return false;
    }

    out_file.write(result_string_.c_str(), result_string_.size());
    out_file.close();

    if (out_file.fail())
    {
        errorcode |= SW_FILEWRITEERROR;
        std::cerr << "Failed writing " << pathname_ << std::endl;
        return false;
    }

    return true;
}

}