# pj64 input plugin with direct N64 controller access for raphnet adapters

This input plugin for project64 uses the direct controller access feature offered by raphnet N64 to USB
adapters (versions 3 and up) to let the emulated game communicate with the controllers directly.

* Perfect accuracy: The controller works exactly as it would on a N64 system, without any calibration or tweaking.
* Rumble Pak and Controller Pak (mempak) support (and potentially other accesories too [not confirmed])
* Support for other peripherals that connect to controller ports (eg: N64 mouse) [not confirmed]
* Low latency

## Project homepage

* English: [Direct N64 controller access plugin for mupen64plus and project 64](http://raphnet.net/programmation/mupen64plus-input-raphnetraw/index_en.php)
* French: [Plugin d'accès direct aux manettes N64 pour mupen64plus et project 64](http://raphnet.net/programmation/mupen64plus-input-raphnetraw/index.php)

## License

This project is licensed under the terms of the GNU General Public License, version 2.
Source code is available on the project homepage and on GitHub.

## Using the plugin

After compiling (or using the binary distribution) two .DLL files must be copied to specific
locations.

1. Copy the following file to `"Project64 installation directory"`/`Plugin`/`Input`:
  * pj64raphnetraw.dll

2. Copy the following file to `"Project64 installation directory"` (i.e. The one with a .EXE)
  * libhidapi-0.dll

Then in project64, go to `menu bar` -> `Options` -> `Settings` -> `Plugins`. Next in the
`Input (controller) plugin` dropdown, select "raphnetraw for Project64 version xx.xx".
Apply and exit the dialog box.

## Building the plugin

The plugin is compiled using mingw-w64 under Linux.

### System requirements

A Linux system such as Debian with the following packages installed: (non-exhaustive list)

* mingw-w64
* autotools-dev
* autoconf
* automake
* libtool
* git
* make

### Environment

#### Directory layout

An HIDAPI build is expected in ../hidapi. i.e. The following directory structure is required:

* ...somedir/pj64raphnetraw
* ...somedir/hidapi

#### preparing the hidapi build

The plugin makefile assumes the following:

* An hidapi configured and compiled with --host i686-w64-mingw32 found in ../hidapi

hidapi can be obtained like this:

```sh
$ cd ...somedir
$ git clone https://github.com/signal11/hidapi.git
```

And then to compile it:
```sh
cd ...somedir/hidapi
./bootstrap
./configure --host i686-w64-mingw32
make
```

#### Compilation

```bash
$ cd .../somedir/
$ cd pj64raphnetraw/src
$ make
```


