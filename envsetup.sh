# Caller needs to define KERNEL_SOURCES
export ARCH='arm'
export CROSS_COMPILE='arm-eabi-'

cat config >"$KERNEL_SOURCES"/.config
make -C "$KERNEL_SOURCES" silentoldconfig scripts
