/*
Copyright 2015-2016 Joanna Hulboj <j@hulboj.org>
Copyright 2015-2016 Milosz Hulboj <m@hulboj.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/


import net.openhft.chronicle.Chronicle;
import net.openhft.chronicle.ChronicleQueueBuilder;
import net.openhft.chronicle.ExcerptTailer;
import net.openhft.koloboke.collect.map.IntLongMap;
import net.openhft.koloboke.collect.map.hash.HashIntLongMaps;
import net.openhft.lang.pool.StringInterner;
import org.apache.commons.cli.CommandLine;
import org.apache.commons.cli.DefaultParser;
import org.apache.commons.cli.Options;

import java.io.IOException;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

public class Pong {

    public static void main(String... args) throws Exception {
        System.setProperty("os.max.pid.bits", "16");

        Options options = new Options();
        options.addOption("i", true, "Input chronicle path");
        options.addOption("n", true, "Number of entries to write");
        options.addOption("w", true, "Number of writer threads");
        options.addOption("r", true, "Number of reader threads");
        options.addOption("x", false, "Delete the output chronicle at startup");

        CommandLine cmd = new DefaultParser().parse(options, args);
        final Path output = Paths.get(cmd.getOptionValue("o", "/tmp/__test/chr"));
        final long maxCount = Long.parseLong(cmd.getOptionValue("n", "10000000"));
        final int writerThreadCount = Integer.parseInt(cmd.getOptionValue("w", "4"));
        final int readerThreadCount = Integer.parseInt(cmd.getOptionValue("r", "4"));
        final boolean deleteOnStartup = cmd.hasOption("x");

        if(deleteOnStartup) {
            FileUtil.removeRecursive(output);
        }

        final Chronicle chr = ChronicleQueueBuilder.vanilla(output.toFile()).build();

        final ExecutorService executor = Executors.newFixedThreadPool(4);
        final List<Future<?>> futures = new ArrayList<>();


        final long totalCount = writerThreadCount * maxCount;
        final long t0 = System.nanoTime();

        for(int i = 0; i != readerThreadCount; ++i) {
            final int tid = i;
            futures.add(executor.submit((Runnable) () -> {
                try {
                    IntLongMap counts = HashIntLongMaps.newMutableMap();
                    ExcerptTailer tailer = chr.createTailer();

                    final StringBuilder sb1 = new StringBuilder();
                    final StringBuilder sb2 = new StringBuilder();

                    long count = 0;
                    while(count != totalCount) {
                        if(!tailer.nextIndex())
                            continue;
                        final int id = tailer.readInt();
                        final long val = tailer.readStopBit();
                        final long longValue = tailer.readLong();
                        sb1.setLength(0);
                        sb2.setLength(0);
                        tailer.read8bitText(sb1);
                        tailer.read8bitText(sb2);
                        if(counts.addValue(id, 1) - 1 != val
                                || longValue != 0x0badcafedeadbeefL
                                || !StringInterner.isEqual("FooBar", sb1)
                                || !StringInterner.isEqual("AnotherFooBar", sb2)) {
                            System.out.println("Unexpected value " + id + ", " + val + ", " + Long.toHexString(longValue) + ", " + sb1.toString() + ", " + sb2.toString());
                            return;
                        }
                        ++count;
                        if(count % 1_000_000 == 0) {
                            long t1 = System.nanoTime();
                            System.out.println(tid + " " + (t1-t0)/1e6 + " ms");
                        }
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }));
        }
        for(Future f : futures) {
            f.get();
        }
        executor.shutdownNow();

        final long t1 = System.nanoTime();
        System.out.println("Done. Rough time=" + (t1-t0)/1e6 + " ms");
    }
}
