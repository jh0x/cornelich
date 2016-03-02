import net.openhft.chronicle.*;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import static org.junit.Assert.assertEquals;

public class ReadWriteCycle {

    @Rule
    public TemporaryFolder path = new TemporaryFolder();

    @Test
    public void readWriteCycleExample() throws Exception {
        final ExecutorService executor = Executors.newFixedThreadPool(3);
        final Chronicle c = ChronicleQueueBuilder.vanilla(path.newFolder())
                .cycleFormat(VanillaChronicle.Cycle.MINUTES.format())
                .cycleLength(60000, false)
                .entriesPerCycle(1 << 29)
                .build();
        final ExcerptTailer t1 = c.createTailer();
        final ExcerptAppender a1 = c.createAppender();
        final ExcerptAppender a2 = c.createAppender();

        final List<Integer> readList = new ArrayList<>();

        executor.submit(() -> {
                    while (readList.isEmpty() || readList.get(readList.size() - 1) != 100) {
                        if (t1.nextIndex()) {
                            readList.add(t1.readInt());
                        }
                    }
                }
        );

        final CountDownLatch latch = new CountDownLatch(1);

        executor.submit(() -> {
                    a1.startExcerpt();
                    a1.writeInt(9999);
                    try {
                        latch.await();
                    } catch (InterruptedException ignored) {
                    }
                    a1.finish();
                }
        );

        executor.submit(() -> {
                    for (int i = 0; i != 100; ++i) {
                        a2.startExcerpt();
                        a2.writeInt(i + 1);
                        a2.finish();
                        try {
                            Thread.sleep(1000);
                        } catch (InterruptedException ignored) {
                        }
                        if (i == 80) {
                            latch.countDown();
                        }
                    }
                }
        );

        executor.awaitTermination(2, TimeUnit.MINUTES);
        final List<Integer> actual = new ArrayList<>();
        final ExcerptTailer t2 = c.createTailer().toStart();
        while (t2.nextIndex()) {
            actual.add(t2.readInt());
        }

        assertEquals(actual, readList);
    }
}
