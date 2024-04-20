import pyfastmask as pf
import numpy as np
import unittest
import os


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


    def test_noise_bin(self):
        mask = np.random.randint(0, 2, (100, 100), dtype=np.uint8)
        self.assert_image_same_after_read_write(mask, 'noise')

    def test_noise_uint8(self):
        mask = np.random.randint(0, 256, (100, 100), dtype=np.uint8)
        self.assert_image_same_after_read_write(mask, 'noise')
