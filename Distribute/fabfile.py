# fabfile.py
import os, re
from datetime import datetime
from my_task import path_list
import os
from my_task import set_work_path, make_zip, collect_files_md5, update_version, get_latest_version
from fabric.api import *

# 服务器登录用户名:
env.user = 'WangBiao'
env.password = os.getenv('SSH_PASSWD')
env.hosts = ['114.215.19.63']

set_work_path()
_VERSION = get_latest_version()
_DST_DIR = "/var/ftp/CloudMonitor"
_ZIP_FILE = _VERSION + ".zip"


def build():
    collect_files_md5()
    _VERSION = update_version()
    make_zip(_VERSION)



def deploy():
    remote_tmp_dir = "/tmp"     # 临时保存目录

    # 远程上传路径
    upload_path = "{TMP_DIR}/{ZIP_NAME}".format(TMP_DIR=remote_tmp_dir, ZIP_NAME=_ZIP_FILE)
    # 解压路径
    extract_path = "{TMP_DIR}/{NAME}".format(TMP_DIR=remote_tmp_dir, NAME=_VERSION)
    # 服务所在位置
    remote_dst = "{REMOTE_DIR}/{NAME}".format(REMOTE_DIR=_DST_DIR, NAME=_VERSION)

    # 上传 zip 文件:
    print(_ZIP_FILE, upload_path)
    put(_ZIP_FILE, upload_path)

    # 解压到新目录:
    with cd(remote_tmp_dir):
        run('unzip -f %s' % _ZIP_FILE)

    sudo("mv {SRC} {DST}".format(SRC=extract_path, DST=_DST_DIR))

    # 删除临时文件
    run("rm -f {ZIP_PATH}".format(ZIP_PATH=upload_path))
