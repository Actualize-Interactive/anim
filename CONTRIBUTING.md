# Contributing to anim

Thanks for your interest in contributing! This guide covers how to build, test,
and submit changes.

## Getting started

You'll need a C++20 compiler (MSVC 2022, GCC 11+, or Clang 14+) and CMake 3.25
or newer.

```bash
# Configure, build, and test in one step:
./build.sh        # Linux / macOS
.\build.ps1       # Windows (PowerShell)
```

Or drive CMake directly:

```bash
cmake -B build -S . -DANIM_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Tests

The test suite uses [Catch2](https://github.com/catchorg/Catch2) (fetched
automatically by CMake) and lives in `tests/`.

- **Every change to library behavior must keep the suite green**, and new
  behavior should come with tests.
- CI builds in **Release** on Linux, macOS, and Windows and runs `ctest`. Note
  that Debug builds on MSVC enable checked iterators, which can surface
  undefined behavior that Release hides — running Debug locally is worthwhile.

## Coding conventions

- **C++20**, no compiler extensions.
- **Enumerators are PascalCase** (e.g. `Function::Bezier`, `HandleMode::Smooth`).
- **Public interfaces are stable.** Avoid changing existing public signatures;
  prefer additive changes. Call out any unavoidable break in your PR.
- **Document the public API in the headers** with Doxygen comments
  (`/** ... */` with `@brief`, `@param`, `@return`, `@throws`). Keep
  documentation on declarations in `include/`; do not add it to the `.cpp`
  implementation files.
- Match the surrounding style; American spelling in comments and docs.

## Documentation

API docs are generated from the header comments with Doxygen + Sphinx:

```bash
bash docs/build_docs.sh    # output in docs/build/html
```

If you change a public signature or its contract, update the doc comment in the
same change.

## Pull requests

- Branch off the current default branch and keep PRs focused.
- Use clear, imperative commit messages with a type prefix (`fix:`, `feat:`,
  `docs:`, `test:`, `chore:`, `ci:`).
- Make sure the build and tests pass before opening the PR.

## License

By contributing, you agree that your contributions are licensed under the
project's [MIT License](LICENSE).
