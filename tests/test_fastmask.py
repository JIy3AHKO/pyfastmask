import unittest
import os
import tempfile

import numpy as np

import pyfastmask as pf


class TestFastMask(unittest.TestCase):

    def assert_image_same_after_read_write(self, img, name):
        # Write the image
        pf.write("test.pfm", img)
        r = pf.read("test.pfm")

        # remove test file
        os.remove("test.pfm")

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

    def test_simple_arange(self):
        mask = np.array([
            [0, 1],
            [2, 3]
        ], dtype=np.uint8)
        self.assert_image_same_after_read_write(mask, 'square4x4')

    def test_all_zeros(self):
        mask = np.zeros((100, 100), dtype=np.uint8)
        self.assert_image_same_after_read_write(mask, 'zeros')

    def test_noise_bin(self):
        mask = np.random.randint(0, 2, (100, 100), dtype=np.uint8)
        self.assert_image_same_after_read_write(mask, 'noise')

    def test_noise_uint8(self):
        mask = np.random.randint(0, 256, (100, 100), dtype=np.uint8)
        self.assert_image_same_after_read_write(mask, 'noise')

    def test_magic_bytes(self):
        mask = np.random.randint(0, 256, (100, 100), dtype=np.uint8)
        pf.write("test.pfm", mask)
        with open("test.pfm", "rb") as f:
            self.assertEqual(f.read(4), b'pfmf')
        os.remove("test.pfm")

    def test_read_wrong_file_produces_error(self):
        with open("test.pfm", "wb") as f:
            f.write(b'wrong')

        with self.assertRaises(ValueError):
            pf.read("test.pfm")

        os.remove("test.pfm")

    def test_read_wrong_version_produces_error(self):
        with open("test.pfm", "wb") as f:
            f.write(b'pfmf\x99')

        with self.assertRaises(ValueError):
            pf.read("test.pfm")

        os.remove("test.pfm")

    def test_info_returns_correct_shape(self):
        mask = np.random.randint(0, 256, (256, 128), dtype=np.uint8)
        pf.write("test.pfm", mask)
        info = pf.info("test.pfm")
        self.assertEqual(info['shape'], (256, 128))
        os.remove("test.pfm")

    def test_info_on_small_file_produces_error(self):
        with open("small.bin", "wb") as f:
            f.write(b'0')

        with self.assertRaises(ValueError):
            pf.info("small.bin")

        os.remove("small.bin")

    def test_read_on_small_file_produces_error(self):
        with open("small.bin", "wb") as f:
            f.write(b'0')

        with self.assertRaises(ValueError):
            pf.info("small.bin")

        os.remove("small.bin")
