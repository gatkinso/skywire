// Copyright (C) 2026 Geoffrey Atkinson
//
// Licensed under the Apache License, Version 2.0 (the "License").
// You may not use this file except in compliance with the License.
// A copy of the License is located at
//
// https://www.apache.org/licenses/LICENSE-2.0.html
//
// or in the "license" file accompanying this file. This file is distributed
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
// express or implied. See the License for the specific language governing
// permissions and limitations under the License.

// The C# driver for skywire_text_diff: prints what the generated C# ring
// message makes of each input, as oracle.cpp does for C++. text_diff.cmake
// builds it in a project beside the generated classes.
//
//     Driver <inputs>

using System;
using System.IO;
using System.Text;

public static class Driver
{
    private static void Line(TextWriter output, string tag, bool ok, byte[] data)
    {
        output.Write(tag);
        if (!ok)
        {
            output.Write(" -\n");
            return;
        }
        output.Write(' ');
        output.Write(BitConverter.ToString(data).Replace("-", "").ToLowerInvariant());
        output.Write('\n');
    }

    public static int Main(string[] args)
    {
        byte[] raw = File.ReadAllBytes(args[0]);
        TextWriter output = new StreamWriter(Console.OpenStandardOutput(), new UTF8Encoding(false), 1 << 16);
        for (int at = 0; at < raw.Length;)
        {
            int n = BitConverter.ToInt32(raw, at);
            byte[] data = new byte[n];
            Array.Copy(raw, at + 4, data, 0, n);
            at += 4 + n;

            ring.RingyMcRingface m = new ring.RingyMcRingface();
            bool ok = m.FromJson(data);
            Line(output, "J", ok, Encoding.UTF8.GetBytes(m.ToJson()));
            m = new ring.RingyMcRingface();
            ok = m.FromXml(data);
            Line(output, "X", ok, Encoding.UTF8.GetBytes(m.ToXml()));
            m = new ring.RingyMcRingface();
            ok = m.FromBson(data);
            Line(output, "B", ok, m.ToBson());
        }
        output.Flush();
        return 0;
    }
}
