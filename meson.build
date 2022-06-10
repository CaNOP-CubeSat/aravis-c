
project('aravis-c-examples', 'c', version: '0.0.1')
cc = meson.get_compiler('c')
aravis_dep = dependency('aravis-0.8')
incdir = include_directories('include')

examples = [
  '01-single-acquisition',
  '02-multiple-acquisition',
  '03-camera-api',
  '04-camera-features',
]

foreach e: examples
  exe = executable (e, e + '.c', dependencies: [aravis_dep])
endforeach
