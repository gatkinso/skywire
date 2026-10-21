//
//   Copyright (C) 2026 Geoffrey Atkinson
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

#include <cctype>
#include "naming.h"

namespace skyloupe
{

std::string snake_case(const std::string& name)
{
    auto upper = [](char c) { return std::isupper(static_cast<unsigned char>(c)) != 0; };
    auto lower_or_digit = [](char c) {
        return std::islower(static_cast<unsigned char>(c)) != 0 || std::isdigit(static_cast<unsigned char>(c)) != 0;
    };

    std::string snake;
    for (size_t i = 0; i < name.size(); i++)
    {
        const char c = name[i];
        if (i > 0 && upper(c) && snake.back() != '_')
        {
            const bool after_word = lower_or_digit(name[i - 1]);
            const bool ends_acronym = upper(name[i - 1]) && i + 1 < name.size() && lower_or_digit(name[i + 1]);
            if (after_word || ends_acronym)
            {
                snake += '_';
            }
        }
        snake += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    return snake;
}

}
