# Low latency orderbook
Builds  
cmake -S .. -B . -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=/root/vcpkg/scripts/buildsystems/vcpkg.cmake  
cmake --build .  

Release builds
cmake -S .. -B . -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/root/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .

Build and Perf  
sudo cpupower frequency-set -g performance
sudo sysctl -w kernel.perf_event_paranoid=1
cmake --build . && perf stat -d -d -d ./benchmarks/llob_bm all 1000000 500 1  


