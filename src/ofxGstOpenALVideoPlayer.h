#pragma once

#include "ofGstVideoPlayer.h"
#include <AL/alc.h>
#include <AL/al.h>

class ofxGstOpenALVideoPlayer : public ofGstVideoPlayer
{
    public:
        ofxGstOpenALVideoPlayer();
        virtual ~ofxGstOpenALVideoPlayer();

        bool loadMovie(string name, ALCdevice *dev, ALCcontext *ctx, ALuint source);


    protected:
        GstElement  *gstALSink;

    private:
        bool				threadAppSink;
        bool				bIsStream;
        ofPixelFormat		internalPixelFormat;
        ofGstVideoUtils *	videoUtils;

        ALCdevice *device;
        ALCcontext *context;
        ALuint source;
};
