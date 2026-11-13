// This is the generated Swift file

#if canImport(Skywire)
import Skywire
#endif
import Foundation

public final class ClassyMcClassface {
    var a_mydouble100: Double = 0
    var b_myuint32_200: UInt32 = 0
    var c_myint64_201: Int64 = 0
    var d_myfloat202: Float = 0
    var e_myint16_203: Int16 = 0
    var f_myuint16_204: UInt16 = 0
    var g_myint32_205: Int32 = 0
    var h_myuint64_206: UInt64 = 0
    var i_charcoal301: Int8 = 0
    var j_mystring302: [UInt8] = [UInt8](repeating: 0, count: 16)
    var k_uchar303: UInt8 = 0
    var l_myuint8_304: UInt8 = 0
    var m_myint8_305: Int8 = 0
    var n_shortstring306: [UInt8] = [UInt8](repeating: 0, count: 4)
    var o_myfloat1000: Float = 0
    var p_myuint32_2000: UInt32 = 0
    var q_myint64_2010: Int64 = 0
    var r_mydouble2020: Double = 0

    public init() {}

    public func serialize(_ stream: SWStream, _ endian: Endianness = .big) -> Bool {
        guard var p = stream.claim(size()) else { return false }
        serialize(into: &p)
        return true
    }

    // Writes the wire representation, size() bytes, at p and moves p past it.
    public func serialize(into p: inout UnsafeMutableRawPointer) {
        SWWire.put(&p, a_mydouble100, .little)
        SWWire.put(&p, b_myuint32_200, .big)
        SWWire.put(&p, c_myint64_201, .big)
        SWWire.put(&p, d_myfloat202, .big)
        SWWire.put(&p, e_myint16_203, .little)
        SWWire.put(&p, f_myuint16_204, .big)
        SWWire.put(&p, g_myint32_205, .little)
        SWWire.put(&p, h_myuint64_206, .little)
        SWWire.put(&p, i_charcoal301, .big)
        SWWire.put(&p, j_mystring302, 16)
        SWWire.put(&p, k_uchar303, .big)
        SWWire.put(&p, l_myuint8_304, .big)
        SWWire.put(&p, m_myint8_305, .big)
        SWWire.put(&p, n_shortstring306, 4)
        SWWire.put(&p, o_myfloat1000, .native)
        SWWire.put(&p, p_myuint32_2000, .little)
        SWWire.put(&p, q_myint64_2010, .native)
        SWWire.put(&p, r_mydouble2020, .big)
    }

    public func deserialize(_ stream: SWStream, _ endian: Endianness = .big) -> Bool {
        guard var p = stream.consume(size()) else { return false }
        deserialize(from: &p)
        return true
    }

    // Reads the wire representation, size() bytes, from p and moves p past it.
    public func deserialize(from p: inout UnsafeRawPointer) {
        a_mydouble100 = SWWire.get(&p, .little)
        b_myuint32_200 = SWWire.get(&p, .big)
        c_myint64_201 = SWWire.get(&p, .big)
        d_myfloat202 = SWWire.get(&p, .big)
        e_myint16_203 = SWWire.get(&p, .little)
        f_myuint16_204 = SWWire.get(&p, .big)
        g_myint32_205 = SWWire.get(&p, .little)
        h_myuint64_206 = SWWire.get(&p, .little)
        i_charcoal301 = SWWire.get(&p, .big)
        SWWire.get(&p, &j_mystring302, 16)
        k_uchar303 = SWWire.get(&p, .big)
        l_myuint8_304 = SWWire.get(&p, .big)
        m_myint8_305 = SWWire.get(&p, .big)
        SWWire.get(&p, &n_shortstring306, 4)
        o_myfloat1000 = SWWire.get(&p, .native)
        p_myuint32_2000 = SWWire.get(&p, .little)
        q_myint64_2010 = SWWire.get(&p, .native)
        r_mydouble2020 = SWWire.get(&p, .big)
    }

    public func size() -> Int {
        return (
            MemoryLayout<Double>.size +
            MemoryLayout<UInt32>.size +
            MemoryLayout<Int64>.size +
            MemoryLayout<Float>.size +
            MemoryLayout<Int16>.size +
            MemoryLayout<UInt16>.size +
            MemoryLayout<Int32>.size +
            MemoryLayout<UInt64>.size +
            MemoryLayout<Int8>.size +
            16 +
            MemoryLayout<UInt8>.size +
            MemoryLayout<UInt8>.size +
            MemoryLayout<Int8>.size +
            4 +
            MemoryLayout<Float>.size +
            MemoryLayout<UInt32>.size +
            MemoryLayout<Int64>.size +
            MemoryLayout<Double>.size
        )
    }

    public func Equals(_ other: ClassyMcClassface) -> Bool {
        return (
            a_mydouble100 == other.a_mydouble100 &&
            b_myuint32_200 == other.b_myuint32_200 &&
            c_myint64_201 == other.c_myint64_201 &&
            d_myfloat202 == other.d_myfloat202 &&
            e_myint16_203 == other.e_myint16_203 &&
            f_myuint16_204 == other.f_myuint16_204 &&
            g_myint32_205 == other.g_myint32_205 &&
            h_myuint64_206 == other.h_myuint64_206 &&
            i_charcoal301 == other.i_charcoal301 &&
            j_mystring302 == other.j_mystring302 &&
            k_uchar303 == other.k_uchar303 &&
            l_myuint8_304 == other.l_myuint8_304 &&
            m_myint8_305 == other.m_myint8_305 &&
            n_shortstring306 == other.n_shortstring306 &&
            o_myfloat1000 == other.o_myfloat1000 &&
            p_myuint32_2000 == other.p_myuint32_2000 &&
            q_myint64_2010 == other.q_myint64_2010 &&
            r_mydouble2020 == other.r_mydouble2020
        )
    }

    public func Get_a_mydouble100() -> Double { return a_mydouble100 }
    public func Get_b_myuint32_200() -> UInt32 { return b_myuint32_200 }
    public func Get_c_myint64_201() -> Int64 { return c_myint64_201 }
    public func Get_d_myfloat202() -> Float { return d_myfloat202 }
    public func Get_e_myint16_203() -> Int16 { return e_myint16_203 }
    public func Get_f_myuint16_204() -> UInt16 { return f_myuint16_204 }
    public func Get_g_myint32_205() -> Int32 { return g_myint32_205 }
    public func Get_h_myuint64_206() -> UInt64 { return h_myuint64_206 }
    public func Get_i_charcoal301() -> Int8 { return i_charcoal301 }
    public func Get_j_mystring302() -> [UInt8] { return j_mystring302 }
    public func Get_k_uchar303() -> UInt8 { return k_uchar303 }
    public func Get_l_myuint8_304() -> UInt8 { return l_myuint8_304 }
    public func Get_m_myint8_305() -> Int8 { return m_myint8_305 }
    public func Get_n_shortstring306() -> [UInt8] { return n_shortstring306 }
    public func Get_o_myfloat1000() -> Float { return o_myfloat1000 }
    public func Get_p_myuint32_2000() -> UInt32 { return p_myuint32_2000 }
    public func Get_q_myint64_2010() -> Int64 { return q_myint64_2010 }
    public func Get_r_mydouble2020() -> Double { return r_mydouble2020 }
    public func Set_a_mydouble100(_ newval: Double) { a_mydouble100 = newval }
    public func Set_b_myuint32_200(_ newval: UInt32) { b_myuint32_200 = newval }
    public func Set_c_myint64_201(_ newval: Int64) { c_myint64_201 = newval }
    public func Set_d_myfloat202(_ newval: Float) { d_myfloat202 = newval }
    public func Set_e_myint16_203(_ newval: Int16) { e_myint16_203 = newval }
    public func Set_f_myuint16_204(_ newval: UInt16) { f_myuint16_204 = newval }
    public func Set_g_myint32_205(_ newval: Int32) { g_myint32_205 = newval }
    public func Set_h_myuint64_206(_ newval: UInt64) { h_myuint64_206 = newval }
    public func Set_i_charcoal301(_ newval: Int8) { i_charcoal301 = newval }
    public func Set_j_mystring302(_ newval: [UInt8]) { j_mystring302 = SWStream.fit(newval, 16) }
    public func Set_k_uchar303(_ newval: UInt8) { k_uchar303 = newval }
    public func Set_l_myuint8_304(_ newval: UInt8) { l_myuint8_304 = newval }
    public func Set_m_myint8_305(_ newval: Int8) { m_myint8_305 = newval }
    public func Set_n_shortstring306(_ newval: [UInt8]) { n_shortstring306 = SWStream.fit(newval, 4) }
    public func Set_o_myfloat1000(_ newval: Float) { o_myfloat1000 = newval }
    public func Set_p_myuint32_2000(_ newval: UInt32) { p_myuint32_2000 = newval }
    public func Set_q_myint64_2010(_ newval: Int64) { q_myint64_2010 = newval }
    public func Set_r_mydouble2020(_ newval: Double) { r_mydouble2020 = newval }
}
