## ytikewk的gstreamer代码库



这里记录一下平时开发gstreamer流媒体框架的一些基本代码以及一些调试方法



### 在开发时首先在命令行测试框架结构是否可行

例如读取rtsp流：

```
gst-launch-1.0 rtspsrc location=rtsp://admin:123456@192.168.1.64 latency=100 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw, width=1280, height=720, format="BGR" ! videoconvert ! autovideosink
```

如果想查看媒体在管道内部的具体信息，可以在命令行代码最后添加 -vv

```
gst-launch-1.0 filesrc location=videotestsrc.mp4 ! avdec_h264 ! videoconvert ! autovideosink -vv   
```

如果是进行保存处理，因为要向管道中发送结束指令（EOS）,需要在命令行代码最后添加 -e

```
gst-launch-1.0 videotestsrc num-buffers=100 ! videoconvert ! video/x-raw ! x264enc ! mp4mux ! filesink location=videotestsrc.mp4 -e
```



使用gst-instpect查看框架组件具体信息，或者所有相关组件

```
gst-inspect-1.0 x264enc
```



```
gst-inspect-1.0| grep "demux"
```



### Repo文件介绍

cpp文件夹存放了一些c++代码框架

media_source文件夹存放了一些代码生成的或者用来测试的媒体文件

(后续)python文件夹存放一些python代码框架，包括nvida的deepstream框架