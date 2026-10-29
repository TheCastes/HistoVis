# HistoVis

Toolkit for histogram operations on images. It includes:
  - Visualizations of image histogram and CDF in RGB, YCbCr and Lab
  - Global and local histogram equalization
  - Histogram matching

It can be used with a GUI (written with Qt), or as a CLI if you prefer.

## Download

Prebuilt binaries are on the [Releases](https://github.com/TheCastes/HistoVis/releases) page.

- **Windows**: unzip and run `histovis_gui.exe` or `histovis.exe`.
- **Linux** (x64, Ubuntu 24.04+, Fedora 40+, Arch): the GUI needs Qt 6.4 or newer:
  ```sh
  sudo apt install libqt6widgets6 qt6-qpa-plugins   # Debian / Ubuntu
  sudo dnf install qt6-qtbase-gui                    # Fedora
  sudo pacman -S qt6-base                            # Arch
  ```

## Build

Requirements: CMake >= 3.21 and a C++20 compiler. Thehe GUI needs Qt 6.4 or newer GUI also needs Qt 6.3 or newer (Widgets).
Image loading and saving use [stb](https://github.com/nothings/stb).

If you want to build only the CLI, turn HISTOVIS_BUILD_GUI flag in CMake as OFF.

### Linux

Dependencies (if you just want the CLI, qt6 is not needed):
```sh
# Debian / Ubuntu
sudo apt install build-essential cmake qt6-base-dev libgl1-mesa-dev
# Fedora
sudo dnf install gcc-c++ cmake qt6-qtbase-devel
# Arch
sudo pacman -S base-devel cmake qt6-base
```

Build:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Windows

Install Qt 6 with the Qt Online Installer, choosing the kit that matches your compiler
(MSVC 2022 64-bit or MinGW 64-bit). Then, from a Visual Studio *x64 Native Tools* prompt
(MSVC) or a Qt MinGW prompt (MinGW):

```bat
cmake -B build -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2022_64
cmake --build build --config Release
```

Replace the path with your Qt kit (e.g. `C:\Qt\6.x.x\mingw_64`). To run the GUI outside Qt Creator,
copy the Qt DLLs next to the executable:

```bat
C:\Qt\6.x.x\msvc2022_64\bin\windeployqt.exe build\src\gui\Release\histovis_gui.exe
```

With MinGW (single-config generator) the executable is in `build\src\gui\` instead of `build\src\gui\Release\`.

## CLI

```
histovis <command> [arguments] [options]
```

| Command                                       | Description                                   |
|-----------------------------------------------|-----------------------------------------------|
| `hist <input> [-o <file>] [-s <space>] [-l]`  | Export the histogram as CSV                   |
| `pdf  <input> [-o <file>] [-s <space>] [-l]`  | Export the PDF as CSV                         |
| `cdf  <input> [-o <file>] [-s <space>] [-l]`  | Export the CDF as CSV                         |
| `equalize <input> <output> [-m <mode>] [-w <n>] [-s <space>]` | Histogram equalization        |
| `match <input> <output> -r <ref> [-s <space>]` | Histogram matching against a reference image |

| Option                      | Description                                                              |
|-----------------------------|--------------------------------------------------------------------------|
| `-o, --output <file>`       | Output file for hist/pdf/cdf (default: stdout)                           |
| `-s, --space <space>`       | hist/pdf/cdf: channels to export, `rgb`, `ycbcr` or `lab` (default: `rgb`) |
|                             | equalize/match: luminance space, `ycbcr` or `lab` (default: `ycbcr`)     |
| `-l, --luminance`           | Export only the luminance: Y of YCbCr, or L* with `-s lab`               |
| `-m, --mode <global\|local>` | Equalization mode (default: `global`)                                   |
| `-w, --window <n>`          | Window size for local mode, odd >= 3 (default: 1/8 of the shorter side)  |
| `-r, --reference <file>`    | Reference image for match                                                |
| `-h, --help`                | Show the help                                                            |

Output images can be `.png`, `.jpg/.jpeg`.

Examples:

```sh
histovis hist photo.jpg -s lab -o hist.csv          # columns value,l,a,b
histovis cdf photo.jpg -l                           # luminance CDF (Y) on stdout
histovis equalize photo.jpg out.png -m local -w 31  # local equalization, 31x31 window
histovis match photo.jpg out.png -r reference.jpg -s lab
```

## License

HistoVis is released under the [MIT License](LICENSE).

Third-party code keeps its own license:
- [stb](https://github.com/nothings/stb) (`external/`): MIT or public domain.
- [Qt 6](https://www.qt.io/) (GUI only): LGPLv3, linked dynamically.