import ctypes
import numpy as np
import os

_lib_path = os.path.join(os.path.dirname(__file__), '_breakthrough.so')
_lib = ctypes.CDLL(_lib_path)

_lib.init_board.argtypes = []
_lib.init_board.restype = None

_lib.set_board.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
_lib.set_board.restype = None

_lib.get_board.argtypes = [ctypes.POINTER(ctypes.c_uint8)]
_lib.get_board.restype = None

_lib.call_ai_turn.argtypes = [ctypes.c_uint8, ctypes.POINTER(ctypes.c_uint8)]
_lib.call_ai_turn.restype = None

_lib.call_gameover.argtypes = []
_lib.call_gameover.restype = ctypes.c_int8

_lib.call_state.argtypes = []
_lib.call_state.restype = ctypes.c_int16

def init_board():
    """Initialize board to empty state."""
    _lib.init_board()

def set_board(board):
    """Set board state from numpy array (8x8 uint8)."""
    if board.shape != (8, 8):
        raise ValueError("board must be 8x8")
    if board.dtype != np.uint8:
        board = board.astype(np.uint8)
    _lib.set_board(board.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)))

def get_board():
    """Get board state as numpy array (8x8 uint8)."""
    board = np.zeros((8, 8), dtype=np.uint8)
    _lib.get_board(board.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)))
    return board

def ai_turn(depth):
    """Get AI move. Returns tuple (r1, c1, r2, c2)."""
    out = (ctypes.c_uint8 * 4)()
    _lib.call_ai_turn(depth, out)
    return tuple(out)

def gameover():
    """Check game over condition. Returns -127 (p1 won), 127 (p2 won), 0 (ongoing)."""
    return _lib.call_gameover()

def state():
    """Get current board evaluation score."""
    return _lib.call_state()
