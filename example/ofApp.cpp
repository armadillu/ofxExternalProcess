#include "ofApp.h"


void ofApp::setup(){

	ofBackground(22);

	// setup "ls -l -a -h" from "/" //////////////////////////////////////////////////
	{

		lsProc.setup(
					 "/", //working dir
					 "ls", //command
					 {"-l", "-a", "-h"} //args (std::vector<string>)
					  );
		lsProc.setLivePipe(ofxExternalProcess::STDOUT_AND_STDERR_PIPE);
	}

	// setup "ping -c 5 127.0.0.1" //////////////////////////////////////////////////
	{

		pingProc.setup(
					 ".", 			//working dir
					 "ping", 		//command
					 {"-c", "5", "127.0.0.1"} 	//args (std::vector<string>)
					 );
		pingProc.setLivePipe(ofxExternalProcess::STDOUT_AND_STDERR_PIPE);

	}

	// Launch Both processess ////////////////////////////////////////////////////////

	lsProc.executeInThreadAndNotify();
	pingProc.executeInThreadAndNotify();
}


void ofApp::update(){

}


void ofApp::draw(){

	float x = 20;
	ofDrawBitmapString("LS Running: " + string(lsProc.isRunning() ? "Yes" : "No"), x, 20);
	ofDrawBitmapString(lsProc.getSmartOutput(), x, 40);

	x = ofGetWidth() * 0.6;
	ofDrawBitmapString("Ping Running: " + string(pingProc.isRunning() ? "Yes" : "No"), x, 20);
	ofDrawBitmapString(pingProc.getSmartOutput(), x, 40);

}


void ofApp::keyPressed(int key){

}


void ofApp::mousePressed(int x, int y, int button){
}

