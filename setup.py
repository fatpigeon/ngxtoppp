from setuptools import setup
from distutils.core import setup, Extension

modules = Extension('ngxtop_cpp',
                    define_macros=[('MAJOR_VERSION', 0),
                                   ('MINOR_VERSION', 1)],
                    include_dirs=['src/'],
                    libraries='',
                    library_dirs=[],
                    sources=['src/main.cpp', 'src/ngxtop++.cpp', 'src/lines.cpp'],
                    extra_compile_args=['-std=c++11'],
                    extra_link_args=['-lboost_regex', '-lboost_iostreams']
)

setup(name='ngxtop_cpp',
      version='0.1.0',
      description='ngxtop effect',
      author='bigpigeon',
      author_email='bigpigeon0@gmail.com',
      url='',
      long_description='',
      ext_modules=[modules])

# setup(
#     name='ngxtop++',
#     version='0.0.2',
#     description='Real-time metrics for nginx server',
#     long_description=open('README.rst').read(),
#     license='MIT',
#
#     url='https://github.com/lebinh/ngxtop',
#     author='Binh Le',
#     author_email='lebinh.it@gmail.com',
#
#     classifiers=[
#         'Development Status :: 4 - Beta',
#         'License :: OSI Approved :: MIT License',
#         'Environment :: Console',
#         'Intended Audience :: Developers',
#         'Intended Audience :: System Administrators',
#         'Programming Language :: Python :: 2',
#         'Programming Language :: Python :: 2.6',
#         'Programming Language :: Python :: 2.7',
#         'Programming Language :: Python :: 3',
#         'Programming Language :: Python :: 3.2',
#         'Programming Language :: Python :: 3.3',
#     ],
#     keywords='cli monitoring nginx system',
#
#     packages=['ngxtop'],
#     install_requires=['docopt', 'tabulate', 'pyparsing'],
#
#     entry_points={
#         'console_scripts': [
#             'ngxtop = ngxtop.ngxtop:main',
#         ],
#     },
# )
