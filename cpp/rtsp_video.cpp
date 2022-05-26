#include <gst/gst.h>
#include <iostream>

using namespace std;

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




int main (int argc, char *argv[])
{
  GstElement *pipeline, *source, *sink, *depay, *parse, *decoder, *convert;
  GstCaps *caps;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;


  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  /* Create the empty pipeline */
  cout << "creating pipeline"<<endl;
  pipeline = gst_pipeline_new ("test-pipeline");

// gst-launch-1.0 rtspsrc location=rtsp://admin:123456@192.168.1.64 latency=100 \
// ! queue ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert 
// ! video/x-raw, width=1280, height=720, format="I420" ! autovideosink


  /* Create the elements */
  source = gst_element_factory_make ("rtspsrc", "source");
  depay = gst_element_factory_make("rtph264depay","depay");
  parse = gst_element_factory_make("h264parse","parse");
  decoder = gst_element_factory_make("avdec_h264",  "decoder");
  convert = gst_element_factory_make("videoconvert", "convert");
  sink = gst_element_factory_make ("autovideosink", "sink");

  /* Create the caps */
  caps=gst_caps_new_simple("video/x-raw",
                                    "width",G_TYPE_INT,1280,
                                    "height",G_TYPE_INT,720,
                                    "format",G_TYPE_STRING,"I420",
                                    NULL);

  /* Set the propoterty of the element*/
  g_object_set (G_OBJECT (source), "latency", 100, NULL);
  g_object_set(GST_OBJECT(source), "location", "rtsp://admin:123456@192.168.1.64/doc/page/preview.asp", NULL);

  



  if (!pipeline || !source || !depay || !parse || !decoder || !convert  || !caps || !sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }


  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), source, depay, NULL);

  g_signal_connect(source, "pad-added", G_CALLBACK(pad_added_handler), depay);

  gst_bin_add_many (GST_BIN (pipeline), parse, decoder, convert, sink, NULL);

  

  if (gst_element_link_many (depay, parse, decoder, convert, NULL) != TRUE) {
    g_printerr ("depay to convert could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  if (gst_element_link_filtered (convert, sink, caps) != TRUE) {
    g_printerr ("convert to sink could not be linked\n");
    gst_object_unref (pipeline);
    return -1;
  }


  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }

  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg =
      gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE,
      static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

  /* Parse message */
  if (msg != NULL) {
    GError *err;
    gchar *debug_info;

    switch (GST_MESSAGE_TYPE (msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error (msg, &err, &debug_info);
        g_printerr ("Error received from element %s: %s\n",
            GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n",
            debug_info ? debug_info : "none");
        g_clear_error (&err);
        g_free (debug_info);
        break;
      case GST_MESSAGE_EOS:
        g_print ("End-Of-Stream reached.\n");
        break;
      default:
        /* We should not reach here because we only asked for ERRORs and EOS */
        g_printerr ("Unexpected message received.\n");
        break;
    }
    gst_message_unref (msg);
  }

  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}
