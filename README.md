# PQProtoM4

Modular benchmarking framework for protocols using post-quantum cryptography on an
STM32 Nucleo-L4R5ZI development board (Arm Cortex-M4).

A protocol (currently the KEM-based PAKE **OCAKE**, or a bare KEM) runs between two parties:

```
 ┌───────────────────────────┐   USB (ST-LINK)    ┌──────────────────────────────┐
 │ Linux host                │◄──────────────────►│ Nucleo-L4R5ZI                │
 │  server/bench_server      │  /dev/ttyACM0      │  client/bench_client.elf     │
 │  (portable C reference    │  19200 baud, 8N1   │  (pqm4 M4-optimised KEMs,    │
 │   KEMs, plays responder)  │                    │   libopencm3, measures)      │
 └───────────────────────────┘                    └──────────────────────────────┘
```

- **client/** is the firmware. It runs the protocol once right after reset, measures every
  step in clock cycles (or stack usage), and finally sends a struct with the results over
  the UART.
- **server/** is a normal Linux program. It acts as the other protocol party over the
  serial port, then receives the result struct and prints it.
- The UART is the Nucleo's **ST-LINK virtual COM port** (LPUART1 on PG7/PG8), so one USB
  cable carries both flashing and communication.

---

## Tested setup

| Component | Version known to work |
|---|---|
| OS | Ubuntu 20.04 (developed and run there). The build was also verified on Ubuntu 24.04 |
| Board | ST Nucleo-L4R5ZI (STM32L4R5ZI) |
| Arm GNU Toolchain | 14.3.Rel1 (`arm-none-eabi-gcc 14.3.1 20250623`). The build also succeeds with Ubuntu's `gcc-arm-none-eabi` 13.2 |
| OpenOCD | `0.12.0+dev`, commit `cbc32c383`, built from source |
| libopencm3 | built from source (`master`) |

The board runs at **20 MHz** with zero flash wait states, so cycle counts are not distorted by
flash latency (same convention as pqm4).

---

## 1. Get the code

```bash
git clone <repo-url> PQProtoM4
cd PQProtoM4
```

> The client's KEM folders (`client/src/crypto_kem/ml-kem-*/m4f*`) share sources through
> **symlinks**. Clone with git or pass it on as a `tar` archive. Copying it as a zip on
> Windows, or through tools that don't preserve symlinks, breaks the build.

---

## 2. Install dependencies

Every step below assumes a fresh Ubuntu installation.

### 2.1 Base packages

```bash
sudo apt update
sudo apt install build-essential git python3 netcat-openbsd \
                 autoconf automake libtool pkg-config texinfo libusb-1.0-0-dev
```

(`autoconf` … `libusb-1.0-0-dev` are only needed to build OpenOCD.)

### 2.2 Arm GNU toolchain (`arm-none-eabi-gcc`)

**Option A: Arm's official release (what we used).**
Download *arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz* from
<https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>, then:

```bash
sudo tar -xf arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi.tar.xz -C /usr/share/
sudo ln -s /usr/share/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/* /usr/local/bin/
```

Any other directory on your `PATH` works too. Alternatively, add the toolchain's `bin/` folder
to `PATH` in your `~/.bashrc`.

**Option B: Ubuntu packages.** On Ubuntu 24.04 this route is simpler, and all firmware
variants compile with the packaged GCC 13.2. Older Ubuntu releases ship a much older
compiler, which has not been tested.

```bash
sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi
```

Install only one of the two, or make sure the one you want comes first in `PATH`. Check with:

```bash
arm-none-eabi-gcc --version
```

### 2.3 OpenOCD (from source)

On Ubuntu 20.04, the `apt` package of OpenOCD (0.10) did not work for us. It is most likely
too old for the STM32L4R5. Newer Ubuntu releases ship newer packages, which may work, but
we have only used a source build:

```bash
git clone --recurse-submodules https://github.com/openocd-org/openocd.git
cd openocd
git checkout cbc32c383                      # exact version we used (optional)
git submodule update --init --recursive
./bootstrap
./configure --enable-stlink                 # ST-LINK support is required
make -j"$(nproc)"
sudo make install                           # installs to /usr/local
cd ..
```

Check that the output of `./configure` shows `ST-Link Programmer  yes`. No custom OpenOCD
config file is needed, because the Makefile uses OpenOCD's stock `board/st_nucleo_l4.cfg`.

**Submodules:** OpenOCD needs its submodules, especially `jimtcl`. If the `libjaylink`
submodule (hosted on gitlab.zapb.de) won't clone, you can skip it because only J-Link
probes need it:

```bash
git submodule update --init jimtcl
./bootstrap nosubmodule
./configure --enable-stlink --disable-jlink
```

**USB permissions:** let normal users access the ST-LINK:

```bash
sudo cp contrib/60-openocd.rules /etc/udev/rules.d/      # run inside the openocd folder
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### 2.4 libopencm3

The client Makefile expects the libopencm3 headers in `/usr/local/include` and the library in
`/usr/local/lib`:

```bash
git clone https://github.com/libopencm3/libopencm3.git
cd libopencm3
make TARGETS=stm32/l4 -j"$(nproc)"          # needs arm-none-eabi-gcc on PATH
sudo cp -r include/libopencm3 /usr/local/include/
sudo cp lib/libopencm3_stm32l4.a /usr/local/lib/
cd ..
```

The linker script (`client/generated.stm32l4r5zi.ld`) is already in the repo, so you don't
need to generate it.

### 2.5 Serial port permissions

```bash
sudo usermod -aG dialout "$USER"
```

Log out and back in for this to take effect.

---

## 3. Connect the board

Plug a USB cable into the **ST-LINK micro-USB connector**. This is the one on the
detachable ST-LINK part at the top of the board, not the USB OTG connector at the bottom.
The board then shows up as an ST-LINK debugger plus a serial port:

```bash
ls /dev/ttyACM*          # usually /dev/ttyACM0
```

If more than one `ttyACM` device exists, run `dmesg | tail` right after plugging in to find
the right one.

---

## 4. Run a benchmark (quick way)

`run.py` cleans, builds the server and firmware, starts the server, flashes the board, and
prints the result:

```bash
python3 run.py -p bench_OCAKE -k ml-kem-512 -i m4fstack
```

| Flag | Meaning | Values |
|---|---|---|
| `-p`, `--proto` | what to benchmark | `bench_OCAKE`, `bench_OCAKE_stack`, `bench_kyber`, `bench_kyber_stack`, `bench_io` |
| `-k`, `--kem` | KEM | `ml-kem-512`, `ml-kem-768`, `ml-kem-1024`, `bikel1` |
| `-i`, `--impl` | firmware KEM implementation | `m4fstack`, `m4fspeed` for ML-KEM; `""` (empty) for `bikel1` |

> **Always pass `-i` for ML-KEM.** Its default is empty, which only works for `bikel1`.
> `run.py` also hides all build and flash output. If the firmware fails to build, the server
> just waits forever. If a run hangs, press Ctrl+C and follow section 5 to see the error.

`run.py` assumes the board is at `/dev/ttyACM0`. If yours differs, edit that line in `run.py`.

---

## 5. Run a benchmark (manual way)

You can also run each step yourself, which shows any errors.

```bash
# 1) Build. Always run 'make clean' in both folders before building. Object files
#    are not separated per KEM/PROTO/IMPL, so leftovers from an earlier build get
#    linked in. (run.py does this for you.)
cd server
make clean && make KEM=ml-kem-512 PROTO=bench_OCAKE
cd ../client
make clean && make KEM=ml-kem-512 PROTO=bench_OCAKE IMPL=m4fstack
cd ..

# 2) Terminal 1: start the server first; it waits for the board
./server/bench_server /dev/ttyACM0

# 3) Terminal 2: flash the board; the benchmark starts right after the reset
cd client
make flash KEM=ml-kem-512 PROTO=bench_OCAKE IMPL=m4fstack
```

For `bikel1`, pass `IMPL=` (empty) to the client build, because the client Makefile defaults to `m4fstack`.

**Running again without reflashing:** start `bench_server` again, then press the black
**RESET** button on the board.

**How `make flash` works:** it first tries to send the commands to an OpenOCD instance that's
already running (telnet port 4444, via `nc`). If none is running, it starts OpenOCD itself
using the stock `board/st_nucleo_l4.cfg`:

```bash
openocd -f board/st_nucleo_l4.cfg -c "program bench_client.elf verify reset exit"
```

Make sure no other OpenOCD instance is running before you flash. If one is running, the `nc`
path can hang.

---

## 6. Reading the results

All timings are **clock cycles at 20 MHz**, so divide by 20,000,000 to get seconds.
`*_stack` benchmarks report **bytes of stack**, measured by filling the stack with
`0xDEADBEEF` and checking how much was overwritten.
Values come from a single run (`bench(1)`).

### `bench_OCAKE`: OCAKE PAKE, board = initiator

The server prints `:)` when key confirmation succeeded, followed by the results:

| Field | What it measures on the board |
|---|---|
| `PROTO` | whole protocol run, end to end |
| `KEY_GEN` | `crypto_kem_keypair` |
| `DECAP` | `crypto_kem_dec` |
| `AES` | AES-CBC encryption of the public key under the password-derived key |
| `SHA` | SHA3-512 key-confirmation tags |
| `SHAKE` | SHAKE256 password → AES key |
| `OTHER` | memcpy / memcmp |
| `COMM` | time spent in UART send/receive, **including waiting for the server** |
| `TOTAL` | KEY_GEN + DECAP + AES + SHA + SHAKE (labelled `KEM+SYM` in `server/bench.txt`) |
| `TOTAL+COMM+Other` | should roughly match `PROTO` |

At 19200 baud, communication dominates. For ML-KEM-512, about 18.2M of the 19.7M cycles are `COMM`.

### `bench_OCAKE_stack`
`PROTO` is the protocol's peak stack usage in bytes.

### `bench_kyber` / `bench_kyber_stack`
`KEYPAIR`, `ENCAP`, `DECAP` give cycles (or stack bytes) of the bare KEM on the board.

### `bench_io`
This is a UART sanity test: 1000 integers are sent in each direction and checked. The server
prints `Send successful 13` if both directions work.

Reference numbers from our runs are in `server/bench.txt`.

---

## 7. Repository layout

```
run.py                     build + flash + run helper
server/
  Makefile                 host build (gcc)
  src/main.c, io.c         serial port setup + send/recv
  src/proto/<PROTO>/       server side of each benchmark
  src/crypto_kem/<KEM>/    portable reference KEMs (ML-KEM, BIKE)
  src/crypto_kem/common/   AES (tiny-AES), FIPS 202, randombytes
  bench.txt                our reference results
  src/host_unidirectional.py  old UART test script (not used by run.py)
client/
  Makefile                 firmware build + 'make flash'
  generated.stm32l4r5zi.ld linker script
  src/main.c, hal.c        clock (20 MHz), UART, RNG, cycle counter
  src/proto/<PROTO>/       board side of each benchmark + measurement
  src/crypto_kem/<KEM>/<IMPL>/  pqm4 M4 implementations (m4fstack / m4fspeed)
  src/crypto_kem/common/   AES, FIPS 202 + Keccak-f1600 assembly
```

**Adding a protocol:** create `src/proto/<name>/` in both `client/` and `server/`, each with
a `bench.h` (defining `bench_t` and `bench()`) and `bench.c`. Then build with `PROTO=<name>`.

---

## 8. Troubleshooting

| Symptom | Likely cause / fix |
|---|---|
| `fatal error: api.h: No such file or directory` (client) | `IMPL` missing or wrong: use `m4fstack`/`m4fspeed` for ML-KEM, empty for `bikel1` |
| `libopencm3/...: No such file` or `cannot find -lopencm3_stm32l4` | libopencm3 not copied to `/usr/local/include` and `/usr/local/lib` (section 2.4) |
| `Error: open failed` from OpenOCD | board not connected, or no USB permission (udev rules, section 2.3), or OpenOCD built without ST-LINK |
| `make flash` only prints `Error 1` | OpenOCD's output is suppressed, so run the `openocd …` command from section 5 by hand inside `client/` to see the actual error |
| `Error opening /dev/ttyACM0: …` followed by garbage numbers | the server keeps running even though the port didn't open (see 9.3), so ignore the numbers. `Permission denied` means you're not in the `dialout` group (section 2.5). `No such file` means the board isn't connected or uses a different device name |
| linker warnings `_fstat is not implemented and will always fail` (BIKE) | harmless |
| `run.py` hangs with no output | firmware didn't build or flash, so run the steps manually (section 5) |
| server hangs after flashing | wrong serial device, or the board ran before the server was started: restart the server, press RESET |
| strange results after switching KEM/PROTO | stale objects, so run `make clean` in both folders |
| HardFault / board hangs in assembly code with a newer toolchain | an assembly function is missing its `.type <name>, %function` directive (see `keccakf1600.S` for the pattern) |

---

## 9. Open issues

These are known problems that haven't been fixed yet. All line numbers refer to the current
state of the repository.

### 9.1 `run.py`: the empty `-i` default breaks ML-KEM firmware builds

**Where:** `run.py:9`, `run.py:20`, `run.py:22`, `run.py:26`

**Problem:** `-i/--impl` defaults to `""` (line 9). `run.py` passes this as `IMPL=` to the
client build (line 22) and to `make flash` (line 26). An explicit `IMPL=` on the command
line overrides the client Makefile's default (`IMPL ?= m4fstack`, `client/Makefile:7`), so
the build looks for sources in `src/crypto_kem/<KEM>//` and finds none. The build fails with
`api.h: No such file or directory`, but `run.py` discards that output. The server then
waits forever for a board that was never flashed.

`run.py` also passes `IMPL` to the server build (line 20). The server Makefile never reads
it, so that does no harm, but it is misleading.

**Suggested fix:**

```python
# run.py:9 – same default as client/Makefile
parser.add_argument('-i', '--impl', nargs='?', default="m4fstack")

# run.py:20 – the server has no IMPL
mk = subprocess.Popen(["make", f"KEM={args.kem}", f"PROTO={args.proto}"], cwd=server_dir, ...)
```

Lines 22 and 26 stay as they are. After this change, BIKE needs an explicit empty value:
`python3 run.py -k bikel1 -i ""`.

### 9.2 OCAKE: AES-CBC only works if the public key size is a multiple of 16 bytes (BIKE)

**Where:**

| | client | server |
|---|---|---|
| `bench_OCAKE/ocake.c` | `apk` declared at line 11, encrypted at line 37, sent at line 41 | received at line 22, decrypted at line 26 |
| `bench_OCAKE_stack/ocake.c` | declared at line 10, encrypted at line 24, sent at line 25 | received at line 22, decrypted at line 26 |
| CBC loop | `client/src/crypto_kem/common/aes.c:505` | `server/src/crypto_kem/common/aes.c:520` |

**Problem:** OCAKE encrypts the KEM public key with AES-CBC under the password-derived key.
That ciphertext is the client's first message. The buffer `apk` is exactly
`CRYPTO_PUBLICKEYBYTES` long, but tiny-AES's CBC functions only process whole 16-byte
blocks (`for (i = 0; i < length; i += AES_BLOCKLEN)`). They don't pad and don't check
the length.

For BIKE-L1 the public key is 1541 bytes:
- The encryption processes 97 blocks (1552 bytes) and writes 11 bytes past `apk` into the
  client's stack.
- Only 1541 bytes are sent, so the last ciphertext block arrives incomplete.
- The server's decryption then also runs 11 bytes past its buffer, and it recovers a wrong
  public key. Key confirmation fails.

ML-KEM is not affected, because its public keys (800, 1184 and 1568 bytes) are all multiples
of 16. **Any KEM whose public key size is not a multiple of 16 will hit the same problem.**

**Suggested fixes** (apply to the client and the server, and to both OCAKE variants):
- *Pad:* define `#define APK_BYTES (((CRYPTO_PUBLICKEYBYTES + 15) / 16) * 16)`. Declare
  `apk` with that size, zero-filled. Then use `APK_BYTES` instead of `sizeof pk` for the
  encryption/decryption and the send/recv, and when hashing `apk` into the transcript
  (client `bench_OCAKE/ocake.c:59`, server `bench_OCAKE/ocake.c:36`). This costs up to
  15 extra bytes on the wire.
- *Or switch to CTR mode,* which has no length restriction. Set `CTR 1` in
  `common/aes.h:23` on both sides and replace the CBC calls with `AES_CTR_xcrypt_buffer`.
  Keep in mind that OCAKE models this step as an ideal cipher, so whichever mode you pick
  is a protocol design decision, not just an implementation detail.

### 9.3 Server: no check whether the serial port opened

**Where:** `server/src/main.c:11`

**Problem:** `initUsart()` returns `-1` when it can't open or configure the port
(`server/src/io.c:8`, `:14`, `:37`). It prints an error, but `main()` ignores the return
value and calls `bench(1)` with `fd = -1`. Every `read()` then fails right away, and the
server prints whatever happens to be in the uninitialised result struct. With a wrong device
name or missing permissions you get an error line followed by garbage numbers, instead of a
clean exit.

**Suggested fix:**

```c
fd = initUsart(argv[1]);          // main.c:11
if (fd < 0) {
    return 1;                     // initUsart() has already printed the reason
}
```

---

## 10. Known bugs

These bugs affect the measured numbers. They're all in the client's
`client/src/proto/bench_OCAKE/ocake.c`, except 10.3.

### 10.1 SHAKE timing subtracts the wrong start value

**Where:** `client/src/proto/bench_OCAKE/ocake.c:25`

**Background:** `b` is the global results struct (`bench_t b;` at `client/src/main.c:10`,
declared `extern` at `ocake.c:5`). Each protocol run adds its timings to `b`
(`ocake.c:94–101`). `bench()` then divides by the number of runs
(`bench_OCAKE/bench.c:14ff`), and `main.c:20` sends `b` to the server. Being a global, `b`
starts out all zero.

**Problem:** line 23 stores the start time in the local variable `shake`, but line 25
computes

```c
shake = hal_get_time() - b.shake;
```

With one run (`bench(1)`, `main.c:19`), `b.shake` is still 0, so the result is the
**absolute timer value**. That is the time since SysTick was started in `hal_setup()`.
Since SHAKE is the first thing measured, this is only slightly more than the SHAKE
duration, so the `SHAKE` values in `bench.txt` (≈12,157 cycles) are close but a bit high.
With more than one run, the second run subtracts the accumulated `b.shake` from a much
later timestamp, and the result becomes meaningless.

**Fix:**

```c
shake = hal_get_time() - shake;
```

**Other variants:** this is the only occurrence.
- `bench_kyber/bench.c:13–29` and `bench_io/bench.c:12–18` also use the form
  `b.x = hal_get_time() - b.x`. That is correct there, because the start time was stored in
  `b.x` itself just before.
- `bench_OCAKE_stack` and the server-side code don't take timings.

### 10.2 Inverted key-confirmation check: the session key is never derived on success

**Where:** `client/src/proto/bench_OCAKE/ocake.c:80–90` and `:99`

**Problem:**

```c
int32_t failure = memcmp(tag_client, tag_server, sizeof tag_client);   // line 80
...
if(failure){                                                            // line 82
    uint64_t shake2 = hal_get_time();
    ... derive session key with SHAKE256 ...
    shake2 = hal_get_time() - shake2;                                   // line 90
}
```

`memcmp` returns 0 when the tags match, so `if(failure)` only derives the session key when
key confirmation **fails**. On every successful run, the final SHAKE256 is skipped and not
included in `PROTO`. In addition, `shake2` is declared inside the block and never added to
`b`, so this step would not show up in `SHAKE` either way. The final derivation is about one
Keccak permutation, which is small compared to `PROTO` (≈19.7M cycles for ML-KEM-512).

**Other variants:** the client's `bench_OCAKE_stack/ocake.c:50` and the server's
`bench_OCAKE/ocake.c:51` and `bench_OCAKE_stack/ocake.c:51` all use `if(!memcmp(...))`,
which is correct.

**Fix:**

```c
uint64_t shake2 = 0;                    // declare before the if
if(!failure){                           // line 82
    shake2 = hal_get_time();
    ...
    shake2 = hal_get_time() - shake2;
}
...
b.shake += shake + shake2;              // line 99
```

Alternatively, record `shake2` in its own field. That needs a new member in `bench_t` in
both `client/src/proto/bench_OCAKE/bench.h` and `server/src/proto/bench_OCAKE/bench.h`, plus
a print line in `server/src/proto/bench_OCAKE/bench.c`.

### 10.3 Serial link: the `0x0D` workaround corrupts or stalls some transfers

**Where:** `server/src/io.c:17–27` (port setup) and `:42–73` (`recv`), and
`client/src/hal.c:109–119` (`send`).

**Background:**
- The server configures the serial port in `initUsart()` but does not switch off input
  translation (`ICRNL`). As a result, Linux turns every `0x0D` byte arriving from the board
  into `0x0A`.
- To work around this, the board's `send()` (`hal.c:112–117`) sends ten `0x0E` bytes after
  every `0x0D`.
- The server's `recv()` (`io.c:51–66`) reads ten more bytes after every `0x0A`. If they are
  all `0x0E`, it changes the byte back to `0x0D`. Otherwise it treats them as data
  (`memcpy` at line 64).

**Problem:** this goes wrong in two cases. Both were reproduced against `bench_server` with a
simulated board.

1. **A real `0x0A` among the last 10 bytes of a message.** The ten-byte lookahead reads past
   the end of the message. It either takes bytes that belong to the next message, or hangs
   if nothing more is coming (e.g. in the final result struct). The `memcpy` at line 64 can
   also write up to 10 bytes past the end of `buf`.
2. **A `0x0D` within 10 bytes after a real `0x0A`.** That `0x0D` arrives as `0x0A` inside the
   lookahead and is copied without being restored. Its ten `0x0E` bytes are then read as
   payload, which corrupts the message and shifts everything after it by 10 bytes.

Only data from the board to the server is affected: the encrypted public key, the 4-byte
`tmp`, the client tag and the result struct. The other direction is fine (`c_oflag = 0`, and
the board's UART doesn't translate). In random-looking data, each real `0x0A` has roughly a
10/256 chance of hitting case 2. An 800-byte encrypted key contains about three of them, so
very roughly 1 in 9 OCAKE runs with ML-KEM-512 is affected, and more with larger keys.
`bench_io` doesn't catch this, because its test pattern (the integers 0–999) never places a
`0x0D` within 10 bytes after a `0x0A`.

**Fix** (change all three parts in the same commit, otherwise the ten `0x0E` bytes arrive as
data):

1. `server/src/io.c`: put the port in raw mode right after `tcgetattr` (after line 15). The
   existing settings on lines 17–27 can stay.
   ```c
   cfmakeraw(&tty);
   ```
2. `server/src/io.c:42–73`: replace `recv()` with a plain read loop:
   ```c
   int recv(int fd, void *buf, size_t nbytes) {
       uint8_t *ptr = (uint8_t *) buf;
       size_t i = 0;
       while (i < nbytes) {
           ssize_t received = read(fd, ptr + i, nbytes - i);
           if (received <= 0) return received;
           i += received;
       }
       return nbytes;
   }
   ```
3. `client/src/hal.c:112–117`: delete the `if(data[i]==13U){ ... }` block, so `send()` just
   sends each byte.

The server side of this fix was tested with a simulated board sending raw bytes: both
failing cases above come through correctly.
