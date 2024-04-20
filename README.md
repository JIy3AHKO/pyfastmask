pyfastmask - Fast image segmentation format
==============

This is a simple format for storing single channel images. It is designed to be fast to read, and to be easy to use in C++ and Python.

Installation
------------

```bash
pip install pyfastmask
```

Usage
-----
For image reading and writing, use the `read` and `write` functions:

```python
import pyfastmask as pf
import numpy as np

img = np.random.randint((100, 100), dtype=np.uint8)

pf.write('mask.pfm', img)
img2 = pf.read('mask.pfm')

assert np.all(img == img2)
```


Benchmark
---------

Reading speed (ms)

| Image | pfm | opencv png  | qoi |
|-------|-----|-------------|-----|
| 1     | 0.1 | 0.2         | 0.3 |
| 2     | 0.1 | 0.2         | 0.3 |
| 3     | 0.1 | 0.2         | 0.3 |
| 4     | 0.1 | 0.2         | 0.3 |
| 5     | 0.1 | 0.2         | 0.3 |

Mask size (kB)

| Image | pfm | opencv png  | qoi |
|-------|-----|-------------|-----|
| 1     | 0.1 | 0.2         | 0.3 |
| 2     | 0.1 | 0.2         | 0.3 |
| 3     | 0.1 | 0.2         | 0.3 |
| 4     | 0.1 | 0.2         | 0.3 |
| 5     | 0.1 | 0.2         | 0.3 |

All measurements are the average of 10 runs.
