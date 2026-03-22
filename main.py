import os

from ui import bench, compile

import sys
if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "bench":
        bench()
    else:
        compile()