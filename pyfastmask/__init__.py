import pathlib
from typing import Dict, Union

import numpy as _np

import _pyfastmask


def _validate_mask_and_prepare(mask: _np.ndarray) -> _np.ndarray:
    if len(mask.shape) == 3 and mask.shape[-1] == 1:
        mask = mask[:, :, 0]

    if len(mask.shape) != 2:
        raise ValueError(f"Array must have format (W, H) or (W, H, 1), but found {mask.shape}")

    if mask.dtype != _np.uint8:
        raise ValueError(f"Array must be np.uint8, got {mask.dtype}")

    mask = _np.ascontiguousarray(mask)

    return mask


def read(path: Union[str, pathlib.Path]) -> _np.ndarray:
    """
    Read pfm file and return mask as numpy array.

    Args:
        path: (str, pathlib.Path) Path to pfm file.

    Returns:
        mask: (np.ndarray) Mask as numpy array.
    Examples:
        >>> mask = read("path/to/mask.pfm")
    """
    path = str(path)
    return _pyfastmask.read(path)


def decode(buffer: bytes) -> _np.ndarray:
    """
    Decode buffer and return mask as numpy array.

    Args:
        buffer: (bytes) Buffer to decode.

    Returns:
        mask: (np.ndarray) Mask as numpy array.
    Examples:
        >>> with open("path/to/mask.pfm", "rb") as f:
        >>>     data = f.read()
        >>> mask = decode(data)
    """
    return _pyfastmask.decode(buffer)


def write(path: Union[str, pathlib.Path], mask: _np.ndarray) -> None:
    """
    Write mask to pfm file.

    Mask must be numpy array with shape (W, H) or (W, H, 1) and dtype np.uint8.

    Args:
        path: (str) Path to save pfm file.
        mask: (np.ndarray) Mask to save.

    Examples:
        >>> mask = np.random.randint(0, 256, (100, 100), dtype=np.uint8)
        >>> write("path/to/mask.pfm", mask)
    """
    path = str(path)
    mask = _validate_mask_and_prepare(mask)
    _pyfastmask.write(path, mask)


def encode(mask: _np.ndarray) -> bytes:
    """
    Encode mask and return as buffer.

    Same as `write` but returns buffer instead of saving to file.

    Args:
        mask: (np.ndarray) Mask to encode.

    Returns:
        buffer: (bytes) Encoded buffer.

    Examples:
        >>> mask = np.random.randint(0, 256, (100, 100), dtype=np.uint8)
        >>> buffer = encode(mask)
        >>> with open("path/to/mask.pfm", "wb") as f:
        >>>     f.write(buffer)
    """
    mask = _validate_mask_and_prepare(mask)
    return _pyfastmask.encode(mask)


def info(path: Union[str, pathlib.Path]) -> Dict[str, int]:
    """
    Get info of pfm file.

    Reads header of pfm file and returns dictionary with:
    - shape: (Tuple[int, int]) Shape of mask.
    - unique_symbols_count: (int) Number of unique symbols in mask.
    - line_count_bit_width: (int) Number of bits used to store amount of runs in line.
    - count_bit_width: (int) Number of bits used to represent count of symbols in run.
    - symbol_bit_width: (int) Number of bits used to represent symbol value.

    Args:
        path: (str) Path to pfm file.

    Returns:
        info: (Dict[str, int]) Dictionary with file info.
    """
    path = str(path)
    return _pyfastmask.info(path)
