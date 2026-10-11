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

#pragma once

#include <string>
#include "ctestgenerator.h"
#include "cpptestgenerator.h"
#include "gotestgenerator.h"
#include "javatestgenerator.h"
#include "py3testgenerator.h"
#include "swifttestgenerator.h"

namespace skyloupe
{

// Relay generators emit a small command-line program, written to the spec's
// outputrelayfilename, used by the cross-language serial test:
//
//     relay <input file> <output file>
//
// The relay reads a serialized message, checks the input is exactly size()
// bytes, deserializes it, checks every member against the shared test values
// (test_values.h) and Equals(), serializes it again, checks the output bytes
// are identical to the input, and writes them to the output file. It prints
// "FAIL <lang>: ..." for each failed check and exits 1, or exits 0 after
// printing "PASS <lang>: ...". Each relay derives from its language's test
// generator to reuse the code that sets the test values.

class CRelayGenerator : public CTestGenerator
{
public:
    CRelayGenerator(Specification& mspec, Specification& lspec) : CTestGenerator(mspec, lspec) {}
    bool generate(int& errorcode) override;
};

class CppRelayGenerator : public CppTestGenerator
{
public:
    CppRelayGenerator(Specification& mspec, Specification& lspec) : CppTestGenerator(mspec, lspec) {}
    bool generate(int& errorcode) override;
};

class GoRelayGenerator : public GoTestGenerator
{
public:
    GoRelayGenerator(Specification& mspec, Specification& lspec) : GoTestGenerator(mspec, lspec) {}
    bool generate(int& errorcode) override;
};

class JavaRelayGenerator : public JavaTestGenerator
{
public:
    JavaRelayGenerator(Specification& mspec, Specification& lspec) : JavaTestGenerator(mspec, lspec) {}
    bool generate(int& errorcode) override;
};

class Py3RelayGenerator : public Py3TestGenerator
{
public:
    Py3RelayGenerator(Specification& mspec, Specification& lspec) : Py3TestGenerator(mspec, lspec) {}
    bool generate(int& errorcode) override;
};

class SwiftRelayGenerator : public SwiftTestGenerator
{
public:
    SwiftRelayGenerator(Specification& mspec, Specification& lspec) : SwiftTestGenerator(mspec, lspec) {}
    bool generate(int& errorcode) override;
};

}
