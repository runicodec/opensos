# Building OpenSoS

## Windows

### Prerequisites

1. **Visual Studio 2019 or 2022** with the "Desktop development with C++" workload
   - This includes MSVC, CMake, and Ninja — no separate installs needed
   - Download from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/)
   - The free "Community" edition works fine; "Build Tools for Visual Studio" also works if you don't want the full IDE

2. **Git** — [git-scm.com](https://git-scm.com/)

3. **vcpkg**
   ```bat
   git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
   C:\vcpkg\bootstrap-vcpkg.bat
   ```
   Then set the `VCPKG_ROOT` environment variable to `C:\vcpkg` (or wherever you cloned it).
   You can do this permanently via System Properties > Environment Variables, or per-session:
   ```bat
   set VCPKG_ROOT=C:\vcpkg
   ```

### Build steps

Open a **"x64 Native Tools Command Prompt for VS 2022"** (or 2019) — search for it in the Start menu. This sets up the MSVC compiler environment that CMake and Ninja need.

```bat
git clone https://github.com/your-org/OpenSoS.git
cd OpenSoS

cmake --preset windows-debug
cmake --build --preset windows-debug
```

The executable will be at `build\Debug\sosandce.exe`. Runtime DLLs are copied automatically by vcpkg alongside the executable.

For a release build:
```bat
cmake --preset windows-release
cmake --build --preset windows-release
```

### Visual Studio IDE

If you prefer working inside Visual Studio rather than the command line:
1. Open Visual Studio
2. Choose "Open a local folder" and select the repo root
3. Visual Studio will detect `CMakePresets.json` automatically
4. Select the `windows-debug` or `windows-release` preset from the toolbar
5. Make sure `VCPKG_ROOT` is set before launching Visual Studio, or set it in `CMakeSettings.json`

### First-time dependency install

On the first configure, vcpkg will download and build all dependencies (SDL2, RmlUi, etc.). This takes several minutes but is only needed once. Subsequent builds are fast.

---

## macOS / Linux

### Prerequisites

1. **Nix** — [nixos.org/download](https://nixos.org/download/)
   ```sh
   sh <(curl -L https://nixos.org/nix/install)
   ```

2. **devenv** — [devenv.sh](https://devenv.sh/)
   ```sh
   nix-env -iA devenv -f https://github.com/NixOS/nixpkgs/tarball/nixpkgs-unstable
   ```
   Or follow the [official devenv installation guide](https://devenv.sh/getting-started/).

### Build steps

```sh
git clone https://github.com/your-org/OpenSoS.git
cd OpenSoS

devenv shell
devenv tasks run sosandce:build
```

The executable will be at `build/sosandce`. Assets are copied to `build/assets/` automatically.

For a release build:
```sh
devenv tasks run sosandce:release
```

### Manual CMake (without devenv tasks)

From inside `devenv shell`, you can also invoke CMake directly using the presets:

```sh
cmake --preset macos-debug
cmake --build --preset macos-debug
```

---

## Cleaning the build

Delete the `build/` directory and reconfigure:

```sh
# macOS / Linux
rm -rf build

# Windows
rmdir /s /q build
```
