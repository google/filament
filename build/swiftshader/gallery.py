#!/usr/bin/env python3

from pathlib import Path
import os

spath = os.path.dirname(os.path.realpath(__file__))

path = Path(spath)

folder = "../../results/"

images = list(path.glob(folder + '*.png'))

images.sort()

gallery = open(path.absolute().joinpath(folder + 'index.html'), 'w')

gallery.write("""<html>
<head>
<script type="module" src="https://unpkg.com/img-comparison-slider@latest/dist/component/component.esm.js"></script>
<script nomodule="" src="https://unpkg.com/img-comparison-slider@latest/dist/component/component.js"></script>
<link rel="stylesheet" href="https://unpkg.com/img-comparison-slider@latest/dist/collection/styles/initial.css"/>
<style>
    h2 {
        font-weight: normal;
        margin-top: 150px;
        margin-bottom: 20px;
    }
    a {
        text-decoration: none;
        font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        color: blue;
    }
    a:hover {
        font-weight: bold;
    }
</style>
</head>
<body>
""")

tag = ''

for image in images:
    group = image.stem.rstrip('0123456789')
    before = f'https://filament-assets.github.io/golden/{group}/{image.name}'
    after = image.name
    gallery.write('\n')
    gallery.write(f'<h2><a href="{image.stem}.json">{image.stem}.json</a></h2>\n')
    gallery.write('<img-comparison-slider>\n')
    gallery.write(f'<img slot="before" src="{before}" /> <img slot="after" src="{after}" />\n')
    gallery.write('</img-comparison-slider>\n')

gallery.write("""</body>
</html>
""")
