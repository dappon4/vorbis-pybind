from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import shutil
import pybind11
import numpy
import glob
import os

# Collect all C sources in lib/
vorbis_c_sources = glob.glob(os.path.join('lib', '*.c'))

# Exclude test/demo files and vorbisfile.c if present, but always include wav_to_pcm.c
vorbis_c_sources = [s for s in vorbis_c_sources if not s.endswith('test_sharedbook') and not s.endswith('psytune.c') and not s.endswith('vorbisfile.c') and not s.endswith('tone.c') and not s.endswith('barkmel.c')]
if 'lib/wav_to_pcm.c' not in vorbis_c_sources:
    vorbis_c_sources.append('lib/wav_to_pcm.c')

ext_modules = [
    Extension(
        'vorbis_pybind',
        sources=vorbis_c_sources + ['lib/vorbis_pybind.cpp'],
        include_dirs=[
            pybind11.get_include(),
            numpy.get_include(),
            'lib',
            'lib/vorbis',
            'lib/books/floor',
            'lib/books/coupled',
            'lib/books/uncoupled',
            'lib/modes',
        ],
        language='c++',
        extra_compile_args=['-std=c++17'],
        libraries=["ogg"],
        library_dirs=["/usr/local/lib", "/lib/x86_64-linux-gnu"],
        extra_link_args=['-logg'],
        extra_objects=["/lib/x86_64-linux-gnu/libogg.so"],
        runtime_library_dirs=["/usr/local/lib", "/lib/x86_64-linux-gnu"],
    )
]


class CustomBuildExt(build_ext):
    def run(self):
        super().run()
        # Find the built .so file and copy to project root
        build_lib = self.build_lib
        for ext in self.extensions:
            ext_path = self.get_ext_fullpath(ext.name)
            # ext_path is relative to build_lib
            full_path = os.path.join(build_lib, ext_path)
            dest_name = os.path.basename(full_path)
            shutil.copy2(full_path, dest_name)
            print(f"Copied {full_path} to {dest_name}")

setup(
    name='vorbis_pybind',
    version='0.1',
    ext_modules=ext_modules,
    install_requires=['pybind11', 'numpy'],
    cmdclass={'build_ext': CustomBuildExt},
)
