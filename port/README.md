# Thendoria Modern Port (WIP)

This folder is the start of a modern port of the DOS Turbo C++ game.

## Feasibility

Porting is very possible, but not a one-click rebuild. The current code uses DOS-specific APIs and memory model assumptions:

- `<conio.h>`, `<dos.h>`, direct ports like `inport(0x60)`
- Mode 13h VGA memory (`0xA000`) and far pointers
- `farmalloc/farfree`, `_fmemcpy`, `_fmemset`
- BIOS timing and retrace behavior

A direct compile in Code::Blocks with a modern compiler will fail without replacing those systems.

## Recommended approach

Use SDL2 as the platform layer and port in stages:

1. Window, input, timing, audio
2. Software frame buffer abstraction replacing `Graph`
3. Sprite/frame blit functions replacing old draw primitives
4. Dialog and battle screens
5. World map and interaction loop

## Build (CMake)

Requirements:

- CMake 3.16+
- SDL2 development package
- SDL2_image development package
- A C++17 compiler (MinGW, MSVC, Clang)

Commands:

```bash
cmake -S . -B build
cmake --build build
./build/thendoria_port
```

## Code::Blocks

You can open CMake projects from Code::Blocks with CMake support, or generate a Code::Blocks project using CMake generator if available in your setup.

## Convert PCX to PNG

The port includes a converter script at `tools/convert_pcx_to_png.py`.

Install dependency:

```bash
py -m pip install -r tools/requirements.txt
```

Convert default assets:

```bash
py tools/convert_pcx_to_png.py
```

Custom paths:

```bash
py tools/convert_pcx_to_png.py --src ../IMG --dst ../assets_png/IMG
```

`SpriteCompat` now supports loading sprite sheets from PNG with `cargarSpritePNG(path)`.

## Next milestones

- `Renderer320x200` compatibility class implemented in `src/Renderer320x200.*` with:
	- `vga()`, `pv1()`, `pv2()` frame buffers
	- `clr`, `volcar`, `putpixel`, `getpixel`, `getframe`, `putframe`
	- `hline`, `vline`, `fillbox`, `box`, `line`, `wait_retrace`
	- 8-bit indexed palette to SDL texture presentation
- Load existing TXT maps/dialogs from `../MAPS` and `../DIALOGS`
- Add a PCX loader replacement path (convert assets or decode PCX)

## Immediate migration path

1. `GraphCompat` wrapper added with old-style pointers (`vga`, `pv1`, `pv2`) and Graph-like methods.
2. `putframe/getframe` and drawing primitives available over indexed buffers.
3. `SpriteCompat` wired to `GraphCompat` (`crear`, `posicionar`, `dibujar`, `dibujart`, `animar`).
4. PNG sprite loading added in `SpriteCompat::cargarSpritePNG(path)`.
5. Map rendering bootstrap added: `MapData` loads `MAPS/*.TXT` and `TileSetCompat` draws tiles from `assets_png/IMG/16x16.png`.
6. Basic camera and collision movement added in `main.cpp` (20x12 viewport, arrow-key movement, `solido` checks).
7. Smooth tile scroll and centered player rendering integrated (closer to original exploration feel).
8. Interaction added (`SPACE`) for `PREGUNTA`, `ACCION`, and `SALIDA`; dialogs page with `ENTER` and map transitions load from `MAPS/<archivo>.TXT`.
9. `FontCompat` added with `putstr(...)` style rendering (foreground/background colors) and accent normalization for dialog text.
10. Next: battle overlays and status UI.
