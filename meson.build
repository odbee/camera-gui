project('opengl_project', ['c', 'cpp'],
  version: '0.1.0',
  default_options: ['cpp_std=c++17'])

# Import filesystem module
fs = import('fs')

# Find dependencies
opengl_dep = dependency('opengl', required: true)
sdl2_dep = dependency('sdl2', required: true)
libcamera_dep = dependency('libcamera', required: true)
fmt_dep = dependency('fmt', required: true) 
egl_dep = dependency('egl', required: true)

dependencies = [opengl_dep, sdl2_dep, libcamera_dep, fmt_dep, egl_dep] 
# Source files
sources = [
  'src/glad.c',
  'src/SDLHandler.cpp',
  'src/ShaderUtils.cpp',
  'src/CameraPreview.cpp',
  'src/SharedContext.cpp',
  'src/EGLBuffersMinimal.cpp',
  'src/GetStreamData.cpp',
]

sources_alt = [
  'src/glad.c',
  'src/SDLHandler.cpp',
  'src/ShaderUtils.cpp',
  'src/CameraPreview_alt.cpp',
  'src/SharedContext.cpp',
  'src/EGLBuffers.cpp',
  'src/GetStreamData.cpp',
]

# Get all asset files
asset_files = run_command('find', meson.current_source_dir() / 'assets', '-type', 'f', check: true).stdout().strip().split('\n')
# Get all shader files  
shader_files = run_command('find', meson.current_source_dir() / 'shaders', '-type', 'f', check: true).stdout().strip().split('\n')

# Custom targets for copying assets and shaders
asset_targets = []
shader_targets = []

# Process assets
foreach asset : asset_files
  if asset != ''
    asset_name = fs.name(asset)
    asset_target = custom_target('asset_' + asset_name,
      input: asset,
      output: asset_name,
      command: ['cp', '@INPUT@', '@OUTPUT@'],
      install: false,
      build_by_default: true
    )
    asset_targets += asset_target
  endif
endforeach

# Process shaders
foreach shader : shader_files
  if shader != ''
    shader_name = fs.name(shader)
    shader_target = custom_target('shader_' + shader_name,
      input: shader,
      output: shader_name,
      command: ['cp', '@INPUT@', '@OUTPUT@'],
      install: false,
      build_by_default: true
    )
    shader_targets += shader_target
  endif
endforeach



# Create executable
executable('camera_gui',
  ['src/main.cpp', sources],
  include_directories: include_directories('include'),
  dependencies: dependencies,
  install: false
)

executable('camera_gui_alt',
  ['src/main_alt.cpp', sources_alt],
  include_directories: include_directories('include'),
  dependencies: dependencies,
  install: false
)


# Ensure assets and shaders are built with the main target
if asset_targets.length() > 0
  alias_target('assets', asset_targets)
endif

if shader_targets.length() > 0
  alias_target('shaders', shader_targets)
endif