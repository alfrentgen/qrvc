import os
from subprocess import check_call, call
from pprint import pprint
from hashlib import md5

common_opts = ' -p 99 '

in_file = '1M.in'
enc_file = '1M.enc'
out_file = '1M.out'
key_file = 'key.yuv'
log_file='test.log'

enc = 'qrve'
dec = 'qrvd'

archs = ['i686', 'x86_64']
c_vals = ['', '-c', '-c key.yuv']
e_vals = ['-e 0', '-e 1', '-e 2', '-e 3']
k_vals = ['', '-k']
r_vals = ['', '-r 9']
m_vals = ['-m slow', '-m mixed']

rm_com = 'rm'
md5_com = 'md5sum'
print(os.getcwd())

def my_check_call(command):
    with open(log_file, "a") as log:
        check_call(command, shell=True, stderr=log, stdout=log)

def my_call(command):
    with open(log_file, "a") as log:
        call(command, shell=True, stderr=log, stdout=log)

def calc_md5(file_name):
    with open(file_name, mode='rb') as file_to_check:
        # read contents of the file
        data = file_to_check.read()    
        # pipe contents of the file through
        md5_returned = md5(data).hexdigest()
        return md5_returned

def verify(*file_names):
    with open(log_file, "a") as log:
        md5_in = calc_md5(in_file)
        md5_out = calc_md5(out_file)
        print('hashsum in:\t', md5_in)
        print('hashsum out:\t', md5_out)
        print('hashsum in:\t', md5_in, file=log, end='\n')
        print('hashsum out:\t', md5_out, file=log, end='\n')
        if md5_in != md5_out:
            exit(0)

def generate_files(*file_names):
    for fn in file_names:
        my_check_call('head -c 1M < /dev/urandom >' + fn)

def delete_files(*file_names):
    for fn in file_names:
        my_call(rm_com + ' ' + fn)

"""def generate_test_command(enc_path, dec_path, enc_config, dec_config, common_config):
    for k,v in enc_config:
        for k,v in dec_config:"""
    

#the test body
delete_files(log_file)
for arch in archs:
    enc_name = '../build_linux_' + arch + '/' + enc
    dec_name = '../build_linux_' + arch + '/' + dec
    for e in e_vals:
        for c in c_vals:
            delete_files(in_file, out_file, key_file)
            generate_files(in_file)

            encode = enc_name + ' -i ' + in_file + ' -o ' + enc_file
            encode = encode + ' ' + e + ' ' + c + common_opts

            decode = dec_name + ' -i ' + enc_file + ' -o ' + out_file
            decode = decode + ' ' + c + common_opts

            print('running: ' + encode)
            my_check_call(encode)
            
            print('running: ' + decode)
            my_check_call(decode)
            
            verify([in_file, out_file])
            print('OK!')
