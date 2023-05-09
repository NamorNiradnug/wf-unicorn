project(
  'wf-unicorn',
  'cpp',
  version: '0.1',
  license: 'MIT',
  default_options: [
    'cpp_std=c++17',
    'warning_level=everything'
  ]
)

wayfire = dependency('wayfire')
wlroots = dependency('wlroots')
wfconfig = dependency('wf-config')

add_project_arguments(['-DWLR_USE_UNSTABLE'], language: ['cpp', 'c'])
add_project_arguments(['-DWAYFIRE_PLUGIN'], language: ['cpp', 'c'])
add_project_link_arguments(['-rdynamic'], language:'cpp')

subdir('src')
subdir('metadata')

install_data(
  'plugin-wf-unicorn.svg',
  install_dir: join_paths(get_option('prefix'), 'share', 'wayfire', 'icons')
)
