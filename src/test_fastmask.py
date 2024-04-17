import pyfastmask as pf
import numpy as np
import cv2
import unittest
import os
import time


class TestFastMask(unittest.TestCase):
    def test_mask(self):
        # Create a mask
        mask = np.zeros((100, 100), dtype=np.int8)
        mask[20:80, 20:80] = 1

        # Create a FastMask object
        pf.writeMask("mask.bin", mask)
        r = pf.readMask("mask.bin")

        # remove mask file
        os.remove("mask.bin")

        # Check the mask
        self.assertTrue(np.allclose(mask, r))

    def test_mask_shape(self):
        mask = np.zeros((100, 224), dtype=np.int8)
        mask[20:80, 20:200] = 1

        pf.writeMask("mask.bin", mask)
        r = pf.readMask("mask.bin")

        # remove mask file
        os.remove("mask.bin")

        # compare shapes
        self.assertTrue(mask.shape == r.shape)
  

    
    def test_mask_correct(self):
        mask = cv2.imread("realmask1ch.png", cv2.IMREAD_GRAYSCALE)
        pf.writeMask("test.bin", mask)
        r = pf.readMask("test.bin")

        self.assertTrue(np.allclose(mask, r))

        # print size of file
        print(f"Size of mask file: {os.path.getsize('test.bin')/1024:.2f} KB")

        # remove mask file
        os.remove("test.bin")



    def test_read_speed(self):
        mask = cv2.imread("realmask1ch.png", cv2.IMREAD_GRAYSCALE)
        pf.writeMask("test.bin", mask)
        
        
        # measure the time to read the mask and print it
        start = time.time()
        for i in range(1000):
            r = pf.readMask("test.bin")
        end = time.time()

        # remove mask file
        os.remove("test.bin")

        print(f"Time to read mask: {(end-start):.2f} ms")

