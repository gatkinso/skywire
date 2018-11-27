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

#ifndef __wiregen_endian_h
#define __wiregen_endian_h

#include <stdint.h>

//
// Credit to Eric Raymond: http://http://esr.ibiblio.org/?p=5095
//
#define SW_IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)

#endif // __wiregen_endian_h

