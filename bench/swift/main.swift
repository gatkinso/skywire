// Times the generated Swift code for the ring message. See ../README.md.
//
//   bench_swift RingyMcRingface.bin

import Dispatch
import Foundation

let rounds = 5
let iters = 200_000
var sink = 0

// Best time per iteration, in nanoseconds, over `rounds` rounds of body().
func bestNs(_ body: () -> Void) -> Double {
    var best = Double.infinity
    for _ in 0..<rounds {
        let t = DispatchTime.now().uptimeNanoseconds
        for _ in 0..<iters { body() }
        best = min(best, Double(DispatchTime.now().uptimeNanoseconds - t) / Double(iters))
    }
    return best
}

// Peak resident memory of this process, in bytes (nil where not measured).
func maxRSS() -> Int? {
    #if os(Windows)
    return nil
    #else
    var usage = rusage()
    #if os(Linux)
    getrusage(__rusage_who_t(RUSAGE_SELF.rawValue), &usage)
    return Int(usage.ru_maxrss) * 1024
    #else
    getrusage(RUSAGE_SELF, &usage)
    return Int(usage.ru_maxrss)
    #endif
    #endif
}

guard CommandLine.arguments.count == 2,
      let data = FileManager.default.contents(atPath: CommandLine.arguments[1]) else {
    FileHandle.standardError.write("usage: bench_swift RingyMcRingface.bin\n".data(using: .utf8)!)
    exit(2)
}
let bin = [UInt8](data)
let m = RingyMcRingface()
let m2 = RingyMcRingface()

// The message must decode and re-encode to the same bytes.
let check = SWStream()
check.write(bin)
guard m.deserialize(check), check.remaining() == 0 else {
    FileHandle.standardError.write("deserialize failed\n".data(using: .utf8)!)
    exit(1)
}
_ = m.serialize(check)
guard check.read(check.remaining()) == bin else {
    FileHandle.standardError.write("serialize does not reproduce the input\n".data(using: .utf8)!)
    exit(1)
}

// One stream used for many round trips must not keep the bytes it has read.
// Measured first, so the timing loops below cannot raise the peak.
let reused = SWStream()
let rssBefore = maxRSS()
for _ in 0..<iters {
    _ = m.serialize(reused)
    _ = m2.deserialize(reused)
}
let rssAfter = maxRSS()

// Like C++, one stream is reused: cleared after each serialize, and drained by
// each deserialize.
let stream = SWStream()
let ser = bestNs {
    _ = m.serialize(stream)
    sink &+= stream.remaining()
    stream.clear()
}
let de = bestNs {
    stream.write(bin)
    _ = m2.deserialize(stream)
    sink &+= Int(m2.Get_u8())
}
let reuse = bestNs {
    _ = m.serialize(reused)
    _ = m2.deserialize(reused)
    sink &+= Int(m2.Get_u8())
}

print(String(format: "RESULT serialize %.1f", ser))
print(String(format: "RESULT deserialize %.1f", de))
print(String(format: "NOTE reused-stream round trip %.1f ns", reuse))
if let before = rssBefore, let after = rssAfter {
    let grewMB = Double(after - before) / 1_048_576
    print(String(format: "NOTE reused stream: peak memory +%.1f MB after %d round trips", grewMB, iters))
}
if sink == 42 { print("") }
