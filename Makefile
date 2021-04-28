TOOLCHAIN:=/opt/riscv-gnu-toolchain/bin
SBI_DIR:=./bootloader/SBI/rustsbi/target/riscv64imac-unknown-none-elf/debug
SBI_NAME_K210:=rustsbi-k210
SBI_NAME_QEMU:=rustsbi-qemu
BIN_SUFFIX:=bin

k210-build:
	@if [ ! -f  "./build/Makefile" ]; then mkdir -p ./build && cd build && cmake .. -DTOOLCHAIN=${TOOLCHAIN}; fi
	cd ./build && make -j$(nproc)
	@if [ ! -f "${SBI_DIR}/${SBI_NAME_K210}" ]; then \
		cd ./bootloader/SBI/rustsbi/platform/k210 && just build; \
	fi
	cp --force ${SBI_DIR}/${SBI_NAME_K210} ./build/
	cp --force ${SBI_DIR}/${SBI_NAME_K210}.${BIN_SUFFIX} ./build/

qemu-build:
	@if [ ! -f  "./build/Makefile" ]; then mkdir -p ./build && cd build && cmake .. -DTOOLCHAIN=${TOOLCHAIN}; fi
	cd ./build && make -j$(nproc)
	@if [ ! -f "${SBI_DIR}/${SBI_NAME_QEMU}" ]; then \
		cd ./bootloader/SBI/rustsbi/platform/qemu && just build; \
	fi
	cp --force ${SBI_DIR}/${SBI_NAME_QEMU} ./build/
	cp --force ${SBI_DIR}/${SBI_NAME_QEMU}.${BIN_SUFFIX} ./build/

qemu-debug:
	echo "Starting qemu debug"



all: k210-build qemu-build
	echo "Build k210 image and qemu image"

clean :
	rm -rf ./build
	rm -rf ${SBI_DIR}/*


