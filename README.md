# ZGM-OS Project

## How to compile

### To compile ZGM-OS

1. install riscv toolchain
2. set toolchain binary directory to your PATH
3. `make qemu-build` to copmile
4. `make qemu-debug` to launch debug

see more usages in Makefile


### To compile rustsbi

1. intall rustup and cargo 
2. `rustup target add riscv-unknown-none-elf`
3. `cargo install just`
4. `cargo install cargo-binutils`
5. `cd ./bootloader/SBI/rustsbi/platform/<platform>` and `just build`

## Debug Guide

1. `make qemu-build`
2. `make qemung-debug`
3. `make gdb`


## Dev Guide

1. tag every place where staticly use buddy page with `// STATIC PHYSICAL PAGE ALLOC`
