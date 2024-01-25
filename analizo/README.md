Analizo是一款可以给C、C++、Java以及C#代码进行评分的开源软件。我们可以使用它来分析代码，并得到如下指标的评分

 - Afferent Connections per Class (ACC) metric
 - Average Cyclomatic Complexity per Method (ACCM) metric
 - Average Method Lines of Code (AMLOC) metric
 - Average Number of Parameters (ANPM) metric
 - Coupling Between Objects (CBO) metric
 - Depth of Inheritance Tree (DIT) metric
 - Lack of Cohesion of Methods (LCOM4) metric
 - Lines of Code (LOC) metric
 - Number of Attributes (NOA) metric
 - Number of Children (NOC) metric
 - Number of Methods (NOM) metric
 - Number of Public Attributes (NPA) metric
 - Number of Public Methods (NPM) metric
 - Response for Class (RFC) metric
 - Structural Complexity (SC) metric

这些评分是我们评价代码质量和可维护性的一种依据。
比如Average Cyclomatic Complexity per Method (ACCM) metric，即平均圈复杂度。这是1976年由Thomas J. McCabe, Sr. 提出来的一种代码复杂度的衡量标准。它的算法也很简单，即

> V(G) = e – n + 2

其中e是边数量，n是节点数量。
![在这里插入图片描述](https://img-blog.csdnimg.cn/direct/52bf013ef7c449fe8a855ba36ae237e7.png)

比如上图显示，if else逻辑的圈复杂度是4-4+2=2;而switch case则是6-5+2=3。即上图中的switch case的复杂度要高些，这是因为它要处理的分支要多一些。更多的分支意味着逻辑复杂，也意味着程序写的复杂。
其他指标后面会展开说。我们先看看如何使用Analizo分析出这些指标。
# 环境准备
Analizo的安装依赖于perl以及其他相关组件，编译和安装的命令如下。

```bash
sudo apt update && sudo apt upgrade -y
sudo apt install perl
sudo apt install doxygen-doxyparse
sudo apt install cpanminus
sudo cpanm File::ShareDir::Install
sudo cpanm FindBin::libs
sudo cpanm App::Cmd::Setup
sudo cpanm Class::Inspector
sudo cpanm Env::Path
sudo cpanm Class::Accessor::Fast
sudo cpanm Graph
sudo cpanm Graph::Writer::Dot
sudo cpanm YAML::XS
sudo cpanm Statistics::Descriptive
sudo cpanm File::HomeDir
sudo cpanm CHI
sudo cpanm File::Copy::Recursive
git clone https://git.launchpad.net/ubuntu/+source/analizo
cd analizo
perl Makefile.PL
make
sudo make install
```
# 分析代码
我们还是分析[《静态分析C语言生成函数调用关系的利器——cflow（二）》](https://blog.csdn.net/breaksoftware/article/details/135805306)中的libevent源码。因为build文件夹是在编译过程中产生的，所以我们需要将其排除。（否则有很多中间文件）

```bash
analizo metrics ../../libevent/ --exclude ../../libevent/build > libevent_matrics.yaml
```
# 整理结果
## 环境准备
我们使用pandas解析上面产生的yaml文件，所以需要安装一些python包。env.sh是[《管理Python虚拟环境的脚本》](https://blog.csdn.net/breaksoftware/article/details/135446309)中介绍的python虚拟环境管理脚本。
```bash
source env.sh init
source env.sh enter
source env.sh install pyyaml
source env.sh install pandas
source env.sh install tabulate
```
## 将结果转换成markdown格式

```python
import pandas as pd
import yaml
import sys

def yaml2md(yaml_file, md_file):
    li = []
    summary = None
    with open(yaml_file, 'r') as f:
        for g in yaml.safe_load_all(f):
            if "_module" in g:
                li.append(g)
            else:
                summary = pd.json_normalize(yaml.safe_load_all(yaml.safe_dump_all([g])))
    df = pd.json_normalize(yaml.safe_load_all(yaml.safe_dump_all(li)))
    df.set_index("_module", inplace=True)
    column_to_move = df.pop("_filename")
    df.insert(len(df.columns), "name", column_to_move)
    with open(md_file, 'w') as f:
        f.write(df.to_markdown())
    
    
if __name__ == "__main__":
    yaml_path = sys.argv[1]
    md_path = sys.argv[2]
    yaml2md(yaml_path, md_path)
```

# 结果展示
| _module                     |   acc |     accm |    amloc |     anpm |   cbo |   dit |   lcom4 |   loc |   mmloc |   noa |   noc |   nom |   npa |   npm |   rfc |   sc | name                                                                             |
|:----------------------------|------:|---------:|---------:|---------:|------:|------:|--------:|------:|--------:|------:|------:|------:|------:|------:|------:|-----:|:---------------------------------------------------------------------------------|
| getopt                      |     5 |  8       | 40       | 2        |     0 |     0 |       1 |    80 |      66 |    12 |     0 |     2 |    12 |     2 |     8 |    0 | ['WIN32-Code/getopt.c', 'WIN32-Code/getopt.h']                                   |
| getopt_long                 |     0 |  5.33333 | 49.3333  | 3        |     1 |     0 |       1 |   148 |      75 |     5 |     0 |     3 |     5 |     3 |    18 |    1 | ['WIN32-Code/getopt_long.c']                                                     |
| tree                        |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['WIN32-Code/tree.h']                                                            |
| arc4random                  |     1 |  2.73333 | 16.4     | 0.733333 |     0 |     0 |       1 |   246 |      40 |     8 |     0 |    15 |     8 |    15 |    61 |    0 | ['arc4random.c']                                                                 |
| buffer                      |    37 |  5.16327 | 29.7551  | 2.29592  |     5 |     0 |      21 |  2916 |     133 |    11 |     0 |    98 |    11 |    98 |   605 |  105 | ['buffer.c', 'include/event2/buffer.h']                                          |
| buffer_iocp                 |     0 |  1.22222 | 26.2222  | 1.88889  |     2 |     0 |       2 |   236 |      72 |     7 |     0 |     9 |     7 |     9 |    79 |    4 | ['buffer_iocp.c']                                                                |
| bufferevent-internal        |     9 |  3       |  9       | 3        |     2 |     0 |       1 |     9 |       9 |    57 |     0 |     1 |    57 |     1 |     9 |    2 | ['bufferevent-internal.h']                                                       |
| bufferevent                 |    25 |  2.09259 | 15.6296  | 2.05556  |    10 |     0 |      28 |   844 |      78 |     0 |     0 |    54 |     0 |    54 |   296 |  280 | ['bufferevent.c', 'include/event2/bufferevent.h']                                |
| bufferevent_async           |     0 |  2.5     | 19.4286  | 2.03571  |     9 |     0 |       4 |   544 |      54 |    10 |     0 |    28 |    10 |    28 |   208 |   36 | ['bufferevent_async.c']                                                          |
| bufferevent_filter          |     2 |  2.57895 | 24       | 2.57895  |     5 |     0 |       2 |   456 |      77 |    10 |     0 |    19 |    10 |    19 |   113 |   10 | ['bufferevent_filter.c']                                                         |
| bufferevent_mbedtls         |     3 |  2       | 10       | 1.75862  |     8 |     0 |      15 |   290 |      50 |     3 |     0 |    29 |     3 |    29 |    85 |  120 | ['bufferevent_mbedtls.c']                                                        |
| bufferevent_openssl         |     4 |  2.82759 | 12.7586  | 1.86207  |     8 |     0 |      19 |   370 |      55 |     2 |     0 |    29 |     2 |    29 |    89 |  152 | ['bufferevent_openssl.c']                                                        |
| bufferevent_pair            |     1 |  3.28571 | 18.8571  | 1.85714  |     6 |     0 |       2 |   264 |      40 |     4 |     0 |    14 |     4 |    14 |    82 |   12 | ['bufferevent_pair.c']                                                           |
| bufferevent_ratelim         |     3 |  2.81818 | 20.3864  | 1.70455  |     7 |     0 |      17 |   897 |      76 |     0 |     0 |    44 |     0 |    44 |   319 |  119 | ['bufferevent_ratelim.c']                                                        |
| bufferevent_sock            |    15 |  5.57143 | 26.619   | 2.71429  |     8 |     0 |       8 |   559 |     112 |     1 |     0 |    21 |     1 |    21 |   177 |   64 | ['bufferevent_sock.c']                                                           |
| bufferevent_ssl             |     3 |  5.06977 | 22.6744  | 1.83721  |     9 |     0 |       3 |   975 |      86 |     1 |     0 |    43 |     1 |    43 |   400 |   27 | ['bufferevent_ssl.c', 'include/event2/bufferevent_ssl.h']                        |
| changelist-internal         |     1 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     5 |     0 |     0 |     5 |     0 |     0 |    0 | ['changelist-internal.h']                                                        |
| CheckFileOffsetBits         |     1 |  1       |  5       | 0        |     0 |     0 |       1 |     5 |       5 |     1 |     0 |     1 |     1 |     1 |     1 |    0 | ['cmake/CheckFileOffsetBits.c']                                                  |
| queue                       |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['compat/sys/queue.h']                                                           |
| defer-internal              |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['defer-internal.h']                                                             |
| epoll_sub                   |     0 |  1.33333 |  8.33333 | 3        |     1 |     0 |       3 |    25 |      12 |     0 |     0 |     3 |     0 |     3 |     4 |    3 | ['epoll_sub.c']                                                                  |
| epolltable-internal         |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     3 |     0 |     0 |     3 |     0 |     0 |    0 | ['epolltable-internal.h']                                                        |
| evbuffer-internal           |     6 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    49 |     0 |     0 |    49 |     0 |     0 |    0 | ['evbuffer-internal.h']                                                          |
| evdns                       |     6 |  5.26087 | 27.236   | 2.32919  |    13 |     0 |       2 |  4385 |     202 |   158 |     0 |   161 |   158 |   161 |  1177 |   26 | ['evdns.c']                                                                      |
| event-internal              |    13 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    82 |     0 |     0 |    82 |     0 |     0 |    0 | ['event-internal.h']                                                             |
| event                       |    58 |  3.22485 | 19.4201  | 1.75148  |    12 |     0 |      31 |  3282 |     170 |    11 |     0 |   169 |    11 |   169 |   989 |  372 | ['event.c']                                                                      |
| event_iocp                  |     3 |  1.76923 | 15.3846  | 1.76923  |     3 |     0 |       6 |   200 |      49 |     2 |     0 |    13 |     2 |    13 |    27 |   18 | ['event_iocp.c']                                                                 |
| event_tagging               |     4 |  2.62069 | 12       | 2.48276  |     2 |     0 |       3 |   348 |      46 |     0 |     0 |    29 |     0 |    29 |    92 |    6 | ['event_tagging.c']                                                              |
| evmap-internal              |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['evmap-internal.h']                                                             |
| evmap                       |     0 |  1.84615 | 18.0769  | 2.5641   |     7 |     0 |       6 |   705 |      77 |     8 |     0 |    39 |     8 |    39 |   228 |   42 | ['evmap.c']                                                                      |
| evrpc-internal              |     2 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    37 |     0 |     0 |    37 |     0 |     0 |    0 | ['evrpc-internal.h']                                                             |
| evrpc                       |     1 |  3.26087 | 20.6304  | 2.63043  |    10 |     0 |      14 |   949 |      57 |     0 |     0 |    46 |     0 |    46 |   297 |  140 | ['evrpc.c']                                                                      |
| evsignal-internal           |     4 |  1       |  1       | 1        |     0 |     0 |       1 |     1 |       1 |     6 |     0 |     1 |     6 |     1 |     1 |    0 | ['evsignal-internal.h']                                                          |
| evthread-internal           |     0 |  2       | 11       | 1        |     1 |     0 |       1 |    11 |      11 |     4 |     0 |     1 |     4 |     1 |     3 |    1 | ['evthread-internal.h']                                                          |
| evthread                    |     5 |  3.22222 | 17       | 1.22222  |     3 |     0 |       1 |   306 |      54 |    13 |     0 |    18 |    13 |    18 |   103 |    3 | ['evthread.c']                                                                   |
| evthread_pthread            |     1 |  3.09091 | 14.6364  | 1.36364  |     2 |     0 |       1 |   161 |      56 |     4 |     0 |    11 |     4 |    11 |    31 |    2 | ['evthread_pthread.c']                                                           |
| evthread_win32              |     5 |  2.9     | 18       | 1.4      |     2 |     0 |       1 |   180 |      65 |     5 |     0 |    10 |     5 |    10 |    40 |    2 | ['evthread_win32.c']                                                             |
| evutil                      |    38 |  4.01389 | 29.4861  | 1.86111  |     8 |     0 |      27 |  2123 |     189 |    17 |     0 |    72 |    17 |    72 |   214 |  216 | ['evutil.c']                                                                     |
| evutil_rand                 |     6 |  1.125   |  6.5     | 1        |     1 |     0 |       5 |    52 |      10 |     1 |     0 |     8 |     1 |     8 |    14 |    5 | ['evutil_rand.c']                                                                |
| evutil_time                 |    22 |  2.45455 | 17.7273  | 1.63636  |     3 |     0 |       7 |   195 |      50 |     0 |     0 |    11 |     0 |    11 |    20 |   21 | ['evutil_time.c']                                                                |
| ht-internal                 |     0 |  1.5     | 11       | 1        |     0 |     0 |       2 |    22 |      12 |     0 |     0 |     2 |     0 |     2 |     2 |    0 | ['ht-internal.h']                                                                |
| http-internal               |     4 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    57 |     0 |     0 |    57 |     0 |     0 |    0 | ['http-internal.h']                                                              |
| http                        |    10 |  3.77523 | 21.3853  | 2.00917  |    17 |     0 |      55 |  4662 |     264 |    23 |     0 |   218 |    23 |   218 |  1257 |  935 | ['http.c', 'include/event2/http.h']                                              |
| buffer_compat               |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/buffer_compat.h']                                               |
| bufferevent_compat          |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/bufferevent_compat.h']                                          |
| bufferevent_struct          |    13 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    15 |     0 |     0 |    15 |     0 |     0 |    0 | ['include/event2/bufferevent_struct.h']                                          |
| dns                         |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/dns.h']                                                         |
| dns_compat                  |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/dns_compat.h']                                                  |
| dns_struct                  |     4 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     6 |     0 |     0 |     6 |     0 |     0 |    0 | ['include/event2/dns_struct.h']                                                  |
| event2/event                |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/event.h']                                                       |
| event_compat                |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/event_compat.h']                                                |
| event_struct                |     6 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    32 |     0 |     0 |    32 |     0 |     0 |    0 | ['include/event2/event_struct.h']                                                |
| http_compat                 |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/http_compat.h']                                                 |
| http_struct                 |     5 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    34 |     0 |     0 |    34 |     0 |     0 |    0 | ['include/event2/http_struct.h']                                                 |
| keyvalq_struct              |     2 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     2 |     0 |     0 |     2 |     0 |     0 |    0 | ['include/event2/keyvalq_struct.h']                                              |
| listener                    |    13 |  4.25    | 18.875   | 2        |     5 |     0 |       1 |   302 |      77 |    18 |     0 |    16 |    18 |    16 |    96 |    5 | ['include/event2/listener.h', 'listener.c']                                      |
| rpc                         |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/rpc.h']                                                         |
| rpc_compat                  |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/rpc_compat.h']                                                  |
| rpc_struct                  |     2 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    15 |     0 |     0 |    15 |     0 |     0 |    0 | ['include/event2/rpc_struct.h']                                                  |
| tag                         |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/tag.h']                                                         |
| tag_compat                  |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/tag_compat.h']                                                  |
| thread                      |     2 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    11 |     0 |     0 |    11 |     0 |     0 |    0 | ['include/event2/thread.h']                                                      |
| util                        |     9 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     8 |     0 |     0 |     8 |     0 |     0 |    0 | ['include/event2/util.h']                                                        |
| visibility                  |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['include/event2/visibility.h']                                                  |
| watch                       |     2 |  1.33333 |  7.16667 | 2.33333  |     3 |     0 |       4 |    43 |      14 |     0 |     0 |     6 |     0 |     6 |    24 |   12 | ['include/event2/watch.h', 'watch.c']                                            |
| ws                          |     3 |  3.35294 | 22.2353  | 2.58824  |     7 |     0 |       4 |   378 |      88 |     2 |     0 |    17 |     2 |    17 |    72 |   28 | ['include/event2/ws.h', 'ws.c']                                                  |
| iocp-internal               |     1 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     1 |     0 |     0 |     1 |     0 |     0 |    0 | ['iocp-internal.h']                                                              |
| ipv6-internal               |     8 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     4 |     0 |     0 |     4 |     0 |     0 |    0 | ['ipv6-internal.h']                                                              |
| kqueue-internal             |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['kqueue-internal.h']                                                            |
| log-internal                |    23 |  1       |  1       | 2.77778  |     0 |     0 |       9 |     9 |       1 |     2 |     0 |     9 |     2 |     9 |     9 |    0 | ['log-internal.h']                                                               |
| log                         |     5 |  1.78571 | 10.0714  | 2.21429  |     1 |     0 |       2 |   141 |      26 |     3 |     0 |    14 |     3 |    14 |    33 |    2 | ['log.c']                                                                        |
| minheap-internal            |     2 |  1.93333 |  7.6     | 1.66667  |     1 |     0 |       3 |   114 |      20 |     3 |     0 |    15 |     3 |    15 |    56 |    3 | ['minheap-internal.h']                                                           |
| mm-internal                 |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['mm-internal.h']                                                                |
| openssl-compat              |     0 |  2       | 10       | 2        |     0 |     0 |       1 |    10 |      10 |     0 |     0 |     1 |     0 |     1 |     1 |    0 | ['openssl-compat.h']                                                             |
| ratelim-internal            |     1 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     9 |     0 |     0 |     9 |     0 |     0 |    0 | ['ratelim-internal.h']                                                           |
| becat                       |     0 |  4.68    | 22.28    | 2.08     |    13 |     0 |       2 |   557 |     157 |    35 |     0 |    25 |    35 |    25 |   176 |   26 | ['sample/becat.c']                                                               |
| dns-example                 |     0 |  7       | 35.1667  | 2.66667  |     7 |     0 |       1 |   211 |     111 |     1 |     0 |     6 |     1 |     6 |    49 |    7 | ['sample/dns-example.c']                                                         |
| event-read-fifo             |     0 |  4       | 40.3333  | 2.66667  |     3 |     0 |       1 |   121 |      77 |     0 |     0 |     3 |     0 |     3 |    19 |    3 | ['sample/event-read-fifo.c']                                                     |
| hello-world                 |     0 |  2.4     | 18.8     | 3        |     6 |     0 |       1 |    94 |      47 |     2 |     0 |     5 |     2 |     5 |    32 |    6 | ['sample/hello-world.c']                                                         |
| hostcheck                   |     1 |  8.8     | 28.2     | 2        |     0 |     0 |       1 |   141 |      58 |     0 |     0 |     5 |     0 |     5 |    11 |    0 | ['sample/hostcheck.c', 'sample/hostcheck.h']                                     |
| http-connect                |     0 |  1.16667 | 17.1667  | 1.83333  |     6 |     0 |       1 |   103 |      43 |     2 |     0 |     6 |     2 |     6 |    51 |    6 | ['sample/http-connect.c']                                                        |
| http-server                 |     1 | 10.875   | 57.25    | 2        |    10 |     0 |       2 |   458 |     165 |    23 |     0 |     8 |    23 |     8 |    79 |   20 | ['sample/http-server.c']                                                         |
| https-client                |     0 | 15.8333  | 89.6667  | 1.33333  |    10 |     0 |       1 |   538 |     409 |     3 |     0 |     6 |     3 |     6 |    57 |   10 | ['sample/https-client.c']                                                        |
| le-proxy                    |     0 |  5       | 36.1429  | 2.28571  |     7 |     0 |       1 |   253 |     102 |     6 |     0 |     7 |     6 |     7 |    61 |    7 | ['sample/le-proxy.c']                                                            |
| openssl_hostname_validation |     1 |  5       | 30.6667  | 2        |     1 |     0 |       1 |    92 |      39 |     0 |     0 |     3 |     0 |     3 |     7 |    1 | ['sample/openssl_hostname_validation.c', 'sample/openssl_hostname_validation.h'] |
| signal-test                 |     2 |  4.5     | 24.5     | 2.5      |     2 |     0 |       1 |    49 |      38 |     1 |     0 |     2 |     1 |     2 |    12 |    2 | ['sample/signal-test.c']                                                         |
| ssl-client-mbedtls          |     0 |  4       | 41.2     | 2.4      |     7 |     0 |       1 |   206 |     159 |     0 |     0 |     5 |     0 |     5 |    27 |    7 | ['sample/ssl-client-mbedtls.c']                                                  |
| time-test                   |     0 |  1.66667 | 26       | 2.66667  |     5 |     0 |       1 |    78 |      48 |     2 |     0 |     3 |     2 |     3 |    25 |    5 | ['sample/time-test.c']                                                           |
| watch-timing                |     0 |  3.3     | 24.1     | 2.2      |     3 |     0 |       1 |   241 |      84 |    15 |     0 |    10 |    15 |    10 |    72 |    3 | ['sample/watch-timing.c']                                                        |
| ws-chat-server              |     0 |  2.1     | 17.5     | 2.3      |     8 |     0 |       1 |   175 |      32 |     2 |     0 |    10 |     2 |    10 |    52 |    8 | ['sample/ws-chat-server.c']                                                      |
| sha1                        |     1 |  2.8     | 40.4     | 2.2      |     0 |     0 |       1 |   202 |     117 |     3 |     0 |     5 |     3 |     5 |    10 |    0 | ['sha1.c', 'sha1.h']                                                             |
| signal                      |     0 |  3       | 27.1538  | 1.92308  |     6 |     0 |       3 |   353 |      50 |     5 |     0 |    13 |     5 |    13 |    93 |   18 | ['signal.c']                                                                     |
| signalfd                    |     0 |  4.4     | 30.8     | 3.2      |     6 |     0 |       2 |   154 |      86 |     1 |     0 |     5 |     1 |     5 |    33 |   12 | ['signalfd.c']                                                                   |
| ssl-compat                  |     3 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    38 |     0 |     0 |    38 |     0 |     0 |    0 | ['ssl-compat.h']                                                                 |
| strlcpy-internal            |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['strlcpy-internal.h']                                                           |
| strlcpy                     |     0 |  7       | 24       | 3        |     0 |     0 |       1 |    24 |      24 |     0 |     0 |     1 |     0 |     1 |     1 |    0 | ['strlcpy.c']                                                                    |
| test-export                 |     0 |  1.5     | 12.5     | 1        |     2 |     0 |       1 |    25 |      15 |     0 |     0 |     2 |     0 |     2 |     6 |    2 | ['test-export/test-export.c']                                                    |
| bench                       |     0 |  9.66667 | 51.6667  | 1.66667  |     4 |     0 |       1 |   155 |      93 |    10 |     0 |     3 |    10 |     3 |    42 |    4 | ['test/bench.c']                                                                 |
| bench_cascade               |     0 |  5.33333 | 39.3333  | 2        |     4 |     0 |       1 |   118 |      57 |     3 |     0 |     3 |     3 |     3 |    19 |    4 | ['test/bench_cascade.c']                                                         |
| bench_http                  |     0 |  8.5     | 63.5     | 2        |     5 |     0 |       1 |   127 |     116 |     2 |     0 |     2 |     2 |     2 |    22 |    5 | ['test/bench_http.c']                                                            |
| bench_httpclient            |    52 |  3.4     | 31.8     | 1.6      |     6 |     0 |       1 |   159 |      63 |    12 |     0 |     5 |    12 |     5 |    49 |    6 | ['test/bench_httpclient.c']                                                      |
| print-winsock-errors        |     0 |  4       | 74       | 2        |     3 |     0 |       1 |    74 |      74 |     0 |     0 |     1 |     0 |     1 |     4 |    3 | ['test/print-winsock-errors.c']                                                  |
| regress                     |    20 |  2.77419 | 24.2419  | 1.56452  |    11 |     0 |      13 |  3006 |     142 |    76 |     0 |   124 |    76 |   124 |   825 |  143 | ['test/regress.c', 'test/regress.h']                                             |
| regress.gen                 |     0 |  1.06557 | 16.4918  | 1.88525  |     2 |     0 |      61 |  1006 |      85 |    71 |     0 |    61 |    71 |    61 |   346 |  122 | ['test/regress.gen.c', 'test/regress.gen.h']                                     |
| regress_buffer              |     0 |  3.30909 | 48.6182  | 1.45455  |     9 |     0 |      38 |  2674 |     240 |    15 |     0 |    55 |    15 |    55 |   485 |  342 | ['test/regress_buffer.c']                                                        |
| regress_bufferevent         |     0 |  2.07407 | 19.0926  | 1.7037   |    15 |     0 |      12 |  1031 |      99 |    16 |     0 |    54 |    16 |    54 |   317 |  180 | ['test/regress_bufferevent.c']                                                   |
| regress_dns                 |     0 |  4.39286 | 42.8929  | 1.75     |    15 |     0 |      12 |  2402 |     389 |    40 |     0 |    56 |    40 |    56 |   601 |  180 | ['test/regress_dns.c']                                                           |
| regress_et                  |     0 |  2.28571 | 26.4286  | 1.85714  |     6 |     0 |       4 |   185 |      56 |     6 |     0 |     7 |     6 |     7 |    43 |   24 | ['test/regress_et.c']                                                            |
| regress_finalize            |     0 |  1.35714 | 19.0714  | 1.71429  |     5 |     0 |       4 |   267 |      88 |     8 |     0 |    14 |     8 |    14 |    78 |   20 | ['test/regress_finalize.c']                                                      |
| regress_http                |     1 |  3.02186 | 28.4536  | 1.87978  |    21 |     0 |       7 |  5207 |     436 |    38 |     0 |   183 |    38 |   183 |  1511 |  147 | ['test/regress_http.c', 'test/regress_http.h']                                   |
| regress_iocp                |     0 |  1.75    | 20.4167  | 1.83333  |     4 |     0 |       3 |   245 |      57 |    12 |     0 |    12 |    12 |    12 |    76 |   12 | ['test/regress_iocp.c']                                                          |
| regress_listener            |     0 |  1.92308 | 23.6923  | 2.38462  |     6 |     0 |       5 |   308 |      75 |     2 |     0 |    13 |     2 |    13 |    68 |   30 | ['test/regress_listener.c']                                                      |
| regress_main                |     8 |  4       | 23.8333  | 1.58333  |    11 |     0 |       7 |   286 |      88 |    11 |     0 |    12 |    11 |    12 |    56 |   77 | ['test/regress_main.c']                                                          |
| regress_mbedtls             |     1 |  2.25    | 19.75    | 1.91667  |     3 |     0 |       2 |   237 |      67 |     6 |     0 |    12 |     6 |    12 |    41 |    6 | ['test/regress_mbedtls.c']                                                       |
| regress_minheap             |     0 |  1.33333 | 18.3333  | 1        |     3 |     0 |       3 |    55 |      40 |     1 |     0 |     3 |     1 |     3 |    10 |    9 | ['test/regress_minheap.c']                                                       |
| regress_openssl             |     2 |  2.4     | 16.8667  | 1.4      |     2 |     0 |       2 |   253 |      40 |     5 |     0 |    15 |     5 |    15 |    41 |    4 | ['test/regress_openssl.c']                                                       |
| regress_rpc                 |     0 |  4.41667 | 32.5     | 1.83333  |    13 |     0 |       3 |   780 |     147 |     6 |     0 |    24 |     6 |    24 |   212 |   39 | ['test/regress_rpc.c']                                                           |
| regress_ssl                 |     2 |  3.66667 | 38.1333  | 2.86667  |    11 |     0 |       2 |   572 |     118 |    25 |     0 |    15 |    25 |    15 |   169 |   22 | ['test/regress_ssl.c']                                                           |
| regress_testutils           |     4 |  1.11111 | 26.7778  | 2.77778  |     6 |     0 |       2 |   241 |      71 |     9 |     0 |     9 |     9 |     9 |    58 |   12 | ['test/regress_testutils.c', 'test/regress_testutils.h']                         |
| regress_thread              |     0 |  2.82353 | 24.8824  | 1.88235  |     7 |     0 |       2 |   423 |      98 |    21 |     0 |    17 |    21 |    17 |   121 |   14 | ['test/regress_thread.c', 'test/regress_thread.h']                               |
| regress_timer_timeout       |     0 |  1.875   | 11.875   | 1.625    |     3 |     0 |       2 |    95 |      23 |     4 |     0 |     8 |     4 |     8 |    26 |    6 | ['test/regress_timer_timeout.c']                                                 |
| regress_util                |     1 |  3.07143 | 33.4762  | 1.19048  |    10 |     0 |      31 |  1406 |     144 |    32 |     0 |    42 |    32 |    42 |   180 |  310 | ['test/regress_util.c']                                                          |
| regress_watch               |     0 |  1.1     | 15       | 2.2      |     5 |     0 |       2 |   150 |      30 |     9 |     0 |    10 |     9 |    10 |    67 |   10 | ['test/regress_watch.c']                                                         |
| regress_ws                  |     1 |  4.4     | 29.2     | 2.6      |     7 |     0 |       2 |   292 |      59 |     1 |     0 |    10 |     1 |    10 |    63 |   14 | ['test/regress_ws.c', 'test/regress_ws.h']                                       |
| regress_zlib                |     0 |  1.66667 | 25.5556  | 2.33333  |     7 |     0 |       2 |   230 |      72 |     5 |     0 |     9 |     5 |     9 |    46 |   14 | ['test/regress_zlib.c']                                                          |
| test-changelist             |     0 |  2       | 25.8     | 2.6      |     7 |     0 |       1 |   129 |      62 |     2 |     0 |     5 |     2 |     5 |    29 |    7 | ['test/test-changelist.c']                                                       |
| test-closed                 |     0 |  3.5     | 27       | 2.5      |     5 |     0 |       1 |    54 |      39 |     1 |     0 |     2 |     1 |     2 |    18 |    5 | ['test/test-closed.c']                                                           |
| test-dumpevents             |     0 |  1.75    | 31.75    | 2.25     |     4 |     0 |       1 |   127 |     112 |     0 |     0 |     4 |     0 |     4 |    19 |    4 | ['test/test-dumpevents.c']                                                       |
| test-eof                    |    13 |  4       | 29       | 2.5      |     4 |     0 |       1 |    58 |      35 |     3 |     0 |     2 |     3 |     2 |    16 |    4 | ['test/test-eof.c']                                                              |
| test-fdleak                 |     0 |  2.66667 | 16.3333  | 2.11111  |     6 |     0 |       1 |   147 |      43 |     2 |     0 |     9 |     2 |     9 |    56 |    6 | ['test/test-fdleak.c']                                                           |
| test-init                   |     0 |  1       | 16       | 2        |     1 |     0 |       1 |    16 |      16 |     0 |     0 |     1 |     0 |     1 |     2 |    1 | ['test/test-init.c']                                                             |
| test-ratelim                |     2 |  4.75    | 31.875   | 2.3125   |    12 |     0 |       1 |   510 |     236 |    46 |     0 |    16 |    46 |    16 |   180 |   12 | ['test/test-ratelim.c']                                                          |
| test-time                   |    15 |  3       | 21.3333  | 2        |     3 |     0 |       1 |    64 |      41 |     3 |     0 |     3 |     3 |     3 |    19 |    3 | ['test/test-time.c']                                                             |
| test-weof                   |     0 |  3.5     | 26       | 2.5      |     3 |     0 |       1 |    52 |      33 |     3 |     0 |     2 |     3 |     2 |    16 |    3 | ['test/test-weof.c']                                                             |
| tinytest                    |     5 |  5.26667 | 30.8     | 1.46667  |     2 |     0 |       3 |   462 |     117 |    26 |     0 |    15 |    26 |    15 |    71 |    6 | ['test/tinytest.c', 'test/tinytest.h']                                           |
| tinytest_demo               |     1 |  1.42857 | 22       | 1.28571  |     2 |     0 |       7 |   154 |      40 |     8 |     0 |     7 |     8 |     7 |    14 |   14 | ['test/tinytest_demo.c']                                                         |
| tinytest_local              |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['test/tinytest_local.h']                                                        |
| tinytest_macros             |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     0 |     0 |     0 |     0 |     0 |     0 |    0 | ['test/tinytest_macros.h']                                                       |
| time-internal               |     0 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |     2 |     0 |     0 |     2 |     0 |     0 |    0 | ['time-internal.h']                                                              |
| util-internal               |     3 |  0       |  0       | 0        |     0 |     0 |       0 |     0 |       0 |    10 |     0 |     0 |    10 |     0 |     0 |    0 | ['util-internal.h']                                                              |
| wepoll                      |     0 |  2.47115 | 12.625   | 1.53846  |     2 |     0 |       3 |  1313 |      77 |    76 |     0 |   104 |    76 |   104 |   439 |    6 | ['wepoll.c', 'wepoll.h']                                                         |

# 指标说明
## Afferent Connections per Class (ACC) metric
类连接性指标。它表示类之间的关联程度。**如果关联度越高，值越大，程序越复杂。**
举个例子，如果Ci类访问了Cj类的一个方法或者属性，则 client(Ci, Cj)值为1；否则值为0。代码中所有类（N个）这样的关系值总和即ACC的值。
数据公式是：

> client(Ci, Cj) = 1, if (Ci => Cj) and (Ci != Cj)
   client(Ci, Cj) = 0, otherwise.
   ACC(Cj) = (sum(client(Ci, Cj)), i = 1 to N)

它是基于这篇论文[《Monitoramento de metricas de codigo-fonte em projetos de software livre》](https://www.teses.usp.br/teses/disponiveis/45/45134/tde-27082013-090242)。
其他参考见

 - [https://manpages.ubuntu.com/manpages//noble/man3/Analizo::Metric::AfferentConnections.3pm.html](https://manpages.ubuntu.com/manpages//noble/man3/Analizo::Metric::AfferentConnections.3pm.html)
 - [https://metacpan.org/pod/Analizo::Metric::AfferentConnections](https://metacpan.org/pod/Analizo::Metric::AfferentConnections)

## Average Cyclomatic Complexity per Method (ACCM) metric
每个函数的圈复杂度指标。**如果值越大，代码越复杂。**
它是计算代码中节点（区块）和边（跳转）之间关系的方法，即
> V(G) = e – n + 2

其中e表示边的数量，n表示节点的数量。
比如下面的代码在逻辑上是相同的，但是写法不同
```c
void foo(void) {
    if (a && b)
        x=1;
    else
        x=2;
}
```

```c
void foo(void) {
    if (a)
        if (b) 
            x=1;
    else
        x=2;
 }
```
它们的流程如下
![在这里插入图片描述](https://img-blog.csdnimg.cn/direct/c0c0554401f84b518c77d512ae1b82d8.png)
第一种写法的圈复杂度是：6（边）-6（节点）+2=2；第二种写法的圈复杂度是：7（边）-6（节点）+2=3。即第二种写法比第一种写法复杂度要高一些。我们从代码层面也能看出来如此。

它是基于论文[《A Complexity Measure》](https://ieeexplore.ieee.org/document/1702388)。
其他参考资料

 - [https://www.geeksforgeeks.org/cyclomatic-complexity/](https://www.geeksforgeeks.org/cyclomatic-complexity/)
 - [https://www.perforce.com/blog/qac/what-cyclomatic-complexity](https://www.perforce.com/blog/qac/what-cyclomatic-complexity)
 - [https://en.wikipedia.org/wiki/Cyclomatic_complexity](https://en.wikipedia.org/wiki/Cyclomatic_complexity)
## Average Method Lines of Code (AMLOC) metric
方法的平均行数指标。如果这个**值很大就不好**，说明大量的逻辑集中在少数方法中，意味着方法逻辑可能比较臃肿，进而意味着复杂度很高。当然也不是越少越好，比如一行逻辑写一个方法。该指标**鼓励大家写出更多短小、易于理解的方法，而不是内部极度臃肿的少数方法。**

它是基于Paulo Roberto Miranda Meirelles的《Monitoring of source code metrics in open source projects》。
## Average Number of Parameters (ANPM) metric
方法的平均参数个数指标。这个值最小是0，最大未知。但是如果值很大，意味着方法关联的参数很多，可能是因为这个方法内部逻辑没有拆解好，导致其包含太多可以独立出去的逻辑，进而需要更多参数来支持。所以这个值越小越好，**越大意味着越复杂**。
它是基于Paulo Roberto Miranda Meirelles的《Monitoring of source code metrics in open source projects》。
## Coupling Between Objects (CBO) metric
对象之间的耦合 (CBO) 是耦合到特定类的类数量的计数，即一个类的方法调用另一个类的方法或访问另一个类的变量的情况。这些调用需要在两个方向上计数，因此类 A 的 CBO 是类 A 引用的类集合和引用类 A 的类集合的大小。由于这是一个集合 - 每个类仅计数一次，即使引用在两个方向上运行，即如果 A 引用 B 并且 B 引用 A，则 B 只计算一次。**越大意味着耦合度越高。**
![在这里插入图片描述](https://img-blog.csdnimg.cn/direct/da6def67f8024fc48accda2dd9466306.png)
比如上图中C的CBO就高，它关联的类也是最多的。

它是基于Shyam R. Chidamber和Chris F. Kemerer的论文[《A metrics suite for object oriented design》](https://ieeexplore.ieee.org/document/295895)。
其他参考资料

 - [https://sites.pitt.edu/~ckemerer/CK%20research%20papers/MetricForOOD_ChidamberKemerer94.pdf](https://sites.pitt.edu/~ckemerer/CK%20research%20papers/MetricForOOD_ChidamberKemerer94.pdf)

## Depth of Inheritance Tree (DIT) metric
最长继承树深度指标。它表示模块到最底层类的继承深度。**越大代表越复杂。**
数学公式是

> DIT(m) = DIT(s(m)) + 1, ifc m != rootClass
DIT(m) = 0, otherwise.

![在这里插入图片描述](https://img-blog.csdnimg.cn/direct/11c148469ede47fbb19a35397b226000.png)
比如上图中D的深度就是3，F的深度就是1。D就比F复杂。

它是基于Eduardo Kessler Piveta, Ana Moreira, Marcelo Soares Pimenta, Joao Araujo, Pedro Guerreiro和R. Tom Price的[《An empirical study of aspect-oriented metrics》](https://core.ac.uk/download/pdf/82730414.pdf)。
其他参考资料

 - [https://learn.microsoft.com/en-us/visualstudio/code-quality/code-metrics-depth-of-inheritance?view=vs-2022](https://learn.microsoft.com/en-us/visualstudio/code-quality/code-metrics-depth-of-inheritance?view=vs-2022)

## Lack of Cohesion of Methods (LCOM4) metric
方法缺乏内聚性指标。在软件工程中，我们希望代码是高内聚低耦合。耦合指标我们可以通过上面的CBO来看，而内聚性则可以看LCOM4指标。**该值越高，表示越缺乏内聚性，表示越不好。**

该指标基于Martin Hitz和Behzad Montazeri的 [《Measuring coupling and cohesion in object-oriented systems》](https://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.409.4862)。
更多资料参考

 - [http://ieeexplore.ieee.org/abstract/document/6178856/](http://ieeexplore.ieee.org/abstract/document/6178856/)
 - [http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.686.2543](http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.686.2543)
 - [https://objectscriptquality.com/docs/metrics/lack-cohesion-methods-lcom4](https://objectscriptquality.com/docs/metrics/lack-cohesion-methods-lcom4)
 - [https://blog.ndepend.com/lack-of-cohesion-methods/](https://blog.ndepend.com/lack-of-cohesion-methods/)
 - [https://www.arisa.se/compendium/node116.html](https://www.arisa.se/compendium/node116.html)

## Lines of Code (LOC) metric
不包括空行和注释的代码行数。可以很好理解：**一般情况下，代码越长越复杂。**
它是基于Taghi M. Khoshgoftaar和John C. Munson的[《The lines of code metric as a predictor of program faults: a critical analysis》](https://ieeexplore.ieee.org/document/139396)。
## Number of Attributes (NOA) metric
类的属性数量指标。如果一个类的属性数量很多，预示着它可能拥有太多的功能，即功能冗余。应该考虑将其拆解成更多高内聚、功能简单的类。**一般情况下，该值越大意味着越复杂。**
它是基于Paulo Roberto Miranda Meirelles的《Monitoring of source code metrics in open source projects》。
## Number of Children (NOC) metric
从属于某个类的直接子类的数量指标。它是一个类对设计和系统的潜在影响的指标。子级的数量越多，父级不正确抽象的可能性就越大，并且可能是滥用子类的情况。但是子级的数量越多，可重用性就越大，因为继承是重用的一种形式。如果一个类有大量子类，则可能需要对该类的方法进行更多测试，从而增加测试时间。所以这个值没有很强的预示性。

它是基于Linda H. Rosenberg和Lawrence E. Hyatt的《Software Quality Metrics for Object-Oriented Environments》。
更多参考资料

 - [https://www.itwissen.info/en/number-of-children-NOC-122384.html#gsc.tab=0](https://www.itwissen.info/en/number-of-children-NOC-122384.html#gsc.tab=0)

## Number of Methods (NOM) metric
类的方法数指标。如果一个类的方法很多，可能预示着它有太多的功能，缺乏内聚性。最好将其拆解成功能独立的小型类。**该指标越大表示越复杂。**
它是基于Paulo Roberto Miranda Meirelles的《Monitoring of source code metrics in open source projects》。
## Number of Public Attributes (NPA) metric
类的公有属性数指标。类的属性应该通过方法来访问，而不是直接暴露给外面使用。所以**该值应该为0，否则就可能不是好的设计。**
它是基于Paulo Roberto Miranda Meirelles的《Monitoring of source code metrics in open source projects》。
## Number of Public Methods (NPM) metric
类的公有方法数指标。如果类的公有方法比较多，意味着它对外提供的服务很多。这个不符合高内聚的设计思想。最好将这样的类拆解成功能独立的类。所以**这个值越小越好。**
它是基于Paulo Roberto Miranda Meirelles的《Monitoring of source code metrics in open source projects》。
## Response for Class (RFC) metric
类的响应指标。它是指一个类的方法数，加上在它内部调用的其他类的方法数。如果这个数越大，说明这个类的耦合性越大，或者功能太多。这个就是需要拆解的类。所以**该值越大，表示越复杂。**

```cpp
class BirdTeam {
public:
	void Run() {
			Bird * bird = new Bird();
			bird->run();
		}
		
	void Fly() {
		Bird * bird = new Bird();
		bird->FlyUp();
		bird->FlyDown();
	}
```
比如上面BirdTeam这个类，它有2个方法，其中调用了Bird的类3的方法。这样它的RFC就是2+3=5。

它是基于Shyam R. Chidamber和Chris F. Kemerer的[《A metrics suite for object oriented design》](https://ieeexplore.ieee.org/document/295895)
其他参考资料

 - [https://www.cnblogs.com/cuihongyu3503319/archive/2006/12/29/606720.html](https://www.cnblogs.com/cuihongyu3503319/archive/2006/12/29/606720.html)

## Structural Complexity (SC) metric
结构复杂度。CBO（耦合度指标）和LCOM4（缺乏内聚性）的乘积与该指标正相关。所以**它越大表示越不好**，因为可能内聚比较差，也可能耦合性太高，也可能两者都很差。
它是基于Paulo Roberto Miranda Meirelles的《Monitoring of source code metrics in open source projects》。

# 代码
[https://github.com/f304646673/tools/tree/main/analizo](https://github.com/f304646673/tools/tree/main/analizo)
# 参考资料
 - [https://www.analizo.org/](https://www.analizo.org/)
 - [https://metacpan.org/dist/Analizo](https://metacpan.org/dist/Analizo)
 - [https://webhostinggeeks.com/howto/how-to-install-perl-on-ubuntu/](https://webhostinggeeks.com/howto/how-to-install-perl-on-ubuntu/)
 - [https://baike.baidu.com/item/%E5%9C%88%E5%A4%8D%E6%9D%82%E5%BA%A6/828737](https://baike.baidu.com/item/%E5%9C%88%E5%A4%8D%E6%9D%82%E5%BA%A6/828737)
 - [https://zhuanlan.zhihu.com/p/139386961](https://zhuanlan.zhihu.com/p/139386961)
 - [https://manpages.ubuntu.com/manpages//noble/man3/Analizo::Metric::AfferentConnections.3pm.html](https://manpages.ubuntu.com/manpages//noble/man3/Analizo::Metric::AfferentConnections.3pm.html)
