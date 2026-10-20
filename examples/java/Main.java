// Fills a Telemetry message, writes it to a file, reads it back and checks
// that the copy is equal. See README.md.
//
//   java -cp classes Main telemetry.bin

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;

import telemetry.Position;
import telemetry.Telemetry;

public final class Main {
    private Main() {
    }

    private static void fail(String message) {
        System.err.println(message);
        System.exit(1);
    }

    public static void main(String[] args) throws IOException {
        if (args.length != 1) {
            System.err.println("usage: java Main FILE");
            System.exit(2);
        }
        Path path = Paths.get(args[0]);

        Position position = new Position();
        position.Set_latitude(47.6062);
        position.Set_longitude(-122.3321);
        position.Set_altitude_m(56.5f);

        Telemetry sent = new Telemetry();
        sent.Set_device_id(4207);
        sent.Set_timestamp_us(1760959800123456L);
        sent.Set_mode(5);
        sent.Set_status(11); // status has 3 bits: 11 is stored as 3
        sent.Set_armed(1);
        sent.Set_callsign("SKY1".getBytes(StandardCharsets.US_ASCII)); // NUL-padded to 8 bytes
        sent.Set_position(position);
        sent.Set_battery_mv(3870);

        // Serialize to size() bytes and write them to the file.
        Files.write(path, sent.serialize());

        // Read the file back and deserialize a second message.
        byte[] back = Files.readAllBytes(path);
        Telemetry received = new Telemetry();
        received.deserialize(back); // throws IllegalArgumentException if the input is short

        if (!received.equals(sent)) {
            fail("the message read back is different");
        }

        String callsign = new String(received.Get_callsign(), StandardCharsets.US_ASCII).replace("\0", "");
        Position p = received.Get_position();
        System.out.printf("wrote and read back %d bytes (size() = %d)%n", back.length, received.size());
        System.out.printf("device_id %d, callsign %s, status %d, position %.4f, %.4f, %.1f m%n",
                received.Get_device_id(), callsign, received.Get_status(),
                p.Get_latitude(), p.Get_longitude(), p.Get_altitude_m());
    }
}
