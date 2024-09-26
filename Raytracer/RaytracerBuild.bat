mkdir build
pushd build
cl -Zi ..\Raytracer.cpp gdi32.lib user32.lib
popd
