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
//   Swift serialization runtime. Byte-for-byte compatible with the C/C++
//   SWStream wire format in lang/cpp/stream.h: each scalar is written in the
//   requested byte order; fixed byte arrays (char[N]) are written verbatim.

import Foundation

public enum Endianness {
    case big
    case little
    case native
}

public final class SWStream {
    private var buffer: [UInt8] = []
    private var readPos: Int = 0

    private static let hostLittleEndian: Bool = (UInt16(1).littleEndian == 1)

    public init() {}

    // MARK: - raw byte access

    public func write(_ bytes: [UInt8]) {
        buffer.append(contentsOf: bytes)
    }

    public func read(_ count: Int) -> [UInt8]? {
        guard count >= 0, buffer.count - readPos >= count else { return nil }
        let out = Array(buffer[readPos..<(readPos + count)])
        readPos += count
        return out
    }

    public func byteSize() -> Int {
        return buffer.count
    }

    // Number of bytes not yet read.
    public func remaining() -> Int {
        return buffer.count - readPos
    }

    // Pad or truncate a byte array to exactly `length` bytes (char[N] semantics).
    public static func fit(_ bytes: [UInt8], _ length: Int) -> [UInt8] {
        if bytes.count == length {
            return bytes
        } else if bytes.count < length {
            return bytes + [UInt8](repeating: 0, count: length - bytes.count)
        }
        return Array(bytes[0..<length])
    }

    // MARK: - byte ordering helpers

    private static func order(_ little: [UInt8], _ endian: Endianness) -> [UInt8] {
        switch endian {
        case .little:
            return little
        case .big:
            return little.reversed()
        case .native:
            return hostLittleEndian ? little : little.reversed()
        }
    }

    private static func littleBytes<T: FixedWidthInteger & UnsignedInteger>(_ value: T) -> [UInt8] {
        let count = MemoryLayout<T>.size
        var v = value
        var bytes = [UInt8](repeating: 0, count: count)
        for i in 0..<count {
            bytes[i] = UInt8(truncatingIfNeeded: v)
            v >>= 8
        }
        return bytes
    }

    private static func fromBytes<T: FixedWidthInteger & UnsignedInteger>(_ bytes: [UInt8], _ endian: Endianness) -> T {
        let little = order(bytes, endian) // undo wire order back to little-endian
        var v: T = 0
        var i = little.count - 1
        while i >= 0 {
            v = (v << 8) | T(little[i])
            i -= 1
        }
        return v
    }

    // MARK: - unsigned integers

    public func serialize(_ value: UInt8, _ endian: Endianness) {
        write([value])
    }

    public func serialize(_ value: UInt16, _ endian: Endianness) {
        write(SWStream.order(SWStream.littleBytes(value), endian))
    }

    public func serialize(_ value: UInt32, _ endian: Endianness) {
        write(SWStream.order(SWStream.littleBytes(value), endian))
    }

    public func serialize(_ value: UInt64, _ endian: Endianness) {
        write(SWStream.order(SWStream.littleBytes(value), endian))
    }

    public func deserialize(_ value: inout UInt8, _ endian: Endianness) {
        if let b = read(1) { value = b[0] }
    }

    public func deserialize(_ value: inout UInt16, _ endian: Endianness) {
        if let b = read(2) { value = SWStream.fromBytes(b, endian) }
    }

    public func deserialize(_ value: inout UInt32, _ endian: Endianness) {
        if let b = read(4) { value = SWStream.fromBytes(b, endian) }
    }

    public func deserialize(_ value: inout UInt64, _ endian: Endianness) {
        if let b = read(8) { value = SWStream.fromBytes(b, endian) }
    }

    // MARK: - signed integers (reinterpret through the unsigned path)
    // A short read leaves value unchanged, as for the unsigned types.

    public func serialize(_ value: Int8, _ endian: Endianness) {
        write([UInt8(bitPattern: value)])
    }

    public func serialize(_ value: Int16, _ endian: Endianness) {
        serialize(UInt16(bitPattern: value), endian)
    }

    public func serialize(_ value: Int32, _ endian: Endianness) {
        serialize(UInt32(bitPattern: value), endian)
    }

    public func serialize(_ value: Int64, _ endian: Endianness) {
        serialize(UInt64(bitPattern: value), endian)
    }

    public func deserialize(_ value: inout Int8, _ endian: Endianness) {
        if let b = read(1) { value = Int8(bitPattern: b[0]) }
    }

    public func deserialize(_ value: inout Int16, _ endian: Endianness) {
        if let b = read(2) { value = Int16(bitPattern: SWStream.fromBytes(b, endian) as UInt16) }
    }

    public func deserialize(_ value: inout Int32, _ endian: Endianness) {
        if let b = read(4) { value = Int32(bitPattern: SWStream.fromBytes(b, endian) as UInt32) }
    }

    public func deserialize(_ value: inout Int64, _ endian: Endianness) {
        if let b = read(8) { value = Int64(bitPattern: SWStream.fromBytes(b, endian) as UInt64) }
    }

    // MARK: - floating point (reinterpret through the unsigned path)

    public func serialize(_ value: Float, _ endian: Endianness) {
        serialize(value.bitPattern, endian)
    }

    public func serialize(_ value: Double, _ endian: Endianness) {
        serialize(value.bitPattern, endian)
    }

    public func deserialize(_ value: inout Float, _ endian: Endianness) {
        if let b = read(4) { value = Float(bitPattern: SWStream.fromBytes(b, endian) as UInt32) }
    }

    public func deserialize(_ value: inout Double, _ endian: Endianness) {
        if let b = read(8) { value = Double(bitPattern: SWStream.fromBytes(b, endian) as UInt64) }
    }

    // MARK: - fixed-length byte arrays (char[N], no byte swap)

    public func serialize(_ value: [UInt8], _ length: Int, _ endian: Endianness) {
        write(SWStream.fit(value, length))
    }

    public func deserialize(_ value: inout [UInt8], _ length: Int, _ endian: Endianness) {
        if let b = read(length) { value = b }
    }
}
