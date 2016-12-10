#!/usr/local/env python
#encoding: utf8
from socket import *
import os

HOST='127.0.0.1' 
PORT=10009   #服务器端口号
BUFSIZ=1024   #缓冲区大小 
ADDR=(HOST,PORT)   #ip和端口构成地址

tcpCliSock=socket(AF_INET,SOCK_STREAM)   #生成新的socket对象
tcpCliSock.connect(ADDR)  #连接服务器

def parent():   #定义子函数
        while True:
                datasend=raw_input('client:')
                if not datasend:
                        break
                tcpCliSock.send(datasend)  #send函数发送数据
                print "send to server %s" %datasend


def child():
        while True:
                data=tcpCliSock.recv(BUFSIZ) #recv函数接收数据
                if not data:
                        break
                print "From server: "
                print data
                print 'client:'
if __name__=='__main__':
        parent()
        tcpCliSock.close()   #关闭连接
