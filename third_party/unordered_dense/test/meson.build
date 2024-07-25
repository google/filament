test_sources = [
    'app/counter.cpp',
    'app/doctest.cpp',
    'app/nanobench.cpp',
    'app/stacktrace.cpp',
    'app/ui/periodic.cpp',
    'app/ui/progress_bar.cpp',
    'app/unordered_dense.cpp',

    'bench/swap.cpp',
    'bench/show_allocations.cpp',
    'bench/quick_overall_map.cpp',
    'bench/game_of_life.cpp',
    'bench/find_random.cpp',
    'bench/copy.cpp',

    'fuzz/run.cpp',

    'unit/assign_to_move.cpp',
    'unit/assignment_combinations.cpp',
    'unit/at.cpp',
    'unit/bucket.cpp',
    'unit/contains.cpp',
    'unit/copy_and_assign_maps.cpp',
    'unit/copyassignment.cpp',
    'unit/count.cpp',
    'unit/ctors.cpp',
    'unit/custom_container_boost.cpp',
    'unit/custom_container.cpp',
    'unit/custom_hash.cpp',
    'unit/deduction_guides.cpp',
    'unit/diamond.cpp',
    'unit/empty.cpp',
    'unit/equal_range.cpp',
    'unit/erase_if.cpp',
    'unit/erase_range.cpp',
    'unit/erase.cpp',
    'unit/explicit.cpp',
    'unit/extract.cpp',
    'unit/fuzz_api.cpp',
    'unit/fuzz_insert_erase.cpp',
    'unit/fuzz_replace_map.cpp',
    'unit/fuzz_string.cpp',
    'unit/hash_char_types.cpp',
    'unit/hash_smart_ptr.cpp',
    'unit/hash_string_view.cpp',
    'unit/hash.cpp',
    'unit/include_only.cpp',
    'unit/initializer_list.cpp',
    'unit/insert_or_assign.cpp',
    'unit/insert.cpp',
    'unit/iterators_empty.cpp',
    'unit/iterators_erase.cpp',
    'unit/iterators_insert.cpp',
    'unit/load_factor.cpp',
    'unit/maps_of_maps.cpp',
    'unit/max.cpp',
    'unit/move_to_moved.cpp',
    'unit/multiple_apis.cpp',
    'unit/namespace.cpp',
    'unit/not_copyable.cpp',
    'unit/not_moveable.cpp',
    'unit/pmr_move_with_allocators.cpp',
    'unit/pmr.cpp',
    'unit/rehash.cpp',
    'unit/replace.cpp',
    'unit/reserve_and_assign.cpp',
    'unit/reserve.cpp',
    'unit/segmented_vector.cpp',
    'unit/set_or_map_types.cpp',
    'unit/set.cpp',
    'unit/std_hash.cpp',
    'unit/swap.cpp',
    'unit/transparent.cpp',
    'unit/try_emplace.cpp',
    'unit/tuple_hash.cpp',
    'unit/unique_ptr.cpp',
    'unit/unordered_set.cpp',
    'unit/vectorofmaps.cpp',
    'unit/windows_include.cpp',
]

# additional compile options
# see https://mesonbuild.com/Reference-tables.html
cpp_args = []
compiler = meson.get_compiler('cpp')
foreach arg : [
            # gcc
            '-Wno-stringop-overflow', # g++ error in fmtlib
            '-Warith-conversion',
            '-Wshadow=global',
            '-Wno-array-bounds', # gcc 13 gives incorrect warning

            # gcc / clang
            '-Wconversion',
            '-Wextra',
            '-Wunreachable-code',
            '-Wuninitialized',
            '-pedantic-errors',
            '-Wold-style-cast',
            '-Wno-unused-function',
            # '-Weffc++', doesn't work with fmt
            
            # '-march=native',
        ]
    if compiler.has_argument(arg)
        cpp_args += [arg]
    endif
endforeach

if compiler.get_id() == 'msvc'
    add_global_arguments(
        '/wd4189', # fmt: 'zero': local variable is initialized but not referenced, fixed in https://github.com/fmtlib/fmt/issues/2891
        '/wd4251', # 'fmt::v8::ostream::file_': class 'fmt::v8::file' needs to have dll-interface to be used by clients of class 'fmt::v8::ostream'
        language: 'cpp')
endif

# for include-what-you-use
#cpp_args += '-isystem'
#cpp_args += '/usr/lib64/clang/14.0.0/include/'

fmt_method = 'auto'
if get_option('cpp_args').contains('-m32')
    # use builtin so we can compile it for 32bit. 
    # Can't use it as a default or sanitizer doesn't work...
    fmt_method = 'builtin'
endif

# use e.g.
# CXX='ccache clang++' BOOST_ROOT=/home/martinus/dev/boost_1_81_0/ meson setup --buildtype release -Dcpp_std=c++17 build
opt_boost = dependency('boost', required: false)
link_args = []
if opt_boost.found()
    add_global_arguments('-DANKERL_UNORDERED_DENSE_HAS_BOOST=1', language: 'cpp')
    link_args += ['-lrt']
else
    add_global_arguments('-DANKERL_UNORDERED_DENSE_HAS_BOOST=0', language: 'cpp')
endif

#opt_absl = dependency('absl_container', required: true, )
#if opt_boost.found()
#    add_global_arguments('-DANKERL_UNORDERED_DENSE_HAS_ABSL=1', language: 'cpp')
#else
#    add_global_arguments('-DANKERL_UNORDERED_DENSE_HAS_ABSL=0', language: 'cpp')
#endif

cpp_args += [
    #'-DFUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION',
    #'-fsanitize-undefined-trap-on-error',
    #'-fsanitize=undefined,address',
    #'-fno-sanitize=thread',
    #'-ftrivial-auto-var-init=pattern',
    #'-g',
]

link_args += [
    #'-fsanitize=undefined,address',
    #'-fno-sanitize=thread'
    #'-Wl,-shuffle-sections' # for benchmarking with mold linker
]

test_exe = executable(
    'udm-test',
    test_sources,
    include_directories: incdir,
    cpp_args: cpp_args,
    link_args: link_args,
    dependencies: [
        dependency('threads'), # add dependency for threads (-lpthread, see https://mesonbuild.com/howtox.html),

        # see what's in the [provide] sections for the dependency names
        dependency('doctest'),
        dependency('fmt', method: fmt_method),

        # disable these two if you don't want them
        #dependency('boost'),
        #dependency('absl_container', default_options: ['warning_level=0', 'werror=false'])
        # dependency('absl_hash', method: 'builtin', default_options: ['warning_level=0', 'werror=false'])
    ],
)

benchmark(
    'bench',
    test_exe,
    args: ['-ns', '-ts=bench'],
    verbose: true)

test(
    'unit',
    test_exe,
    verbose: true)

