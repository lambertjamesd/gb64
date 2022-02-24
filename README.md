A gameboy emulator meant to run on N64 hardware.

If you just want to use the emulator go here
[https://lambertjamesd.github.io/gb64/romwrapper/romwrapper.html](https://lambertjamesd.github.io/gb64/romwrapper/romwrapper.html)

To build, you will need to locate the original gameboy's boot rom and save it to a file named `data/dmg_boot.bin`. As well as `data/cgb_bios.bin` for the gameboy colors.
You should be able to find both of these files here
[https://gbdev.gg8.se/files/roms/bootroms/](https://gbdev.gg8.se/files/roms/bootroms/)

You will also need to supply the gameboy rom you want to build into an N64 rom. You should put the gameboy rom into the `data` folder
and update the include path in [spec](./spec) under the `gbrom` section