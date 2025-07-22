from setuptools import setup, Extension
import pybind11

ext_modules = [
    Extension(
        'test_ogg_link',
        sources=['test_ogg_link.cpp'],
        include_dirs=[pybind11.get_include()],
        libraries=["ogg"],
        library_dirs=["/usr/local/lib", "/lib/x86_64-linux-gnu"],
        extra_link_args=['-logg'],
        extra_objects=["/lib/x86_64-linux-gnu/libogg.so"],
        runtime_library_dirs=["/usr/local/lib", "/lib/x86_64-linux-gnu"],
        language='c++',
    )
]

setup(
    name='test_ogg_link',
    version='0.1',
    ext_modules=ext_modules,
)
