const z = 4;
const lonsize = 360 / 2 ** z;
const latsize = 180 / 2 ** z;

for (let y = 0; y < 2 ** z; y++) {
  for (let x = 0; x < 2 ** z; x++) {
    const minlon = x * lonsize - 180;
    const maxlat = -(y - 2 ** z) * latsize - 90;
    const minlat = maxlat - latsize;
    const maxlon = minlon + lonsize;
    console.log(
      `mkdir -p ./${z}/${x}; time ./globe render -i ./globe.bin -o ./${z}/${x}/${y}.png --minlon=${minlon} --minlat=${minlat} --maxlon=${maxlon} --maxlat=${maxlat};`,
    );
  }
}
