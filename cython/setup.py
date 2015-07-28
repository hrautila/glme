
from distutils.core import setup
from distutils.extension import Extension
import shutil, os

try:
    from Cython.Distutils import build_ext
    from Cython.Build import cythonize
    from distutils.command.sdist import sdist as _sdist
except ImportError:
    use_cython = False
else:
    use_cython = True

## We would like to have symlinks to ../src and ../src/inc but sdist tarball does not
## dereference symlinks. Python source distribution needs to have actual files so we
## keep local copy of gobber.c and gobber.h and track modification times.
try:
    if os.stat('gobber.c').st_mtime < os.stat('../src/gobber.c').st_mtime:
        print('Update copy of ../src/gobber.c')
        shutil.copy('../src/gobber.c', 'gobber.c')
except FileNotFoundError:
    print('Make local copy of ../src/gobber.c')
    shutil.copy('../src/gobber.c', 'gobber.c')

try:
    if os.stat('gobber.h').st_mtime < os.stat('../src/inc/gobber.h').st_mtime:
        print('Update copy of ../src/inc/gobber.h')
        shutil.copy('../src/inc/gobber.h', 'gobber.h')
except FileNotFoundError:
    print('Make local copy of ../src/inc/gobber.h')
    shutil.copy('../src/inc/gobber.h', 'gobber.h')

cmdclass = {}
ext_modules = []

if use_cython:
    ext_modules += [
        Extension('glme',
                  sources=['glme.pyx', 'gobdec.c', 'gobber.c']
              )
        ]
    cmdclass.update({'build_ext': build_ext})

    ## This will force updating glme.c file included in source dist
    class sdist(_sdist):
        def run(self):
            cythonize(['glme.pyx'])
            _sdist.run(self)

    cmdclass['sdist'] = sdist

else:
    ext_modules += [
        Extension('glme',
                  sources=['glme.c', 'gobdec.c', 'gobber.c'],
              )
        ]


setup(
    name = 'glme',
    version = '0.1',
    description = 'A binary Gob Like Message Encoding',
    author = 'Harri Rautila',
    author_email = 'harri.rautila@gmail.com',
    keywords = 'binary, encoding, message',
    license='BSD',
    url = 'https://github.com/hrautila/glme',
    cmdclass = cmdclass,
    ext_modules = ext_modules
)

