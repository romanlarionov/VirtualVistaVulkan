
VirtualVistaVulkan
==================

Personal implimentation of a rendering engine written with exclusive support for the Vulkan graphics/compute api. 

Dependencies
------------

* This uses the stb image loading library: https://github.com/nothings/stb
I include stb_image.h

* I also use https://github.com/syoyo/tinygltfloader
for asset importing

Build
-----

Currently only support Windows.

I have exclusively been writing and testing this on Windows 10 with Visual Studio 15. One day I'll extend it to support Linux/OS X and GCC.

Goals
-----

- [x] Renderable OBJ models
- [ ] Light support
- [ ] Renderable OBJ models with multiple submeshes w/ distinct materials
- [ ] Render multiple distinct models
- [ ] Support physically plausible, BRDF based materials
- [ ] Support for area lights with Linearly Transformed Cosines
