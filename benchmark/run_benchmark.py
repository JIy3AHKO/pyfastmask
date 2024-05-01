import argparse
import abc
import os
import time
from typing import List, Dict, Union

import cv2
import numpy as np
import qoi


import pyfastmask as pf


def draw_md_table(data: List[Dict[str, Union[str, float]]], units: str) -> str:
    headers = data[0].keys()
    table = "| " + " | ".join(headers) + " |\n"
    table += "| " + " | ".join(["---" for _ in headers]) + " |\n"

    for row in data:
        table += "| "
        lowest = min([v for k, v in row.items() if k != "Image"])

        for k, v in row.items():
            if k == "Image":
                table += f"{v} | "
            else:
                if v == lowest:
                    table += f"**{v:.2f} {units}** | "
                else:
                    table += f"{v:.2f} {units} | "
        table += "\n"

    return table


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


class CV2PngMethod(Method):
    def __init__(self):
        super().__init__("cv2_png", "png")

    def save(self, path: str, mask: np.ndarray):
        cv2.imwrite(path, mask)

    def read(self, path: str) -> np.ndarray:
        mask = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        return mask


class CV2MethodCompression9(Method):
    def __init__(self):
        super().__init__("cv2_png_cmp9", "png")

    def save(self, path: str, mask: np.ndarray):
        cv2.imwrite(path, mask, [cv2.IMWRITE_PNG_COMPRESSION, 9])

    def read(self, path: str) -> np.ndarray:
        mask = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        return mask


class CV2BmpMethod(Method):
    def __init__(self):
        super().__init__("cv2_bmp", "bmp")

    def save(self, path: str, mask: np.ndarray):
        cv2.imwrite(path, mask)

    def read(self, path: str) -> np.ndarray:
        mask = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        return mask


class NPZMethod(Method):
    def __init__(self):
        super().__init__("npz", "npz")

    def save(self, path: str, mask: np.ndarray):
        np.savez_compressed(path, mask=mask)

    def read(self, path: str) -> np.ndarray:
        mask = np.load(path)["mask"]
        return mask


class NPYMethod(Method):
    def __init__(self):
        super().__init__("npy", "npy")

    def save(self, path: str, mask: np.ndarray):
        np.save(path, mask)

    def read(self, path: str) -> np.ndarray:
        mask = np.load(path)
        return mask


class QOIMethod(Method):
    def __init__(self):
        super().__init__("qoi", "qoi")

    def save(self, path: str, mask: np.ndarray):
        # we need to save the mask as a color image, since qoi does not support grayscale images
        mask = mask[..., np.newaxis]
        mask = np.concatenate([mask, np.zeros_like(mask), np.zeros_like(mask)], axis=-1)
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


def generate_test_images_csv(images_dir: str) -> List[List[str]]:
    res = []
    for img_name in sorted(os.listdir(images_dir)):
        bname, ext = os.path.splitext(img_name)
        res.append([bname, os.path.join(images_dir, img_name)])

    return res


def aggregate_results(
        data: List[Dict[str, float]],
        agg_name: str,
        methods: List[Method],
        fn,
) -> Dict[str, str]:
    agg_row = {"Image": agg_name}
    for m in methods:
        agg_row[m.name] = fn(data, m.name)

    return agg_row


def get_average(data: List[Dict[str, float]], method_name: str) -> float:
    return sum([row[method_name] for row in data]) / len(data)


def get_median(data: List[Dict[str, float]], method_name: str) -> float:
    return np.median([row[method_name] for row in data])


def test_read_speed(
        images_dir: str,
        n_iterations: int = 100,
        update_readme: bool = False,
        methods: List[str] = None,
):
    test_images = generate_test_images_csv(images_dir)

    available_methods = [
        cls() for cls in globals().values() if isinstance(cls, type) and issubclass(cls, Method) and cls != Method
    ]

    if methods is None:
        methods = available_methods
    else:
        methods = [m for m in available_methods if m.name in methods]

    speed_table = []
    size_table = []

    for img_name, img_path in test_images:
        print(f"Image: {img_name}")
        row_speed = {"Image": img_name}
        row_size = {"Image": img_name}

        for m in methods:
            iter_time, size = test_speed_and_size(img_path, m, n_iter=n_iterations)
            row_speed[m.name] = iter_time
            row_size[m.name] = size / 1024
        speed_table.append(row_speed)
        size_table.append(row_size)

    speed_table.append(aggregate_results(speed_table, "Average", methods, get_average))
    speed_table.append(aggregate_results(speed_table, "Median", methods, get_median))

    size_table.append(aggregate_results(size_table, "Average", methods, get_average))
    size_table.append(aggregate_results(size_table, "Median", methods, get_median))

    speed_markdown = draw_md_table(speed_table, "ms")
    print(speed_markdown)
    print()

    size_markdown = draw_md_table(size_table, "KiB")
    print(size_markdown)

    if update_readme:
        with open("README.md", "r") as f:
            readme = f.read()

        start_str = "Reading speed:"
        end_str = "All measurements are averaged over 1000 iterations. The best result is highlighted in bold."

        start = readme.find(start_str)
        end = readme.find(end_str)

        new_readme = [
            readme[:start + len(start_str) + 1],
            speed_markdown,
            "Mask size:\n",
            size_markdown,
            readme[end:]
        ]

        new_readme = "\n".join(new_readme)

        with open("README.md", "w") as f:
            f.write(new_readme)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--images-dir", type=str, required=True)
    parser.add_argument("--n-iterations", type=int, default=100)
    parser.add_argument("--methods", type=str, nargs="+", default=None)
    parser.add_argument("--update-readme", action="store_true")
    args = parser.parse_args()

    test_read_speed(args.images_dir, args.n_iterations, args.update_readme, args.methods)
