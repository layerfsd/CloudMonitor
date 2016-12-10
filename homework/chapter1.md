# 1. Socket套接字的概念

**socket**

> **网络上的两个程序通过一个双向的通信实现数据的交换，这个连接的一端称为一个**`socket。`**`建立网络通信连接至少要一对端口号（socket）。Socket是操作系统内核中的一个数据结构，它是网络中的节点进行相互通信的门户。它是网络进程的ID。网络通信，归根到底还是进程间的通信（不同计算机上的进程间通信, 又称进程间通信, IP协议进行的主要是端到端通信）。在网络中，每一个节点（计算机或路由）都有一个网络地址，也就是IP地址。两个进程通信时，首先要确定各自所在的网络节点的网络地址。但是，网络地址只能确定进程所在的计算机，而一台计算机上很可能同时运行着多个进程，所以仅凭网络地址还不能确定到底是和网络中的哪一个进程进行通信，因此套接口中还需要包括其他的信息，也就是端口号（PORT）。在一台计算机中，一个端口号一次只能分配给一个进程，也就是说，在一台计算机中，端口号和进程之间是一一对应关系。所以，使用端口号和网络地址的组合可以唯一的确定整个网络中的一个网络进程`**
>
> * **端口号的范围从0~65535，一类是由互联网指派名字和号码公司ICANN负责分配给一些常用的应用程序固定使用的“周知的端口”，其值一般为0~1023, 用户自定义端口号一般大于等于1024**
>
> **每一个socket都用一个半相关描述{协议、本地地址、本地端口}来表示；一个完整的套接字则用一个相关描述{协议、本地地址、本地端口、远程地址、远程端口}来表示。socket也有一个类似于打开文件的函数调用，该函数返回一个整型的socket描述符，随后的连接建立、数据传输等操作都是通过socket来实现的**

# 2.通信方式

---

全双工的通信方式即在发送数据的同时也能够接收数据，两者同步进行。不会产生阻塞。此程序是在通信双方利用fork函数产生子进程，利用多进程工作方式避免了阻塞的产生

---

# 3.服务器端

---

**1.函数功能**

> listen函数

**使服务器的这个端口和IP处于监听状态，等待网络中某一客户机的连接请求。如果客户端有连接请求，端口就会接受这个连接**

> **accept函数**

**接受远程计算机的连接请求，建立起与客户机之间的通信连接。服务器处于监听状态时，如果某时刻获得客户机的连接请求，此时并不是立即处理这个请求，而是将这个请求放在等待队列中，当系统空闲时再处理客户机的连接请求。**

> bind函数

**将套接字绑定到地址, python下,以元组（host,port）的形式表示地址**

> recv函数

**接收远端主机传来的数据**

> send函数

**发送数据给指定的远端主机**

> close函数

**关闭套接字**

---

**2.代码段**

```py
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
        while True:
                pid=os.fork() #fork函数产生子进程
                if pid<=0:
                        print "fork failed!"
                        break
                elif pid==0:#进入子进程
                        child()
                else:
                        datac=raw_input("server:")
                        if not datac:
                                break
                        tcpCliSock.send('%s' %(datac)) #发送数据
                        print "send to client %s"%datac


if __name__=='__main__':
        parent()
        tcpCliSock.close() #关闭连接
```

# 4. 客户端

**1.函数功能**

---

> connect函数

**用来请求连接远程服务器**

---

**2.代码段**

```py
#!/usr/local/env python

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
                pid=os.fork()    #fork函数产生子进程
                if pid<=0:
                        print "fork failed!"
                        break
                elif pid==0:#进入子进程
                        child() 
                else:
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
```

# 5. fork函数

fork（）函数通过系统调用创建一个与原来进程几乎完全相同的进程，也就是两个进程可以做完全相同的事，但如果初始参数或者传入的变量不同，两个进程也可以做不同的事。一个进程调用fork（）函数后，系统先给新的进程分配资源，例如存储数据和代码的空间。然后把原来的进程的所有值都复制到新的新进程中，只有少数值与原来的进程的值不同。相当于克隆了一个自己。fork调用的一个奇妙之处就是它仅仅被调用一次，却能够返回两次。

