import os
from subprocess import check_call
from pprint import pprint

prod_dir='production'

dir_key='dir'
opt_key='opt'
toolch_key='tool'
prod_key='prod_file'

config = {
    'linux_i686' :
    {
        dir_key : 'build_linux_i686',
        opt_key : '-G "CodeBlocks - Unix Makefiles" -DUSE_X86_32=ON -DBUILD_STATIC=ON',
        toolch_key : '',
        prod_key : 'qrvc_linux_x86_32.tar.gz'
    },

    'windows_i686' :
    {
        dir_key : 'build_windows_i686',
        opt_key : '-G "CodeBlocks - Unix Makefiles" -DUSE_X86_32=ON -DBUILD_STATIC=ON',
        toolch_key : '-DCMAKE_TOOLCHAIN_FILE="../cmake-mingw-toolchains/Toolchain-Ubuntu-mingw32.cmake"',
        prod_key : 'qrvc_win_x86_32.tar.gz'
    },

    'linux_x86_64' :
    {
        dir_key : 'build_linux_x86_64',
        opt_key : '-G "CodeBlocks - Unix Makefiles" -DUSE_X86_32=OFF -DBUILD_STATIC=ON',
        toolch_key : '',
        prod_key : 'qrvc_linux_x86_64.tar.gz'
    },

    'windows_x86_64' :
    {
        dir_key : 'build_windows_x86_64',
        opt_key : '-G "CodeBlocks - Unix Makefiles" -DUSE_X86_32=OFF -DBUILD_STATIC=ON',
        toolch_key : '-DCMAKE_TOOLCHAIN_FILE="../cmake-mingw-toolchains/Toolchain-Ubuntu-mingw64.cmake"',
        prod_key : 'qrvc_win_x86_64.tar.gz'
    }
}

def my_check_call(command):
    with open("build.log", "w") as build_log:
        check_call(command, shell=True, stderr=build_log, stdout=build_log)
    

print(os.getcwd())
pprint(config)
print()

remdir_command = 'cmake -E remove_directory '
mkdir_command = 'cmake -E make_directory '
print(remdir_command + prod_dir)
print(mkdir_command + prod_dir)
my_check_call(remdir_command + prod_dir)
my_check_call(mkdir_command + prod_dir)
print()
for key in config.keys():

    #clean/create build dir
    print(remdir_command + config[key][dir_key])
    my_check_call(remdir_command + config[key][dir_key])
    print(mkdir_command + config[key][dir_key])
    my_check_call(mkdir_command + config[key][dir_key])

    #go to build dir
    print('os.chdir(' + config[key][dir_key] + ')')
    os.chdir(config[key][dir_key])
    print("In: " + os.getcwd())

    #configure/build/copy
    configure_command = 'cmake ' + config[key][opt_key] + ' ' + config[key][toolch_key] + ' ..'
    build_command = 'cmake --build .'
    copy_command = 'cmake -E copy ' + config[key][prod_key] + ' ../' + prod_dir + '/' + config[key][prod_key]
    print(configure_command)
    my_check_call(configure_command)
    print(build_command)
    my_check_call(build_command)
    print(copy_command)
    my_check_call(copy_command)

    #go back
    print('os.chdir("..")')
    os.chdir('..')
    print(os.getcwd())
    print()
