# Caller needs to define KERNEL_SOURCES
export ARCH='arm'
export CROSS_COMPILE='arm-eabi-'

cat config >"$KERNEL_SOURCES"/arch/arm/configs/n5dim_defconfig
make -C "$KERNEL_SOURCES" n5dim_defconfig modules_prepare
