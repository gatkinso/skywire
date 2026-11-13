// This is the generated Swift round-trip test

import Foundation

@main
struct ClassyMcClassfaceTest {
    static func check(_ cond: Bool, _ msg: String) {
        if !cond {
            print("FAIL: \(msg)")
            exit(1)
        }
    }

    static func main() {
        let input = ClassyMcClassface()
        input.Set_a_mydouble100(3.14159)
        input.Set_b_myuint32_200(3000000000)
        input.Set_c_myint64_201(-9000000000)
        input.Set_d_myfloat202(3.5)
        input.Set_e_myint16_203(-12345)
        input.Set_f_myuint16_204(60000)
        input.Set_g_myint32_205(-2000000000)
        input.Set_h_myuint64_206(12000000000)
        input.Set_i_charcoal301(65)
        input.Set_j_mystring302(Array("skywire".utf8))
        input.Set_k_uchar303(200)
        input.Set_l_myuint8_304(201)
        input.Set_m_myint8_305(-50)
        input.Set_n_shortstring306(Array("skywire".utf8))
        input.Set_o_myfloat1000(3.5)
        input.Set_p_myuint32_2000(3000000000)
        input.Set_q_myint64_2010(-9000000000)
        input.Set_r_mydouble2020(3.14159)

        let stream = SWStream()
        check(input.serialize(stream), "serialize failed")

        let output = ClassyMcClassface()
        check(output.deserialize(stream), "deserialize failed")

        check(input.Equals(output), "round-trip value mismatch")
        check(input.size() == output.size(), "size mismatch")

        let full = SWStream()
        check(input.serialize(full), "serialize failed")
        let short = SWStream()
        short.write(full.read(input.size() - 1)!)
        let untouched = ClassyMcClassface()
        check(!untouched.deserialize(short), "short buffer accepted")
        check(untouched.Equals(ClassyMcClassface()), "short buffer modified the destination")
        check(short.remaining() == input.size() - 1, "short buffer consumed bytes")

        let want: [UInt8] = [
            0x6e, 0x86, 0x1b, 0xf0, 0xf9, 0x21, 0x09, 0x40,
            0xb2, 0xd0, 0x5e, 0x00, 0xff, 0xff, 0xff, 0xfd,
            0xe7, 0x8e, 0xe6, 0x00, 0x40, 0x60, 0x00, 0x00,
            0xc7, 0xcf, 0xea, 0x60, 0x00, 0x6c, 0xca, 0x88,
            0x00, 0x78, 0x41, 0xcb, 0x02, 0x00, 0x00, 0x00,
            0x41, 0x73, 0x6b, 0x79, 0x77, 0x69, 0x72, 0x65,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0xc8, 0xc9, 0xce, 0x73, 0x6b, 0x79, 0x77,
            0x00, 0x00, 0x60, 0x40, 0x00, 0x5e, 0xd0, 0xb2,
            0x00, 0xe6, 0x8e, 0xe7, 0xfd, 0xff, 0xff, 0xff,
            0x40, 0x09, 0x21, 0xf9, 0xf0, 0x1b, 0x86, 0x6e,
        ]
        let wire = SWStream()
        check(input.serialize(wire), "serialize failed")
        check(wire.read(wire.remaining())! == want, "wire mismatch")
        check(input.serialize(wire), "serialize failed")
        wire.clear()
        check(wire.remaining() == 0, "clear() left bytes")
        check(input.serialize(wire), "serialize after clear() failed")
        check(wire.read(wire.remaining())! == want, "wire mismatch after clear()")

        let reused = SWStream()
        var pending = 0
        for i in 0..<1000 {
            check(input.serialize(reused), "reused stream serialize failed")
            pending += 1
            if i % 3 != 2 {
                let got = ClassyMcClassface()
                check(got.deserialize(reused) && input.Equals(got), "reused stream round-trip mismatch")
                pending -= 1
            }
            check(reused.remaining() == pending * input.size(), "reused stream byte count")
        }
        check(reused.byteSize() == reused.remaining(), "byteSize() is not the unread byte count")
        while pending > 0 {
            let got = ClassyMcClassface()
            check(got.deserialize(reused) && input.Equals(got), "reused stream drain mismatch")
            pending -= 1
        }
        check(reused.remaining() == 0, "reused stream not drained")
        print("PASS ClassyMcClassface round-trip")
    }
}
