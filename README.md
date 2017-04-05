# ray-tracing

## Build and Run without GUI

To run the ray tracer, you need to install `libpng` first:

```
ubuntu$ sudo apt install libpng-dev
mac$ sudo brew install libpng
```

Then build and run:

```bash
mkdir -p build && cd build
cmake -DGUI=OFF ..
make
./raytracer-cli
```

## Build and Run with GUI

To run GUI, you need to install `GLFW3` and `SDL2` first:

```
ubuntu$ sudo apt install freeglut3-dev libsdl2-dev libpng-dev
mac$ brew install glfw sdl2 libpng
```

Then build and run:

```bash
mkdir -p build && cd build
cmake ..
make -j
./raytracer-gui
```

## Run GUI on Remote Server

Remote OpenGL program can be forwarded to local using [xpra](https://xpra.org).

```
server$ sudo apt install xpra
server$ xpra start :100
server$ export DISPLAY=:100
server$ ./raytracer-gui
server$ xpra stop

ubuntu-client$ sudo apt install xpra
ubuntu-client$ xpra attach ssh:host:port:100
mac-client$ brew cask install xpra
mac-client$ /Applications/Xpra.app/Contents/MacOS/Xpra attach ssh:host:port:100
```
