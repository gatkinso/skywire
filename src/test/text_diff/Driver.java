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

// The Java driver for skywire_text_diff: prints what the generated Java ring
// message makes of each input, as oracle.cpp does for C++. text_diff.cmake
// compiles it with the generated classes.
//
//     java -cp classes Driver <inputs>

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.Arrays;
import ring.RingyMcRingface;

public final class Driver {
    private Driver() {
    }

    private static void line(OutputStream out, String tag, RingyMcRingface msg, String format) throws IOException {
        StringBuilder text = new StringBuilder(tag);
        if (msg == null) {
            text.append(" -");
        } else {
            byte[] data;
            if (format.equals("json")) {
                data = msg.toJson().getBytes(StandardCharsets.UTF_8);
            } else if (format.equals("xml")) {
                data = msg.toXml().getBytes(StandardCharsets.UTF_8);
            } else {
                data = msg.toBson();
            }
            text.append(' ');
            for (byte b : data) {
                text.append(String.format("%02x", b & 0xFF));
            }
        }
        text.append('\n');
        out.write(text.toString().getBytes(StandardCharsets.US_ASCII));
    }

    public static void main(String[] args) throws IOException {
        byte[] raw = Files.readAllBytes(Paths.get(args[0]));
        try (OutputStream out = new BufferedOutputStream(System.out)) {
            int at = 0;
            while (at < raw.length) {
                int n = (raw[at] & 0xFF) | (raw[at + 1] & 0xFF) << 8 | (raw[at + 2] & 0xFF) << 16 | (raw[at + 3] & 0xFF) << 24;
                byte[] data = Arrays.copyOfRange(raw, at + 4, at + 4 + n);
                at += 4 + n;

                RingyMcRingface json = null;
                try {
                    json = RingyMcRingface.fromJson(data);
                } catch (IllegalArgumentException e) {
                    json = null;
                }
                line(out, "J", json, "json");
                RingyMcRingface xml = null;
                try {
                    xml = RingyMcRingface.fromXml(data);
                } catch (IllegalArgumentException e) {
                    xml = null;
                }
                line(out, "X", xml, "xml");
                RingyMcRingface bson = null;
                try {
                    bson = RingyMcRingface.fromBson(data);
                } catch (IllegalArgumentException e) {
                    bson = null;
                }
                line(out, "B", bson, "bson");
            }
        }
    }
}
