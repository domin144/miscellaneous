# SPDX-FileCopyrightText: 2024 Dominik Wójt <domin144@o2.pl>
#
# SPDX-License-Identifier: Apache-2.0

project('miscellaneous', 'cpp')

boost_dep = dependency('boost', modules : ['program_options'])

#add_executable(plplot_playground plplot_playground.cpp)
#target_link_libraries(plplot_playground plplotcxxd)
executable('dynamic_template', ['dynamic_template.cpp'])
executable(
    'cache_test',
    ['cache_test.cpp', 'cache.h', 'utils.h', 'trees_and_heaps.h'])
executable('hp_4510s_fan_control', ['hp_4510s_fan_control.cpp'])
executable('pi', ['pi.cpp'])
executable('update_dir', ['update_dir.cpp'], dependencies : [boost_dep])