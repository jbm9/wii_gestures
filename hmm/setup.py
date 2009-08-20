#!/usr/bin/env python

#!/usr/bin/env python

"""
setup.py file for SWIG hmm
"""

from distutils.core import setup, Extension


hmm_module = Extension('_hmm',
                           sources=['hmm_wrap.c', 'hmm.c'],
                           )

setup (name = 'hmm',
       version = '0.1',
       author      = "Noisebridge Machine Learning",
       description = """""",
       ext_modules = [hmm_module],
       py_modules = ["hmm"],
       )

