# netlink
基于UV库框架实现的有线网络连接检测

# compile
## gcc
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=/home/CUAV/kaisen/opt/host-local -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

## arm
cmake .. -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc -DCMAKE_PREFIX_PATH=/home/CUAV/kaisen/opt/host-arm -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

or

cmake .. -DCMAKE_TOOLCHAIN_FILE="/home/CUAV/kaisen/rv1126_rv1109_linux_v2.1.0_20210512/buildroot/output/rockchip_ltelink2/host/share/buildroot/toolchainfile.cmake" \
         -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
