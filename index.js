for (let y = 0; y < 4; y++) {
  for (let x = 0; x < 4; x++) {
    const minlon = x * 90 - 180;
    const minlat = -(y - 4) * 45 - 135;
    const maxlon = minlon + 90;
    const maxlat = -(y - 4) * 45 - 90;
    console.log(
      `mkdir -p ./2/${x}; ./globe render -i ./globe.bin -o ./2/${x}/${y}.png --minlon=${minlon} --minlat=${minlat} --maxlon=${maxlon} --maxlat=${maxlat};`,
    );
  }
}

//➜  globe-c git:(gh-pages) ✗ ./globe render -i ./globe.bin -o ./demo/1/1/0.png --minlon=0 --minlat=0 --maxlon=180 --maxlat=90;
