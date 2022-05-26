## 这里介绍文件产生的代码



### 保存一张图片，直到结束

```
gst-launch-1.0 rtspsrc location=rtsp://admin:seu228228@192.168.1.64 latency=100 num-buffers=1 ! queue ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw, width=1280, height=720, format="BGR" ! jpegenc ! filesink location=test.jpeg
```



### 保存videotestsrc的视频为mp4文件

```
gst-launch-1.0 videotestsrc is-live=true pattern=0 ! videoconvert ! video/x-raw ! x264enc ! mp4mux ! filesink location=videotestsrc.mp4 -e
```



### 查看mp4视频

```
gst-launch-1.0 filesrc location=videotestsrc.mp4 ! qtdemux ! h264parse ! avdec_h264 ! videoconvert ! autovideosink
```



### 读取rtsp摄像头

```
gst-launch-1.0 rtspsrc location=rtsp://admin:123456@192.168.1.64 latency=100 ! queue ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw, width=1280, height=720, format="I420" ! autovideosink
```

