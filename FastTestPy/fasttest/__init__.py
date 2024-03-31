import os
import sys


BASE_PATH = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
BUILD_PATH = os.path.join(BASE_PATH, "out", "build", "x64-Release", "bin")
sys.path.append(BUILD_PATH)

import fasttest_internal
