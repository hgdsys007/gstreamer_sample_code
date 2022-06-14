#include <gst/gst.h>
#include <gst/gstbuffer.h>
#include <gst/app/gstappsink.h>
#include <iostream>
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;
#include <ctime>


typedef struct gstStruct
{
  // gboolean saved=false;
  GstElement *pipeline;
  GMainLoop *loop;
  // string name;
  // string location;
  string rtsp;
  // int totalImg;
  // int interval;
  // Mat myFrame;
  unsigned char* MatData;
  long long buffersize;
} gstStruct;

/* This function will be called by the pad-added signal */
static void
pad_added_handler (GstElement * src, GstPad * new_pad, gpointer depay)
{
  GstElement *sink_depay = (GstElement *) depay;
  GstPad *sink_pad = gst_element_get_static_pad (sink_depay, "sink");
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;
  cout<<"pad adding..."<<endl;
  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad),
      GST_ELEMENT_NAME (src));

  /* If our converter is already linked, we have nothing to do here */
  if (gst_pad_is_linked (sink_pad)) {
    g_print ("We are already linked. Ignoring.\n");
    goto exit;
  }

  /* Check the new pad's type */
  new_pad_caps = gst_pad_get_current_caps (new_pad);
  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  new_pad_type = gst_structure_get_name (new_pad_struct);
  if (!g_str_has_prefix (new_pad_type, "application/x-rtp")) {
    g_print ("It has type '%s' which is not application/x-rtp. Ignoring.\n",
        new_pad_type);
    goto exit;
  }

  /* Attempt the link */
  ret = gst_pad_link (new_pad, sink_pad);
  if (GST_PAD_LINK_FAILED (ret)) {
    g_print ("Type is '%s' but link failed.\n", new_pad_type);
  } else {
    g_print ("Link succeeded (type '%s').\n", new_pad_type);
  }

exit:
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);

  /* Unreference the sink pad */
  gst_object_unref (sink_pad);
}



GstFlowReturn new_preroll(GstAppSink *appsink, gpointer data) {
  g_print ("Got preroll!\n");
  return GST_FLOW_OK;
}

GstFlowReturn new_sample(GstAppSink *appsink, gpointer data) {
  static int framecount = 0;
  framecount++;
  GstSample *sample = gst_app_sink_pull_sample(appsink);
  GstCaps *caps = gst_sample_get_caps(sample);
  GstBuffer *buffer = gst_sample_get_buffer(sample);
  GstStructure *structure = gst_caps_get_structure(caps,0);
  const int width=g_value_get_int(gst_structure_get_value(structure,"width"));
  const int height=g_value_get_int(gst_structure_get_value(structure,"height"));
  string format = g_value_get_string(gst_structure_get_value(structure,"format"));


  // ---- Read frame and convert to opencv format ---------------
  GstMapInfo map;
  gst_buffer_map (buffer, &map, GST_MAP_READ);

  // convert gstreamer data to OpenCV Mat, you could actually resolve height / width from caps...
  // Mat frame(Size(width, height), CV_8UC3, (char*)map.data, Mat::AUTO_STEP);
  // clock_t start,end1, end2;
  // start=clock();
  memcpy(((gstStruct *)data)->MatData, (unsigned char*)map.data, width*height*3);
  // end1=clock();
  // // myfunction (name2, location, rtsp, pData);
  // // end2=clock();
  // double endtime1=(double)(end1-start)/CLOCKS_PER_SEC;
  // // double endtime2=(double)(end2-start)/CLOCKS_PER_SEC;
	// cout<<"Total time:"<<endtime1*1000<<"ms"<<endl;	//ms为单位

  // ((CustomData *)data)->MatData = (unsigned char*)map.data;
  // Mat frame_save;
  // cvtColor(frame, frame_save, COLOR_YUV2BGR_I420);
  // Mat &srcImage;

  gst_buffer_unmap(buffer, &map);

  // ------------------------------------------------------------
  // show caps on first frame
  // if (framecount == 1) {
  //   g_print ("%s\n", gst_caps_to_string(caps));
  //   // cout << "****media info****" << endl << "width: " << width << "; height: " << height << "; format: " << format << endl;
  //   // cout << "****buffer info****" << endl << "width: " << frame.cols << "; height: " << frame.rows << "; channel: " << frame.channels() << endl;
  // }
  // print dot every 30 frames
  // if (true) {
    // g_print ("save!\n");
    // string ss;
    // ss << "./gstreamer_sample_code/media_source/img/" << framecount/30 << ".jpg";
    // ss = ((CustomData *)data)->location + ((CustomData *)data)->name;
    // cout<<ss<<endl;
    // cv::imwrite(ss.c_str(), frame);

    // ((CustomData *)data)->saved = true;
    // gst_element_set_state (((CustomData *)data)->pipeline , GST_STATE_READY);
    // gst_element_set_state (((CustomData *)data)->pipeline, GST_STATE_NULL);
    // gst_object_unref (((CustomData *)data)->pipeline);
  g_main_loop_quit (((gstStruct *)data)->loop);
    // return GST_FLOW_EOS;
  // }

  gst_sample_unref (sample);
  return GST_FLOW_OK;
}


uint32_t getRtspPicture (string rtsp, unsigned char* pData)
{
  GstElement *source, *sink, *depay, *parse, *decoder, *convert;
  GstCaps *caps;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
  gstStruct data;
  // data.name = name;
  // data.location = location;
  data.MatData = pData;
  // gpointer point;
  data.loop = g_main_loop_new (NULL, FALSE);

  /* Initialize GStreamer */
  gst_init (NULL, NULL);

  /* Create the empty pipeline */
  // cout << "creating pipeline"<<endl;
  data.pipeline = gst_pipeline_new ("test-pipeline");

// gst-launch-1.0 rtspsrc location=rtsp://admin:admin@192.168.1.64 latency=100 
// ! queue ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert 
// ! video/x-raw, width=1280, height=720, format="I420" ! autovideosink


  /* Create the elements */
  source = gst_element_factory_make ("rtspsrc", "source");
  depay = gst_element_factory_make("rtph264depay","depay");
  parse = gst_element_factory_make("h264parse","parse");
  decoder = gst_element_factory_make("avdec_h264",  "decoder");
  convert = gst_element_factory_make("videoconvert", "convert");
  // sink = gst_element_factory_make ("autovideosink", "sink");
  sink = gst_element_factory_make ("appsink", "sink");

  /* Create the caps */
  caps=gst_caps_new_simple("video/x-raw",
                                    // "width",G_TYPE_INT,1280,
                                    // "height",G_TYPE_INT,720,
                                    "format",G_TYPE_STRING,"BGR",
                                    NULL);

  /* Set the propoterty of the element*/
  g_object_set (G_OBJECT (source), "latency", 100, NULL);
  g_object_set(GST_OBJECT(source), "location", rtsp.c_str(), NULL);


  if (!data.pipeline || !source || !depay || !parse || !decoder || !convert  || !caps || !sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }

  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (data.pipeline), source, depay, NULL);

  g_signal_connect(source, "pad-added", G_CALLBACK(pad_added_handler), depay);

  gst_bin_add_many (GST_BIN (data.pipeline), parse, decoder, convert, sink, NULL);


  if (gst_element_link_many (depay, parse, decoder, convert, NULL) != TRUE) {
    g_printerr ("depay to convert could not be linked.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

  if (gst_element_link_filtered (convert, sink, caps) != TRUE) {
    g_printerr ("convert to sink could not be linked\n");
    gst_object_unref (data.pipeline);
    return -1;
  }

  GstElement *appsink = gst_bin_get_by_name (GST_BIN (data.pipeline), "sink");
  gst_app_sink_set_emit_signals((GstAppSink*)appsink, true);
  gst_app_sink_set_drop((GstAppSink*)appsink, true);
  gst_app_sink_set_max_buffers((GstAppSink*)appsink, 1);
  GstAppSinkCallbacks callbacks = { NULL, new_preroll, new_sample };
  gst_app_sink_set_callbacks (GST_APP_SINK(appsink), &callbacks, &data, NULL);


  /* Start playing */
  ret = gst_element_set_state (data.pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }
  // cout<<2*GST_SECOND<<endl;
  ret = gst_element_get_state (data.pipeline, NULL, NULL, -1);
  // cout<<ret<<endl;
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (data.pipeline);
    return -1;
  }
  g_main_loop_run (data.loop);

  gst_element_set_state (data.pipeline, GST_STATE_NULL);
  gst_object_unref (data.pipeline);
  return 0;
}


int main (int argc, char *argv[]){
  // string location = "./gstreamer_sample_code/media_source/img/";
  // string name1 = "1.jpg";
  // string name2 = "2.jpg";
  string rtsp = "rtsp://root:pass@192.168.0.190/axis-media/media.amp";
  unsigned char* pData = new unsigned char[7173600];
  // // cout<<pData<<endl;
  // clock_t start,end1, end2;
  // start=clock();
  getRtspPicture (rtsp, pData);

  // end1=clock();
  // // myfunction (name2, location, rtsp, pData);
  // // end2=clock();
  // double endtime1=(double)(end1-start)/CLOCKS_PER_SEC;
  // // double endtime2=(double)(end2-start)/CLOCKS_PER_SEC;
	// cout<<"Total time:"<<endtime1*1000<<"ms"<<endl;	//ms为单位
  Mat frame(Size(1080, 1920), CV_8UC3, (char*)pData, Mat::AUTO_STEP);
  cv::imwrite("./gstreamer_sample_code/media_source/img/test.jpg", frame);
  cout<<"./gstreamer_sample_code/media_source/img/test.jpg"<<endl;
	// cout<<"Total time:"<<endtime2*1000<<"ms"<<endl;	//ms为单位
}