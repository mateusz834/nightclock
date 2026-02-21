# Nightclock

Proxy for a [TM1640](https://dl.icdst.org/pdfs/files3/1a3723ca081e8824039eb846c3e42050.pdf) LED digital clock, that
intercepts all data sent from the digital clock controler to the TM1640 and overrides the brightness to my needs.

## Setup & Build

```sh
sudo pacman -Sy cmake arm-none-eabi-newlib arm-none-eabi-gcc
```

Clone [pico-sdk](https://github.com/raspberrypi/pico-sdk) with all submodules.

```sh
git clone --recurse-submodules https://github.com/raspberrypi/pico-sdk.git
```

```sh
PICO_SDK_PATH=/path/to/pico-sdk cmake -S . -B build
cmake --build build --target clock
cp build/clock.uf2 /mount/rpi-pico
```
