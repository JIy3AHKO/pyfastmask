import pyfastmask as pf
import numpy as np
import cv2
import unittest
import os
import time
import glob
import qoi


def test_speed(path, save_fn, read_fn, ext, n_iter=1000):
    # measure the time to read the mask  per iteration in ms

    mask = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    mask_path = f"test.{ext}"
    save_fn(mask_path, mask)

    start = time.time()
    for i in range(n_iter):
        r = read_fn(mask_path)
    end = time.time()

    os.remove(mask_path)

    return (end-start)/n_iter*1000

class TestFastMask(unittest.TestCase):
    def test_mask(self):
        # Create a mask
        mask = np.zeros((100, 100), dtype=np.int8)
        mask[20:80, 20:80] = 1

        # Create a FastMask object
        pf.write("mask.bin", mask)
        r = pf.read("mask.bin")

        # remove mask file
        os.remove("mask.bin")

        # Check the mask
        self.assertTrue(np.allclose(mask, r))

    def test_mask_shape(self):
        mask = np.zeros((100, 224), dtype=np.int8)
        mask[20:80, 20:200] = 1

        pf.write("mask.bin", mask)
        r = pf.read("mask.bin")

        # remove mask file
        os.remove("mask.bin")

        # compare shapes
        self.assertTrue(mask.shape == r.shape)
  

    
    def test_mask_correct(self):
        mask = cv2.imread("test_imgs/a.png", cv2.IMREAD_GRAYSCALE)
        pf.write("test.bin", mask)
        r = pf.read("test.bin")

        self.assertTrue(np.allclose(mask, r))

        # print size of file
        print(f"Size of mask file: {os.path.getsize('test.bin')/1024:.2f} KB")

        # remove mask file
        os.remove("test.bin")



    def test_read_speed(self):
        test_images = glob.glob("test_imgs/*.png")

        def qoi_write(path, mask):
            mask = np.concatenate([mask[..., None], mask[..., None], mask[..., None]], axis=-1)
            qoi.write(path, mask)

        methods = [
            ("pfm", pf.write, pf.read, 'pfm'),
            ("cv2", cv2.imwrite, cv2.imread, 'png'),
            ("qoi", qoi_write, qoi.read, 'qoi')
        ]

        for img in test_images:
            print(f"Image: {img}")
            for name, wr, re, ext in methods:
                print(f"Time per iteration for {name}: {test_speed(img, wr, re, ext):.2f} ms")

