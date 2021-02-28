# Particle Editor

A experimental particle editor + header only particle library (particles.h) in early stages of development (ALPHA version).
Made with [raylib](https://github.com/raysan5/raylib) and [raygui](https://github.com/raysan5/raygui)

## Build

On Linux assumming raylib is compiled as a *.so and headers are in `/usr/local/lib/` just run 
`gcc editor.c gui.c -o Editor -I/usr/local/include/ -lraylib -lm -std=c99 -O3`

On Windows/Mac have no idea, sorry!
*...use a build system you say ...what is that?!*

## Features / Usage
- drop a `.dps` file (like the ones in examples) to load a particle system in the editor
- drop a texture when a emitter is active to set/change its texture
- drop a texture when no emitter is active (use the X button) to set/change a placeholder
- CTR+C/CTR+V when mouse is not inside the UI window to copy/paste emitters
- CTR+C/CTR+V when mouse is ove a color widget to copy/paste colors


# Bugs
...over 9000 (well not quite but might be a few)
