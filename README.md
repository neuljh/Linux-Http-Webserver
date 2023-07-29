# Linux-Http-Webserver
# 一个简单的在linux操作系统下的HttpWeb服务器，可以实现解析HTML,JPG和CPP等代码文件，CGI程序

1.启动Web服务器

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/5dc08501-eea8-4de7-91dd-3101b649ad34)

循环监听端口9190。

2.显示目录列表

首先随便进入一个目录，这里我们选择文件webserv_linux.cpp（即本实验的c++源文件）所在的目录：

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/94ed2093-7ba2-474b-a8bf-cc86749883d8)

进入终端，查看所在目录：

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/bf539f35-8c98-459e-863d-fd42b4ed2e2f)

得到所在目录为：/home/neuljh/c/linux1/experiments

输入命令：ls -l：

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/7d5604d1-a951-406e-b4da-ed8545895bb4)

在浏览器中输入网址：
http://localhost:9190/home/neuljh/c/linux1/experiments：

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/6f4253ed-64d3-4f50-8f16-303fb0409ab6)

发现浏览器输出内容和在终端的输出内容相同。

3.解析.html文件

首先查看index.html文件源码：

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/77d85b4f-f508-40ca-a5cd-79f869d40431)

在浏览器中输入网址：http://localhost:9190/index.html：

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/da2d9550-825c-433b-944d-59b957f74d76)

Html文件解析成功。

4.解析.jpg/.png文件

首先查看图片文件bg.jpg:

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/064e3ecb-9efa-46cb-96f0-cce32295727b)

在浏览器中输入网址：http://localhost:9190/bg.jpg

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/551f3d43-d526-493e-b465-12d5e42b62ea)

解析图片文件成功。

5.显示文件内容

这里以hello.cpp文件为例子，首先查看hello.cpp文件源码：

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/61254eda-67e4-4c39-931a-450ac6ac223a)

在所在文件目录输入命令：cat hello.cpp

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/0ea8317c-7ff5-48be-a611-7bb5625ae062)

在浏览器中输入网址：http://localhost:9190/hello.cpp

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/63fb9a7f-89bd-4773-b975-5b0f15672b29)

发现浏览器输出内容和在终端的输出内容相同。

6.运行.cgi程序
(1)创建hi.cgi文件，并创建shell脚本

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/3f2da03b-adfe-4170-a478-89d7b9815583)

(2)使用chmod命令修改hi.cgi权限为755

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/8dedc3a6-772a-484f-844b-3bda948149fd)

(3)运行hi.cgi程序

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/bd6cf354-510a-4e76-98d9-0c5a80b4fc46)


7.请求无效时显示错误信息

当对应路径或者目录不存在时，此时请求无效，浏览器会显示404错误。

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/c18440ec-fb55-417b-b4c0-c7bf08451ec3)

8.日志记录

在本实验中我还加入了日志记录功能，实时记录浏览器的请求内容及时间戳。相关信息记录在同目录下的log.txt文件中。

![image](https://github.com/neuljh/Linux-Http-Webserver/assets/132900799/ce4700e1-ff81-406a-ae86-555cf0f59a27)

使用cat命令查看log.txt文件的日志内容。
