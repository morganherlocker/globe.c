GLOBE.c
---

C parser for the [Global Land One-kilometer Base Elevation (GLOBE)](https://www.ngdc.noaa.gov/mgg/topo/report/globedocumentationmanual.pdf) dataset. This dataset is a compilation of many elevation datasets combined by NOAA in 1999, representing a global-coverage ~1km resolution digital elevation model.

This data is suitable for medium-resolution global terrain modeling in a light memory footprint. The raw data from NOAA is 1.8GB uncompressed.

## Download

```sh
# downloading the archive
curl https://www.ngdc.noaa.gov/mgg/topo/DATATILES/elev/all10g.tgz

# unpacking the files
tar -xvzf all10g.tgz

# size of each file
du -h all10/*
 99M    all10/a10g
 99M    all10/a11g
 99M    all10/b10g
 99M    all10/c10g
 99M    all10/d10g
124M    all10/e10g
124M    all10/f10g
124M    all10/g10g
124M    all10/h10g
124M    all10/i10g
124M    all10/j10g
124M    all10/k10g
124M    all10/l10g
 99M    all10/m10g
 99M    all10/n10g
 99M    all10/o10g
 99M    all10/p10g

# size of all files
du -h all10
  1.8G   all10
```

## Build

Requires clang, clang-format, clang-tidy.

```sh
make
```

## Format

```sh
make lint;
```

## Use (WIP)

```sh
globe --noaa ./all10 -o ./globe.bin merge
globe --in ./globe.bin -o ./globe.ppm render
```
