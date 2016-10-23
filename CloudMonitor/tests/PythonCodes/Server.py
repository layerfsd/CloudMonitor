#! /usr/bin/python3
#-*-coding:utf-8-*-

from BLL.OperateDb import insertUploadInfo, getHash, insertFile
import socket 
# import threading
import struct
import os
import ssl
import time
from BLL.writeLog import log
from BLL.threadpools import ThreadPool


CONSTANT = 0
#HOST = "10.102.1.116"
HOST = "0.0.0.0"
PORT = 50005

'''
@author:        屈亮亮
@createTime:    2016-9-13
@function:    文件接收
'''
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
        self.localPath = path  #文件存放位置
        self.localFileName = self.gainlocalFileName()   #本地文件名
        self.writeLog = log()
        
        
    ''' 
    @author: 屈亮亮
    @function: 获取本地文件名
    @createTime 2016-9-14
    '''
    def gainlocalFileName(self):
        currentTime =  str(time.time()).split('.')
        currentTime = currentTime[0] + currentTime[1]
        print(currentTime + ".doc")
        return currentTime + ".doc"
        
    
    def gainCurrentTime(self):
        TIMEFORMAT='%Y-%m-%d %H:%M:%S'
        currentTime =  time.strftime(TIMEFORMAT, time.localtime())
        return currentTime
                
                
    ''' 
    @author: 屈亮亮
    @function: 接受文件头
    @createTime 2016-9-14
    '''
    def recvFile(self, connstream):
        
        try:
            FILEHEAD_SIZE = struct.calcsize("128s32s32s32s48s20sI")
            fHead = connstream.recv(FILEHEAD_SIZE)    #接收头文件
            
            self.fileName, self.fileHash, self.passwd, self.userName, self.HostVersion, self.hostMAC, self.fileSize = struct.unpack("128s32s32s32s48s20sI", fHead)
            self.fileName = self.fileName.decode("utf8").rstrip('\0')
            self.fileHash = self.fileHash.decode("utf8").rstrip('\0')
            self.userName = self.userName.decode("utf8").rstrip('\0')
            self.HostVersion = self.HostVersion.decode("utf8").rstrip('\0')
            self.hostMAC = self.hostMAC.decode("utf8").rstrip('\0')
            self.passwd = self.passwd.decode("utf8").rstrip('\0')
            self.fileSize = self.fileSize
            
            print("fileHash: %s, hostMAC:%s fileSize :%d \n"%(self.fileHash, self.hostMAC, self.fileSize))
            
            
            
        except Exception as e:
            self.writeLog.writeFile("%s (%s, %s) %s"%(self.gainCurrentTime(), self.hostMAC, self.hostIP, "Error: data revc err. \n"))
            print(e)

    ''' 
    @author: 屈亮亮
    @function: 操作数据库
    @createTime 2016-9-15
    '''                
    def operateDb(self):
        try:
            getHashIndex = getHash(self.fileHash)
            print("hash:", self.fileHash)

        except:
            self.writeLog.writeFile("%s (%s, %s) %s"%(self.gainCurrentTime(), self.hostMAC, self.hostIP, "Error: database fileHash select err. \n"))
            print("Error: database fileHash select err. \n")
        
        if getHashIndex == 0:
            try:
                insertFile(self.localFileName, self.fileSize, self.localPath, self.fileHash)
    
            except: 
                self.writeLog.writeFile("%s (%s, %s) %s"%(self.gainCurrentTime(), self.hostMAC, self.hostIP, "Error: database fileInf insert file err. \n"))
                print("Error: database fileInf insert file err. \n")
        
        currentTime = self.gainCurrentTime()
        try:
            insertUploadInfo(currentTime, self.fileName, self.hostIP, self.hostMAC, self.HostVersion,
                self.fileHash)
        
        except:
            self.writeLog.writeFile("%s (%s, %s) %s"%(self.gainCurrentTime(), self.hostMAC, self.hostIP, "Error: database userInf insert file err. \n"))
            print("Error: database userInf insert file err. \n")
            
        return getHashIndex
    
            
    ''' 
    @author: 屈亮亮
    @function: 写入文件
    @createTime 2016-9-15
    '''        
    def writeFile(self, connstream):
        
        file = os.path.join(self.localPath,self.localFileName)
        resetSize = self.fileSize   #定义剩余文件大小
        
        try:
            fp = open(file, 'wb')
            
            while(resetSize != 0):
                fileData = connstream.recv(1024)
                fp.write(fileData)            
                resetSize -= len(fileData) 
                
            fp.close()
            
        
        except:
            self.writeLog.writeFile("%s (%s, %s) %s"%(self.gainCurrentTime(), self.hostMAC, self.hostIP, "Error: write file err. \n"))
            print("Error: write file err.")
            
            
            
    ''' 
    @author: 屈亮亮
    @function: 文件写入运行
    @createTime 2016-9-15
    '''
    def run(self, connstream, address):
        
        self.hostIP = address[0]
        self.recvFile(connstream)  
#         ret = self.operateDb()
        
#         print("ret: ", ret)
#         
#         if ret == 0 or ret != 0:
#             connstream.send("ok".encode('utf_8'))
        self.writeFile(connstream, )
#         else:
#             connstream.send("no".encode('utf_8'))
            
        global CONSTANT
        CONSTANT -= 1
        
        connstream.shutdown(socket.SHUT_RDWR)      
        connstream.close()
            
                    
if __name__ == "__main__":
    
    context = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
    context.load_cert_chain(certfile="cert.pem", keyfile="key.pem")
    
    servSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    servSock.bind((HOST,PORT))
    servSock.listen(10)
    
#     old Data 2016/10/18
#     pools = threadpool.ThreadPool(50)
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

#    old Data 2016/10/18
#         requests = threadpool.makeRequests(f.run, (connstream, address))
#         [pools.putRequest(req) for req in requests]
#         recvFileThread = threading.Thread(target=f.run,args=(connstream, address))
#         recvFileThread.start()
#         pools.wait()
        
        
