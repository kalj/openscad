# Fuzz targets

libFuzzer targets for the file importers and the parser:

| Target | Exercises |
|---|---|
| `fuzz_import_stl` | `import_stl()` (ASCII and binary STL) |
| `fuzz_import_off` | `import_off()` |
| `fuzz_import_obj` | `import_obj()` |
| `fuzz_import_3mf` | `import_3mf()` (lib3mf) |
| `fuzz_import_svg` | `import_svg()` (libsvg, libxml2) |
| `fuzz_import_dxf` | `DxfData` and `toPolygon2d()` |
| `fuzz_import_json` | `import_json()` |
| `fuzz_parse_scad` | the `.scad` lexer and parser (no evaluation) |

Each importer target writes the input to a temp file in `$TMPDIR` and imports it.
An exception escaping an importer is reported as a crash, since nothing above the importers catches it.

## Building

The targets need clang. Build all of OpenSCAD with ASan/UBSan and libFuzzer coverage instrumentation:

```bash
F="-fsanitize=address,undefined,fuzzer-no-link -fno-sanitize-recover=undefined -fno-omit-frame-pointer"
cmake -B build-fuzz -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_FLAGS="$F" -DCMAKE_CXX_FLAGS="$F" \
  -DHEADLESS=ON -DENABLE_TESTS=OFF -DUSE_MIMALLOC=OFF -DENABLE_FUZZING=ON
ninja -C build-fuzz fuzz_import_svg   # or any other target
```

On Debian/Ubuntu, libFuzzer comes with the `libclang-rt-<version>-dev` package.

## Running

Seed the corpus with the matching test files, e.g. for SVG:

```bash
mkdir -p corpus/svg && cp tests/data/svg/*.svg corpus/svg/
build-fuzz/fuzz_import_svg corpus/svg -dict=tests/fuzz/svg.dict -max_len=65536 -timeout=10 -rss_limit_mb=4096
```

Run a single reproducer with `build-fuzz/fuzz_import_svg crash-<hash>`, and set `OPENSCAD_FUZZ_VERBOSE=1`
to see the importer's warnings.
