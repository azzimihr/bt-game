import os
os.environ['NUMBA_ENABLE_AVX'] = '1'
os.environ["NUMBA_LOOP_VECTORIZE"] = "1"
os.environ["NUMBA_DISABLE_INTEL_SVML"] = "0"
os.environ["NUMBA_CPU_NAME"] = "znver2"
os.environ["NUMBA_BOUNDSCHECK"] = "0"
os.environ['NUMBA_OPT'] = '3'

from ui import bench, compile

import sys
if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "bench":
        bench()
    else:
        compile()