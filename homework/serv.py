#!/usr/local/env python
#coding=utf-8
from socket import *
import os

HOST=''
PORT=10009  #服务器端口号
BUFSIZ=1024  #缓冲区大小
ADDR=(HOST,PORT)  #ip和端口构成地址

tcpSerSock=socket(AF_INET,SOCK_STREAM)  #生成新的socket对象
tcpSerSock.bind(ADDR)  #将ip与端口号绑定
tcpSerSock.listen(5) #监听

print "Waiting for connection......"
tcpCliSock,addr=tcpSerSock.accept()   #利用accept函数被动连接
print "connected from:",addr #获取连接客户端的ip和端口号


def child():
        while True:
                data=tcpCliSock.recv(BUFSIZ) #接收数据
                if not data:
                        break
                print "From client:"
                print data
                print 'server:'

def parent():
        pid=os.fork() #fork函数产生子进程

        if pid==0:      #与子进程交互
                child()
        else:           #在父进程中与子进程聊天
                datac=raw_input("server:")
                if not datac:
                        break
                tcpCliSock.send('%s' %(datac)) #发送数据
                print "send to client %s"%datac


if __name__=='__main__':
        parent()
        tcpCliSock.close() #关闭连接
