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

#pragma once

const unsigned int SW_SUCCESS                   = 0x00000000;
const unsigned int SW_ERROR                     = 0x00000001;
const unsigned int SW_UNKNOWNLANG               = 0x00000002;
const unsigned int SW_FILEREADERROR             = 0x00000003;
const unsigned int SW_BLANKINPUTFILENAME        = 0x00000004;
const unsigned int SW_BLANKINPUTSTRING          = 0x00000005;
const unsigned int SW_STRINGREADERROR           = 0x00000006;
const unsigned int SW_FILEWRITEERROR            = 0x00000007;

const unsigned int SW_LANG_C                    = 0x01000000;
const unsigned int SW_LANG_CPP                  = 0x02000000;
                                                
const unsigned int SW_FILE_HEADER               = 0x00000100;
const unsigned int SW_FILE_IMP                  = 0x00000200;

const unsigned int SW_LANG_NOMEMBERS            = 0x00000010;

const unsigned int SW_LANG_C_INCLUDES           = 0x00000020;
const unsigned int SW_LANG_C_STRUCTNAME         = 0x00000040;
const unsigned int SW_LANG_C_HDR_SERIALIZE      = 0x00000080;
const unsigned int SW_LANG_C_HDR_DESERIALIZE    = 0x00000100;
const unsigned int SW_LANG_C_HDR_SIZE           = 0x00000200;
const unsigned int SW_LANG_C_HDR_GET            = 0x00000400;
const unsigned int SW_LANG_C_HDR_SET            = 0x00000800;
const unsigned int SW_LANG_C_HDR_EQUALS         = 0x00001000;
const unsigned int SW_LANG_C_IMP_SERIALIZE      = 0x00002000;
const unsigned int SW_LANG_C_IMP_DESERIALIZE    = 0x00004000;
const unsigned int SW_LANG_C_IMP_SIZE           = 0x00008000;
const unsigned int SW_LANG_C_IMP_GET            = 0x00010000;
const unsigned int SW_LANG_C_IMP_SET            = 0x00020000;
const unsigned int SW_LANG_C_IMP_EQUALS         = 0x00040000;

const unsigned int SW_LANG_CPP_NAMESPACE        = 0x00080000;
const unsigned int SW_LANG_CPP_PUBLIC           = 0x00100000;
const unsigned int SW_LANG_CPP_PRIVATE          = 0x00200000;
const unsigned int SW_LANG_CPP_CONSTRUCTOR      = 0x00400000;
const unsigned int SW_LANG_CPP_DESTRUCTOR       = 0x00800000;
const unsigned int SW_LANG_CPP_INCLUDE          = 0x01000000;

const unsigned int SW_LANG_IMP_CONS      = 0x00000001;
const unsigned int SW_LANG_IMP_PRINT     = 0x00000002;
const unsigned int SW_LANG_RUNTIME       = 0x00000003;
const unsigned int SW_LANG_STRUCTBLOCK   = 0x00000004;
