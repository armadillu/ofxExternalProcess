//
//  ofxExternalProcess.cpp
//  PG_WALL
//
//  Created by Oriol Ferrer Mesià on 26/11/15.
//
//

#include "ofxExternalProcess.h"

#include "Poco/Process.h"
#include "Poco/StreamCopier.h"
#include "Poco/Buffer.h"

ofxExternalProcess::ofxExternalProcess(){

	state = IDLE;
	scriptPath = ofToDataPath("scripts/script.sh");
	liveReadPipe = STDOUT_PIPE;
	isSetup = false;
	pendingNotification = false;
	sleepMSAfterFinished = 0;
}


void ofxExternalProcess::setup(string scrptPath, vector<string> args){

}

void ofxExternalProcess::update(){

	if(pendingNotification){
		ofNotifyEvent(eventScriptEnded, result, this);
		ofLogNotice("ofxExternalProcess") << "Notify Listeners that " << scriptPath << " is done";
	}
}


void ofxExternalProcess::executeBlocking(){
	threadedFunction();
}

void ofxExternalProcess::executeInThreadAndNotify(){
	string localArgs;
	for(string & arg : args){
		localArgs += arg + " ";
	}
	ofLogNotice("ofxExternalProcess") << "Start Thread running external process " << scriptPath << " with args: [" << localArgs << "]";
	startThread();
}


string ofxExternalProcess::getStdOut(){
	string out;
	lock();
	out = stdOutput;
	unlock();
	return out;
}


string ofxExternalProcess::getStdErr(){
	string out;
	lock();
	out = errOutput;
	unlock();
	return out;
}


void ofxExternalProcess::threadedFunction(){

	state = RUNNING;

	//output = ofSystem(ofToDataPath("scripts/script.sh"));

	std::string cmd(scriptPath);

	std::vector<std::string> args;
	//args.push_back("-ax");

	Poco::Pipe outPipe;
	Poco::Pipe errPipe;

	Poco::ProcessHandle ph = Poco::Process::launch(cmd, args, 0, &outPipe, &errPipe);
	Poco::PipeInputStream istrStd(outPipe);
	Poco::PipeInputStream istrErr(errPipe);

	//read one pipe "live", without blocking, so that we can get live status
	if(liveReadPipe == STDERR_PIPE){
		readStreamWithProgress(istrErr, errOutput);
	}else{
		readStreamWithProgress(istrStd, stdOutput);
	}

	result.statusCode = ph.wait();

	std::stringstream ss;

	if(liveReadPipe == STDERR_PIPE){
		Poco::StreamCopier::copyStream(istrStd, ss);
		stdOutput = ss.str();
	}else{
		Poco::StreamCopier::copyStream(istrErr, ss);
		errOutput = ss.str();
	}

	if(sleepMSAfterFinished > 0){
		state = SLEEPING_AFTER_RUN;
		ofSleepMillis(sleepMSAfterFinished); //hold the status output 5 secs on screen
	}
	pendingNotification = true;
	result.stdOutput = stdOutput;
	result.errOutput = errOutput;
	state = IDLE;
}


void ofxExternalProcess::readStreamWithProgress(Poco::PipeInputStream & input,
														 string & output){

	std::stringstream ss;
	int bufferSize = 1;
	Poco::Buffer<char> buffer(bufferSize);
	std::streamsize sz = 0;
	input.read(buffer.begin(), bufferSize);
	std::streamsize n = input.gcount();

	while (n > 0){
		sz += n;
		ss.write(buffer.begin(), n);
		if (input && ss){
			input.read(buffer.begin(), bufferSize);
			n = input.gcount();
		}
		else n = 0;
		lock();
		output = ss.str();
		unlock();
	}
}
