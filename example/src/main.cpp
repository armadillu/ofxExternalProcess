#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){

	//ofSetCurrentRenderer(ofGLProgrammableRenderer::TYPE);

	int s = 3;
	ofSetupOpenGL(384 * s, 224 * s, OF_WINDOW);			// <-------- setup the GL context

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp( new ofApp());
}
