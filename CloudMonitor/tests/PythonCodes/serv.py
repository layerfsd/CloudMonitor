#! /usr/bin/python3
#-*-coding:utf-8-*-

from BLL.OperateDb import insertUploadInfo, getHash, insertFile
import socket 
import struct
import os
import ssl
import time
import hashlib
from BLL.writeLog import log
from BLL.threadpools import ThreadPool


CONSTANT = 0
HOST = "0.0.0.0"
PORT = 50005
FILEKEEP_DIR = "/tmp/safe"

HEAD_FORMAT = "!3sI"
HEAD_SIZE = struct.calcsize(HEAD_FORMAT)
MAX_PACKET_SIZE = 1024


registed_list = []
computer_list = []
warnings_list  = []


#数据库接口--注册用户名、主机信息
def insert_user2DB(user_num, pc_config):
    #当且仅当用户名有效时，才允许用户注册
    if is_user_recorded(user_num):
        registed_list.append(user_num)
        computer_list.append(pc_config)
        return True
    return False



#数据库接口--当前hash值对应的文件是否存在
def is_hash_here(file_hash):
    return None


#数据库接口--文件上传记录
def log_file_upload(user_num, file_name):
    pass

#数据库接口--检查用户名是否允许注册
def is_user_recorded(user_num):
    return user_num in ['3130931001', '3130931002', '3130931003']


#将数据库接口--用户名加入已注册用户的信息表中
def is_user_registed(user_num):
    return user_num in registed_list


#记数据库接口--记录警报日志
def record_warnings(info):
    warnings_list.append(info)
    return True


def send_info(sock, kind, info):
    buf = info.encode()
    rest_pktSize = len(buf)
    buf = struct.pack(HEAD_FORMAT, kind.encode(), rest_pktSize) + buf
    return sock.send(buf) > 0


def reply_client(sock, status):
    if sock is None:
        return False
    buf = ''
    if status not in ("OK", "FAILED"):
        return False
    confirm = status.encode()
    rest_pktSize = len(confirm)
    buf = struct.pack(HEAD_FORMAT, "RPL".encode(), rest_pktSize)
    ret = sock.send(buf+confirm)
    return True


def get_file_hash(file_path):
    with open(file_path, 'rb') as f:
        md5obj = hashlib.md5()
        md5obj.update(f.read())
        filehash = md5obj.hexdigest()
        return filehash


def send_file(sock, file_name):
    cur_path = os.path.join(FILEKEEP_DIR, file_name)
    print("cur_path: ", cur_path)
    if os.path.exists(cur_path):
        send_info(sock, "RPL", "OK")
        send_info(sock, "RPL", get_file_hash(cur_path))
        print("waiting for reply...")
        #print(sock.recv(100))
        rpl, info = get_reply_info(sock)
        print("client: ", rpl, info)
        #如果待发送的文件与请求方已有文件相同
        #则取消传送
        if "EXISTED" == info:
            print("%s have not update."%(file_name))
            return True
    else:
        print("%s not existed!" %(cur_path))
        send_info(sock, "RPL", "FAILED") 
        return False

    file_size = os.path.getsize(cur_path)
    send_info(sock, "RPL", str(file_size))
    rest_size = file_size
    fp = open(cur_path, 'rb')
    print("transfering %d bytes" % (rest_size))
    while rest_size:
        buf = fp.read(MAX_PACKET_SIZE)
        sock.send(buf)
        rest_size -= len(buf)
    fp.close()
    return True

def download_file(sock, file_name):
    file_path = os.path.join(FILEKEEP_DIR, file_name)
    rpl, file_size = get_reply_info(sock)
    file_size = int(file_size)
    rest_size = file_size
    print ("%s %s bytes"%(file_path, file_size))
    fp = open(file_path, "wb")
    while rest_size:
        buf = sock.recv(MAX_PACKET_SIZE)
        fp.write(buf)
        rest_size -= len(buf)
    fp.close()
    print ("reveived %s done"%(file_path))
    return True


def recv_file(sock, user_num, file_name):
    cur_path = os.path.join(FILEKEEP_DIR, file_name)
    if not os.path.exists(FILEKEEP_DIR):
        os.mkdir(FILEKEEP_DIR)

    rpl, file_hash = get_reply_info(sock)

    print ("client: ", file_hash)
    #检查当前内容的文件是否已经保存在本地
    check_result = is_hash_here(file_hash)
    if check_result is not None:
        send_info(sock, "RPL", "EXISTED")
        log_file_upload(user_num, file_name)
        return True
    #通知对方可以发送文件了
    send_info(sock, "RPL", "BEGIN")
    download_file(sock, file_name)
    log_file_upload(user_num, file_name)


def get_reply_info(sock):
    buf = ''
    ret = False
    buf = sock.recv(HEAD_SIZE)
    if len(buf) <= 0:
        return None
    rpl, rest_pktSize = struct.unpack(HEAD_FORMAT, buf)
    info = sock.recv(rest_pktSize).decode()
    return (rpl.decode(), info)



def parse_register_info(info):
    #user_num = info.split('\n')[0]
    #lists = info.split('\n')[1:]
    lists = info.split('\n')
    reg = {}
    #reg['user_num'] = user_num
    for ite in lists:
        ite = ite.strip()
        if len(ite) <= 0:continue
        dev = ite.split(' ')[0]
        dev_name, dev_num = dev.split(':')
        reg[dev_name] = {}
        dev_num = int(dev_num)
        reg[dev_name]['num'] = dev_num
        items = ite.split(' ')[1:]
        for (i, e) in enumerate(items):
            addr, desc = e.split('-')
            reg[dev_name][i] = (desc, addr)
            #reg[dev_name][desc] = addr
    #hd_nums = reg['HDS']['num']
    return reg

    
def register_user(sock, user_num, data):
    pc_info = parse_register_info(data)
    if insert_user2DB(user_num, pc_info):
        return True
    return False

class File:
    def __init__(self,path="/tmp/Files"):
        self.fileName = None
        self.fileSize = None
        self.fileHash = None
        self.userName = None
        self.HostVersion = None
        self.hostMAC = None
        self.passwd = None
        self.hostIP = None
        self.user_num = None
        
            
    def auth(self, sock):
        #发送认证消息
        send_info(sock, "ATH", "WHO ARE YOU")

        #接收用户名
        rpl, user_num = get_reply_info(sock) 
        self.user_num = user_num

        #检查用户是否已经注册过
        if is_user_registed(user_num): 
            print("%s login Ok"%(user_num))
            return reply_client(sock, "OK")
        else:
            #当用户尚未注册时，返回登录失败，要求用户提供主机网卡和硬盘序列号信息
            reply_client(sock, "FAILED")
            rpl, data = get_reply_info(sock) 
            print("registing %s "%(user_num))
            if register_user(sock, user_num, data):
                return send_info(sock, "RPL", "OK")
            else:
                send_info(sock, "RPL", "FAILED")
                return False


    def end_connection(self, connstream):
        global CONSTANT
        CONSTANT -= 1
        connstream.shutdown(socket.SHUT_RDWR)      
        connstream.close()

    def run(self, connstream, address):
        ret = False 
        self.hostIP = address[0]
        if not  self.auth(connstream):
            self.end_connection(connstream)
            
        while True:
            cmd, info = get_reply_info(connstream)
            print("client: ", cmd, info)
            if "DNF" == cmd:
                send_file(connstream, info)
            elif "LOG" == cmd:
                record_warnings(info)
            elif "UPD" == cmd:
                recv_file(connstream, self.user_num, info)
            elif "END" == cmd:
                break
        #session end
        self.end_connection(connstream)

            
                    
if __name__ == "__main__":
    context = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
    context.load_cert_chain(certfile="cert.pem", keyfile="key.pem")
    
    servSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    servSock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    servSock.bind((HOST,PORT))
    servSock.listen(10)
    pool = ThreadPool()
    
    print("server %s listened by: %d \n"%(HOST,PORT))
    
    while(1):
        recvFrom, address = servSock.accept()
        connstream = context.wrap_socket(recvFrom, server_side=True)
        print("connect from :", address)
        CONSTANT += 1
        print("connect count: %s \n"%(CONSTANT, ))
        f = File()
        pool.add_job(f.run, connstream, address)
