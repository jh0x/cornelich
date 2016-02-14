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
import net.openhft.chronicle.ExcerptAppender;
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

public class Ping {


    public static void main(String... args) throws Exception {
        System.setProperty("os.max.pid.bits", "16");

        Options options = new Options();
        options.addOption("o", true, "Output chronicle path");
        options.addOption("n", true, "Number of entries to write");
        options.addOption("w", true, "Number of writer threads");
        options.addOption("x", false, "Delete the output chronicle at startup");

        CommandLine cmd = new DefaultParser().parse(options, args);
        final Path output = Paths.get(cmd.getOptionValue("o", "/tmp/__test/chr"));
        final long maxCount = Long.parseLong(cmd.getOptionValue("n", "10000000"));
        final int threadCount = Integer.parseInt(cmd.getOptionValue("w", "4"));
        final boolean deleteOnStartup = cmd.hasOption("x");

        if(deleteOnStartup) {
            FileUtil.removeRecursive(output);
        }

        final Chronicle chr = ChronicleQueueBuilder.vanilla(output.toFile()).build();

        final ExecutorService executor = Executors.newFixedThreadPool(4);
        final List<Future<?>> futures = new ArrayList<>();

        final long t0 = System.nanoTime();

        for(int i = 0; i != threadCount; ++i) {
            final int id = i;
            futures.add(executor.submit((Runnable) () -> {
                try {
                    ExcerptAppender appender = chr.createAppender();
                    for(int count = 0; count != maxCount; ++count) {
                        appender.startExcerpt();
                        appender.writeInt(id);
                        appender.writeStopBit(count);
                        appender.writeLong(0x0badcafedeadbeefL);
                        appender.writeUTFΔ("FooBar");
                        appender.writeUTFΔ("AnotherFooBar");
                        appender.finish();
                    }
                    System.out.println("Id: " + id + " finished. Last idx=" + Long.toHexString(appender.lastWrittenIndex()));
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
