# Edraw - A drawing library for embedded systems #
Edraw is a free software drawing library that aims to be a replacement for
libcairo on embedded devices (especially ebook readers with epaper displays).

Embedded systems usually have one kind of display and, unlike on desktop pcs,
the applications do not need to support all kinds of hardware. As a consequence
edraw is designed to be compiled for a single kind of output devices.

Features:
* Cairo-like library interface
* High efficency by a simple desgin
* Support for grayscale displays including color translation
* Small and modular codebase

Planned features:
* Bezier curve support
* Xlib backend for debugging
* Truecolor framebuffer backend

## Building the library ##
As edraw wants to be simple, it does not use autoconf/automake/etc. Instead it
can be compiled using a hand-written makefile.

Example to cross compile edraw statically with dietlibc for an arm-based ebook
reader at debug level 'fatal':

    make HOST="diet arm-linux-gnueabi-" DEBUG=fatal

A successful compilation results in the symlink 'edraw.a' that points to the
library archive you can link your programs against.

### Dependencys ###
The only dependency is a libc implementation. It is successfully tested with
* glibc
* dietlibc

### Compile options ###
CC: The C compiler to use

HOST: Prefix for all build commands (use it for cross compiling)

DEBUG: Set the debug level
* fatal -> print error messages on fatal errors, then exit
* warn -> additionally print warnings
* debug -> show debug messages (for development)
* calls -> inform about all calls of library functions (useful for finding
  segfaults)

## License ##
All files have a copyright/license notice at their beginning. The license
generally used for this project is the ISC license.

