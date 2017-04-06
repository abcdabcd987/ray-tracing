# ray-tracing

## Features

* Phong Model
* Phong Shading
* Multi-threaded Rendering
* Spatial Subdivision Using K-d Tree
* A Graphics User Interface for Development
* Load Scene from `.json` File
* Save Rendered Image to `.png` File
* Effects
    * Soft Shadow
    * Transparency
    * Reflection
    * Refraction
    * Diffusive Reflection
    * Color Bleeding
    * Texture

## Build and Run without GUI

To run the ray tracer, you need to install `libpng` first:

```
ubuntu$ sudo apt install libpng-dev
mac$ sudo brew install libpng
```

Then build and run:

```
$ mkdir -p build && cd build
$ cmake -DGUI=OFF .. && make
$ ./raytracer-cli
usage: ./raytracer-cli [options]
options:
   -w <INT>        image width
   -h <INT>        image height
   -d <INT>        ray tracing depth
   -r <INT>        number of diffuse reflect samples
   -l <FLOAT>      number of light samples per unit volume
   -j <INT>        number of thread workers
   -o <STRING>     path to output png image
   -f <STRING>     path to scene json
```

## Build and Run with GUI

To run GUI, you need to install `GLFW3` and `SDL2` first:

```
ubuntu$ sudo apt install freeglut3-dev libsdl2-dev libpng-dev
mac$ brew install glfw sdl2 libpng
```

Then build and run:

```
$ mkdir -p build && cd build
$ cmake .. && make -j
$ ./raytracer-gui
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

## Scene 1

![Scene 1](rendered/scene1.png?raw=true)

```
$ ./raytracer-cli -w 2400 -h 1800 -d 5 -r 1 -l 1000 -j 8 -o ../rendered/scene1.png -f ../scene/scene1.json
========== scene information ==========
                primitives    4
                 triangles    246354
=========== render settings ===========
                     width    2400
                    height    1800
               trace depth    5
   diffuse reflect samples    1
  light samples per volume    1000.000
                   workers    8
rendered 4320000/4320000 pixels using 8 workers in 299.432s...done
```

- [x] **Soft Shadow**: Look at shadows!
- [x] **Transparency**: You can see through the bunny!
- [x] **Reflection**: The back wall is actually a mirror.
- [ ] **Refraction**: Not in this scene. (actually, the transparent bunny is because of refraction.)
- [ ] **Color Bleeding**: Not in this scene.
- [x] **Texture**: The ground, the back wall and the watermelon are textured.
- [x] **Phong Shading**: You can see the teapot looks better than it would if rendered with flat shading. 

## Scene 2

![Scene 2](rendered/scene2.png?raw=true)

```
$ ./raytracer-cli -w 2400 -h 1800 -d 5 -r 256 -l 1000 -j 8 -o ../rendered/scene2.png -f ../scene/scene2.json
========== scene information ==========
                primitives    7
                 triangles    100966
=========== render settings ===========
                     width    2400
                    height    1800
               trace depth    5
   diffuse reflect samples    256
  light samples per volume    1000.000
                   workers    8
```

- [x] **Soft Shadow**: Look at shadows!
- [ ] **Transparency**: Not in this scene.
- [x] **Reflection**: The back wall is actually a mirror.
- [x] **Refraction**: The ball acts like a magnifier.
- [x] **Color Bleeding**: Look at the ball and the ground.
- [x] **Texture**: The back wall.
- [x] **Phong Shading**: You can see objects looks better than it would if rendered with flat shading.

## Thanks to

* <http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_1_Introduction.shtml>
* <https://www.scratchapixel.com/>
* <https://github.com/ppwwyyxx/Ray-Tracing-Engine>
* (and also you many want to refer to `ref: ` comments in the code)
