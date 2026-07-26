mkdir build
set -e
cd build
cmake ..
make -j2

#./build/Anodyne
./CompileContent
cd ..


