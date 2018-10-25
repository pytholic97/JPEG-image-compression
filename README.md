# JPEG Compression in C

The credit for the following files is due to [nothings/stb](https://github.com/nothings/stb)

1. stb_image.h
2. stb_image_write.h

## Usage

Compiling main.c:

```
gcc -o mn main.c -lm
```
Invoking elf:

```
./mn <path to image> image_height image_width
```
The output image after recovery from the compression is stored in the same path
as the elf file with the name: 'jpeg_comp.bmp'.

## TODO
1. Test and verfiy for various images.
2. Change q50 dynamically and observe compression.
3. Web based interface.
