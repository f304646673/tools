gprof是一个C语言程序性能分析工具。在编译期间，我们给编译指令增加-pg选项，就可以将检测代码插入到源码中。然后使用gprof启动编译程序，它会收集程序运行的流程以及其他相关数据。最后我们使用gprof2dot将这些数据转换成dot文件，使用graphviz进行图形化展示。
![在这里插入图片描述](https://img-blog.csdnimg.cn/direct/162b0fac0f8e45fb9a90dd450789ccbf.png)

以[《静态分析C语言生成函数调用关系的利器——cflow（二）》](https://blog.csdn.net/breaksoftware/article/details/135805306)中的libevent为例。
# 准备工作
## 下载libevent代码
git clone https://github.com/libevent/libevent.git

## 安装编译依赖
```bash
sudo apt-get install libssl-dev
```
## 编译libevent
```bash
cd libevent
mkdir build && cd build
cmake ..     # Default to Unix Makefiles.
make
```
# 收集运行数据
## 编译插入检测代码的可执行程序
我们还是选用test-time.c为例子。
因为我们不希望使用静态库链接的形式，所以直接编译整个源码。
主要关注的就是-pg -c选项的新增，其他的命令我们在[《静态分析C语言生成函数调用关系的利器——cally和egypt》](https://blog.csdn.net/breaksoftware/article/details/135831473)已经见过。

```bash
gcc `find . -regextype posix-extended -regex '^./[^/]*\.c$' ! -name 'wepoll.c' ! -name 'win32select.c' ! -name 'evthread_win32.c' ! -name 'buffer_iocp.c' ! -name 'bufferevent_async.c' ! -name 'arc4random.c' ! -name 'event_iocp.c' ! -name 'bufferevent_mbedtls.c'` \
 ./test/test-time.c \
 -pg \
 -I./build/include/ -I./include -I./ \
 -L./build/lib/ -lcrypto -lssl \
 -DLITTLE_ENDIAN -D__clang__ \
 -UD_WIN32 -UDMBEDTLS_SSL_RENEGOTIATION \
 -o test-time
```
## 收集数据
执行下面指令，就能将程序运行的数据记录到test-time.output中。但是这个文件还是不能直接被图形化表达，需要进行一些处理。
```bash
gprof test-time > test-time.output
```
# 数据转换
上一步gprof采集的数据分为两部分，其中一个是调用关系（Call graph）

```yaml
		     Call graph (explanation follows)


granularity: each sample hit covers 4 byte(s) for 100.00% of 0.01 seconds

index % time    self  children    called     name
                0.00    0.01       1/1           main [3]
[1]    100.0    0.00    0.01       1         event_base_dispatch [1]
                0.01    0.00       1/1           event_base_loop [2]
-----------------------------------------------
                0.01    0.00       1/1           event_base_dispatch [1]
[2]    100.0    0.01    0.00       1         event_base_loop [2]
                0.00    0.00   13033/13033       clear_time_cache [41]
                0.00    0.00   13032/13032       timeout_next [43]
                ……
```
这个数据格式我们需要转换成dot格式，以方便graphviz转换成图片。
## 环境准备
然后使用[《管理Python虚拟环境的脚本》](https://blog.csdn.net/breaksoftware/article/details/135446309)中的脚本构建虚拟环境，并安装gprof2dot

```bash
source env.sh init
source env.sh enter
source env.sh install gprof2dot
```
## 转换为dot

```bash
gprof2dot -e0 -n0  test-time.output>test-time.dot
```

```yaml
digraph {
	graph [fontname=Arial, nodesep=0.125, ranksep=0.25];
	node [fontcolor=white, fontname=Arial, height=0, shape=box, style=filled, width=0];
	edge [fontname=Arial];
	1 [color="#ff0000", fontcolor="#ffffff", fontsize="10.00", label="event_base_dispatch\n100.00%\n(0.00%)\n1×"];
	1 -> 2 [arrowsize="1.00", color="#ff0000", fontcolor="#ff0000", fontsize="10.00", label="100.00%\n1×", labeldistance="4.00", penwidth="4.00"];
	2 [color="#ff0000", fontcolor="#ffffff", fontsize="10.00", label="event_base_loop\n100.00%\n(100.00%)\n1×"];
	2 -> 41 [arrowsize="0.35", color="#0d0d73", fontcolor="#0d0d73", fontsize="10.00", label="0.00%\n13033×", labeldistance="0.50", penwidth="0.50"];
	2 -> 42 [arrowsize="0.35", color="#0d0d73", fontcolor="#0d0d73", fontsize="10.00", label="0.00%\n13032×", labeldistance="0.50", penwidth="0.50"];
	2 -> 43 [arrowsize="0.35", color="#0d0d73", fontcolor="#0d0d73", fontsize="10.00", label="0.00%\n13032×", labeldistance="0.50", penwidth="0.50"];
	2 -> 45 [arrowsize="0.35", color="#0d0d73", fontcolor="#0d0d73", fontsize="10.00", label="0.00%\n13031×", labeldistance="0.50", penwidth="0.50"];
	2 -> 47 [arrowsize="0.35", color="#0d0d73", fontcolor="#0d0d73", fontsize="10.00", label="0.00%\n13031×", labeldistance="0.50", penwidth="0.50"];
	2 -> 49 [arrowsize="0.35", color="#0d0d73", fontcolor="#0d0d73", fontsize="10.00", label="0.00%\n13031×", labeldistance="0.50", penwidth="0.50"];
……
```
# 转换为图片
## 环境准备
```bash
sudo apt-get install python3 graphviz
```
## 转换图片
```bash
dot test-time.dot -Tpng -o test-time.png
```
![请添加图片描述](https://img-blog.csdnimg.cn/direct/9c32740de6664f47a7ecfdbf21987f17.png)
# 参考代码
[https://github.com/f304646673/tools/tree/main/gprof2dot](https://github.com/f304646673/tools/tree/main/gprof2dot)
# 参考资料

 - [https://ftp.gnu.org/old-gnu/Manuals/gprof-2.9.1/html_mono/gprof.html](https://ftp.gnu.org/old-gnu/Manuals/gprof-2.9.1/html_mono/gprof.html)
 - [https://en.wikipedia.org/wiki/Gprof](https://en.wikipedia.org/wiki/Gprof)
