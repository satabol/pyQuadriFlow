import ctypes
import os 
from sys import platform 
def load_library():
    if platform == 'linux' or platform == 'linux2':        
        QuadriFlow_clib = ctypes.CDLL(os.path.join(os.path.dirname(__file__),'ctypes_QuadriFlow.so'))
        pass
    elif platform == 'darwin':
        # OSX 
        pass
    elif platform == 'win32':        
        here = os.path.dirname(__file__).replace('\\','/') 
        QuadriFlow_clib = ctypes.CDLL(os.path.join(here,"ctypes_QuadriFlow.dll"))
    return QuadriFlow_clib

if __name__ == "__main__":
    QuadriFlow_clib = load_library()