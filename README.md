# Windows UNC path test

Test harness for [openscad/openscad#5425](https://github.com/openscad/openscad/issues/5425):
OpenSCAD built with MSYS2/MinGW fails on files opened through UNC paths.
It also covers [openscad/openscad#7198](https://github.com/openscad/openscad/issues/7198):
`use <>` and `include <>` fail when the folder name has non-ASCII characters.

The workflow builds OpenSCAD with MSYS2 UCRT64 on `windows-latest` for:

- `master`
- each candidate fix merged into master:
  [#7004](https://github.com/openscad/openscad/pull/7004),
  [#7151](https://github.com/openscad/openscad/pull/7151),
  [#7187](https://github.com/openscad/openscad/pull/7187)
- all three merged together (`combined`)
- `fix1`: [kalj:do-export-cwd](https://github.com/kalj/openscad/tree/do-export-cwd), which stops
  command-line runs aborting when `do_export()` can't change the working directory
- `all`: the three PRs plus `fix1`
- `tests-master`, `tests-all`: the UNC CTest tests from
  [kalj:unc-nocrash-tests](https://github.com/kalj/openscad/tree/unc-nocrash-tests), without and with the fixes
- `wide-open`: [kalj:win-wide-open](https://github.com/kalj/openscad/tree/win-wide-open)
  (#7207), the fix for #7198 that opens `use <>` and `include <>` files through wide-character
  paths
- `wide-no-sfc`: #7207 without its `SourceFileCache.cc` change, to show that change is needed

Each build then runs `openscad.exe -o out.echo` on small test files (`use <>`,
`include <>`, a warning, a syntax error, and `use <>` of a file named `dummyö.scad`)
reached through:

| location | path |
|---|---|
| `local` | `C:\unc-test\proj\...` |
| `unc-admin` | `\\localhost\C$\unc-test\proj\...` |
| `unc-share` | `\\localhost\unctest\...` (share created with `New-SmbShare`) |
| `unc-cwd` | relative file name, working directory `\\localhost\unctest` |
| `mapped` | `Z:\...`, with `Z:` mapped to `\\localhost\unctest` |
| `cross-drive` | `D:\unc-test\proj\...`, working directory `C:\` |
| `nonascii` | `C:\unc-test\pröj\...` |
| `nonascii-cwd` | relative file name, working directory `C:\unc-test\pröj` (the #7198 case, without a non-ASCII command-line argument) |
| `nonascii-unc` | `\\localhost\C$\unc-test\pröj\...` |

Each run is reported as `OK`, `FAIL` (expected output missing), `CRASH`
(exception or abort) or `SKIPPED`. The summary job collects the results into a
table on the workflow run page; full logs are in the `unc-results-*` artifacts.
The summary also says whether each build initialized localization: without a
`locale` folder next to the exe, OpenSCAD never calls `setlocale(LC_ALL, "")`,
which affects how `fopen()` treats non-ASCII file names.

The branch only contains this harness, so pushing it doesn't run any of the
project's other workflows. Run it again with "Run workflow" on the Actions tab.

A separate `fs-diagnostics` job builds `scripts/unc-fs-diag.cpp` with the same
toolchain and prints what each `std::filesystem` step (`absolute()`,
`parent_path()`, `generic_string()`, `canonical()`, `relative()`,
`current_path()`) returns for local, UNC and mapped-drive paths, to find where a
UNC path loses its `\\server\share` prefix.
