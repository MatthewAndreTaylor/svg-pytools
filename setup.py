from setuptools import setup, Extension
from distutils.command.build_ext import build_ext as build_ext_orig


class build_ext(build_ext_orig):
    def build_extension(self, ext):
        return super().build_extension(ext)

    def get_export_symbols(self, ext):
        return super().get_export_symbols(ext)

    def get_ext_filename(self, ext_name):
        return super().get_ext_filename(ext_name)
    
# '-std=c++11'

setup(
    include_package_data=True,
    ext_modules= [
            Extension(
            "svgtools",
            ["src/svgtools.cpp"]
        ),
    ],
    cmdclass={"build_ext": build_ext},
)
