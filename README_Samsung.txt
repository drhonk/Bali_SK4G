HOW TO BUILD KERNEL

1. Visit http://www.codesourcery.com/, download and install Sourcery G++ Lite 2009q3-68 toolchain for ARM EABI.

2. Extract kernel source and move into the top directory.

3. Execute 'make sidekick_rev02_defconfig'.

4. Execute 'make' or 'make -j<n>' where '<n>' is the number of multiple jobs to be invoked simultaneously.
