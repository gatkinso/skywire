//
//   Copyright (C) 2017 Geoffrey Atkinson
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

#pragma once

#include <string>
#include "result_presenter.h"

namespace skyloupe 
{

class FileWriter : public ResultPresenter
{
public:
    FileWriter() = delete;
    FileWriter(const std::string& pathname, const std::string& result_string);
    virtual ~FileWriter() = default;

    bool present(int& errorcode) override;

private:
    bool write_file(int& errorcode);

    const std::string& result_string_;
    const std::string& pathname_;
};

}
