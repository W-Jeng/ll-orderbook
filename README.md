# Low latency orderbook
Builds  
cmake -S .. -B . -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=/root/vcpkg/scripts/buildsystems/vcpkg.cmake  
cmake --build .  

Release builds
cmake -S .. -B . -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/root/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .



