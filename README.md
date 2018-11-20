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

## [1.1] A Note About Security

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

On Windows:

	> cmake -G "Visual Studio 14 2015 Win64" /
	-DCMAKE_PREFIX_PATH=c:\Users\username\apps\protobuf-3.6.1-debug;c:\Users\username\apps\gtest-1.8.1-debug / 
	-DCMAKE_INSTALL_DIR=c:\Users\username\apps\skywire-debug /
	e:\source\skywire 

	cmake --build . --target install --config Debug

On Linux:
	> cmake -G "Unix Makefiles" /
	-DCMAKE_BUILD_TYPE=Debug /
	-DCMAKE_PREFIX_PATH=~/apps/protobuf-3.6.1-debug;~/apps/gtest-1.8.1-debug /
	-DCMAKE_INSTALL_DIR=~/apps/skywire-debug /
	~/source/skywire

    cmake --build . --target install

On Mac (using Xcode):
	> cmake -G "Xcode" /
	-DCMAKE_BUILD_TYPE=Debug /
	-DCMAKE_PREFIX_PATH=~/apps/protobuf-3.6.1-debug;~/apps/gtest-1.8.1-debug /
	-DCMAKE_INSTALL_DIR=~/apps/skywire-debug /
	~/source/skywire

You will have to modify the paths appropriately for your system.

The following cmake generators have been tested:

    cmake -G "Visual Studio 14 2015 Win64"
    cmake -G "Visual Studio 14 2015"
    cmake -G "Xcode"
    cmake -G "Unix Makefiles"

# [4.0] Running unit tests

The unit tests are written using Google Test and are managed with CTest.  After a successful build,
type

	> ctest

to run them.

# [5.0] Deploying and running

    > wiregen.exe --input e:\some_directory\MyClass.json --outdir E:\some_other_directory

# [6.0] Configuration files 

All configuration files are in JSON format.

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
https://github.com/gatkinso/qwikpirate
    


