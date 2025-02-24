from setuptools import setup

import codecs
import schema


setup(
    name=schema.__name__,
    version=schema.__version__,
    author="Vladimir Keleshev",
    author_email="vladimir@keleshev.com",
    description="Simple data validation library",
    license="MIT",
    keywords="schema json validation",
    url="https://github.com/keleshev/schema",
    py_modules=['schema'],
    long_description=codecs.open('README.rst', 'r', 'utf-8').read(),
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Topic :: Utilities",
        "Programming Language :: Python :: 2.6",
        "Programming Language :: Python :: 2.7",
        "Programming Language :: Python :: 3.2",
        "Programming Language :: Python :: 3.3",
        "Programming Language :: Python :: 3.4",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: Implementation :: PyPy",
        "License :: OSI Approved :: MIT License",
    ],
)
