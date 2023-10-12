from setuptools import setup, Extension
from setuptools.command.sdist import sdist as _sdist
from Cython.Build import cythonize
from pathlib import Path
import shutil
import sys
import os


class sdist(_sdist):
    """Custom sdist command to include the xlfparser header."""

    def run(self):
        # Copy the xlfparser.h file from the parent folder
        src = Path(__file__).parent.parent / "include" / "xlfparser.h"
        if src.exists():
            dst = Path(__file__).parent / "include" / "xlfparser.h"
            os.makedirs(dst.parent, exist_ok=True)
            shutil.copyfile(src, dst)

        return super().run()


extra_compile_args = []

if sys.platform == "win32":
    extra_compile_args.append("/std:c++17")


# The include folder is either in the current folder if building from the sdist
# or in the parent folder if building from source
include_dir = Path(__file__).parent / "include"
if not (include_dir / "xlfparser.h").exists():
    include_dir = Path(__file__).parent.parent / "include"


extensions = [
    Extension(
        "xlfparser._xlfparser",
        sources=[
            "xlfparser/_xlfparser.pyx"
        ],
        include_dirs=[
            str(include_dir)
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
    long_description=open("README.md", "rt").read(),
    long_description_content_type="text/markdown",
    license=open("LICENSE", "rt").read(),
    url="https://github.com/pyxll/xlfparser",
    include_package_data=True,
    ext_modules=cythonize(
        extensions,
        compiler_directives=directives
    ),
    cmdclass={
        "sdist": sdist
    },
    install_requires=[
        "Cython"
    ]
)
