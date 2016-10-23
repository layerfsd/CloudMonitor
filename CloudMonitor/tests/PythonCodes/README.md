网络通信 -- 用户登录与注册模块说明

文件说明：
	test_serv.py 是在开发 服务端时做的单元测试
	可通过 python3 -m unittest test_serv.py 运行，检查服务端模块的正确性

运行说明：
	将 serv.py clnt.py 拷贝到 safeproject 目录下，依次启动服务端和客户端即可


网络通信流程：
1. 服务端向客户端发送 "WHO ARE YOU",要求客户端提供 认证信息
2. 客户端,检查服务端的认证字段是否为 "WHO ARE YOU", 
	如果是，则向服务端发送 ATH user_num
	否则认定服务端非法，并且终止连接
3. 服务端检查 user_num 是否已经注册，是则返回登录成功，否则返回登录失败
4. 客户端如果收到 登录成功则进行下一步，否则发送主机信息以注册
5. 当服务端 收到客户端发来的注册信息时，首先检查用户名是否合法
	如果用户名合法(用户名是否存在与企业员工信息中)，则记录用户主机信息并且将用户名添加至注册表
	如果用户名不合法，则返回注册失败

服务端接口说明：
	1. 将用户名、主机信息插入数据库
		insert_user2DB(user_num, pc_config)
		user_num = str()
		pc_config = {}
		返回值：
			如果写入数据库成功，返回 True
			写入数据库失败，返回 False

	2. 查询用户名是否合法（用户名是否为企业所属）
		is_user_recorded(user_num)
		user_num = str()
		返回值：
			如果用户名合法，返回 True
			不合法，返回 False

	3. 查询用户名是否已经注册
		is_user_registed(user_num)
		user_num = str()
		返回值：
			如果用户名已注册，返回 True
			如果用户名尚未注册,返回 False
		

数据说明 -- 用户主机注册信息格式说明：

		用户名是一个18字节以内的字符串
			user_num = '1234567890'
		客户端发送的注册信息格式为：
			data = 'MAC:2 11:22:33:44:55:66-wireless aa:bb:cc:dd:ee:ff-wired\nHDS:2 9CD29CD2-machine C5F3BFE9-ssd\n'
		服务端处理之后的注册信息格式为一个字典。
			pc_config['HDS']['num'] 获取到硬盘数量	
			pc_config['HDS'][int]   获取到指定下标的硬盘信息
			desc, addr = pc_config['HDS'][int] 通过这个表达式可以获取到硬盘的描述和序列号

			pc_config= {
				'HDS': 
						{
							0: ('machine', '9CD29CD2'), 
							1: ('ssd', 'C5F3BFE9'), 
							'num': 2
						}, 

				'MAC': 
						{
							0: ('wireless', '11:22:33:44:55:66'), 
							1: ('wired', 'aa:bb:cc:dd:ee:ff'), 
							'num': 2
						}
			}
