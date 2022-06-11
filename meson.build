project('aravis-c-examples', 'c', version: '0.0.1')

message('=============== Meson should be run as root for this project! ===============')
imc_dep = dependency('MagickCore', fallback : ['imagemagick-7.0', 'im_dep'])
imw_dep = dependency('MagickWand', fallback : ['imagemagick-7.0', 'im_dep'])
aravis_dep = dependency('aravis-0.8', fallback : ['aravis-0.8', 'aravis_dep'])

examples = [
  '01-single-acquisition',
  '02-multiple-acquisition',
  '03-camera-api',
  '04-camera-features',
]

foreach e: examples
  exe = executable (e, e + '.c', dependencies: [imc_dep, imw_dep, aravis_dep])
endforeach
