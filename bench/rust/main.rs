//! Times the generated Rust code for the ring message. See ../README.md.
//!
//!     bench_rust RingyMcRingface.bin

#![allow(dead_code)]

mod ringy_inner;
mod ringy_mc_ringface;

use ringy_mc_ringface::RingyMcRingface;
use std::hint::black_box;
use std::time::Instant;

const ROUNDS: u32 = 5;
const ITERS: u32 = 5_000_000;

/// Best time per iteration, in nanoseconds, over `ROUNDS` rounds of `body`.
fn best_ns(mut body: impl FnMut()) -> f64 {
    let mut best = f64::MAX;
    for _ in 0..ROUNDS {
        let t = Instant::now();
        for _ in 0..ITERS {
            body();
        }
        best = best.min(t.elapsed().as_nanos() as f64 / f64::from(ITERS));
    }
    best
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 2 {
        eprintln!("usage: bench_rust RingyMcRingface.bin");
        std::process::exit(2);
    }
    let bin = std::fs::read(&args[1]).unwrap_or_else(|e| {
        eprintln!("cannot read {}: {e}", args[1]);
        std::process::exit(2);
    });

    // The message must decode and re-encode to the same bytes.
    let m = RingyMcRingface::from_bytes(&bin).unwrap_or_else(|e| {
        eprintln!("deserialize failed: {e}");
        std::process::exit(1);
    });
    if m.serialize() != bin {
        eprintln!("serialize does not reproduce the input");
        std::process::exit(1);
    }

    let mut out = Vec::with_capacity(4096);
    let ser = best_ns(|| {
        out.clear();
        black_box(&m).serialize_into(&mut out);
        black_box(&out);
    });
    let ser_vec = best_ns(|| {
        black_box(black_box(&m).serialize());
    });
    let mut m2 = RingyMcRingface::default();
    let de = best_ns(|| {
        m2.deserialize(black_box(&bin)).unwrap();
        black_box(&m2);
    });

    println!("RESULT serialize {ser:.1}");
    println!("RESULT deserialize {de:.1}");
    println!("NOTE serialize() to a new Vec {ser_vec:.1} ns");
}
