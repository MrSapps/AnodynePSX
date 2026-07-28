mkdir build
set -e
cd build
cmake ..
make -j2

#./build/Anodyne
./CompileContent
cd ..
make
~/Downloads/mGBA-0.10.5-appimage-x64.appimage anodyne.gb


