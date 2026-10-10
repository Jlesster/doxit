project(
  'doxit',
  'cpp',
  version: '0.0.1',
  default_options: [
    'cpp_std=c++23',
    'warning_level=3',
  ],
)

src = files(
  'src/docs.cpp',
  'src/main.cpp',
)

incl = include_directories('src')

deps = [dependency('trux', fallback: ['trux', 'trux_dep'])]

executable(
  'doxit',
  src,
  include_directories: incl,
  dependencies: deps,
  install: false,
)
