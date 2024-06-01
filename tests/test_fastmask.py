import unittest
import os
import numpy as np
import tempfile

import pyfastmask as pf


class TempFile:
    def __init__(self):
        self.name = tempfile.mktemp()

    def __enter__(self):
        return self.name

    def __exit__(self, exc_type, exc_val, exc_tb):
        if os.path.exists(self.name):
            os.remove(self.name)


class TestWrite(unittest.TestCase):
    def test_write_rgb_produce_error(self):
        mask = np.random.randint(0, 256, (100, 100, 3), dtype=np.uint8)

        with self.assertRaises(ValueError):
            with TempFile() as f:
                pf.write(f, mask)

    def test_write_rgba_produce_error(self):
        mask = np.random.randint(0, 256, (100, 100, 4), dtype=np.uint8)

        with self.assertRaises(ValueError):
            with TempFile() as f:
                pf.write(f, mask)

    def test_write_float_produce_error(self):
        mask = np.random.rand(100, 100).astype(np.float32)

        with self.assertRaises(ValueError):
            with TempFile() as f:
                pf.write(f, mask)

    def test_write_int_produce_error(self):
        mask = np.random.randint(0, 256, (100, 100), dtype=np.int32)

        with self.assertRaises(ValueError):
            with TempFile() as f:
                pf.write(f, mask)

    def test_write_wh1_not_produce_error(self):
        mask = np.random.randint(0, 256, (33, 44, 1), dtype=np.uint8)

        with TempFile() as f:
            pf.write(f, mask)
            mask_after = pf.read(f)

        self.assertEqual(mask_after.shape, (33, 44))


class TestReadWrite(unittest.TestCase):
    def setUp(self):
        self.test_params = []

        square_mask = np.zeros((100, 100), dtype=np.uint8)
        square_mask[20:80, 20:80] = 1
        self.test_params.append(('square', square_mask))

        arange_mask = np.arange(256).reshape(16, 16).astype(np.uint8)
        self.test_params.append(('arange', arange_mask))

        simple_arange_mask = np.array([
            [0, 1],
            [2, 3]
        ], dtype=np.uint8)
        self.test_params.append(('simple_arange', simple_arange_mask))

        simple_arange_backward_mask = np.array([
            [9, 8, 7],
            [6, 5, 4],
            [3, 2, 1]
        ], dtype=np.uint8)
        self.test_params.append(('simple_arange_backward', simple_arange_backward_mask))

        all_zeros_mask = np.zeros((100, 100), dtype=np.uint8)
        self.test_params.append(('all_zeros', all_zeros_mask))

        all_constant_mask = np.full((101, 101), 127, dtype=np.uint8)
        self.test_params.append(('all_constant', all_constant_mask))

        binary_noise = np.random.randint(0, 2, (1234, 2345), dtype=np.uint8)
        self.test_params.append(('binary_noise', binary_noise))

        noise = np.random.randint(0, 256, (77, 171), dtype=np.uint8)
        self.test_params.append(('noise', noise))

        non_contiguous_mask = np.zeros((100, 100), dtype=np.uint8)
        non_contiguous_mask[20:80, 20:80] = 1
        non_contiguous_mask = non_contiguous_mask[::2, ::2]

        self.test_params.append(('non_contiguous', non_contiguous_mask))

    def test_read_write_consistency_disk(self):
        for name, img in self.test_params:
            with self.subTest(name=name):
                with TempFile() as f:
                    pf.write(f, img)
                    r = pf.read(f)
                    np.testing.assert_array_equal(img, r, err_msg=f"Mask {name} is different after read/write to disk.")

    def test_read_write_consistency_bytes(self):
        for name, img in self.test_params:
            with self.subTest(name=name):
                buffer = pf.encode(img)
                r = pf.decode(buffer)
                np.testing.assert_array_equal(img, r, err_msg=f"Mask {name} is different after read/write to bytes.")

    def test_read_on_small_file_produces_error(self):
        with tempfile.NamedTemporaryFile(mode="wb") as f:
            f.write(b'0')

            with self.assertRaises(ValueError):
                pf.read(f.name)

    def test_read_wrong_file_produces_error(self):
        with tempfile.NamedTemporaryFile(mode="wb") as f:
            f.write(b'wrong')

            with self.assertRaises(ValueError):
                pf.read(f.name)

    def test_read_wrong_version_produces_error(self):
        with tempfile.NamedTemporaryFile(mode="wb") as f:
            f.write(b'pfmf\x99')

            with self.assertRaises(ValueError):
                pf.read(f.name)


class TestInfo(unittest.TestCase):
    def test_magic_bytes(self):
        mask = np.random.randint(0, 256, (100, 100), dtype=np.uint8)

        with TempFile() as f:
            pf.write(f, mask)
            with open(f, "rb") as f:
                self.assertEqual(f.read(4), b'pfmf')

    def test_info_returns_correct_shape(self):
        mask = np.random.randint(0, 256, (256, 128), dtype=np.uint8)
        with TempFile() as f:
            pf.write(f, mask)
            info = pf.info(f)
            self.assertEqual(info['shape'], (256, 128))

    def test_info_on_small_file_produces_error(self):
        with tempfile.NamedTemporaryFile(mode="wb") as f:
            f.write(b'0')

            with self.assertRaises(ValueError):
                pf.info(f.name)
