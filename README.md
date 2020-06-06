A gameboy emulator meant to run on N64 hardware.

To build, you will need to locate the original gameboy's boot rom and save it to a file named `data/dmg_boot.bin`.
You should be able to find that file here
[https://gbdev.gg8.se/files/roms/bootroms/](https://gbdev.gg8.se/files/roms/bootroms/)

You will also need to supply the gameboy rom you want to build into an N64 rom. You should put the gameboy rom into the `data` folder
and update the include path in [spec](./spec) under the `gbrom` section