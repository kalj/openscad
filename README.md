# Windows UNC path test

Test harness for [openscad/openscad#5425](https://github.com/openscad/openscad/issues/5425):
OpenSCAD built with MSYS2/MinGW fails on files opened through UNC paths.

The workflow builds OpenSCAD with MSYS2 UCRT64 on `windows-latest` for:

- `master`
- each candidate fix merged into master:
  [#7004](https://github.com/openscad/openscad/pull/7004),
  [#7151](https://github.com/openscad/openscad/pull/7151),
  [#7187](https://github.com/openscad/openscad/pull/7187)
- all three merged together (`combined`)

Each build then runs `openscad.exe -o out.echo` on small test files (`use <>`,
`include <>`, a warning and a syntax error) reached through:

| location | path |
|---|---|
| `local` | `C:\unc-test\proj\...` |
| `unc-admin` | `\\localhost\C$\unc-test\proj\...` |
| `unc-share` | `\\localhost\unctest\...` (share created with `New-SmbShare`) |
| `unc-cwd` | relative file name, working directory `\\localhost\unctest` |
| `mapped` | `Z:\...`, with `Z:` mapped to `\\localhost\unctest` |
| `cross-drive` | `D:\unc-test\proj\...`, working directory `C:\` |

Each run is reported as `OK`, `FAIL` (expected output missing), `CRASH`
(exception or abort) or `SKIPPED`. The summary job collects the results into a
table on the workflow run page; full logs are in the `unc-results-*` artifacts.

The branch only contains this harness, so pushing it doesn't run any of the
project's other workflows. Run it again with "Run workflow" on the Actions tab.
