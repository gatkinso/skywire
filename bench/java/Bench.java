// Times the generated Java code for the ring message. See ../README.md.
//
//   java -cp classes Bench RingyMcRingface.bin

import java.nio.ByteBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.Arrays;

import ring.RingyMcRingface;

public class Bench {
    // The first rounds warm up the JIT; the best round is reported.
    private static final int ROUNDS = 6;
    private static final int ITERS = 2_000_000;

    private static long sink;

    private interface Body {
        void run();
    }

    // Best time per iteration, in nanoseconds, over ROUNDS rounds of body.
    private static double bestNs(Body body) {
        double best = Double.MAX_VALUE;
        for (int r = 0; r < ROUNDS; r++) {
            long t = System.nanoTime();
            for (int i = 0; i < ITERS; i++) {
                body.run();
            }
            best = Math.min(best, (System.nanoTime() - t) / (double) ITERS);
        }
        return best;
    }

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.err.println("usage: java Bench RingyMcRingface.bin");
            System.exit(2);
        }
        byte[] bin = Files.readAllBytes(Path.of(args[0]));
        RingyMcRingface m = new RingyMcRingface();
        RingyMcRingface m2 = new RingyMcRingface();

        // The message must decode and re-encode to the same bytes.
        m.deserialize(bin);
        if (!Arrays.equals(m.serialize(), bin)) {
            System.err.println("serialize does not reproduce the input");
            System.exit(1);
        }

        ByteBuffer buf = ByteBuffer.allocate(4096);
        double ser = bestNs(() -> {
            buf.clear();
            m.serialize(buf);
            sink += buf.get(0);
        });
        double serArray = bestNs(() -> sink += m.serialize()[0]);
        double de = bestNs(() -> {
            m2.deserialize(bin);
            sink += m2.Get_u8();
        });
        double hash = bestNs(() -> sink += m.hashCode());

        System.out.printf("RESULT serialize %.1f%n", ser);
        System.out.printf("RESULT deserialize %.1f%n", de);
        System.out.printf("NOTE serialize() to a new byte[] %.1f ns; hashCode() %.1f ns%n", serArray, hash);
        if (sink == 42) {
            System.out.println();
        }
    }
}
