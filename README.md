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

Reading speed:

| Image | pfm | cv2_png | cv2_png_cmp9 | cv2_bmp | npz | npy | qoi |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1080x1920_2cols | 0.13 ms | 4.07 ms | 2.34 ms | 0.91 ms | 0.96 ms | **0.11 ms** | 1.18 ms | 
| 1280x768_gradient_lr | 0.86 ms | 3.95 ms | 0.69 ms | 0.28 ms | 0.31 ms | **0.06 ms** | 1.45 ms | 
| 1280x768_gradient_ud | 0.06 ms | 2.09 ms | 0.99 ms | 0.28 ms | 0.45 ms | **0.06 ms** | 0.55 ms | 
| 1280x960_102cols | 0.19 ms | 2.72 ms | 1.63 ms | 0.35 ms | 0.72 ms | **0.07 ms** | 0.80 ms | 
| 1280x960_111cols | 0.22 ms | 2.79 ms | 1.87 ms | 0.35 ms | 0.76 ms | **0.07 ms** | 0.84 ms | 
| 1280x960_130cols | 0.22 ms | 2.82 ms | 2.26 ms | 0.35 ms | 0.76 ms | **0.07 ms** | 0.83 ms | 
| 1280x960_134cols | 0.22 ms | 2.82 ms | 1.93 ms | 0.36 ms | 0.73 ms | **0.07 ms** | 0.82 ms | 
| 1280x960_24cols | 0.15 ms | 2.63 ms | 1.52 ms | 0.35 ms | 0.65 ms | **0.07 ms** | 0.77 ms | 
| 1280x960_74cols | 0.17 ms | 2.74 ms | 1.52 ms | 0.35 ms | 0.65 ms | **0.07 ms** | 0.79 ms | 
| 1280x960_95cols | 0.25 ms | 2.88 ms | 1.66 ms | 0.35 ms | 0.69 ms | **0.07 ms** | 0.85 ms | 
| 1920x1080_noise | 4.68 ms | 4.01 ms | 3.42 ms | 0.59 ms | 0.70 ms | **0.10 ms** | 8.31 ms | 
| 224x224_5cols | 0.05 ms | 0.22 ms | 0.19 ms | **0.02 ms** | 0.14 ms | 0.03 ms | 0.09 ms | 
| 512x512_noise | 0.60 ms | 0.50 ms | 0.45 ms | 0.07 ms | 0.21 ms | **0.03 ms** | 1.03 ms | 
| 960x1280_120cols | 0.16 ms | 2.66 ms | 1.86 ms | 0.35 ms | 0.73 ms | **0.07 ms** | 0.75 ms | 
| 960x1280_124cols | 0.20 ms | 2.77 ms | 2.20 ms | 0.35 ms | 0.75 ms | **0.08 ms** | 0.81 ms | 
| 960x1280_131cols | 0.20 ms | 2.72 ms | 1.83 ms | 0.35 ms | 0.78 ms | **0.07 ms** | 0.77 ms | 
| 960x1280_60cols | 0.17 ms | 2.59 ms | 1.75 ms | 0.35 ms | 0.68 ms | **0.07 ms** | 0.76 ms | 
| Average | 0.50 ms | 2.65 ms | 1.65 ms | 0.36 ms | 0.63 ms | **0.07 ms** | 1.26 ms | 
| Median | 0.20 ms | 2.73 ms | 1.70 ms | 0.35 ms | 0.70 ms | **0.07 ms** | 0.82 ms | 

Mask size:

| Image | pfm | cv2_png | cv2_png_cmp9 | cv2_bmp | npz | npy | qoi |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 1080x1920_2cols | 10.33 KiB | 14.63 KiB | **9.77 KiB** | 2026.05 KiB | 14.68 KiB | 2025.12 KiB | 40.03 KiB | 
| 1280x768_gradient_lr | 440.27 KiB | 161.37 KiB | **2.43 KiB** | 961.05 KiB | 5.75 KiB | 960.12 KiB | 638.77 KiB | 
| 1280x768_gradient_ud | **0.93 KiB** | 4.77 KiB | 2.62 KiB | 961.05 KiB | 1.73 KiB | 960.12 KiB | 15.77 KiB | 
| 1280x960_102cols | 38.14 KiB | 36.92 KiB | **17.58 KiB** | 1201.05 KiB | 19.42 KiB | 1200.12 KiB | 50.62 KiB | 
| 1280x960_111cols | 47.71 KiB | 44.49 KiB | **22.69 KiB** | 1201.05 KiB | 23.24 KiB | 1200.12 KiB | 57.36 KiB | 
| 1280x960_130cols | 49.46 KiB | 45.63 KiB | 29.73 KiB | 1201.05 KiB | **26.64 KiB** | 1200.12 KiB | 57.88 KiB | 
| 1280x960_134cols | 47.53 KiB | 44.69 KiB | **21.91 KiB** | 1201.05 KiB | 22.08 KiB | 1200.12 KiB | 56.17 KiB | 
| 1280x960_24cols | 25.83 KiB | 25.44 KiB | **15.29 KiB** | 1201.05 KiB | 16.70 KiB | 1200.12 KiB | 41.24 KiB | 
| 1280x960_74cols | 39.37 KiB | 38.44 KiB | **18.64 KiB** | 1201.05 KiB | 20.02 KiB | 1200.12 KiB | 50.31 KiB | 
| 1280x960_95cols | 56.63 KiB | 52.16 KiB | **24.20 KiB** | 1201.05 KiB | 24.49 KiB | 1200.12 KiB | 64.41 KiB | 
| 1920x1080_noise | 2521.83 KiB | 2030.52 KiB | 2030.52 KiB | 2026.05 KiB | 2025.86 KiB | **2025.12 KiB** | 6384.99 KiB | 
| 224x224_5cols | 11.39 KiB | 10.13 KiB | 6.48 KiB | 50.05 KiB | **6.12 KiB** | 49.12 KiB | 14.99 KiB | 
| 512x512_noise | 318.99 KiB | 257.02 KiB | 257.02 KiB | 257.05 KiB | 256.32 KiB | **256.12 KiB** | 807.09 KiB | 
| 960x1280_120cols | 39.45 KiB | 29.62 KiB | 19.07 KiB | 1201.05 KiB | **18.37 KiB** | 1200.12 KiB | 42.09 KiB | 
| 960x1280_124cols | 42.59 KiB | 42.10 KiB | **22.77 KiB** | 1201.05 KiB | 23.00 KiB | 1200.12 KiB | 54.16 KiB | 
| 960x1280_131cols | 42.29 KiB | 39.75 KiB | 22.78 KiB | 1201.05 KiB | **21.62 KiB** | 1200.12 KiB | 50.42 KiB | 
| 960x1280_60cols | 33.34 KiB | 30.99 KiB | **15.65 KiB** | 1201.05 KiB | 19.18 KiB | 1200.12 KiB | 43.75 KiB | 
| Average | 221.53 KiB | 171.10 KiB | **149.36 KiB** | 1146.64 KiB | 149.72 KiB | 1145.71 KiB | 498.24 KiB | 
| Median | 42.44 KiB | 40.92 KiB | **20.49 KiB** | 1201.05 KiB | 20.82 KiB | 1200.12 KiB | 52.39 KiB | 

All measurements are averaged over 1000 iterations. The best result is highlighted in bold.

Tests
---------
```bash
pip install -r test-requirements.txt
python -m unittest discover tests/
```
