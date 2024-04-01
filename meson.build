project('figura', 'cpp', version: '0.1.0', default_options : ['warning_level=3', 'cpp_std=c++20'])

dependencies  = [dependency('libpng')]
sources       = []
include       = include_directories('include')

figura_dep = declare_dependency(
    dependencies        : dependencies
  , include_directories : include
  )

if not meson.is_subproject()
  subdir('example')
endif
