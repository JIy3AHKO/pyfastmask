import pyfastmask as pf
import numpy as np
import unittest

class TestFastMask(unittest.TestCase):
    def test_mask(self):
        # Create a mask
        mask = np.zeros((100, 100), dtype=np.int8)
        mask[20:80, 20:80] = 1

        # Create a FastMask object
        pf.writeMask("mask.bin", mask)
        r = pf.readMask("mask.bin").reshape(100, 100)

        # Check the mask
        self.assertTrue(np.allclose(mask, r))


