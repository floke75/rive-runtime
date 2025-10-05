# Rive Runtime Windows Contributor Guide

This guide documents the Windows-specific parts of the C++ runtime that
contributors interact with, including repository entry points, setup steps,
build workflows, and testing or sample runners.

## Windows prerequisites
- Install Visual Studio 2022 with the Desktop development with C++ workload so
the build scripts can call the Developer Command Prompt/PowerShell shortcuts
and access the Direct3D compiler (`fxc`).【F:build/setup_windows_dev.bat†L1-L18】【F:build/setup_windows_dev.ps1†L1-L23】
- Install Git for Windows (Git Bash) so the batch/PowerShell wrappers can invoke
`sh build_rive.sh` and other POSIX shell scripts without manual environment
setup.【F:build/build_rive.bat†L1-L8】【F:build/build_rive.ps1†L1-L11】
- Ensure CMake is available (bundled with Visual Studio 2022) before building
the bundled GLFW dependency used by the Windows samples.【F:skia/dependencies/make_glfw.sh†L1-L25】

## Key directories and entry points
- `premake5_v2.lua` defines feature toggles (text, layout, scripting, audio) and
applies the Windows static-library configuration for the core `rive` target that
Visual Studio solutions consume.【F:premake5_v2.lua†L3-L164】
- Public headers in `include/rive`, such as the renderer interface in
`renderer.hpp`, expose the rendering, math, and animation APIs that Windows
clients link against.【F:include/rive/renderer.hpp†L5-L155】
- Runtime implementation sources under `src/` import `.riv` files, instantiate
artboards/state machines, and surface data-binding helpers consumed by Windows
apps.【F:src/file.cpp†L1-L159】
- The Direct3D backends live in `renderer/include/rive/renderer/d3d11` and
`renderer/include/rive/renderer/d3d12`, detailing render-target ownership,
pipeline state, and resource helpers for Windows GPU workloads.【F:renderer/include/rive/renderer/d3d11/render_context_d3d_impl.hpp†L5-L170】【F:renderer/include/rive/renderer/d3d12/render_context_d3d12_impl.hpp†L5-L160】
- `renderer/premake5.lua` wires Windows samples (for example, `path_fiddle`) to
GLFW, Direct3D libraries, and optional Skia/Dawn integrations while enabling the
runtime feature toggles selected during generation.【F:renderer/premake5.lua†L1-L176】
- The shared `TestingWindow` abstraction in `tests/common/testing_window.hpp`
exposes Direct3D 11/12 backends plus utility hooks for Windows-focused renderer
and sample tests.【F:tests/common/testing_window.hpp†L5-L199】
- Sample runners like `tests/player/player.cpp` demonstrate how Windows targets
obtain a `TestingWindow`, load `.riv` files, and drive animation update loops for
manual validation.【F:tests/player/player.cpp†L5-L190】

## Environment initialization
- Run `build\setup_windows_dev.bat` from `cmd.exe` to append the repository
build scripts to `PATH`, set `RIVE_ROOT`, and launch the Visual Studio 2022
Developer Command Prompt automatically when `fxc` is missing.【F:build/setup_windows_dev.bat†L1-L18】
- The PowerShell variant `build\setup_windows_dev.ps1` performs the same setup,
including automatically importing the Visual Studio 2022 developer shell before
restoring the original working directory.【F:build/setup_windows_dev.ps1†L1-L23】

## Building on Windows
- Use `build\build_rive.bat` (cmd) or `build\build_rive.ps1` (PowerShell) to
initialize the environment and forward arguments to the shared
`build_rive.sh` driver running inside Git Bash.【F:build/build_rive.bat†L1-L8】【F:build/build_rive.ps1†L1-L11】
- `build/build_rive.sh` defaults the generator to Visual Studio 2022 on Windows,
threads feature flags and platform arguments into Premake, and writes the final
argument list to `out/<config>/.rive_premake_args` so incremental runs can detect
mismatched options.【F:build/build_rive.sh†L200-L217】【F:build/build_rive.sh†L283-L293】
- The script bootstraps Premake (including the Windows Bootstrap batch path) and
caches helper repositories under `build/dependencies`, ensuring consistent tool
usage across invocations.【F:build/build_rive.sh†L222-L278】
- After project generation the script dispatches to `msbuild.exe` for
Visual Studio solutions, optionally targeting specific projects that you pass on
the command line (for example, `unit_tests`).【F:build/build_rive.sh†L304-L337】

## Running Windows tests and samples
- Execute `tests\unit_tests\test.bat` to set up the Visual Studio environment
(if necessary) and forward arguments to the shared `test.sh` runner.【F:tests/unit_tests/test.bat†L1-L9】
- On Windows, `tests/unit_tests/test.sh` selects the default clang-cl toolset,
invokes `build_rive.sh` with runtime tooling/audio/scripting options, and then
runs the produced `unit_tests` binary from the generated output directory.【F:tests/unit_tests/test.sh†L4-L123】
- Before launching renderer samples such as `path_fiddle`, build the GLFW
Release libraries once via `skia/dependencies/make_glfw.sh` so the Windows
projects can link against the expected static artifacts.【F:skia/dependencies/make_glfw.sh†L12-L25】【F:renderer/premake5.lua†L85-L103】
- Interactive samples rely on `TestingWindow`, so they inherit the same backend
selection and render-loop helpers described above when validating Direct3D
behavior manually.【F:tests/common/testing_window.hpp†L37-L199】【F:tests/player/player.cpp†L143-L190】

## Troubleshooting tips
- If you see a Premake argument mismatch error, run the build again with the
`clean` argument (for example, `build_rive.bat clean debug`) to remove the stale
output directory before regenerating solutions.【F:build/build_rive.sh†L217-L289】
- Missing Direct3D dependencies typically indicate the Visual Studio developer
shell was not loaded; re-run the appropriate `setup_windows_dev` script so
`fxc`, `msbuild.exe`, and associated SDK headers are on `PATH`.【F:build/setup_windows_dev.bat†L1-L18】【F:build/setup_windows_dev.ps1†L1-L23】

## Community-reported Windows 11 build notes *(unconfirmed)*
The following reports come from community members and have not been validated by
the core maintainers. Treat them as hypotheses to verify when diagnosing local
issues.

### Suggested environment setup *(unconfirmed)*
- Visual Studio 2022 workloads: Desktop development with C++, .NET desktop
  development, and potentially Game development with Unity. Some reports also
  install optional components like Windows 11 SDK, IncrediBuild, GitHub
  integration, Clang tools for Windows, and C++ profiling tools.
- Git Bash installed and used to launch `cmd.exe`, which then runs
  `build\setup_windows_dev.bat` so the Visual Studio developer environment and
  Unix-like tools coexist.
- Additional PATH adjustments may be required for `msbuild.exe` or a GNU Make
  installation (for example, MSYS2 or GnuWin32) when the scripts cannot locate
  them automatically.
- Clang/LLVM toolchain for MSVC (`clang-cl`) may need to be installed when the
  build pipeline requests it.

### Workflow reported to succeed *(unconfirmed)*
1. Open the repository within Git Bash.
2. Launch `cmd.exe` from inside Git Bash, then run `build\setup_windows_dev.bat`
   to enter a Visual Studio 2022 Developer Command Prompt that still has access
   to Unix-style commands.
3. Invoke `build_rive.bat rebuild out/release` (from the `renderer` directory in
   one report) instead of relying on Visual Studio's IDE build to avoid shell
   command mismatches.
4. Inspect generated artifacts under `renderer/out/release/`, including
   `path-fiddle.exe` and the `rive.sln` solution.

### Community workarounds *(unconfirmed)*
- Shader compilation: Adjust `renderer/src/shaders/Makefile` so the `fxc`
  invocations quote paths compatible with Windows and ensure the correct `make`
  binary is earlier on `PATH` (for example, `C:\msys64\usr\bin`).
- Premake configuration: Some users insert `require("rive_build_config")` in
  `premake5.lua` (below `require('setup_compiler')`) and re-run
  `dependencies/premake-core/bin/release/premake5.exe` manually with flags such
  as `vs2022 --with_rive_text --with_rive_layout` when the wrapper scripts fail.
- Compiler/linker fixes: Remove `Wno-atomic-alignment` from generated Visual
  Studio project files, adjust Release configurations that define both `DEBUG`
  and `NDEBUG`, and ensure the runtime library is set to `/MTd` for Debug.
- Toolchain gaps: Installing GNU Make (for example, via GnuWin32), adding
  `msbuild.exe` to `PATH`, and enabling Clang support in Visual Studio have been
  cited as necessary on some setups.

### Additional observations *(unconfirmed)*
- A cross-platform sample project (`yup`) implements the Rive renderer via
  CMake and can serve as a reference for integrating with other toolchains.
- One user encountered D3D12 resource state errors on enterprise NVIDIA
  hardware; switching upload-heap resources to use
  `D3D12_RESOURCE_STATE_GENERIC_READ` resolved the issue in their environment.
