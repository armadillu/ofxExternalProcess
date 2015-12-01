//
//  ofxExternalProcess.cpp
//  LP
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
	liveReadPipe = STDOUT_PIPE;
	isSetup = false;
	pendingNotification = false;
	sleepMSAfterFinished = 0;
}


void ofxExternalProcess::setup(string workingDir, string scriptCommand, vector<string> args){

	scriptWorkingDir = workingDir;
	this->scriptCommand = scriptCommand;
	commandArgs = args;
	isSetup = true;
}

void ofxExternalProcess::setLivePipe(OUT_PIPE pipe){
	liveReadPipe = pipe;
}

void ofxExternalProcess::update(){

	if(pendingNotification){
		ofNotifyEvent(eventScriptEnded, result, this);
		ofLogNotice("ofxExternalProcess") << "Notify Listeners that " << scriptWorkingDir << "/" << scriptCommand << " is done";
	}
}


void ofxExternalProcess::executeBlocking(){
	if(!isSetup){
		ofLogError("ofxExternalProcess") << "not setup! cant execute!";
		return;
	}
	threadedFunction();
}

void ofxExternalProcess::executeInThreadAndNotify(){

	if(!isSetup){
		ofLogError("ofxExternalProcess") << "not setup! cant execute!";
		return;
	}

	string localArgs;
	for(string & arg : commandArgs){
		localArgs += arg + " ";
	}
	ofLogNotice("ofxExternalProcess") << "Start Thread running external process " << scriptWorkingDir << "/" << scriptCommand << " with args: [" << localArgs << "]";
	state = RUNNING;
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

string ofxExternalProcess::getCombinedOutput(){
	string out;
	lock();
	out = combinedOutput;
	unlock();
	return out;
}

string ofxExternalProcess::getSmartOutput(){

	switch (liveReadPipe) {
		case STDOUT_PIPE: return getStdOut();
		case STDERR_PIPE: return getStdErr();
		case STDOUT_AND_STDERR_PIPE: return getCombinedOutput();
	}
	return "";
}


void ofxExternalProcess::threadedFunction(){

	state = RUNNING;

	Poco::Pipe outPipe;
	Poco::Pipe errPipe;
	Poco::Pipe combinedPipe;

	Poco::Process::Args args = commandArgs;
	Poco::PipeInputStream istrStd(outPipe);
	Poco::PipeInputStream istrErr(errPipe);
	Poco::PipeInputStream istrCombined(combinedPipe);

	try{
		Poco::ProcessHandle ph = Poco::Process::launch(
													   scriptCommand,
													   args,
													   scriptWorkingDir,
 													   0, //inPipe TODO!
													   liveReadPipe == STDOUT_AND_STDERR_PIPE ? &combinedPipe : &outPipe,
													   liveReadPipe == STDOUT_AND_STDERR_PIPE ? &combinedPipe : &errPipe
													   );

		//read one pipe "live", without blocking, so that we can get live status updates from the process
		if(liveReadPipe == STDOUT_AND_STDERR_PIPE){
			readStreamWithProgress(istrCombined, combinedOutput);
		}else{
			if(liveReadPipe == STDERR_PIPE){
				readStreamWithProgress(istrErr, errOutput);
			}else{
				readStreamWithProgress(istrStd, stdOutput);
			}
		}

		result.statusCode = ph.wait();

	}catch(const Poco::Exception& exc){
		ofLogFatalError("ofxExternalProcess::exception") << exc.displayText();
	}

	std::stringstream ss;

	if(liveReadPipe != STDOUT_AND_STDERR_PIPE){ //if we didnt read both pipes at the same time,
												//fully read the one that hasn't beed read yet
		if(liveReadPipe == STDERR_PIPE){
			Poco::StreamCopier::copyStream(istrStd, ss);
			stdOutput = ss.str();
		}else{
			Poco::StreamCopier::copyStream(istrErr, ss);
			errOutput = ss.str();
		}
	}

	if(sleepMSAfterFinished > 0){
		state = SLEEPING_AFTER_RUN;
		ofSleepMillis(sleepMSAfterFinished); //hold the status output 5 secs on screen
	}

	pendingNotification = true;
	result.stdOutput = stdOutput;
	result.errOutput = errOutput;
	result.combinedOutput = combinedOutput;

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
