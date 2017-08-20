#include "ofApp.h"


#define ALCheck(Func) ((Func), ALCheckError(__FILE__, __LINE__))


//--------------------------------------------------------------
ofApp::~ofApp()
{
    for(unsigned int i=0;i< videos.size();i++) {
        videos[i]->stop();
        delete videos[i];
        alDeleteSources(1, &sources[i]);
    }

    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}

//--------------------------------------------------------------
void ofApp::setup()
{
	ofBackground(0);
	ofSetLogLevel( OF_LOG_VERBOSE );

    cam.setPosition(0,0,0);

    ofDirectory dir;
    dir.open("videos");
    dir.allowExt("mp4");
    dir.listDir();
    dir.sort();

    numVideos = dir.size();
    ofLogNotice("Setup") << "Found " << numVideos << " videos.";

    selectedVideo = 0;

    for(size_t i = 0; i < numVideos;i++) {
        ofVideoPlayer* vid = new ofVideoPlayer();
        videos.push_back(vid);
    }

    setupOpenAL();

    for(size_t i = 0; i < numVideos;i++) {
        shared_ptr<ofxGstOpenALVideoPlayer> player (new ofxGstOpenALVideoPlayer);
        players.push_back(std::move(player));

        videos[i]->setPlayer(players[i]);
        bool bOk = players[i]->loadMovie(dir.getPath(i),device, context, sources[i]);
        cout << "loading: " << dir.getPath(i) << endl;
        if(!bOk) cout << "ERROR loading movie" << endl;
        videos[i]->setLoopState(OF_LOOP_NORMAL);
    }

    videos[selectedVideo]->play();
}

//--------------------------------------------------------------
void ofApp::update()
{
    ofMatrix4x4 rotmat;
    ofQuaternion q = cam.getOrientationQuat();
    q.get(rotmat);

    ofVec3f fwd = ofVec3f(-rotmat(2,0), -rotmat(2,1), -rotmat(2,2));
    ofVec3f up = ofVec3f(rotmat(1,0), rotmat(1,1), rotmat(1,2));
    float Orientation[] = {fwd.x, fwd.y, fwd.z, up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, Orientation);
    alListener3f(AL_POSITION, cam.getPosition().x, cam.getPosition().y, cam.getPosition().z);

    for(unsigned int i=0; i< videos.size(); i++) {
        videos[i]->update();
        ALCheck(alSource3f(sources[i], AL_POSITION, 0,0,-600));
    }

    if(videos[selectedVideo]->getPosition() > 0.95f )
    {
        videos[selectedVideo]->stop();
        selectedVideo++;

        if(selectedVideo > numVideos-1) {
            selectedVideo = 0;
        }
        videos[selectedVideo]->play();
    }

}

//--------------------------------------------------------------
void ofApp::draw()
{
    cam.begin();
    ofPushStyle();
    drawVideos();
    ofPopStyle();
    cam.end();
}

//--------------------------------------------------------------
void ofApp::drawVideos()
{
    ofSetColor(255,255,255);

    ofPushMatrix();
    ofTranslate(-videos[selectedVideo]->getWidth()/2,-videos[selectedVideo]->getHeight()/2,-600);
    videos[selectedVideo]->draw(0,0);
    ofPopMatrix();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{

	if(key == ' ') {
        videos[selectedVideo]->stop();
        selectedVideo++;
        if(selectedVideo > numVideos-1) {
            selectedVideo = 0;
        }
        videos[selectedVideo]->play();
	}
}

//--------------------------------------------------------------
bool ofApp::setupOpenAL()
{
    /* Open a device and set up a context */
    device = alcOpenDevice(NULL);
    if(!device)
    {
        fprintf(stderr, "Failed to open a device\n");
        return false;
    }
    printf("Opened device: %s\n", alcGetString(device, ALC_DEVICE_SPECIFIER));

    context = alcCreateContext(device, NULL);
    if(!context)
    {
        alcCloseDevice(device);
        fprintf(stderr, "Failed to create a context\n");
        return false;
    }
    if(alcMakeContextCurrent(context) == ALC_FALSE)
    {
        fprintf(stderr, "OpenAL: could not make context current - exiting\n");
        return false;
    }

    for(size_t i = 0; i < numVideos;i++) {
        /* Create the source to play from */
        ALuint source;
        alGenSources(1, &source);
        if(alGetError() != AL_NO_ERROR)
        {
            alcDestroyContext(context);
            alcCloseDevice(device);
            fprintf(stderr, "Failed to create a source\n");
            return false;
        }

        /* In the case of mono files, put it in front to play out the (virtual)
         * front-center speaker */
        alSourcei(source, AL_SOURCE_RELATIVE, AL_FALSE);
        //ALCheck(alSource3f(source, AL_POSITION, 0.0f, 0.0f, -2.0f));
        alSourcef(source, AL_GAIN, 3.0f);
        alSourcef(source, AL_ROLLOFF_FACTOR, 0.01f);
        alSourcef(source, AL_PITCH, 1.0f);

        alListener3f(AL_POSITION, 0, 0, 0);
        //    float Orientation[] = {0, 0, -1, 0.f, 1.f, 0.f};
        //    alListe nerfv(AL_ORIENTATION, Orientation);

        alGetError();

        sources.push_back(source);
    }
    return true;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{

}
