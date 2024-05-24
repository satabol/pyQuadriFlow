from setuptools import setup, find_packages

NAME = 'pyQuadriFlow'
VERSION = '0.0.1'
# - First Wrapper
DESCRIPTION = 'Python QuadriFlow.'
LONG_DESCRIPTION = 'A Python wrapper for the QuadriFlow C++ Library.'

setup(
    name = NAME,
    version = VERSION,
    author = 'Satabol',
    description = DESCRIPTION,
    long_description = LONG_DESCRIPTION,
    packages = find_packages(),
    include_package_data=True,
    package_data={f'{NAME}.clib':['*.so','*.dll']}, # https://stackoverflow.com/questions/70334648/how-to-correctly-install-data-files-with-setup-py
    install_requires = [],    
    keywords = ['quadriflow', 'quad', 'remeshing', 'quadrangulation',],
    classifiers= [
        "Development Status :: 2 - Pre-Alpha",
        "Intended Audience :: Developers",        
        "Programming Language :: Python :: 3",        
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX :: Linux",
    ]
)