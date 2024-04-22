pyfastmask - Fast image segmentation format
==============

This is a simple format for storing single channel images. It is designed to be fast to read, and to be easy to use in C++ and Python.

Installation
------------

```bash
git clone git@github.com:JIy3AHKO/pyfastmask.git
cd pyfastmask
pip install .
```

Usage
-----
For image reading and writing, use the `read` and `write` functions:

```python
import pyfastmask as pf
import numpy as np

img = np.random.randint(0, 256, (100, 100), dtype=np.uint8)

pf.write('mask.pfm', img)
img2 = pf.read('mask.pfm')

assert np.all(img == img2)
```


Benchmark
---------
You can run benchmark localy with:

```bash
pip install -r test-requirements.txt
python benchmark/run_benchmark.py
```

Reading speed (ms)

|   Image   |  pfm  |   cv2  |   qoi  |
|-----------|-------|--------|--------|
|bin_circles|0.15 ms| 3.61 ms| 0.66 ms|
|   noise   |3.85 ms|24.77 ms|11.03 ms|
|  realmask |0.26 ms| 7.07 ms| 1.28 ms|

Mask size (kB)
<TBD>

Tests
---------
```bash
pip install -r test-requirements.txt
python -m unittest discover tests/
```


