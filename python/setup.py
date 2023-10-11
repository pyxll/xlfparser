from setuptools import setup, Extension
from Cython.Build import cythonize
from pathlib import Path
import sys


extra_compile_args = []

if sys.platform == "win32":
    extra_compile_args.append("/std:c++17")


extensions = [
    Extension(
        "xlfparser._xlfparser",
        sources=[
            "xlfparser/_xlfparser.pyx"
        ],
        include_dirs=[
            str(Path(__file__).parent.parent / "include")
        ],
        extra_compile_args=extra_compile_args
    )
]


directives = {
    "language_level": "3",  # We assume Python 3 code
    "boundscheck": False,  # Do not check array access
    "wraparound": False,  # a[-1] does not work
    "embedsignature": False,  # Do not save typing / docstring
    "always_allow_keywords": False,  # Faster calling conventions
    "initializedcheck": False,  # We assume memory views are initialized
}


setup(
    name="xlfparser",
    version="0.0.1",
    packages=["xlfparser"],
    author="Tony Roberts",
    author_email="tony@pyxll.com",
    description="Excel formula parser.",
    url="https://github.com/pyxll/xlfparser",
    ext_modules=cythonize(
        extensions,
        compiler_directives=directives
    ),
    install_requires=[
        "Cython"
    ]
)
