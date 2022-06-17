## 文件夹内代码文件介绍



### videotestsrc.cpp

使用官网教程[Basic tutorial 2](https://gstreamer.freedesktop.org/documentation/tutorials/basic/concepts.html?gi-language=c)，为了适配C++代码格式，在47行msg=...进行了修改，使用如下代码编译

```
g++ videotestsrc.cpp -o videotestsrc `pkg-config --cflags --libs gstreamer-1.0`
```

运行成功后应该出现

![Screenshot from 2022-05-25 14-36-19](https://raw.githubusercontent.com/ytikewk/Blog_pics/main/Screenshot%20from%202022-05-25%2014-36-19.png)



实际上使用代码实现了

```
gst-launch-1.0 videotestsrc ! autovideosink
```

基于如上文件，可以对于代码进行如下适配





### rtsp_video.cpp

借鉴了这篇[博客](https://gist.github.com/SJRyu/3ec4c0ad9820242eff9ced1b516e78a8)，读取rtsp流，并用autovideosink进行播放

使用如下代码进行编译

```
g++ rtsp_video.cpp -o rtsp_video `pkg-config --cflags --libs gstreamer-1.0`
```

注意修改自己的rtsp流路径

实际上实现了如下gstreamer代码

```
gst-launch-1.0 rtspsrc location=rtsp://admin:seu228228@192.168.1.64 latency=100 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! video/x-raw, width=1280, height=720, format="I420" ! autovideosink
```





### rtsp_appsink.cpp


appsrc和appsink教程可以参考官网教程，地址如下：

https://gstreamer.freedesktop.org/documentation/application-development/advanced/pipeline-manipulation.html?gi-language=c#grabbing-data-with-appsink

实现了从rtsp流获取图像并转化成opencv图像，每间隔30帧保存一帧图像，图像保存到../media_source/img文件夹下

使用如下代码编译

```
g++ rtsp_appsink.cpp -o rtsp_appsink `pkg-config --cflags --libs gstreamer-1.0 opencv4 gstreamer-app-1.0 glib-2.0 gobject-2.0`
```





### rtspToJpg.cpp

从rtsp读取到appsink，并使用opencv保存为一张jpg图片

使用如下代码编译

```
 g++ rtspToJpg.cpp -o rtspToJpg `pkg-config --cflags --libs gstreamer-1.0 opencv4 gstreamer-app-1.0 glib-2.0 gobject-2.0`
```



### rtspToappsink_ptr_rtspCheck.cpp

基于rtspToJpg.cpp文件，使用指针传递数据，对于插件系统开发更加有好。同时增加了报错机制，在pipeline没有内容输出时进行报错

```
  // gst_element_get_state会让ret变为待改变状态，gst_element_get_state会等待ret真正改变
  ret = gst_element_get_state (data.pipeline, NULL, NULL, -1);
  // cout<<ret<<endl;
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }
```



