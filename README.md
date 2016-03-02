[TOC]

# Cornelich

## Example

### Writing

```
#include <cornelich/vanilla_chronicle_settings.h>
#include <cornelich/vanilla_chronicle.h>
#include <cornelich/formatters.h>

namespace chr = cornelich;

int main()
{
    chr::vanilla_chronicle_settings settings("/tmp/example");
    chr::vanilla_chronicle chronicle(settings);

    auto appender = chronicle.create_appender();
    appender.start_excerpt(1024);
    appender.write(42);
    appender.write("Some text", chr::writers::chars());
    appender.finish();
}
```

### Reading

```
#include <cornelich/vanilla_chronicle_settings.h>
#include <cornelich/vanilla_chronicle.h>
#include <cornelich/formatters.h>

#include <iostream>

namespace chr = cornelich;

int main()
{
    chr::vanilla_chronicle_settings settings("/tmp/example");
    chr::vanilla_chronicle chronicle(settings);

    auto tailer = chronicle.create_tailer();
    while(true)
    {
        if(!tailer.next_index())
            continue;
        std::cout << "int:  " << tailer.read<int>() << '\n';
        std::cout << "text: " << tailer.read(chr::readers::chars()) << '\n';
    }

}
```

## Brief

An experimental C++11 library for reading and writing [OpenHFT](https://github.com/OpenHFT)
[VanillaChronicles](https://github.com/OpenHFT/Chronicle-Queue)
(persistent, unbounded queues that support multiple readers/writers).

## Requirements, dependencies and third-party libraries used

- C++11 (because of some convenient language features and [date library](https://github.com/HowardHinnant/date))
- [Boost](http://www.boost.org/)
    - `filesystem` for simple filesystem operations
    - `mapped_region`, `file_mapping` for mmaping chronicle regions
    - `multi_index` to implement the cache
    - `qi` and `karma` to convert between numbers and strings
    - `iterator_range` for iterating over the directories
    - `algorithm::starts_with`
    - `string_ref`
- [Catch](https://github.com/philsquared/Catch) (included in `contrib`)
- [Date](https://github.com/HowardHinnant/date) (included in `contrib`)
- [cmdparser](https://github.com/FlorianRappl/CmdParser) (included in `contrib`)


## Disclaimer

- A subset of VanillaChronicle functionality implemented:
    - appending / tailing
    - basic encoders/decoders (POD types, stop-bit-encoded numbers, non-unicode strings)
- Implementation uses some Linux specific functions. As it is now it will not compile under VS.
- No guarantees given. There may be bugs. There might be incompatibilities with the Java version.

## Limitations

- issue #1 - Tailer might miss entries written by a slow writer when multiple writers are present. Same behaviour [can be observed](https://github.com/OpenHFT/Chronicle-Queue/issues/245) in the Java VanillaChronicle. Sample code: [C++](cornelich_test/cornelich_read_write_cycle.cpp), [Java](cornelich_sample/java/src/test/java/ReadWriteCycle.java)

## Example

### Writing example

Initial conditions:

- Empty chronicle with base `path` set to `/path`
- Cycle format set to `YYYYMMDD`
- Two writer threads (each with its own `appender`):
     -  `t1` with `tid1=4660=0x1234`
     -  `t2` with `tid2=43981=0xABCD`

#### `t1` creates an excerpt with capacity `x`

1. Evaluate the current cycle number - in our example it is the number of days since epoch: `20160116` -> `16816=0x41b0`
2. If there is no data region linked with the current appender:
    - Find the number of the next data region to use (`/path/20160116/data-4660-XXXX`). In this case there are no data regions yet and `XXXX=0`
    - Create a new memory mapped file to store the data
3. Check if there is at least `x + 4` bytes left in the current region. `4` additional bytes are needed to store the final size (written when finalising) of the excerpt. If there is not enough space then map another region with incremented file number (`XXXX+1`)

![T1 maps a region and creates and excerpt](doc/img/1_t1_maps_region.png?raw=true "T1 maps a region and creates and excerpt")

#### `t1` writes some data into the excerpt

Let's say `t1` writes `x1 <= x` bytes of data

![T1 writes some data](doc/img/2_t1_writes_some_data.png?raw=true "T1 writes some data")

#### `t2` creates an excerpt with capacity `y` and writes `y1 <= y` bytes of data

The steps are identical to those described above (the only difference is in naming of the data files `/path/20160116/data-42981-XXXX`)

![T2 creates and excerpt and writes some data](doc/img/3_t2_writes_some_data.png?raw=true "T2 creates and excerpt and writes some data")

#### `t1` and `t2` finish the excerpt at *'the same time'*

1. `t1` writes the total length (bitwise NOT) of the written data (`y1`) into the reserved first four bytes of the excerpt (`memory_order_release`).
1. `t1` evaluates the `idx1` value that should get appended to the index file. `idx1=(thread_id << X) | data_offset`
    - `X` comes from the settings
    - `data_offset` depends on the file number and the offset of data in the current region

`t2` does the same and ends up with `idx2`.

![T1 and T2 writes length](doc/img/4_t1t2_write_len.png?raw=true "T1 and T2 writes length")

##### `t1` and `t2` try to append `idx1` and `idx2` to the index file

TODO: How the index file gets mapped

![Index region gets mapped](doc/img/5_index_region.png?raw=true "Index region gets mapped")


- During excerpt finalisation an index entry gets appended to the index file.
- Vanilla chronicle supports multiple writers (thus `t1` and `t2` in this simple example)

- Index writes become serialised. The logic is fairly simple as the index region is initially zero-filled:

`t1` wants to write an entry `e1=idx1`. It does `CAS64` with the *'current index tail'* (`offset`) expecting there to be a zero value.


![T1 CAS](doc/img/6_t1_cas.png?raw=true "T1 CAS")


*Let's say* it succeeds. **`t1` has finalised the excerpt.**

![T1T2 CAS](doc/img/7_t1t2_cas.png?raw=true "T1T2 CAS")


`t2` wants to write an entry `e2=idx2`. It does `CAS64` with the *'current index tail'* (`offset`) expecting there to be a zero value. **In our example it fails -> as `t1` was first and wrote `e1` into `offset`**. `t2` has to advance the offset (`+8`) and try again.


![T2 CAS](doc/img/8_t2_cas.png?raw=true "T2 CAS")

#### Index file

![Chronicle Index](doc/img/9_index.png?raw=true "Chronicle Index")

#### Chronicle object

Each appender after successfully writing an excerpt and commiting the changes to the index will try to update the
value of `last_written_index` variable in the chronicle object. It only does that if the current `last_written_index`
value is lower than the new one. Writing is done atomically using CAS.


### Reading example

#### Empty chronicle

- A call to `next_index` attempts to find the first chronicle index entry in the chronicle:
    - trying to find the earliest index file -> no such file
- Usually the reader will just sit busy spinning till something gets written to the chronicle. Each of the checks is a bit costly - as it has to go and check for the presence of the files.

#### Chronicle with some data

Let's use the chronicle from the writing example:

```
/path/20160116/index-0
               data-4660-0
               data-43981-0
```

- The first call to `next_index` attempts to find the first chronicle index entry in the chronicle:
    - The earliest cycle directory is identified (`20160116`)
    - Corresponding cycle `0x41b0`
    - The earliest chronicle index entry would be `0x41b0_0000000000000000`
    - `index-0` get memory mapped and the chronicle index entry (0x0000000000000000) gets read atomically
    - From the read value (0x1234_0000000000000004) we deduce the excerpt location:
        - `thread_id = 0x1234 = 4660`
        - `data_file_number=0x00000000`
        - `data_offset=0x00000004`
    - `data-4660-0` gets mapped into memory
    - Excerpt length (bitwise NOT) gets read (`memory_order_acquire`) from `data_offset-4`)
    - The excerpt is ready to be used