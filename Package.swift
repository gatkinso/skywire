// swift-tools-version:5.9
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
//  The Swift runtime, SWStream, as the library Skywire, so a Swift package can
//  depend on skywire instead of copying lang/swift/SWStream.swift:
//
//    .package(url: "https://github.com/gatkinso/skywire.git", from: "0.10.0")
//    .product(name: "Skywire", package: "skywire")
//
//  Generated Swift imports Skywire when it is there (#if canImport), so the
//  same files build either way.

import PackageDescription

let package = Package(
    name: "skywire",
    products: [
        .library(name: "Skywire", targets: ["Skywire"]),
    ],
    targets: [
        .target(name: "Skywire", path: "lang/swift", exclude: ["lang.swift.json"], sources: ["SWStream.swift"]),
    ]
)
