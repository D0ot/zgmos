# Make Uitls 
#
# Usages: 
# in the project root folder
#
# 'make all' to build for qemu and k210
# 'make qemu-build' build for qemu
# 'make k210-build' etc...
# 'make clean' remove build directory, rustsbi build is not cleaned
#
# 'make sbi-build' to build rustsbi, you should install all required dependencies yourself
# 'make sbi-clean' to clean the rustsbi build
#
# 'make qemu-debug' to launch qemu debug environment, no graphic

# We can modify the default toolchain used here
# do not commit changes here !!! 
TOOLCHAIN:=""
SBI_LATEST_DIR:=./bootloader/SBI/rustsbi/target/riscv64imac-unknown-none-elf/debug
SBI_PRECOMPILED_DIR:=./bootloader/SBI
SBI_NAME_K210:=rustsbi-k210
SBI_NAME_QEMU:=rustsbi-qemu
BIN_SUFFIX:=bin

k210-build:
	@if [ ! -f  "./build/Makefile" ]; then mkdir -p ./build && cd build && cmake .. -DTOOLCHAIN=${TOOLCHAIN} -DK210=1; fi
	cd ./build && make -j$(nproc)
	@if [ -f "${SBI_LATEST_DIR}/${SBI_NAME_K210}" ]; then \
		cp --force ${SBI_LATEST_DIR}/${SBI_NAME_K210} ./build/; \
		cp --force ${SBI_LATEST_DIR}/${SBI_NAME_K210}.${BIN_SUFFIX} ./build/; \
	else \
		cp --force ${SBI_PRECOMPILED_DIR}/${SBI_NAME_K210} ./build/; \
		cp --force ${SBI_PRECOMPILED_DIR}/${SBI_NAME_K210}.${BIN_SUFFIX} ./build/; \
	fi
	


qemu-build:
	@if [ ! -f  "./build/Makefile" ]; then mkdir -p ./build && cd build && cmake .. -DTOOLCHAIN=${TOOLCHAIN} -DQEMU=1; fi
	cd ./build && make -j$(nproc)
	@if [ -f "${SBI_LATEST_DIR}/${SBI_NAME_QEMU}" ]; then \
		cp --force ${SBI_LATEST_DIR}/${SBI_NAME_QEMU} ./build/; \
		cp --force ${SBI_LATEST_DIR}/${SBI_NAME_QEMU}.${BIN_SUFFIX} ./build/; \
	else \
		cp --force ${SBI_PRECOMPILED_DIR}/${SBI_NAME_QEMU} ./build/; \
		cp --force ${SBI_PRECOMPILED_DIR}/${SBI_NAME_QEMU}.${BIN_SUFFIX} ./build/; \
	fi

QEMU_OPTIONS = -machine virt
QEMU_OPTIONS += -kernel ./build/zgmos
QEMU_OPTIONS += -bios ./build/rustsbi-qemu
QEMU_OPTIONS += -m 8M
QEMU_OPTIONS += -smp 2
QEMU_OPTIONS += -S -gdb tcp::3008
QEMU_OPTIONS += -drive file=fs.img,if=none,format=raw,id=x0
QEMU_OPTIONS += -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0
QEMU_OPTIONS += -global virtio-mmio.force-legacy=false

# TODO
qemu-debug:
	echo "Starting qemu debug"
	qemu-system-riscv64 ${QEMU_OPTIONS}

QEMU_OPTIONS_NG = ${QEMU_OPTIONS}
QEMU_OPTIONS_NG += "--nographic"

# TODO
qemung-debug:
	echo "Starting qemu debug, nographic"
	qemu-system-riscv64 ${QEMU_OPTIONS_NG}

# TODO
gdb:
	riscv64-unknown-elf-gdb -s ./build/zgmos -x ./scripts/gdbinit

	


sbi-build:
	@if [ ! -f "${SBI_LATEST_DIR}/${SBI_NAME_K210}" ]; then \
		cd ./bootloader/SBI/rustsbi/platform/k210 && just build; \
	fi
	@if [ ! -f "${SBI_LATEST_DIR}/${SBI_NAME_QEMU}" ]; then \
		cd ./bootloader/SBI/rustsbi/platform/qemu && just build; \
	fi

sbi-clean:
	rm -rf ${SBI_LATEST_DIR}/*
	

all: k210-build qemu-build
	echo "Build k210 image and qemu image"

clean :
	rm -rf ./build

