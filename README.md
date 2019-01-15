# Skywire - Multilanguage Message Serialization Generator

                                        )
            _                          /
         _..\'.___.--._______.----.__.'/
    ,---',>  _/                     .'
     '',____,. __.       ..--''.    (____
            \  .___)..--'       '..____. '._
            '.__)                      '.__)

mrf

Skywire (c) 2014-2018 Skyloupe, LLC.  All rights reserved.

# [1.0] Introduction

Welcome to Skywire!

In this repository you will find a simple yet surprisingly versatile message 
serialization generator for C and C++.

Based on Google Protocol Buffer technology, Skywire takes protocol buffers to 
a place it fears to tread:  namely, the ability to define a wire protocol.

Yes, that's right.  Bits.  Endianness.  Terrible things to behold.  Oh my.

Unlike generators like Protocol Buffers, the Skywire generator (known as 
"wiregen") is asymmetric:  Wiregen generated code only has to run on one 
side of the connection:  if you have an ICD for the component or device you 
are trying to communicate with, wiregen gives you the ability to define a 
"stencil" for a C++ class (or C struct) that can be used to generate code 
to interface with it.

The stencil itself is defined using a JSON object which is in turn serialized
into a Protocol Buffer message.  This message can be used as is in the
traditional Protocol Buffer manner, or a new C++ class (or C struct!) can be
generated that will give the developer the ability to serialize the message 
data "natively" for the targeted ICD or device.

The developer is given wide latitude in defining the manner and style of the 
generated code, so that the generated code can adhere to in-house naming and 
style conventions to the greatest degree possible.

In addition to this, wiregen can generate unit tests for the generated code.
While these are not necessary for generating messages using default settings,
it comes in very handy when the wiregen generator has been customized.  All 
unit tests are generated using the Google Test unit test framework.

## [1.1] A Note About Security

Currently the generated code targets a fully Open Source runtime called "Redchip."
Redchip is perfectly fine for most prototypes.  It is pretty fast and stable.
However Redchip is intended for prototyping and internal IPC:  not a lot of work 
has gone into Redchip to do things like check for errors, handle data corruption, 
or prevent buffer overruns.

To handle these more advanced conditions, a fee-based runtime called "Blackchip"
is being developed.  Blackchip will also provide generators do things like 
generate your Doxygen comments for you.

## [1.2] Source Code Repository

At the time of this writing, the Skywire source code can be found at:

https://github.com/skyloupe/skywire

# [2.0] Build Requirements

cmake version 3.6 or higher

## [2.1] Build Requirements (Windows)

One of the following development environment(s):

Visual Studio 15 Professional (or newer)

## [2.2] Build Requirements (Linux)

gcc 4.9 (or higher)

## [2.3] Build Requirements (OSX, MacOS)

XCode 8.1 (or newer)

# [3.0] Build Instructions

Skywire is heavily dependent on CMake.  No other build system is currently supported.
That said, we'd be happy to integrate others should there be enough requests for 
it.

## [3.1] Windows

	cmake -G "Visual Studio 14 2015 Win64" e:\source\skywire 

	cmake --build . --target install --config [Debug|Release|DebWithRelInfo|MinRelSize]

### [3.1.1] A note about building on Windows

What is it with Windows?  Why must already complex tasks be further muddied
with needless complexity?  Sigh...  it is a mystery.

That said, it is a mystery that we must embrace if we are to target the OS that
runs 90% of the computers on the planet.  CMake handles most of this for us, but
things become further muddled when another large company (that rhymes with "Google")
decides to "simplify" things by forcing certain compiler switches in their 
repositories... but by doing so in an inconsistent manner as is the case with 
Protocol Buffers and GTest.

Should you decide to build against either of these locally the following definition
should be invoked in CMake at build time:

-DCMAKE_PREFIX_PATH=c:\Users\username\apps\protobuf-3.6.1-debug;c:\Users\username\apps\gtest-1.8.1-debug

The paths of course must be craft to match your development harness.  Note that 
this can also be done for Linux and Mac builds hf need be. 

On Linux:
	cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=[Debug | Release] ~/source/skywire

    cmake --build . --target install

On Mac (using Xcode):
	cmake -G "Xcode" -DCMAKE_BUILD_TYPE=[Debug | Release] ~/source/skywire

You will have to modify the paths appropriately for your system.

The following cmake generators have been tested:

    cmake -G "Visual Studio 14 2015 Win64"
    cmake -G "Visual Studio 14 2015"
    cmake -G "Xcode"
    cmake -G "Unix Makefiles"

# [4.0] Running unit tests

The unit tests are written using Google Test and are managed with CTest.  After a successful build,
type

	ctest

to run them.

# [5.0] Deploying and running

    wiregen.exe --input_file e:\some_directory\MyClass.json --outdir E:\some_other_directory

# [6.0] Configuration files 

TODO

# [7.0] Syntax Overview

As stated above, the input to wiregen is a JSON file,  For example:

Invoking wiregen like so: (the example is Windows-centric)

    wiregen.exe --generate_tests --input_file e:\example.json --outdir e:\

with the file Example.json....

```
    {
       "stringValues":{  
          "classname":"Example",
          "outputheaderfilename":"Example.h",
          "outputtestfilename":"ExampleUnitTest.cpp",
          "namespace":"SomeNamespace",
          "language": "c++",
          "lang_file": "lang.cpp.json"
       },
       "members":{  
          "100":{  
             "type":"float",
             "name":"myfloat100",
             "endian":1
          },    
          "200":{  
             "type":"uint32_t",
             "name":"myuint32_200",
             "endian":0
          },
          "201":{  
             "type":"int64_t",
             "name":"myint64_201"
          },        
       },
       "includes":[  
          "\"cstream.h\"",
          "<stdint.h>"
       ]
    }
```

...will generate the following code:

```
    // This is the generated file (e:\Example .h)

    #pragma once

    #include "cstream.h"
    #include <stdint.h>

    namespace SomeNamespace {
        class Example : public skyloupe::Streamable {
        public:
            Example() {}
            virtual ~Example() {}
            bool serialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) const {
                str.serialize(myfloat100, skyloupe::Endianness::Little);
                str.serialize(myuint32_200, skyloupe::Endianness::Big);
                str.serialize(myint64_201, skyloupe::Endianness::Big);
                return true;
            }
            bool deserialize(skyloupe::Stream& str, skyloupe::Endianness endian = skyloupe::Endianness::Big) {
                str.deserialize(myfloat100, skyloupe::Endianness::Little);
                str.deserialize(myuint32_200, skyloupe::Endianness::Big);
                str.deserialize(myint64_201, skyloupe::Endianness::Big);
                return true;
            }
            size_t size() const {
                return(
                    sizeof(float) +
                    sizeof(uint32_t) +
                    sizeof(int64_t)
                );
            }
            bool operator==(const Example& other) const {
                if(this == &other) return true;
                return(
                    myfloat100 == other.myfloat100 &&
                    myuint32_200 == other.myuint32_200 &&
                    myint64_201 == other.myint64_201
                );
            }
            float Get_myfloat100() const { return myfloat100; }
            uint32_t Get_myuint32_200() const { return myuint32_200; }
            int64_t Get_myint64_201() const { return myint64_201; }
            void Set_myfloat100(const float newval) { myfloat100 = newval; }
            void Set_myuint32_200(const uint32_t newval) { myuint32_200 = newval; }
            void Set_myint64_201(const int64_t newval) { myint64_201 = newval; }
        private:
            float myfloat100;
            uint32_t myuint32_200;
            int64_t myint64_201;
        };
    }
```

...along with the generated unit test:

```
    // This is the generated file (e:\ExampleUnitTest.cpp)

    #include <gmock/gmock.h>
    #include <gtest/gtest.h>
    #include "Example.h"

    TEST(ExampleTest, SerDes)
    {
        SomeNamespace::Example in, out;
        skyloupe::Stream stream;

        in.Set_myfloat100(11.1f);
        in.Set_myuint32_200(14);
        in.Set_myint64_201(-30);

        bool success = in.serialize(stream);
        ASSERT_TRUE(success);

        success = out.deserialize(stream);
        ASSERT_TRUE(success);
    }

    TEST(ExampleTest, Size)
    {
        SomeNamespace::Example msg;

        ASSERT_EQ(msg.size(), sizeof(float)+sizeof(uint32_t)+sizeof(int64_t));
    }

    TEST(ExampleTest, EqualityOperator)
    {
        SomeNamespace::Example in, out;
        skyloupe::Stream stream;

        in.Set_myfloat100(11.1f);
        in.Set_myuint32_200(14);
        in.Set_myint64_201(-30);

        bool success = in.serialize(stream);
        ASSERT_TRUE(success);

        success = out.deserialize(stream);
        ASSERT_TRUE(success);

        ASSERT_EQ(in, out);
    }
```

Note the the runtime is located in <topsrc>/lang/cpp.  (This is where the skyloupe namespace 
stuff is defined.)

To generate c C code use this file:

````
{
   "stringValues":{  
      "classname":"Example",
      "outputheaderfilename":"Example.h",
      "outputcfilename":"Example.c",
      "namespace":"SomeNamespace",
      "language": "c",
      "lang_file": "lang.c.json",
      "outputtestfilename":"ExampleUnitTest.cpp"
   },
   "members":{  
      "100":{  
         "type":"float",
         "name":"myfloat100",
         "endian":1
      },    
      "200":{  
         "type":"uint32_t",
         "name":"myuint32_200",
         "endian":0
      },
      "201":{  
         "type":"int64_t",
         "name":"myint64_201"
      }
   },
   "includes":[  
      "\"cstream.h\"",
      "<stdint.h>"
   ]
}

````

Keep in mind the JSON values are case sensitive.  You should get the files:
    Example.c
    Example.h
    and Example.cpp (note that the Google Test unit tests are always C++).

Note that the C and C++ generates equivalent code.  See the generated_tests in the
source for verification of this.

# [7.1] Lang(uage) Files

Customization of generated code can be performed in the lang.xxx.json files.

Note the to generate appropriate code, the proper language must be specified in
the message JSON file.

More detail on this to come.

# [Appendix] Credits

This software is dedicated to all of the former racing Greyhounds who didn't
find a home after their racing "career" ended.  If you are considering getting 
a dog, please consider rescuing a Grey.  They are awesome pets.

In the U.S. visit: http://www.ngap.org/index.html

Skywire is offered under the Apache 2.0 license.
https://www.apache.org/licenses/LICENSE-2.0
(c)2014-2018 Skyloupe, LLC.  All rights reserved.

Skywire uses and bundles the following third party software packages:

Google Protocol Buffers - (Apache 2.0)
https://developers.google.com/protocol-buffers/

Google Test - (Apache 2.0)
https://github.com/google/googletest

Qwikpirate - a command line parsing tool (LGPL)
https://github.com/skyloupe/qwikpirate
