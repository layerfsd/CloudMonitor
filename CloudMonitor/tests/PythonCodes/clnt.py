#! /usr/bin/python3
#-*- coding:utf-8 -*-

'''
@author:        屈亮亮
@createTime:    2016-9-13
@function:文件上传
'''
import socket  
import struct  
import os  
import ssl
import uuid
import time
import platform
import hashlib

MAX_PACKET_SIZE = 1024

def get_reply_info(sock):
    buf = ''
    ret = False
    buf = sock.recv(HEAD_SIZE)
    if len(buf) <= 0:
        return  None
    rpl, rest_pktSize = struct.unpack(HEAD_FORMAT, buf)
    info = sock.recv(rest_pktSize).decode()
    return (rpl, info)


def send_info(sock, kind, info):
    buf = info.encode()
    rest_pktSize = len(buf)
    buf = struct.pack(HEAD_FORMAT, kind.encode(), rest_pktSize) + buf
    sock.send(buf)
    print ("client: ", kind, info)
    return True


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
    print("%d bytes sent" %(ret))
    return True


def getMAC():
    mac=uuid.UUID(int = uuid.getnode()).hex[-12:]
    if len(mac) == 12:
        mac = mac[0:2] + mac[2:4] + mac[4:6] +mac[6:8] +mac[8:10] +mac[10: ]
        return mac
    else:
        return '0'


def getsysName():
    sysName = platform.uname()[0]
    
    if len(sysName) > 0:
        return  sysName
    else:
        return '0'

def getMd5Hash(fileName):
    if not os.path.exists(fileName):
        return '0'*32
    with open(fileName,'rb') as f:
        md5obj = hashlib.md5()
        md5obj.update(f.read())
        filehash = md5obj.hexdigest()
        return filehash

def init_sock():
    sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)  
    ssl_sock = ssl.wrap_socket(sock, ca_certs="cert.pem", cert_reqs=ssl.CERT_REQUIRED) 
    e=0  
    try:  
        #ssl_sock.connect(('10.102.1.116',50005))  
        ssl_sock.connect(('192.168.43.132',50005))  
    except Exception as error:  
        print(error)
        return None
    return ssl_sock


HEAD_FORMAT = "!3sI"
HEAD_SIZE   = struct.calcsize(HEAD_FORMAT)

def end_connection(ssl_sock):
    if ssl_sock is not None:
        ssl_sock.close()


def get_mac():
    return "MAC:2 11:22:33:44:55:66-wireless aa:bb:cc:dd:ee:ff-wired\n"


def get_hds():
        return "HDS:2 9CD29CD2-machine C5F3BFE9-ssd\n"

def get_user():
    return '3130931002'


def register(sock):
    pc_config = get_mac() + get_hds()
    send_info(sock, "ATH", pc_config)

    rpl = ssl_sock.recv(HEAD_SIZE)
    cmd, rest_pktSize = struct.unpack(HEAD_FORMAT, rpl)
    cmd = cmd.decode()
    ret = False
    if "RPL" == cmd:
        rpl = ssl_sock.recv(rest_pktSize).decode()
        if rpl == "OK":
            ret = True
        else:
            ret = False
    return ret

def get_reply(sock):
    buf = ''
    buf = sock.recv(HEAD_SIZE)
    rpl, rest_pktSize = struct.unpack(HEAD_FORMAT, buf)
    buf = sock.recv(rest_pktSize).decode()
    if buf == "OK":
        return True
    return False


def authentication(sock):
    rpl, info = get_reply_info(sock)
    if "WHO ARE YOU" == info:
        print ("server: ", info)
    else:
        return False
    #login
    send_info(sock, "ATH", get_user())
    #if login failed try register new user
    if get_reply(sock) is False:
        print ("registing %s" %(get_user()))
        return register(sock)
    return True



def get_keywords(sock, file_name):
    if None in (sock, file_name):
        return False


def get_file_hash(file_name):
    return "1"*32


def get_file(sock, file_name):
    send_info(sock, "DNF", "keywords.txt")
    rpl, status = get_reply_info(sock)
    print ("server: ", status)

    if "FAILED" == status:
        print ("server don't have [%s]" %(file_name))
        return False

    rpl, file_hash = get_reply_info(sock)
    #检测新文件是否与本地文件 hash 相同
    print ("server: ", file_hash)
    if file_hash == getMd5Hash(file_name):
        send_info(sock, "RPL", "EXISTED")
        return True
    else:
        send_info(sock, "RPL", "BEGIN")

    rpl, file_size = get_reply_info(sock)
    file_size = int(file_size)
    rest_size = file_size
    print ("%s %s bytes"%(file_name, file_size))
    fp = open(file_name, "wb")
    while rest_size:
        buf = sock.recv(MAX_PACKET_SIZE)
        fp.write(buf)
        rest_size -= len(buf)
    fp.close()
    print ("reveived %s done"%(file_name))


def send_log(sock):
    cur_time = time.ctime()
    message = cur_time + " test.docx 加密-2 防御-3"
    ret = send_info(ssl_sock, "LOG", message)
    print ("LOG ", ret)


def upload_file(sock, cur_path):
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


def send_file(sock, file_name): 
    #文件全路径
    cur_path = file_name
    if not os.path.exists(cur_path):
        return False
    send_info(sock, "UPD", file_name)
    send_info(sock, "RPL", getMd5Hash(cur_path))
    #看看服务端要不要我的文件
    rpl, info = get_reply_info(sock)
    print("server: ", rpl, info)
    #如果服务端已经存在当前文件,就停止传输文件
    if "EXISTED" == info:
        print("%s have not update."%(file_name))
        return True

    print("%s not existed!" %(cur_path))
    print("start sending ", file_name)
    return upload_file(sock, cur_path)


ssl_sock = init_sock()
ret = authentication(ssl_sock)
print("Authentication: ", ret)
switch = True
if ret and switch:
    get_file(ssl_sock, "keywords.txt")

send_log(ssl_sock)
send_file(ssl_sock, "data")
send_info(ssl_sock, "END", "Bye-bye.")
