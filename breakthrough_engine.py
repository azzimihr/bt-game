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
    _lib.init_board()

def set_board(board):
    if board.dtype != np.uint8:
        board = board.astype(np.uint8)
    _lib.set_board(board.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)))

def get_board():
    board = np.zeros((8, 8), dtype=np.uint8)
    _lib.get_board(board.ctypes.data_as(ctypes.POINTER(ctypes.c_uint8)))
    return board

def ai_turn(depth):
    out = (ctypes.c_uint8 * 4)()
    _lib.call_ai_turn(depth, out)
    return tuple(out)

def gameover():
    return _lib.call_gameover()

def state():
    return _lib.call_state()
