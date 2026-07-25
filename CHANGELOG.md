# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
While the major version is `0`, breaking changes may land in a minor release.

## [Unreleased]

## [0.2.0] - 2026-07-25

First release prepared for the public repository. It contains two small
breaking renames that bring the public API in line with the project's naming
conventions; both are mechanical find-and-replace changes for callers.

### Changed

- **Breaking:** `Id::isValid()` is now `Id::is_valid()`, matching the
  lower_snake_case convention used by every other method.
- **Breaking:** `GrabbedHandle` enumerators are now PascalCase, matching every
  other enum in the library:
  `GrabbedHandle::none` → `GrabbedHandle::None`,
  `GrabbedHandle::in_handle` → `GrabbedHandle::InHandle`,
  `GrabbedHandle::out_handle` → `GrabbedHandle::OutHandle`.
- `build.sh` and `build.ps1` now build the library and test suite only, run
  from the repository root regardless of the working directory, and pass the
  build configuration through to `ctest`. Use `examples/run_example.*` to build
  and launch the viewer.
- The project version is now defined only in `CMakeLists.txt`; the Doxygen and
  Sphinx configurations derive it from there instead of repeating it.
- Documentation dependencies are pinned to tested version ranges so an upstream
  release cannot break the Pages deploy without a reviewed change.
- CI, docs, and release workflows updated to current action versions, with
  explicit least-privilege `permissions` blocks.

### Added

- `CHANGELOG.md`, `CODE_OF_CONDUCT.md`, and `SECURITY.md`.
- Issue and pull request templates, and a Dependabot configuration that keeps
  GitHub Actions up to date.
- Naming conventions documented in `CONTRIBUTING.md` (PascalCase for types and
  enumerators, lower_snake_case for functions and variables).

### Fixed

- `build.ps1` called `Pop-Location` without a matching `Push-Location`, and ran
  `ctest` without `-C`, which finds no tests on multi-config generators such as
  Visual Studio.
- `.gitignore` matched `*.cmake` repository-wide, silently ignoring any
  hand-written CMake module added under `cmake/`.

### Removed

- `examples/gl_loader.cpp`, which was unreferenced by any build file or source
  since the examples moved to glad.

## [0.1.2] - 2026-05-28

Repository prepared for publication: MIT license, contributor guide, Doxygen +
Sphinx API documentation published to GitHub Pages, and a release workflow that
publishes a single source archive.

## [0.1.1] - 2025-07-05

Enumerators converted to PascalCase; per-channel `Extend` control for
out-of-range evaluation; equality operators for `Animation` and `Channel`;
explicit `Animation` copy methods; additional `Channel::create_keyframe`
overloads and a `keyframes()` accessor; stable per-channel `Id`.

## [0.1.0] - 2025-05-29

Initial release: `Animation` and `Channel` containers, keyframes with
`Constant` / `Linear` / `Bezier` interpolation, the `HandleMode` family of
Bézier handle constraints, sampling helpers, and the Catch2 test suite.

<!-- Entries for 0.1.x are summarized retrospectively; this file was introduced
     in 0.2.0. History was rewritten between the v0.1.1 and v0.1.2 tags, so the
     compare links below are more reliable than a commit-by-commit listing. -->

[Unreleased]: https://github.com/Actualize-Interactive/anim/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/Actualize-Interactive/anim/compare/v0.1.2...v0.2.0
[0.1.2]: https://github.com/Actualize-Interactive/anim/releases/tag/v0.1.2
[0.1.1]: https://github.com/Actualize-Interactive/anim/releases/tag/v0.1.1
[0.1.0]: https://github.com/Actualize-Interactive/anim/releases/tag/v0.1.0
