# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
While the major version is `0`, breaking changes may land in a minor release.

## [Unreleased]

### Changed

- **Breaking:** sampling by rate now covers a half-open range. `end_time` is no
  longer sampled, so a span of n sample periods yields n samples rather than
  n + 1: `evaluate_range_by_rate(0, 4, 30)` returns 120 values, the last at
  3.9667, and `num_samples(30)` over a 4 second channel returns 120 rather than
  121. This is the convention frame- and audio-rate hosts expect, and it makes
  the sample count the product of duration and rate as callers assume. Callers
  who want the closing sample can use `Channel::evaluate_range`, which is
  unchanged and still spans a closed range, or extend the end by one period.
  Affects `Channel::evaluate_range_by_rate`, `Channel::num_samples` and
  `Animation::num_samples`.
- **Breaking:** `Animation::num_samples` returns `size_t`, matching
  `Channel::num_samples`. The two counted the same thing in different types.
- **Breaking:** `Channel::evaluate_range` now returns exactly the requested
  number of samples in every case. It previously collapsed to a single sample
  when `start_time` and `end_time` were equal, which silently broke callers
  sizing a buffer from the count they passed in. An empty range now gives that
  many copies of the value at that time.

### Fixed

- `Channel::evaluate_range_by_rate` did not sample at the requested rate when
  the range was not a whole number of sample periods. It rounded the count up
  and then handed it to `evaluate_range`, which spreads a count across a closed
  range, compressing the spacing to fit: 1.05 seconds at 30 Hz came back as 33
  values 0.0328 apart rather than 30 Hz. Sample times are now derived from the
  sample index, so the spacing is exactly `1 / sample_rate` for any range.
- `Channel::evaluate_range` validated its range only for sample counts large
  enough to reach the sampling loop, so `evaluate_range(10, 0, 0)` returned a
  value while `evaluate_range(10, 0, 2)` threw on the same reversed range. The
  range is now checked first, for every count. A count of zero returns an empty
  vector rather than one sample, and a negative count is rejected rather than
  quietly treated as one sample.
- The sample count is no longer inflated by floating-point error. It was
  computed with `ceil`, so a duration whose product with the rate landed a few
  ulps above a whole number, as 4.0 seconds at 30 Hz can, produced an extra
  sample spanning a fraction of a period.

## [0.3.0] - 2026-07-26

Two breaking changes that tighten the public API: `Id` lookups return
references, and ids can no longer be constructed by callers. Both are small,
mechanical changes for callers. The `glad` dependency is gone, so the examples
now build with one fewer third-party package and without a CMake policy
override.

### Changed

- **Breaking:** the `Id` lookups on `Animation` now return references rather
  than pointers, matching the index and name overloads —
  `channel(Id)` and `operator[](Id)`, in both the const and non-const forms.
  They never returned null: the underlying `unordered_map::at` throws
  `std::out_of_range` on a miss, so the pointer return only invited dead null
  checks. Behavior on a miss is unchanged; callers replace `->` with `.`
  ([#52](https://github.com/Actualize-Interactive/anim/issues/52)).
- The release workflow now refuses to publish a tag that is not an ancestor of
  `main`. Squash-merging a pull request rewrites the commit, so a tag pushed to
  the pre-merge branch tip builds and tests green while pointing at a commit
  reachable only from the tag itself.

### Added

- `Animation::sort_channels()`, sorting channels by name, and an overload
  taking a comparator for any other ordering. Both are stable. Only the index
  order changes: the channels themselves are not moved, so ids keep resolving
  and references taken beforehand stay valid.

### Removed

- **Breaking:** `Id`'s constructor is now private, so ids can only originate
  from the library — obtain them from `Channel::id()`, or use `Id::invalid()`
  for a sentinel. A fabricated id was never able to do anything a real one
  could not, but because ids are handed out from one counter shared by every
  `Animation`, a hand-made id could silently resolve to an unrelated channel.
- The `glad` dependency. The examples now rely on the loader that Dear ImGui
  already bundles, and the handful of direct GL calls in
  `curve_visualization` are OpenGL 1.1 core, resolved by linking `OpenGL::GL`.
  This also removes the `CMAKE_POLICY_VERSION_MINIMUM` workaround that glad
  0.1.36 required under CMake 4
  ([#53](https://github.com/Actualize-Interactive/anim/issues/53)).

### Fixed

- The `curve_visualization` example showed no plot on a first run. Its plot
  window was opened without a size, so it auto-fitted to its content — but that
  content is a plot sized `ImVec2(-1, -1)`, meaning "fill the available space".
  On the first frame the two resolved to nothing, the window collapsed to a few
  pixels behind the curve editor, and ImGui persisted that size to `imgui.ini`
  from then on. Both windows now get a first-run position and size derived from
  the viewport, using `ImGuiCond_FirstUseEver` so an arranged layout is kept.
- `imgui.ini`, which the `curve_visualization` example writes to the working
  directory, is now ignored rather than showing up as untracked noise in the
  repository root. Alternate build directories (`build-*/`) are ignored too.

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
- CI now builds with CMake 4.4. The `windows-latest` runner image ships Visual
  Studio 2026, for which a generator first exists in CMake 4.2; the previously
  pinned CMake 3.26 fell back to NMake and could not find a compiler at all.
- Releases now take their description from this file's entry for the tag, with
  GitHub's generated notes appended, instead of being published with an empty
  body and filled in by hand. Tagging a version with no changelog entry now
  fails the release rather than publishing a blank description.
- The release workflow no longer fails when started manually. Publishing needs
  a tag, so on `workflow_dispatch` it skips the publish step and runs purely as
  a dry run of the cross-platform build and test gate.

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

[Unreleased]: https://github.com/Actualize-Interactive/anim/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/Actualize-Interactive/anim/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/Actualize-Interactive/anim/compare/v0.1.2...v0.2.0
[0.1.2]: https://github.com/Actualize-Interactive/anim/releases/tag/v0.1.2
[0.1.1]: https://github.com/Actualize-Interactive/anim/releases/tag/v0.1.1
[0.1.0]: https://github.com/Actualize-Interactive/anim/releases/tag/v0.1.0
