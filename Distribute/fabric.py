#encoding：utf8

# filename: fabric.py

import os
import sys
import time
import hashlib

def cal_file_hash(file_path):
    with open(file_path, 'rb') as f:
        md5obj = hashlib.md5()
        md5obj.update(f.read())
        filehash = md5obj.hexdigest()
        return filehash

def get_curtime():
    return time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))


# return all files in which a new version need
def collect_files_md5(saved_path=r'Release\DAta\hashlist.txt'):
    save_dir = os.path.dirname(saved_path)
    if not os.path.exists(save_dir):
        print("making dir: %s" %(save_dir))
        os.mkdirs(save_dir)

    # local_path, install_path
    path_list = [
        ('x64\\Release\\HooKeyboard-64.dll', 'HooKeyboard-64.dll'),
        ('x64\\Release\\MonitorService-64.exe', 'MonitorService-64.exe'),
        (r'Release\autoStart.exe', 'autoStart.exe'),
        (r'Release\CloudMonitor.exe', 'CloudMonitor.exe'),
        (r'Release\Daemon.exe', 'Daemon.exe'),
        (r'Release\HooKeyboard.dll', 'HooKeyboard.dll'),
        (r'Release\libcrypto-1_1.dll', 'libcrypto-1_1.dll'),
        (r'Release\libssl-1_1.dll', 'libssl-1_1.dll'),
        (r'Release\Monitor.exe', 'Monitor.exe'),
        (r'Release\MonitorService.exe', 'MonitorService.exe'),
        (r'Release\msvcp140.dll', 'msvcp140.dll'),
        (r'Release\msvcr110.dll', 'msvcr110.dll'),
        (r'Release\RemoveSelf.exe', 'RemoveSelf.exe'),
        (r'Release\Service.exe', 'Service.exe'),
        (r'Release\Uninstall.exe', 'Uninstall.exe'),
        (r'Release\Update.exe', 'Update.exe'),
        (r'Release\vcruntime140.dll', 'vcruntime140.dll'),
        (r'Release\all2txt\a2t.id', r'all2txt\a2t.id'),
        (r'Release\all2txt\a2t.key', r'all2txt\a2t.key'),
        (r'Release\all2txt\a2tcmd.exe', r'all2txt\a2tcmd.exe'),
        (r'Release\all2txt\a2thlp.dll', r'all2txt\a2thlp.dll'),
        (r'Release\all2txt\a2thtm.dll', r'all2txt\a2thtm.dll'),
        (r'Release\all2txt\a2tpdf.dll', r'all2txt\a2tpdf.dll'),
        (r'Release\all2txt\a2trtf.dll', r'all2txt\a2trtf.dll'),
        (r'Release\DATA\config.ini', r'DATA\config.ini'),
        (r'Release\tools\7-zip.dll', r'tools\7-zip.dll'),
        (r'Release\tools\7z.dll', r'tools\7z.dll'),
        (r'Release\tools\7z.exe', r'tools\7z.exe'),
        (r'Release\tools\openssl.exe', r'tools\openssl.exe'),
    ]
    fp = open(saved_path, 'w')
    for local_path, install_path in path_list:
        md5_str = cal_file_hash(local_path)
        print(md5_str, install_path, file=fp)
    print("# update time: ", get_curtime(), file=fp)
    fp.close()


# check if all the file_paths exist
def check_existence(need_files):
    if need_files is None:
        return False

    bret_value = True
    for cur_path in need_files:
        if not os.path.exists(cur_path):
            bret_value = False
            print("[error] [%s] not exists" % (cur_path))
            break

    return bret_value


# set work path to the root of this repo
def set_work_path():
    work_dir = os.path.dirname(os.getcwd())
    print(os.getcwd(), work_dir)
    os.chdir(work_dir)
    print("set workpath to %s OK" % (work_dir))
    if not os.path.exists('.git'):
        print("set workpath to %s FAILED" % (work_dir))
        return sys.exit(1)
    return True

def update_version(saved_path=r'Release\DATA\VERSION'):
    save_dir = os.path.dirname(saved_path)
    if not os.path.exists(save_dir):
        print("making dir: %s" %(save_dir))
        os.mkdirs(save_dir)

    # get latest_version from git command
    output = os.popen('git tag')

    # 倒数第一个是空行，倒数第二个即时最新版本号
    latest_version = output.read().split('\n')[-2]

    fp = open(saved_path, 'w')
    print(latest_version, file=fp)
    fp.close()



set_work_path()
collect_files_md5()
update_version()
