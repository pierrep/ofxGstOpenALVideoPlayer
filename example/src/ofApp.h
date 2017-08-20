#pragma once

#include "ofMain.h"
#include "ofxGstOpenALVideoPlayer.h"

#define ALCheck(Func) ((Func), ALCheckError(__FILE__, __LINE__))



class ofApp : public ofBaseApp
{
  public:

    ~ofApp();
	void setup();
	void update();
	void draw();
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	bool setupOpenAL();
	void drawVideos();

	ofLight				light;
	ofEasyCam			cam;

    vector<ofVideoPlayer *>   videos;
    vector<shared_ptr<ofxGstOpenALVideoPlayer>> players;
    size_t selectedVideo;
    size_t numVideos;

    ALCdevice *device;
    ALCcontext *context;
    vector<ALuint> sources;

};

////////////////////////////////////////////////////////////
/// Check last OpenAL error
///
////////////////////////////////////////////////////////////
inline void ALCheckError(const std::string& File, unsigned int Line)
{
    // Get the last error
    ALenum ErrorCode = alGetError();

    if (ErrorCode != AL_NO_ERROR)
    {
        std::string Error, Desc;

        // Decode the error code
        switch (ErrorCode)
        {
            case AL_INVALID_NAME :
            {
                Error = "AL_INVALID_NAME";
                Desc  = "an unacceptable name has been specified";
                break;
            }

            case AL_INVALID_ENUM :
            {
                Error = "AL_INVALID_ENUM";
                Desc  = "an unacceptable value has been specified for an enumerated argument";
                break;
            }

            case AL_INVALID_VALUE :
            {
                Error = "AL_INVALID_VALUE";
                Desc  = "a numeric argument is out of range";
                break;
            }

            case AL_INVALID_OPERATION :
            {
                Error = "AL_INVALID_OPERATION";
                Desc  = "the specified operation is not allowed in the current state";
                break;
            }

            case AL_OUT_OF_MEMORY :
            {
                Error = "AL_OUT_OF_MEMORY";
                Desc  = "there is not enough memory left to execute the command";
                break;
            }
        }

        // Log the error
        std::cerr << "An internal OpenAL call failed in "
                  << File.substr(File.find_last_of("\\/") + 1) << " (" << Line << ") : "
                  << Error << ", " << Desc
                  << std::endl;
    }
}

