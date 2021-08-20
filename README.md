# rundll32_dll_loader

攻击者：

1、VPS上放置远程DLL，并防止Flash进行

受害者：

1、下载Flash安装，远程下载攻击者VPS的DLL到缓存目录。

2、rundll32进行加载本地缓存目录中的DLL。

3、接着自动创建计划任务，内容为rundll32加载本地dll。

4、发送HTTP请求到远程VPS中取消钓鱼目标，实现无感钓鱼。


