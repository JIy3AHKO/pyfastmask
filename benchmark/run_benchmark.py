import abc
import os
import time
from typing import List

import qoi
import cv2
import numpy as np
from py_markdown_table.markdown_table import markdown_table

import pyfastmask as pf


class Method(abc.ABC):
    def __init__(self, name: str, extension: str):
        self.name = name
        self.extension = extension

    @abc.abstractmethod
    def save(self, path: str, mask: np.ndarray):
        pass

    @abc.abstractmethod
    def read(self, path: str) -> np.ndarray:
        pass


class PFMMethod(Method):
    def __init__(self):
        super().__init__("pfm", "pfm")

    def save(self, path: str, mask: np.ndarray):
        pf.write(path, mask)

    def read(self, path: str) -> np.ndarray:
        return pf.read(path)


class CV2Method(Method):
    def __init__(self):
        super().__init__("cv2", "png")

    def save(self, path: str, mask: np.ndarray):
        mask = np.concatenate([mask[..., None], mask[..., None], mask[..., None]], axis=-1)
        cv2.imwrite(path, mask)

    def read(self, path: str) -> np.ndarray:
        mask = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        return mask


class QOIMethod(Method):
    def __init__(self):
        super().__init__("qoi", "qoi")

    def save(self, path: str, mask: np.ndarray):
        # we need to save the mask as a color image, since qoi does not support grayscale images
        mask = mask[..., np.newaxis]
        mask = np.concatenate([mask, mask, mask], axis=-1)
        qoi.write(path, mask)

    def read(self, path: str) -> np.ndarray:
        mask = qoi.read(path)
        return mask


def test_speed_and_size(path, method: Method, n_iter=100):
    # measure the time to read the mask  per iteration in ms

    mask = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    mask_path = f"test.{method.extension}"
    method.save(mask_path, mask)

    size = os.path.getsize(mask_path)

    start = time.time()
    for i in range(n_iter):
        r = method.read(mask_path)
    end = time.time()

    os.remove(mask_path)

    return (end-start)/n_iter*1000, size


def read_csv(path: str) -> List[List[str]]:
    with open(path, 'r') as f:
        return [line.strip().split(',') for line in f.readlines()]


def test_read_speed(images_csv: str = "test_images.csv", n_iterations: int = 100):
    test_images = read_csv(images_csv)

    methods = [PFMMethod(), CV2Method(), QOIMethod()]

    speed_table = []
    size_table = []

    for img_name, img_path in test_images:
        print(f"Image: {img_name}")
        row_speed = {"Image": img_name}
        row_size = {"Image": img_name}
        for m in methods:
            iter_time, size = test_speed_and_size(img_path, m, n_iter=n_iterations)
            row_speed[m.name] = f"{iter_time:.2f} ms"
            row_size[m.name] = f"{size} Byte"
        speed_table.append(row_speed)
        size_table.append(row_size)

    markdown = markdown_table(speed_table).get_markdown()
    print(markdown)

    markdown = markdown_table(size_table).get_markdown()
    print(markdown)


test_read_speed(images_csv=os.path.join(os.path.dirname(__file__), "test_images.csv"), n_iterations=100)