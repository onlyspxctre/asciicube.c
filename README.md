# asciicube.c

A spinning cube rendered in the terminal with ASCII characters, written in **pure C**. Calculates the positions of points on the 3D cube with rotation matrices and linear algebra, and then projects these points onto a 2D surface (aka the terminal screen) with each surface represented by an ASCII character. Inspired heavily by <see below>.

This project notably comes with a bunch of improvements compared to **cube.c**, namely:

- Improved math code precision and cleanliness (original had `float` -> `int` truncation, leading to inaccuracy and rendering issues)
- Proper viewport transformation from coordinates to screen buffer and ASCII rasterization code
- Dynamic viewport scaling based on terminal width and column (capped at `320x76`)
- Support for `-O2` optimizations (original code had speedup issues with `usleep()` due to physics not tied to delta time)
    - Alternate buffer and hidden cursor escape codes (fixes a lot of flickering issues when using optimizations)
- Buffered writes to `stdout` (reduce the amount of write overhead drastically)
- Proper frame times and frame pacing, rendering at steady 60 FPS (or whatever you prefer)

## TODO

- [ ] Support for transparent background
- [x] Support for dynamic viewport (fullscreen to alternate buffer)
- [ ] Lambertian reflectance instead of static characters for each surface face
- [ ] Rendering other shapes beyond just a cube
- [ ] Controlling the rotation of the cube with keyboard shortcuts

## Inspiration

- [tarantino07/cube.c](https://github.com/tarantino07/cube.c)
- [donut.c](https://github.com/tarantino07/cube.c)

## License

This project is licensed under the [MIT License](LICENSE).
