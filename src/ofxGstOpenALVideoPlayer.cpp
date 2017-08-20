#include "ofxGstOpenALVideoPlayer.h"

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/app/gstappsink.h>

ofxGstOpenALVideoPlayer::ofxGstOpenALVideoPlayer()
{
	threadAppSink				= false;
    bIsStream					= false;
   	videoUtils = getGstVideoUtils();
    videoUtils->setSinkListener(this);
}

ofxGstOpenALVideoPlayer::~ofxGstOpenALVideoPlayer()
{
    stop();
}

bool ofxGstOpenALVideoPlayer::loadMovie(string name, ALCdevice *dev, ALCcontext *ctx, ALuint src)
{
    internalPixelFormat = getPixelFormat();
    threadAppSink = isThreadedAppSink();

    device = dev;
    context = ctx;
    source = src;

	close();
	if( name.find( "file://",0 ) != string::npos){
		bIsStream		= false;
	}else if( name.find( "://",0 ) == string::npos){
		GError * err = NULL;
		gchar* name_ptr = gst_filename_to_uri(ofToDataPath(name).c_str(),&err);
		name = name_ptr;
		g_free(name_ptr);
		if(err) g_free(err);
		bIsStream		= false;
	}else{
		bIsStream		= true;
	}
	ofLogVerbose("ofxGstVideoPlayer") << "loadMovie(): loading \"" << name << "\"";

	ofGstUtils::startGstMainLoop();

#if GST_VERSION_MAJOR==0
	GstElement * gstPipeline = gst_element_factory_make("playbin2","player");
#else
	GstElement * gstPipeline = gst_element_factory_make("playbin","player");
#endif
	g_object_set(G_OBJECT(gstPipeline), "uri", name.c_str(), (void*)NULL);

	// create the oF appsink for video rgb without sync to clock
	GstElement * gstSink = gst_element_factory_make("appsink", "app_sink");

	gst_base_sink_set_sync(GST_BASE_SINK(gstSink), true);
	gst_app_sink_set_max_buffers(GST_APP_SINK(gstSink), 8);
	gst_app_sink_set_drop (GST_APP_SINK(gstSink),true);
	gst_base_sink_set_max_lateness  (GST_BASE_SINK(gstSink), -1);

#if GST_VERSION_MAJOR==0
	GstCaps *caps;
	int bpp;
	switch(internalPixelFormat){
	case OF_PIXELS_GRAY:
		bpp = 8;
		caps = gst_caps_new_simple("video/x-raw-gray",
			"bpp", G_TYPE_INT, bpp,
			"depth", G_TYPE_INT, 8,
			NULL);
		break;
	case OF_PIXELS_RGB:
		bpp = 24;
		caps = gst_caps_new_simple("video/x-raw-rgb",
			"bpp", G_TYPE_INT, bpp,
			"depth", G_TYPE_INT, 24,
			"endianness",G_TYPE_INT,4321,
			"red_mask",G_TYPE_INT,0xff0000,
			"green_mask",G_TYPE_INT,0x00ff00,
			"blue_mask",G_TYPE_INT,0x0000ff,
			NULL);
		break;
	case OF_PIXELS_RGBA:
		bpp = 32;
		caps = gst_caps_new_simple("video/x-raw-rgb",
			"bpp", G_TYPE_INT, bpp,
			"depth", G_TYPE_INT, 32,
			"endianness",G_TYPE_INT,4321,
			"red_mask",G_TYPE_INT,0xff000000,
			"green_mask",G_TYPE_INT,0x00ff0000,
			"blue_mask",G_TYPE_INT,0x0000ff00,
			"alpha_mask",G_TYPE_INT,0x000000ff,
			NULL);
		break;
	case OF_PIXELS_BGRA:
		bpp = 32;
		caps = gst_caps_new_simple("video/x-raw-rgb",
			"bpp", G_TYPE_INT, bpp,
			"depth", G_TYPE_INT, 32,
			"endianness",G_TYPE_INT,4321,
			"red_mask",G_TYPE_INT,0x0000ff00,
			"green_mask",G_TYPE_INT,0x00ff0000,
			"blue_mask",G_TYPE_INT,0xff000000,
			"alpha_mask",G_TYPE_INT,0x000000ff,
			NULL);
		break;
	default:
		bpp = 32;
		caps = gst_caps_new_simple("video/x-raw-rgb",
			"bpp", G_TYPE_INT, bpp,
			"depth", G_TYPE_INT, 24,
			"endianness",G_TYPE_INT,4321,
			"red_mask",G_TYPE_INT,0xff0000,
			"green_mask",G_TYPE_INT,0x00ff00,
			"blue_mask",G_TYPE_INT,0x0000ff,
			NULL);
		break;
	}
#else
	string mime="video/x-raw";

	GstCaps *caps;
	if(internalPixelFormat==OF_PIXELS_NATIVE){
		//caps = gst_caps_new_any();
		caps = gst_caps_from_string((mime + ",format={RGBA,BGRA,RGB,BGR,RGB16,GRAY8,YV12,I420,NV12,NV21,YUY2}").c_str());
		/*
		GstCapsFeatures *features = gst_caps_features_new (GST_CAPS_FEATURE_META_GST_VIDEO_GL_TEXTURE_UPLOAD_META, NULL);
		gst_caps_set_features (caps, 0, features);*/
	}else{
		string format = ofGstVideoUtils::getGstFormatName(internalPixelFormat);
		caps = gst_caps_new_simple(mime.c_str(),
											"format", G_TYPE_STRING, format.c_str(),
											NULL);
	}
#endif


	gst_app_sink_set_caps(GST_APP_SINK(gstSink), caps);
	gst_caps_unref(caps);

	if(threadAppSink){
		GstElement * appQueue = gst_element_factory_make("queue","appsink_queue");
		g_object_set(G_OBJECT(appQueue), "leaky", 0, "silent", 1, (void*)NULL);
		GstElement* appBin = gst_bin_new("app_bin");
		gst_bin_add(GST_BIN(appBin), appQueue);
		GstPad* appQueuePad = gst_element_get_static_pad(appQueue, "sink");
		GstPad* ghostPad = gst_ghost_pad_new("app_bin_sink", appQueuePad);
		gst_object_unref(appQueuePad);
		gst_element_add_pad(appBin, ghostPad);

		gst_bin_add(GST_BIN(appBin), gstSink);
		gst_element_link(appQueue, gstSink);

		g_object_set (G_OBJECT(gstPipeline),"video-sink",appBin,(void*)NULL);
	}else{
		g_object_set (G_OBJECT(gstPipeline),"video-sink",gstSink,(void*)NULL);
	}

#ifdef TARGET_WIN32
	GstElement *audioSink = gst_element_factory_make("directsoundsink", NULL);
	g_object_set (G_OBJECT(gstPipeline),"audio-sink",audioSink,(void*)NULL);
#endif

#ifdef TARGET_LINUX_ARM
#if GST_VERSION_MINOR<4
	/*if(dynamic_cast<ofAppEGLWindow*>(ofGetWindowPtr())){
		EGLDisplay display = ((ofAppEGLWindow*)ofGetWindowPtr())->getEglDisplay();
		GstEGLDisplay * gstEGLDisplay = gst_egl_display_new (display,NULL);
		GstContext *context = gst_context_new_egl_display(gstEGLDisplay,true);
		GstMessage * msg = gst_message_new_have_context (GST_OBJECT (gstPipeline), context);
		gst_element_post_message (GST_ELEMENT_CAST (gstPipeline), msg);
	}*/
#else

	/*if(dynamic_cast<ofAppEGLWindow*>(ofGetWindowPtr())){
		EGLDisplay display = ((ofAppEGLWindow*)ofGetWindowPtr())->getEglDisplay();
		GstGLDisplayEGL * gstEGLDisplay = gst_gl_display_egl_new_with_egl_display (display);
		GstContext *context = gst_context_new();
		gst_gl_display_create_context (context, gstEGLDisplay);
		GstMessage * msg = gst_message_new_have_context (GST_OBJECT (gstPipeline), context);
		gst_element_post_message (GST_ELEMENT_CAST (gstPipeline), msg);
	}*/
#endif
#endif


    GstElement  *gstALSink = gst_element_factory_make("openalsink", NULL);
    if(gstALSink != NULL)
    {
        // Set a custom device, or context and source id, to play with. If
        // none of these are given, the sink will open its own device,
        // create its own context, and generate its own source.
        if(context != NULL)
        {
            g_object_set(G_OBJECT(gstALSink), "user-context", context, NULL);
            g_object_set(G_OBJECT(gstALSink), "user-source", source, NULL);
        }
        else
        {
            g_object_set(G_OBJECT(gstALSink), "user-device", device, NULL);
        }

        g_object_set(G_OBJECT(gstPipeline), "audio-sink", gst_object_ref(gstALSink), NULL);
    }
    else {
        g_warning("Failed to create openal sink");
    }


	return videoUtils->setPipelineWithSink(gstPipeline,gstSink,bIsStream) &&
				videoUtils->startPipeline() &&
				(bIsStream || allocate());
}

