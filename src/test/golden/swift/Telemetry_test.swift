// This is the generated Swift round-trip test

import Foundation

@main
struct TelemetryTest {
    static func check(_ cond: Bool, _ msg: String) {
        if !cond {
            print("FAIL: \(msg)")
            exit(1)
        }
    }

    static func main() {
        let input = Telemetry()
        input.Set_device_id(3000000000)
        input.Set_timestamp_us(12000000000)
        input.Set_mode(5)
        input.Set_status(5)
        input.Set_armed(1)
        input.Set_callsign(Array("skywire".utf8))
        input.Set_battery_mv(60000)

        let stream = SWStream()
        check(input.serialize(stream), "serialize failed")

        let output = Telemetry()
        check(output.deserialize(stream), "deserialize failed")

        check(input.Equals(output), "round-trip value mismatch")
        check(input.size() == output.size(), "size mismatch")

        let full = SWStream()
        check(input.serialize(full), "serialize failed")
        let short = SWStream()
        short.write(full.read(input.size() - 1)!)
        let untouched = Telemetry()
        check(!untouched.deserialize(short), "short buffer accepted")
        check(untouched.Equals(Telemetry()), "short buffer modified the destination")
        check(short.remaining() == input.size() - 1, "short buffer consumed bytes")

        let reused = SWStream()
        var pending = 0
        for i in 0..<1000 {
            check(input.serialize(reused), "reused stream serialize failed")
            pending += 1
            if i % 3 != 2 {
                let got = Telemetry()
                check(got.deserialize(reused) && input.Equals(got), "reused stream round-trip mismatch")
                pending -= 1
            }
            check(reused.remaining() == pending * input.size(), "reused stream byte count")
        }
        check(reused.byteSize() == reused.remaining(), "byteSize() is not the unread byte count")
        while pending > 0 {
            let got = Telemetry()
            check(got.deserialize(reused) && input.Equals(got), "reused stream drain mismatch")
            pending -= 1
        }
        check(reused.remaining() == 0, "reused stream not drained")
        input.Set_mode(UInt8(truncatingIfNeeded: UInt64(0x15)))
        check(input.Get_mode() == 5, "mode setter kept bits outside the field")
        input.Set_status(UInt8(truncatingIfNeeded: UInt64(0xd)))
        check(input.Get_status() == 5, "status setter kept bits outside the field")
        input.Set_armed(UInt8(truncatingIfNeeded: UInt64(0x3)))
        check(input.Get_armed() == 1, "armed setter kept bits outside the field")
        print("PASS Telemetry round-trip")
    }
}
