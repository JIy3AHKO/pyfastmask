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


def read(path: str) -> _np.ndarray:
    return _pyfastmask.read(path)


def decode(buffer: bytes) -> _np.ndarray:
    return _pyfastmask.decode(buffer)


def write(path: str, mask: _np.ndarray) -> None:
    mask = _validate_mask_and_prepare(mask)
    _pyfastmask.write(path, mask)


def encode(mask: _np.ndarray) -> bytes:
    mask = _validate_mask_and_prepare(mask)
    return _pyfastmask.encode(mask)


def info(path: str) -> _pyfastmask.Header:
    return _pyfastmask.info(path)
