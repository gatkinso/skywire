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

#pragma once

#include <cstdint>
#include <vector>

#include "cstream.h"

namespace fuzz
{

// A C stream whose unread bytes are a copy of the input, held in storage (so
// not to be cstream_fini'd), as when an application wraps a received buffer.
// The copy gives ASan exact bounds.
inline Cstream input_stream(const uint8_t* data, size_t size, std::vector<char>& storage)
{
    storage.assign(data, data + size);
    Cstream in;
    in.buffer_ = storage.data();
    in.size_ = storage.size();
    in.head_ = in.buffer_;
    in.tail_ = in.buffer_ + size;
    return in;
}

}
