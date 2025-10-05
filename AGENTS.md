# Rive Runtime Windows Contributor Guide

This guide captures Windows-specific entry points, tooling, and workflows for
the C++ runtime. It focuses on the officially supported scripts and project
layout in this repository.

## Renderer and project layout
- `renderer/README.md` documents how to build and run the reference desktop
  sample (`path_fiddle`) with the Rive Renderer, the native GPU renderer that is
  bundled with this repository.【F:renderer/README.md†L1-L23】
- `renderer/premake5_pls_renderer.lua` defines the projects and shader build
  steps for `rive_pls_renderer`, the default renderer library used by desktop
  samples and platform integrations.【F:renderer/premake5_pls_renderer.lua†L1-L129】【F:renderer/premake5_pls_renderer.lua†L191-L232】
- `premake5_v2.lua` configures the core `rive` static library along with feature
  toggles for text, layout, audio, scripting, and tooling so the generated
  solutions share a consistent interface across platforms.【F:premake5_v2.lua†L1-L113】
- Public headers under `include/rive`—for example, `renderer.hpp`—expose the
  renderer, animation, and math APIs that Windows clients link against.【F:include/rive/renderer.hpp†L5-L155】
- Runtime implementation sources in `src/` provide the artboard loading and
  animation plumbing that Windows hosts exercise via the generated Visual Studio
  solutions.【F:src/file.cpp†L1-L159】

## Windows prerequisites
- Install Visual Studio 2022 so the scripts can launch the Developer Command
  Prompt (`VsDevCmd`) and access Windows tooling such as `fxc` and
  `msbuild.exe`. The setup scripts search the default Enterprise and Community
  install locations when Direct3D tools are missing.【F:build/setup_windows_dev.bat†L1-L18】【F:build/setup_windows_dev.ps1†L1-L23】
- Install Git for Windows (or another POSIX-compatible shell) so the batch and
  PowerShell wrappers can invoke `sh build_rive.sh` without manual environment
  configuration.【F:build/build_rive.bat†L1-L9】【F:build/build_rive.ps1†L1-L11】
- Ensure the Clang toolchain is available. The build scripts default the
  Windows toolset to `clang`, and apply Clang-specific warning filters when the
  toolset is not overridden.【F:build/rive_build_config.lua†L41-L58】【F:build/rive_build_config.lua†L300-L335】

## Environment initialization
- `build/setup_windows_dev.bat` appends the repository build scripts to `PATH`,
  sets `RIVE_ROOT`, and launches the Visual Studio 2022 developer environment if
  `fxc` is not already available in the shell.【F:build/setup_windows_dev.bat†L1-L18】
- `build/setup_windows_dev.ps1` performs the same initialization in
  PowerShell, using `Launch-VsDevShell.ps1` when the Direct3D compiler is
  absent.【F:build/setup_windows_dev.ps1†L1-L23】

## Building on Windows
- Use `build\build_rive.bat` (cmd) or `build\build_rive.ps1` (PowerShell) to
  enter the configured environment and forward arguments to the shared
  `build_rive.sh` driver.【F:build/build_rive.bat†L1-L9】【F:build/build_rive.ps1†L1-L11】
- `build/build_rive.sh` selects the Visual Studio 2022 generator on Windows,
  records the Premake arguments under `out/<config>/.rive_premake_args`, builds
  or reuses dependencies under `build/dependencies`, and finally drives
  `msbuild.exe` against the generated solution. It also enforces argument
  consistency so incremental builds warn when options change.【F:build/build_rive.sh†L186-L246】【F:build/build_rive.sh†L266-L341】
- The generated Windows samples (for example, `path_fiddle`) link against
  Direct3D, OpenGL, and GLFW through `renderer/premake5.lua`, which also exposes
  optional integrations such as WebGPU Dawn or Skia via command-line
  toggles.【F:renderer/premake5.lua†L1-L108】【F:renderer/premake5.lua†L109-L188】

## Running Windows tests and samples
- `tests\unit_tests\test.bat` sets up the Visual Studio environment (when
  required) and forwards all options to `tests/unit_tests/test.sh`.【F:tests/unit_tests/test.bat†L1-L9】
- `tests/unit_tests/test.sh` enables Windows defaults like the `clang` toolset,
  invokes `build_rive.sh` with the runtime feature flags needed for unit tests,
  and then runs the resulting binary from the generated output directory.【F:tests/unit_tests/test.sh†L1-L83】【F:tests/unit_tests/test.sh†L96-L127】
- Desktop samples such as `path_fiddle` use `TestingWindow` to create Direct3D,
  OpenGL, or other renderer backends. The helper lives in
  `tests/common/testing_window.hpp` with platform-specific implementations under
  `tests/common/`.【F:tests/common/testing_window.hpp†L1-L120】【F:tests/common/testing_window.hpp†L210-L290】

## Optional renderer integrations
- The Rive Renderer (`rive_pls_renderer`) is included by default. To experiment
  with alternative renderers, pass the relevant Premake toggles. For example,
  `--with-skia` pulls in the Skia renderer support files and libraries, while
  `--with-dawn` and `--with-webgpu` enable WebGPU variants.【F:renderer/premake5.lua†L1-L89】【F:renderer/premake5.lua†L90-L156】
- Shader assets for the Rive Renderer are compiled ahead of time by the Premake
  logic in `renderer/premake5_pls_renderer.lua`, which builds Direct3D and SPIR-V
  outputs on Windows when the associated options are present.【F:renderer/premake5_pls_renderer.lua†L63-L129】【F:renderer/premake5_pls_renderer.lua†L129-L189】

## Community-reported Windows 11 build notes *(unconfirmed)*
The following reports are community sourced and have not been verified by the
core maintainers. Treat them as hypotheses to try when diagnosing local issues.

### Suggested environment setup *(unconfirmed)*
- Some contributors install additional Visual Studio workloads (such as .NET
  desktop development or Game development with Unity) alongside Desktop
  development with C++ to ensure optional tools are present.
- Git Bash is used to launch `cmd.exe`, then `build\setup_windows_dev.bat`, so
  Visual Studio tooling and Unix-style commands can co-exist in the same shell.
- PATH tweaks may be necessary for `msbuild.exe` or GNU Make when the default
  installation paths differ from the scripts’ expectations.
- Installing the Clang/LLVM toolchain for MSVC (`clang-cl`) has resolved toolset
  selection issues for some setups.

### Workflow reported to succeed *(unconfirmed)*
1. Open the repository inside Git Bash.
2. Launch `cmd.exe` from Git Bash, then run `build\setup_windows_dev.bat` to
   enter a Visual Studio 2022 Developer Command Prompt that still exposes
   Unix-like commands.
3. Invoke `build_rive.bat rebuild out/release` (sometimes from the `renderer`
   directory) instead of building from the Visual Studio IDE to avoid shell
   incompatibilities.
4. Inspect generated artifacts under `renderer/out/release/`, including
   `path-fiddle.exe` and `rive.sln`.

### Community workarounds *(unconfirmed)*
- Adjusting `renderer/src/shaders/Makefile` so `fxc` invocations quote Windows
  paths or point to the desired Make binary has unblocked shader compilation in
  some environments.
- Adding `require("rive_build_config")` to `premake5.lua`, manually invoking
  `dependencies/premake-core/bin/release/premake5.exe`, and editing generated
  Visual Studio projects to remove unsupported flags (for example,
  `Wno-atomic-alignment`) have helped contributors who run into Premake or
  project configuration errors.
- Installing GNU Make, exposing `msbuild.exe` on `PATH`, or toggling Release
  preprocessor definitions inside Visual Studio have been cited as necessary for
  certain configurations.

### Additional observations *(unconfirmed)*
- The `yup` repository (https://github.com/kunitoki/yup) offers a CMake-based
  integration of the Rive renderer that some contributors use as a reference.
- Updating Direct3D 12 upload-heap resource states to
  `D3D12_RESOURCE_STATE_GENERIC_READ` resolved initialization failures on
  enterprise NVIDIA drivers for one community member.
