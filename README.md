## 必要的环境

- 安装`boost`库
- 安装`muduo`库
- 安装`Nginx`
- 安装`MySQL`
- 安装`redis`

## 构建项目

### 创建数据库

sql文件在source文件夹下

```
#shell终端执行
mysql -u root -p your passward
#mysql终端执行
create database chat;
#shell终端执行
source chat.sql
```

### 编译项目以及执行

```
cmake -S . -B build

cmake --build build
```

进入到bin文件夹下，你将看到服务端和客户端两个可执行文件

```
# 启动服务端
./ChatServer 6000 

# 启动客户端
./ChatClient 127.0.0.1 8000
```

source文件夹下的PDF文件记录学习的过程，也许有些问题能够在里面得到解答

客户端连接到端口是nginx负载均衡器的端口，而服务器是在负载均衡器的背后，因此服务器开启的端口并不是客户端去直连的端口

## 未来计划

- [ ] 数据库连接池
- [ ] Nginx性能优化