# 目标 #
致力于编写一个运行效率高、占用资源少的拼音输入法引擎.


# 进展 #
  * 运行效率
> 感兴趣的网友可以自行测试一下，反正在本人使用的过程中，从未感受到响应延迟.

> -- 不懂测试方法的某人打酱油路过！
  * 资源占用
> 个人感觉已经被压缩至最低了，130万词汇量占用内存8M，如果你的内存严重不足，操作系统会继续压缩，直至3M上下.

> -- 什么原理？简直是胡说八道！
  * 输入法主流特性完全支持，如自动调整词频、用户自定义词语、删除错误词语、模糊拼音、拼音纠错、临时英文模式、挂载多码表.

> -- 呃，说什么好呢？


# 获取代码 #
  * `svn checkout http://pye1.googlecode.com/svn/trunk/ pye-read-only`
> 这是拼音引擎的代码，只能用于测试，没有实际利用的价值，当然了，如果想在此基础上继续开发，那就是它了.
  * `svn checkout http://pye1.googlecode.com/svn/ibus/ ibus-read-only`
> 这是基于如上引擎，利用ibus平台编写的拼音输入法，如果想要尝试，那么就是它了(相关使用方法见Wiki).


# 历史 #

2010-10-02 重构代码完成.

2009-11-09 阶段性完整版本.

2009-10-30 终于实现了一个可输入的版本.