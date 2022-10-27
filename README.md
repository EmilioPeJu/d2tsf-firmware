# Diamond II TimeStamp Feeder firmware

The purpose of this project is obtaining the current time and date from GPS and
converting it to the format expected by a EVG or EVM.

Four signals are output:
- shift0 (TTL): a raising edge indicates that a `0` must be shifted to the
    timestamp register
- shift1 (TTL): a raising edge indicates that a `1` must be shifted to the
    timestamp register
- PPS (TTL): Pulse Per Second, this happens always before the shifting of the
    timestamp
- 1MHz (TTL): 1 MHz signal which is synchronous to PPS
