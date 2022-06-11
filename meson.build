project('aravis-c-examples', 'c', version: '0.0.1')

cc = meson.get_compiler('c')
im_dep = dependency('ImageMagick-7.0')
aravis_dep = dependency('aravis-0.8')

examples = [
  '01-single-acquisition',
  '02-multiple-acquisition',
  '03-camera-api',
  '04-camera-features',
]

foreach e: examples
  exe = executable (e, e + '.c', dependencies: [im_dep, aravis_dep])
endforeach
