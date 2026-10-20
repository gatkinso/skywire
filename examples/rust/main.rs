//! Fills a Telemetry message, writes it to a file, reads it back and checks
//! that the copy is equal. See README.md.
//!
//!     example_rust telemetry.bin

// The generated modules are the whole message API; this program uses part of it.
#[allow(dead_code)]
mod position;
#[allow(dead_code)]
mod telemetry;

use position::Position;
use std::process::exit;
use telemetry::Telemetry;

fn main() -> std::io::Result<()> {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 2 {
        eprintln!("usage: example_rust FILE");
        exit(2);
    }
    let path = &args[1];

    let mut position = Position::default();
    position.set_latitude(47.6062);
    position.set_longitude(-122.3321);
    position.set_altitude_m(56.5);

    let mut sent = Telemetry::default();
    sent.set_device_id(4207);
    sent.set_timestamp_us(1_760_959_800_123_456);
    sent.set_mode(5);
    sent.set_status(11); // status has 3 bits: 11 is stored as 3
    sent.set_armed(1);
    sent.set_callsign(b"SKY1"); // NUL-padded to 8 bytes
    sent.set_position(position);
    sent.set_battery_mv(3870);

    // Serialize to Telemetry::SIZE bytes and write them to the file.
    std::fs::write(path, sent.serialize())?;

    // Read the file back and deserialize a second message; a short input is
    // an UnexpectedEof error.
    let back = std::fs::read(path)?;
    let received = Telemetry::from_bytes(&back)?;

    if received != sent {
        eprintln!("the message read back is different");
        exit(1);
    }

    let callsign = String::from_utf8_lossy(received.callsign());
    let p = received.position();
    println!(
        "wrote and read back {} bytes (size() = {})",
        back.len(),
        received.size()
    );
    println!(
        "device_id {}, callsign {}, status {}, position {:.4}, {:.4}, {:.1} m",
        received.device_id(),
        callsign.trim_end_matches('\0'),
        received.status(),
        p.latitude(),
        p.longitude(),
        p.altitude_m()
    );
    Ok(())
}
