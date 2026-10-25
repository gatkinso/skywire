// Fills a Telemetry message, writes it to a file, reads it back and checks
// that the copy is equal. See README.md.
//
//   dotnet run -- telemetry.bin

using System;
using System.Globalization;
using System.IO;
using System.Text;
using telemetry;

public static class Program
{
    public static int Main(string[] args)
    {
        if (args.Length != 1)
        {
            Console.Error.WriteLine("usage: Example FILE");
            return 2;
        }
        string path = args[0];

        Telemetry sent = new Telemetry();
        sent.device_id = 4207;
        sent.timestamp_us = 1760959800123456UL;
        sent.mode = 5;
        sent.status = 11; // status has 3 bits: 11 is stored as 3
        sent.armed = 1;
        sent.Set_callsign(Encoding.ASCII.GetBytes("SKY1")); // NUL-padded to 8 bytes
        sent.position.latitude = 47.6062;
        sent.position.longitude = -122.3321;
        sent.position.altitude_m = 56.5f;
        sent.battery_mv = 3870;

        // Serialize to Size() bytes and write them to the file.
        File.WriteAllBytes(path, sent.Serialize());

        // Read the file back and deserialize a second message.
        byte[] back = File.ReadAllBytes(path);
        Telemetry received = new Telemetry();
        received.Deserialize(back); // throws ArgumentException if the input is short

        if (!received.Equals(sent))
        {
            Console.Error.WriteLine("the message read back is different");
            return 1;
        }

        string callsign = Encoding.ASCII.GetString(received.Get_callsign()).TrimEnd('\0');
        Position p = received.position;
        Console.WriteLine("wrote and read back {0} bytes (Size() = {1})", back.Length, received.Size());
        Console.WriteLine(string.Format(CultureInfo.InvariantCulture,
            "device_id {0}, callsign {1}, status {2}, position {3:F4}, {4:F4}, {5:F1} m",
            received.device_id, callsign, received.status, p.latitude, p.longitude, p.altitude_m));
        return 0;
    }
}
