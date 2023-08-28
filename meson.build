project('ecs', 'cpp',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'buildtype=debugoptimized',
    'cpp_std=c++17'
  ])

src = [
  'ecs.cpp',
]

exe = executable(
  'ecs',
  sources: src,
  install : true
)

test('basic', exe)
