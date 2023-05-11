# Unit Test

Unit tests for emulated chips and cores.

![UnitTest](/UserManual/imgstore/UnitTest.jpg)

Each emulated chip class contains a friendly class that has access to all internals.

- M6502Core: M6502CoreUnitTest::UnitTest
- APUSim: APUSimUnitTest::UnitTest
- PPUSim: PPUSimUnitTest::UnitTest

## MegaCycles

Right now there are three tests that are not running successfully. They are used to measure the performance of the 6502 Core, APU and PPU.

Current performance:

```
Core executed 1789772 cycles in real 2282 msec
You're 2.28 times slower :(

APU+Core executed 21477272 cycles in real 18297 msec
You're 18.30 times slower :(

PPU executed 21477272 cycles in real 21828 msec
You're 21.83 times slower :(
```
