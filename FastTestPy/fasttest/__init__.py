import os
import sys

#  C:/Users/natha/miniconda3/python.exe -m pybind11_stubgen fasttest_internal -o C:\Users\natha\OneDrive\Desktop\C++\FastTest\FastTestPy\fasttest

BASE_PATH = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
# BUILD_PATH = os.path.join(BASE_PATH, "out", "build", "x64-Release", "bin")
BUILD_PATH = os.path.join(BASE_PATH, "out", "build", "x64-Clang-Release", "bin")
sys.path.append(BUILD_PATH)

import fasttest_internal
