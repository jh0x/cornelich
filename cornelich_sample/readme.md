# Sample applications
## Ping & Pong
A pair of fairly simple applications with implementation in C++ and Java:

 - `ping` is used to generate a chronicle. Command line options:
	 - `x` - whether to delete the output chronicle on startup
	 - `o` - output path where the chronicle will be generated (default `/tmp/__test/chr`)
	 - `w` - number of writer threads (default `4`)
	 - `n` - number of entries that should be written by each thread
 - `pong` is used to read from the generated chronicle. Command line options:
	 - `x` - whether to delete the input chronicle on startup
	 - `i` - input path (default `/tmp/__test/chr`)
	 - `w` - number of writer threads (default `4`)
	 - `n` - number of entries that got written by each writer thread
	 - `r` - number of readers (each of them will read the whole chronicle - that is `w * n` entries)

The contents of each excerpt follows the pattern:

```
appender.startExcerpt();
appender.writeInt(id);
appender.writeStopBit(count);
appender.writeLong(0x0badcafedeadbeefL);
appender.writeUTFΔ("FooBar");
appender.writeUTFΔ("AnotherFooBar");
appender.finish();
```


### Sample usage


```
# From one shell
pong -x
# This will wait for the ping

# From a different shell
ping
```

