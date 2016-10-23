#! /usr/bin/python
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
import platform
import hashlib

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
    with open(fileName,'rb') as f:
        md5obj = hashlib.md5()
        md5obj.update(f.read())
        filehash = md5obj.hexdigest()
        return filehash
i = 100    
while(i > 0):
    sock = socket.socket(socket.AF_INET,socket.SOCK_STREAM)  
    ssl_sock = ssl.wrap_socket(sock, ca_certs="cert.pem", cert_reqs=ssl.CERT_REQUIRED) 
    e=0  
    try:  
        #ssl_sock.connect(('10.102.1.116',50005))  
        ssl_sock.connect(('127.0.0.1',50005))  
    except(socket.timeout,e):  
        print('timeout',e) 
    except(socket.error,e):  
        print('error',e)  
    except e:  
        print('any',e) 
    if not e:
        #filename = input("fileName>")#输入文件名 
        #filename = "nginxConfig.text"  
        filename = "test.txt"  
        localFileName = os.path.join("/tmp/Files", filename)
        filename = filename.encode('utf-8')
        
        fileHash = getMd5Hash(localFileName)
        if fileHash:
            fileHash = fileHash.encode('utf-8')
            
        userName = "".encode('utf-8')
        
        version = getsysName()
        if version != '0':
            HostVersion = version.encode('utf-8')
        else:
            print("ERROR: get HostVersion err.")
        
        mac = getMAC()
        
        if mac != '0': 
            hostMAC = mac.encode('utf-8')
        else:
            print("ERROR: get MAC err.")
         
        passwd = "".encode('utf-8')
        headValues = (filename, fileHash, userName, HostVersion, hostMAC, 
            passwd, os.stat(localFileName).st_size)
        
        filePack = struct.Struct('128s32s32s32s48s20sI')#编码格式大小
        fhead = filePack.pack(*headValues)#按照规则进行打包  
        ssl_sock.send(fhead) 
        readRes = ssl_sock.recv(4).decode("utf8")
        if readRes == "ok":
            fp = open(localFileName,'rb')  
            while 1:        #发送文件  
                filedata = fp.read(1024)  
                if not filedata:  
                    break  
                ssl_sock.send(filedata)
            print("sending over...")  
            fp.close()
            
        ssl_sock.close()
    
