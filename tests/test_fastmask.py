import pyfastmask as pf
import numpy as np
import cv2
import unittest
import os
import time
import glob
import qoi

TEST_IMG_DIR_GLOB = os.path.join(os.path.dirname(__file__), "imgs", '*.png')


def test_speed_and_size(path, save_fn, read_fn, ext, n_iter=100):
    # measure the time to read the mask  per iteration in ms

    mask = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    mask_path = f"test.{ext}"
    save_fn(mask_path, mask)

    size = os.path.getsize(mask_path)

    start = time.time()
    for i in range(n_iter):
        r = read_fn(mask_path)
    end = time.time()

    os.remove(mask_path)

    return (end-start)/n_iter*1000, size

class TestFastMask(unittest.TestCase):

    def assert_image_same_after_read_write(self, img, name):
        # Write the image
        pf.write("test.bin", img)
        r = pf.read("test.bin")

        # remove test file
        os.remove("test.bin")

        # Check the image
        np.testing.assert_array_equal(img, r, err_msg=f"Mask {name} is not equal to the read mask.")


    def test_mask(self):
        # Create a mask
        mask = np.zeros((100, 100), dtype=np.uint8)
        mask[20:80, 20:80] = 1

        self.assert_image_same_after_read_write(mask, 'square')

    def test_mask_arange(self):
        mask = np.arange(256).reshape(16, 16).astype(np.uint8)
        self.assert_image_same_after_read_write(mask, 'arange')


    def test_mask_correct(self):
        for mask_path in glob.glob(TEST_IMG_DIR_GLOB):
            mask = cv2.imread(mask_path, cv2.IMREAD_GRAYSCALE)
            self.assert_image_same_after_read_write(mask, mask_path)

    def test_read_speed(self):
        test_images = glob.glob(TEST_IMG_DIR_GLOB)

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
                iter_time, size = test_speed_and_size(img, wr, re, ext)
                print(f"Time per iteration for {name}: {iter_time:.2f} ms; Size: {size} Byte")
