import numpy as _np

import _pyfastmask


def read(path: str) -> _np.ndarray:
    return _pyfastmask.read(path)


def read_bytes(data: bytes) -> _np.ndarray:
    return _pyfastmask.read_bytes(data)


def write(path: str, mask: _np.ndarray) -> None:
    mask = _np.ascontiguousarray(mask)
    _pyfastmask.write(path, mask)


def write_bytes(mask: _np.ndarray) -> bytes:
    mask = _np.ascontiguousarray(mask)
    return _pyfastmask.write_bytes(mask)


def info(path: str) -> _pyfastmask.Header:
    return _pyfastmask.info(path)
