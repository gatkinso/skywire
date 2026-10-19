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
    // Raw storage, like the C++ SWStream: unread bytes are
    // [readPos, writePos). Reading the last unread byte rewinds both to 0, and
    // a write that does not fit first slides unread bytes to the front, so a
    // stream used for many messages reuses its space instead of growing.
    // (Raw storage rather than [UInt8] also avoids a dynamic exclusivity
    // check on every field.)
    private var storage: UnsafeMutableRawPointer
    private var capacity: Int = 0
    private var writePos: Int = 0
    private var readPos: Int = 0

    public init() {
        storage = UnsafeMutableRawPointer.allocate(byteCount: 0, alignment: 1)
    }

    deinit {
        storage.deallocate()
    }

    // MARK: - raw byte access

    @inline(__always)
    public func write(_ bytes: [UInt8]) {
        let count = bytes.count
        guard count > 0 else { return }
        ensure(count)
        bytes.withUnsafeBytes { src in
            (storage + writePos).copyMemory(from: src.baseAddress!, byteCount: count)
        }
        writePos += count
    }

    @inline(__always)
    public func read(_ count: Int) -> [UInt8]? {
        guard count >= 0, writePos - readPos >= count else { return nil }
        let out = [UInt8](UnsafeRawBufferPointer(start: storage + readPos, count: count))
        advance(count)
        return out
    }

    // Number of bytes not yet read (the same as remaining()).
    @inline(__always)
    public func byteSize() -> Int {
        return remaining()
    }

    // Number of bytes not yet read.
    @inline(__always)
    public func remaining() -> Int {
        return writePos - readPos
    }

    // Makes room for count more bytes, so the writes that follow do not
    // reallocate. Generated serialize() calls this with the message size.
    @inline(__always)
    public func reserve(_ count: Int) {
        ensure(count)
    }

    // Drops the unread bytes, keeping the storage for the next writes.
    @inline(__always)
    public func clear() {
        readPos = 0
        writePos = 0
    }

    // Reserves count bytes at the end of the stream and returns where to write
    // them; they count as written. Space already read is reused before the
    // storage grows. Returns nil (nothing reserved) if count is not positive.
    // Generated serialize() claims a whole message at once.
    @inline(__always)
    public func claim(_ count: Int) -> UnsafeMutableRawPointer? {
        guard count > 0 else { return nil }
        ensure(count)
        let p = storage + writePos
        writePos += count
        return p
    }

    // Consumes the next count unread bytes and returns where they start, or nil
    // (nothing consumed) if fewer are unread or count is not positive. The
    // bytes stay valid until the next write. Generated deserialize() consumes
    // a whole message at once.
    @inline(__always)
    public func consume(_ count: Int) -> UnsafeRawPointer? {
        guard count > 0, writePos - readPos >= count else { return nil }
        let p = UnsafeRawPointer(storage + readPos)
        advance(count)
        return p
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

    // MARK: - buffer management

    // Makes room for count more bytes at writePos.
    @inline(__always)
    private func ensure(_ count: Int) {
        if capacity - writePos < count {
            grow(count)
        }
    }

    @inline(never)
    private func grow(_ count: Int) {
        let unread = writePos - readPos
        if readPos > 0 {
            // Reuse the space already read before allocating more.
            storage.copyMemory(from: storage + readPos, byteCount: unread)  // memmove
            readPos = 0
            writePos = unread
            if capacity - writePos >= count {
                return
            }
        }
        // At least double, so growth stays amortized O(1).
        let newCapacity = Swift.max(unread + count, capacity * 2, 64)
        let newStorage = UnsafeMutableRawPointer.allocate(byteCount: newCapacity, alignment: 1)
        newStorage.copyMemory(from: storage, byteCount: unread)
        storage.deallocate()
        storage = newStorage
        capacity = newCapacity
    }

    @inline(__always)
    private func advance(_ count: Int) {
        readPos += count
        if readPos == writePos {
            readPos = 0
            writePos = 0
        }
    }

    // MARK: - byte ordering helpers

    @inline(__always)
    private func put<T: FixedWidthInteger>(_ value: T, _ endian: Endianness) {
        let wire: T
        switch endian {
        case .big:
            wire = value.bigEndian
        case .little:
            wire = value.littleEndian
        case .native:
            wire = value
        }
        let size = MemoryLayout<T>.size
        ensure(size)
        storage.storeBytes(of: wire, toByteOffset: writePos, as: T.self)
        writePos += size
    }

    // Returns nil, consuming nothing, if fewer than T's size bytes are unread.
    @inline(__always)
    private func get<T: FixedWidthInteger>(_ endian: Endianness) -> T? {
        let size = MemoryLayout<T>.size
        guard writePos - readPos >= size else { return nil }
        let wire = storage.loadUnaligned(fromByteOffset: readPos, as: T.self)
        advance(size)
        switch endian {
        case .big:
            return T(bigEndian: wire)
        case .little:
            return T(littleEndian: wire)
        case .native:
            return wire
        }
    }

    // MARK: - unsigned integers

    @inline(__always)
    public func serialize(_ value: UInt8, _ endian: Endianness) {
        put(value, endian)
    }

    @inline(__always)
    public func serialize(_ value: UInt16, _ endian: Endianness) {
        put(value, endian)
    }

    @inline(__always)
    public func serialize(_ value: UInt32, _ endian: Endianness) {
        put(value, endian)
    }

    @inline(__always)
    public func serialize(_ value: UInt64, _ endian: Endianness) {
        put(value, endian)
    }

    @inline(__always)
    public func deserialize(_ value: inout UInt8, _ endian: Endianness) {
        if let v: UInt8 = get(endian) { value = v }
    }

    @inline(__always)
    public func deserialize(_ value: inout UInt16, _ endian: Endianness) {
        if let v: UInt16 = get(endian) { value = v }
    }

    @inline(__always)
    public func deserialize(_ value: inout UInt32, _ endian: Endianness) {
        if let v: UInt32 = get(endian) { value = v }
    }

    @inline(__always)
    public func deserialize(_ value: inout UInt64, _ endian: Endianness) {
        if let v: UInt64 = get(endian) { value = v }
    }

    // MARK: - signed integers (reinterpret through the unsigned path)
    // A short read leaves value unchanged, as for the unsigned types.

    @inline(__always)
    public func serialize(_ value: Int8, _ endian: Endianness) {
        put(UInt8(bitPattern: value), endian)
    }

    @inline(__always)
    public func serialize(_ value: Int16, _ endian: Endianness) {
        put(UInt16(bitPattern: value), endian)
    }

    @inline(__always)
    public func serialize(_ value: Int32, _ endian: Endianness) {
        put(UInt32(bitPattern: value), endian)
    }

    @inline(__always)
    public func serialize(_ value: Int64, _ endian: Endianness) {
        put(UInt64(bitPattern: value), endian)
    }

    @inline(__always)
    public func deserialize(_ value: inout Int8, _ endian: Endianness) {
        if let v: UInt8 = get(endian) { value = Int8(bitPattern: v) }
    }

    @inline(__always)
    public func deserialize(_ value: inout Int16, _ endian: Endianness) {
        if let v: UInt16 = get(endian) { value = Int16(bitPattern: v) }
    }

    @inline(__always)
    public func deserialize(_ value: inout Int32, _ endian: Endianness) {
        if let v: UInt32 = get(endian) { value = Int32(bitPattern: v) }
    }

    @inline(__always)
    public func deserialize(_ value: inout Int64, _ endian: Endianness) {
        if let v: UInt64 = get(endian) { value = Int64(bitPattern: v) }
    }

    // MARK: - floating point (reinterpret through the unsigned path)

    @inline(__always)
    public func serialize(_ value: Float, _ endian: Endianness) {
        put(value.bitPattern, endian)
    }

    @inline(__always)
    public func serialize(_ value: Double, _ endian: Endianness) {
        put(value.bitPattern, endian)
    }

    @inline(__always)
    public func deserialize(_ value: inout Float, _ endian: Endianness) {
        if let v: UInt32 = get(endian) { value = Float(bitPattern: v) }
    }

    @inline(__always)
    public func deserialize(_ value: inout Double, _ endian: Endianness) {
        if let v: UInt64 = get(endian) { value = Double(bitPattern: v) }
    }

    // MARK: - value-returning reads
    // Generated deserialize() checks remaining() >= size() first and then
    // assigns each member from one of these, which lets the optimizer drop
    // the exclusivity checks an inout member would need. A short read
    // consumes nothing and returns 0.

    @inline(__always)
    public func read(_ endian: Endianness) -> UInt8 {
        return get(endian) ?? 0
    }

    @inline(__always)
    public func read(_ endian: Endianness) -> UInt16 {
        return get(endian) ?? 0
    }

    @inline(__always)
    public func read(_ endian: Endianness) -> UInt32 {
        return get(endian) ?? 0
    }

    @inline(__always)
    public func read(_ endian: Endianness) -> UInt64 {
        return get(endian) ?? 0
    }

    @inline(__always)
    public func read(_ endian: Endianness) -> Int8 {
        return Int8(bitPattern: get(endian) ?? 0)
    }

    @inline(__always)
    public func read(_ endian: Endianness) -> Int16 {
        return Int16(bitPattern: get(endian) ?? 0)
    }

    @inline(__always)
    public func read(_ endian: Endianness) -> Int32 {
        return Int32(bitPattern: get(endian) ?? 0)
    }

    @inline(__always)
    public func read(_ endian: Endianness) -> Int64 {
        return Int64(bitPattern: get(endian) ?? 0)
    }

    @inline(__always)
    public func read(_ endian: Endianness) -> Float {
        return Float(bitPattern: get(endian) ?? 0)
    }

    @inline(__always)
    public func read(_ endian: Endianness) -> Double {
        return Double(bitPattern: get(endian) ?? 0)
    }

    // MARK: - fixed-length byte arrays (char[N], no byte swap)

    // Writes exactly length bytes: value, truncated or padded with zeros.
    @inline(__always)
    public func serialize(_ value: [UInt8], _ length: Int, _ endian: Endianness) {
        guard length > 0 else { return }
        ensure(length)
        let count = Swift.min(value.count, length)
        value.withUnsafeBytes { src in
            if count > 0 {
                (storage + writePos).copyMemory(from: src.baseAddress!, byteCount: count)
            }
        }
        if count < length {
            (storage + writePos + count).initializeMemory(as: UInt8.self, repeating: 0, count: length - count)
        }
        writePos += length
    }

    // A short read leaves value unchanged.
    @inline(__always)
    public func deserialize(_ value: inout [UInt8], _ length: Int, _ endian: Endianness) {
        guard length >= 0, writePos - readPos >= length else { return }
        let src = UnsafeRawBufferPointer(start: storage + readPos, count: length)
        if value.count == length {
            // Copy into the existing array rather than allocating a new one.
            value.withUnsafeMutableBytes { dst in
                if length > 0 {
                    dst.baseAddress!.copyMemory(from: src.baseAddress!, byteCount: length)
                }
            }
        } else {
            value = [UInt8](src)
        }
        advance(length)
    }
}

// Stores and loads of one value at a raw pointer, advancing it past the
// value, for generated serialize(into:) and deserialize(from:) once the whole
// message has been claimed or consumed. Nothing here checks bounds.
public enum SWWire {
    @inline(__always)
    static func wire<T: FixedWidthInteger>(_ value: T, _ endian: Endianness) -> T {
        switch endian {
        case .big:
            return value.bigEndian
        case .little:
            return value.littleEndian
        case .native:
            return value
        }
    }

    @inline(__always)
    public static func put<T: FixedWidthInteger>(_ p: inout UnsafeMutableRawPointer, _ value: T, _ endian: Endianness) {
        p.storeBytes(of: wire(value, endian), as: T.self)
        p += MemoryLayout<T>.size
    }

    @inline(__always)
    public static func put(_ p: inout UnsafeMutableRawPointer, _ value: Float, _ endian: Endianness) {
        put(&p, value.bitPattern, endian)
    }

    @inline(__always)
    public static func put(_ p: inout UnsafeMutableRawPointer, _ value: Double, _ endian: Endianness) {
        put(&p, value.bitPattern, endian)
    }

    // Writes exactly length bytes: value, truncated or padded with zeros.
    @inline(__always)
    public static func put(_ p: inout UnsafeMutableRawPointer, _ value: [UInt8], _ length: Int) {
        let count = Swift.min(value.count, length)
        if count > 0 {
            value.withUnsafeBytes { p.copyMemory(from: $0.baseAddress!, byteCount: count) }
        }
        if count < length {
            (p + count).initializeMemory(as: UInt8.self, repeating: 0, count: length - count)
        }
        p += length
    }

    @inline(__always)
    public static func get<T: FixedWidthInteger>(_ p: inout UnsafeRawPointer, _ endian: Endianness) -> T {
        let raw = p.loadUnaligned(as: T.self)
        p += MemoryLayout<T>.size
        switch endian {
        case .big:
            return T(bigEndian: raw)
        case .little:
            return T(littleEndian: raw)
        case .native:
            return raw
        }
    }

    @inline(__always)
    public static func get(_ p: inout UnsafeRawPointer, _ endian: Endianness) -> Float {
        return Float(bitPattern: get(&p, endian))
    }

    @inline(__always)
    public static func get(_ p: inout UnsafeRawPointer, _ endian: Endianness) -> Double {
        return Double(bitPattern: get(&p, endian))
    }

    // Reads length bytes into value, reusing its storage when it already has
    // that length.
    @inline(__always)
    public static func get(_ p: inout UnsafeRawPointer, _ value: inout [UInt8], _ length: Int) {
        if value.count == length {
            if length > 0 {
                let src = p
                value.withUnsafeMutableBytes { $0.baseAddress!.copyMemory(from: src, byteCount: length) }
            }
        } else {
            value = [UInt8](UnsafeRawBufferPointer(start: p, count: length))
        }
        p += length
    }
}
