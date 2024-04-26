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
You can run benchmark locally with:

```bash
pip install -r test-requirements.txt
python benchmark/run_benchmark.py --images-dir <dir_with_test_images> --n-iterations 1000
```

Reading speed (ms):

| Image                | pfm | cv2_png | cv2_png_cmp9 | cv2_bmp | qoi |
|----------------------| --- | --- | --- | --- | --- |
| 1280x960_130cols     | **0.19 ms** | 2.81 ms | 2.26 ms | 0.51 ms | 0.83 ms | 
| 1080x1920_2cols      | **0.11 ms** | 4.05 ms | 2.34 ms | 0.60 ms | 1.17 ms | 
| 1280x960_24cols      | **0.13 ms** | 2.63 ms | 1.52 ms | 0.35 ms | 0.78 ms | 
| 1280x960_134cols     | **0.20 ms** | 2.81 ms | 1.93 ms | 0.35 ms | 0.82 ms | 
| 960x1280_131cols     | **0.18 ms** | 2.72 ms | 1.83 ms | 0.35 ms | 0.79 ms | 
| 1920x1080_noise      | 4.42 ms | 3.98 ms | 3.42 ms | **0.60 ms** | 8.39 ms | 
| 1280x960_74cols      | **0.15 ms** | 2.74 ms | 1.52 ms | 0.36 ms | 0.79 ms | 
| 1280x960_95cols      | **0.22 ms** | 2.88 ms | 1.66 ms | 0.35 ms | 0.85 ms | 
| 1280x768_gradient_ud | **0.05 ms** | 2.09 ms | 1.01 ms | 0.28 ms | 0.56 ms | 
| 1280x768_gradient_lr | 0.74 ms | 3.95 ms | 0.70 ms | **0.28 ms** | 1.45 ms | 
| 1280x960_111cols     | **0.20 ms** | 2.79 ms | 1.87 ms | 0.35 ms | 0.82 ms | 
| 960x1280_120cols     | **0.14 ms** | 2.65 ms | 1.86 ms | 0.35 ms | 0.75 ms | 
| 960x1280_60cols      | **0.15 ms** | 2.59 ms | 1.74 ms | 0.35 ms | 0.76 ms | 
| 960x1280_124cols     | **0.18 ms** | 2.77 ms | 2.20 ms | 0.35 ms | 0.80 ms | 
| 512x512_noise        | 0.54 ms | 0.50 ms | 0.45 ms | **0.07 ms** | 1.04 ms | 
| 224x224_5cols        | 0.04 ms | 0.22 ms | 0.19 ms | **0.02 ms** | 0.09 ms | 
| 1280x960_102cols     | **0.16 ms** | 2.71 ms | 1.63 ms | 0.35 ms | 0.79 ms | 
| Average              | 0.46 ms | 2.64 ms | 1.66 ms | **0.35 ms** | 1.26 ms | 
| Median               | **0.18 ms** | 2.73 ms | 1.70 ms | 0.35 ms | 0.81 ms | 

Mask size (kB)

| Image                | pfm | cv2_png | cv2_png_cmp9 | cv2_bmp | qoi |
|----------------------| --- | --- | --- | --- | --- |
| 1280x960_130cols     | 49.46 KB | 45.63 KB | **29.73 KB** | 1201.05 KB | 57.88 KB | 
| 1080x1920_2cols      | 10.33 KB | 14.63 KB | **9.77 KB** | 2026.05 KB | 40.03 KB | 
| 1280x960_24cols      | 25.83 KB | 25.44 KB | **15.29 KB** | 1201.05 KB | 41.24 KB | 
| 1280x960_134cols     | 47.53 KB | 44.69 KB | **21.91 KB** | 1201.05 KB | 56.17 KB | 
| 960x1280_131cols     | 42.28 KB | 39.75 KB | **22.78 KB** | 1201.05 KB | 50.42 KB | 
| 1920x1080_noise      | 2521.83 KB | 2030.52 KB | 2030.52 KB | **2026.05 KB** | 6384.99 KB | 
| 1280x960_74cols      | 39.37 KB | 38.44 KB | **18.64 KB** | 1201.05 KB | 50.31 KB | 
| 1280x960_95cols      | 56.63 KB | 52.16 KB | **24.20 KB** | 1201.05 KB | 64.41 KB | 
| 1280x768_gradient_ud | **0.93 KB** | 4.77 KB | 2.62 KB | 961.05 KB | 15.77 KB | 
| 1280x768_gradient_lr | 440.27 KB | 161.37 KB | **2.43 KB** | 961.05 KB | 638.77 KB | 
| 1280x960_111cols     | 47.71 KB | 44.49 KB | **22.69 KB** | 1201.05 KB | 57.36 KB | 
| 960x1280_120cols     | 39.45 KB | 29.62 KB | **19.07 KB** | 1201.05 KB | 42.09 KB | 
| 960x1280_60cols      | 33.34 KB | 30.99 KB | **15.65 KB** | 1201.05 KB | 43.75 KB | 
| 960x1280_124cols     | 42.59 KB | 42.10 KB | **22.77 KB** | 1201.05 KB | 54.16 KB | 
| 512x512_noise        | 318.99 KB | **257.02 KB** | **257.02 KB** | 257.05 KB | 807.09 KB | 
| 224x224_5cols        | 11.39 KB | 10.13 KB | **6.48 KB** | 50.05 KB | 14.99 KB | 
| 1280x960_102cols     | 38.14 KB | 36.92 KB | **17.58 KB** | 1201.05 KB | 50.62 KB | 
| Average              | 221.53 KB | 171.10 KB | **149.36 KB** | 1146.64 KB | 498.24 KB | 
| Median               | 42.44 KB | 40.92 KB | **20.49 KB** | 1201.05 KB | 52.39 KB | 



Tests
---------
```bash
pip install -r test-requirements.txt
python -m unittest discover tests/
```
