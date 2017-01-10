# encoding: utf8

# filename: fabric.py

from config import file_list
import os
import sys


# return all files in which a new version need
def collect_files(path_config):
    need_files = []

    need_files.extend(path_config["appendix_files"])
    return need_files


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
    os.chdir(work_dir)
    if not os.path.exists('.git'):
        return False
    return True

print(set_work_path())
path_list = collect_files(file_list)
print(check_existence(path_list))