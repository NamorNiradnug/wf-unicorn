project(
  'wf-unicorn',
  'cpp',
  version: '0.1',
  license: 'MIT',
  default_options: [
    'cpp_std=c++17',
    'warning_level=1'
  ]
)

wayfire = dependency('wayfire')
wlroots = dependency('wlroots')
wfconfig = dependency('wf-config')
rsvg = dependency('librsvg-2.0')

add_project_arguments(['-DWLR_USE_UNSTABLE'], language: ['cpp', 'c'])
add_project_arguments(['-DWAYFIRE_PLUGIN'], language: ['cpp', 'c'])
add_project_link_arguments(['-rdynamic'], language:'cpp')

icon_dir = join_paths(get_option('prefix'), 'share', 'wayfire', 'icons')
icon_name = 'plugin-wf-unicorn.svg'
icon_path = join_paths(icon_dir, icon_name)

subdir('src')
subdir('metadata')

install_data(
  icon_name,
  install_dir: icon_dir
)
