pyfastmask - Fast image segmentation format
==============

![Tests](https://github.com/JIy3AHKO/pyfastmask/actions/workflows/build-and-test.yml/badge.svg?branch=master)

This is a simple format for storing single channel images with low-frequency data (e.g. semantic segmentation masks).

It has a size similar to PNG, but is much faster (up to 20x) to read.


Installation
------------

### From PyPI:

The easiest way to install the latest version is by using pip:

```bash
pip install pyfastmask
```

### From source:

```bash
git clone git@github.com:JIy3AHKO/pyfastmask.git
cd pyfastmask
pip install -e .
```

Usage
-----
For image reading and writing, use the `read` and `write` functions:

```python
import numpy as np
import pyfastmask as pf

img = np.random.randint(0, 256, (100, 100), dtype=np.uint8)

pf.write('mask.pfm', img)
img2 = pf.read('mask.pfm')

np.testing.assert_array_equal(img, img2)
```


Benchmark
---------
See [BENCHMARK.md](BENCHMARK.md) for more detailed information.

| Image             | pyfastmask  | opencv png     | cv2_bmp     | qoi        |
|-------------------|-------------|----------------|-------------|------------|
| Median Read Time  | **0.09 ms** | 1.71 ms        | 0.35 ms     | 0.81 ms    | 
| Average Size      | 217.35 KiB  | **149.36 KiB** | 1146.64 KiB | 498.24 KiB | 


Format Description
--------
The pyfastmask efficiently compresses and stores image segmentation masks using Run-Length Encoding (RLE) and line-differential encoding.
All values are stored with different bit widths, depending on the number of unique symbols and the mask size - it helps to reduce the size of encoded data.

### Storage Structure

The format organizes data into three main sections: header, symbol mapping, and line-by-line encoded data.

#### 1. Header

- Magic Byte: Format identifier.
- Version Byte: Format version.
- Symbol Bit Width: Bits for each symbol.
- Count Bit Width: Bits for run lengths.
- Line Count Bit Width: Bits for the number of runs per line.
- Unique Symbols Count: Number of unique symbols.
- Mask Height: Mask height in pixels.
- Mask Width: Mask width in pixels.

#### 2. Symbol Mapping

Lists unique symbols in the mask, each encoded with 8 bits.

Semantic segmentation masks usually have a small number of unique symbols, so we can use a small number of bits to encode each symbol.

#### 3. Line-by-Line Encoding

Encodes mask data line by line:
- First Line: Encoded with standard RLE.
- Subsequent Lines: Encoded with sparse RLE on the difference between the current and previous lines.

First line is represented as: (Number of runs), (Symbol, Run Length), (Symbol, Run Length), ...

Subsequent lines are represented as: (Number of runs), (Offset, Symbol, Run Length), (Offset, Symbol, Run Length), ...

Where:
- Number of runs: Number of runs in the line.
- Symbol: Symbol index from the symbol mapping.
- Run Length: Number of pixels with the same symbol.
- Offset: Number of pixels to skip from the previous line.

### Encoding Process

1. Encode Lines:
   - First Line: Standard RLE.
   - Subsequent Lines: Sparse RLE.
2. Estimate Bit Widths: Calculate the number of bits required to store each value.
3. Write Header and Symbol Mapping.
4. Pack Data: Combine all encoded data into a byte stream.


### Decoding Process

1. Read Header and Symbol Mapping.
2. Decode first line with standard RLE.
3. On subsequent lines:
   - copy the previous line
   - apply sparse RLE to the copied line

Testing
---------
To run tests, use the following command:
```bash
pip install -r test-requirements.txt
python -m unittest discover tests/
```

Contributing
------------
Contributions are welcome! If you want to contribute, please create an issue or a pull request.
