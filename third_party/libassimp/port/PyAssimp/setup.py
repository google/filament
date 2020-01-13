 #!/usr/bin/env python
 # -*- coding: utf-8 -*-
import os
from distutils.core import setup

def readme():
    with open('README.rst') as f:
        return f.read()

setup(name='pyassimp',
      version='4.1.4',
      license='ISC',
      description='Python bindings for the Open Asset Import Library (ASSIMP)',
      long_description=readme(),
      url='https://github.com/assimp/assimp',
      author='ASSIMP developers',
      author_email='assimp-discussions@lists.sourceforge.net',
      maintainer='Séverin Lemaignan',
      maintainer_email='severin@guakamole.org',
      packages=['pyassimp'],
      data_files=[
                  ('share/pyassimp', ['README.rst']),
                  ('share/examples/pyassimp', ['scripts/' + f for f in os.listdir('scripts/')])
                 ],
      requires=['numpy']
      )
