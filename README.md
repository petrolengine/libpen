### libpen 是一款优秀的发动机引擎。更简洁，更效率是我的追求目标。

## 一、功能模块

### 1. 网络接口（待续）

​    实现了基本的网络接口， 包括: 监听端口(listener)， 连接其他服务器(connecter)， 设置超时(timer)。

​    这些功能都通过配置文件进行管理。支持tcp，unix连接，支持socket，http通信。

### 2. 线程池（待续）

​    可以工具需求配置线程池大小，设未0，表示使用单线程模式。

### 3. 内存管理

​    libpen内部所有内存都可以统一管理，支持内存的重复使用，避免出现内存的碎片化。

​    理想状态下，活跃的程序运行一段时间后，不需要再申请和释放内存了。

### 4. 计时器

​    基于最小堆的实现，支持单次计时和循环计时。

### 5. 配置文件

​    配置文件支持类型检查，支持所有的基本类型， 并且支持自定义的类型，和数组类型。调用接口简单，高效。

​    缺点是添加字段、修改字段类型需要重新编译。

### 6. 文件监控

   可以监控文件的状态，出发文件改动事件

## 二、故事背景

​      工作以后，只要我有一点空闲时间，我就在开发libpen，已经过去5年时间了。在这5年里，随着我接触不同的项目和框架，我的想法在不停的改变。

​     一开始我叫他 libCommon，他似乎可以做任何事情, 并且我坚持用C语言实现。后来有一段时间我觉得C++11特别好用，我就把libCommon用C++全部重新实现了一遍。最后的这个版本还是C语言实现的，理由我不知道对不对，我觉得对于我的代码C++太复杂了，不够简洁，C语言已经可以很好的满足我面相对象的需求了。

​    我认为一个庞大的系统，应该分成不同的层次，它最核心的代码应该尽可能的简单。例如: word 支持宏编程，我们可以直接在word里面编辑、运行、调试代码，但是他的核心代码，一定不会是用宏写的。

## 三、 开源项目比较

​     当我给别人介绍这个项目时，总会被问到，他和libevent有什么的区别。我在开发的过程中也经常会想，libevent和我有什么区别。

​    单单看网络部分，libevent真的很好，但是，libpen和libevent完全不同。libevent提供的只是我的第一个功能模块，监听端口，连接其他服务器需要编程。这个在libpen里面，只需要配置就能完成了。
