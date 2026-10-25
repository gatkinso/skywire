// Times the generated C# code for the ring message. See ../README.md.
//
//   dotnet bin/Bench.dll RingyMcRingface.bin

using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using ring;

public static class Bench
{
    // The first rounds let tiered compilation reach its optimized code; the
    // best round is reported.
    private const int Rounds = 8;
    private const int Iters = 2000000;

    private static long sink;

    // Each loop is its own method, without a delegate call per iteration, so
    // only the generated code is timed.
    private static double SerializeNs(RingyMcRingface m, byte[] buf)
    {
        Stopwatch watch = Stopwatch.StartNew();
        for (int i = 0; i < Iters; i++)
        {
            int offset = 0;
            m.Serialize(buf, ref offset);
            sink += buf[0];
        }
        return watch.Elapsed.TotalMilliseconds * 1e6 / Iters;
    }

    private static double SerializeArrayNs(RingyMcRingface m)
    {
        Stopwatch watch = Stopwatch.StartNew();
        for (int i = 0; i < Iters; i++)
        {
            sink += m.Serialize()[0];
        }
        return watch.Elapsed.TotalMilliseconds * 1e6 / Iters;
    }

    private static double DeserializeNs(RingyMcRingface m, byte[] bin)
    {
        Stopwatch watch = Stopwatch.StartNew();
        for (int i = 0; i < Iters; i++)
        {
            m.Deserialize(bin);
            sink += m.u8;
        }
        return watch.Elapsed.TotalMilliseconds * 1e6 / Iters;
    }

    private static double HashNs(RingyMcRingface m)
    {
        Stopwatch watch = Stopwatch.StartNew();
        for (int i = 0; i < Iters; i++)
        {
            sink += m.GetHashCode();
        }
        return watch.Elapsed.TotalMilliseconds * 1e6 / Iters;
    }

    private static string Format(double ns)
    {
        return ns.ToString("F1", CultureInfo.InvariantCulture);
    }

    public static int Main(string[] args)
    {
        if (args.Length != 1)
        {
            Console.Error.WriteLine("usage: Bench RingyMcRingface.bin");
            return 2;
        }
        byte[] bin = File.ReadAllBytes(args[0]);
        RingyMcRingface m = new RingyMcRingface();
        RingyMcRingface m2 = new RingyMcRingface();

        // The message must decode and re-encode to the same bytes.
        m.Deserialize(bin);
        byte[] again = m.Serialize();
        if (again.Length != bin.Length)
        {
            Console.Error.WriteLine("serialize does not reproduce the input");
            return 1;
        }
        for (int i = 0; i < bin.Length; i++)
        {
            if (again[i] != bin[i])
            {
                Console.Error.WriteLine("serialize does not reproduce the input");
                return 1;
            }
        }

        byte[] buf = new byte[4096];
        double ser = double.MaxValue, serArray = double.MaxValue, de = double.MaxValue, hash = double.MaxValue;
        for (int r = 0; r < Rounds; r++)
        {
            ser = Math.Min(ser, SerializeNs(m, buf));
            serArray = Math.Min(serArray, SerializeArrayNs(m));
            de = Math.Min(de, DeserializeNs(m2, bin));
            hash = Math.Min(hash, HashNs(m));
        }

        Console.WriteLine("RESULT serialize " + Format(ser));
        Console.WriteLine("RESULT deserialize " + Format(de));
        Console.WriteLine("NOTE Serialize() to a new byte[] " + Format(serArray) + " ns; GetHashCode() " + Format(hash) + " ns; .NET " + Environment.Version);
        if (sink == 42)
        {
            Console.WriteLine();
        }
        return 0;
    }
}
