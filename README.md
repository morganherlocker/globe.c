GLOBE.c
---

C parser & CLI toolset for the [Global Land One-kilometer Base Elevation (GLOBE)](https://www.ngdc.noaa.gov/mgg/topo/report/globedocumentationmanual.pdf) dataset. GLOBE is a global 1km-resolution digital elevation model, compiled by NOAA in 1999.

The data is suitable for medium-resolution global terrain modeling. The raw data is 1.8GB uncompressed.

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

Requires clang.

```sh
make
```

## Format

Requires clang-format, clang-tidy.

```sh
make lint;
```

## CLI

### help

```sh
globe -h
```

### merge

Flatten shards into a single global array and writes to raw bin file. `./globe.bin` is 1.7G raw, 231M zstd compressed.

```sh
globe merge -i ./all10 -o ./globe.bin;

du -h globe.bin*
# 1.7G    globe.bin
# 241M    globe.bin.zst
```

## render

Write a png of a bounding box.

```sh
globe render -i ./globe.bin -o globe.png;
```

## table

Write csv table file, with the format: lon, lat, elevation.

```sh
globe table -i ./globe.bin -o globe.csv;
```

This csv can be converted to parquet with following duckdb command for more efficient storage and querying:

```sh
# Convert to parquet.
duckdb -c "copy (select * from globe.csv where elev > 0) to 'globe.parquet' (FORMAT PARQUET, COMPRESSION ZSTD, ROW_GROUP_SIZE 100_000)"
# Select highest and lowest point on earth.
duckdb -c 'select * from globe.csv order by elev desc limit 1; select * from globe.csv order by elev asc limit 1;'
# ┌───────────┬──────────┬───────┐
# │    lon    │   lat    │ elev  │
# │  double   │  double  │ int64 │
# ├───────────┼──────────┼───────┤
# │ 86.874268 │ 28.00589 │  8752 │
# └───────────┴──────────┴───────┘
# ┌───────────┬───────────┬───────┐
# │    lon    │    lat    │ elev  │
# │  double   │  double   │ int64 │
# ├───────────┼───────────┼───────┤
# │ 35.320187 │ 30.997511 │  -407 │
# └───────────┴───────────┴───────┘
```