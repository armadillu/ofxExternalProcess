//
//  ofxExternalProcess.h
//  LP
//
//  Created by Oriol Ferrer Mesià on 26/11/15.
//
//

#ifndef __LP__ofxExternalProcess__
#define __LP__ofxExternalProcess__

#include "ofMain.h"
#include "Poco/PipeStream.h"

class ofxExternalProcess : public ofThread {

public:

	enum OUT_PIPE{
		STDOUT_PIPE,
		STDERR_PIPE,
		STDOUT_AND_STDERR_PIPE
	};

	struct ScriptResult{
		int statusCode;
		string stdOutput;
		string errOutput;
		string combinedOutput;
		ScriptResult(){
			statusCode = -1;
		}
	};

	ofxExternalProcess();

	void setup(string workingDir, string scriptCommand, vector<string> args);
	void setSleepTimeAfterDone(int ms){ sleepMSAfterFinished = ms;} //sleep this # of ms after ext process ends
	void setLivePipe(OUT_PIPE pipe);///when running threaded, you can get live stdOut/stdErr output
									///from one of the pipes, but not both.
									///you get to choose which one here

	void setLivePipeOutputDelay(int ms){outputPipeReadDelay = ms;} ///use this to slow down stdout stderr
	//void setLivePipeBufferReadSize(int s){readBufferSize = s;}

	void update(); //must call this if you care about getting notified when ext process ends

	bool isRunning(){return state != IDLE;}


	string getStdOut();	///returns what the process has spitted out so far
	string getStdErr();
	string getCombinedOutput();
	string getSmartOutput(); //returns the appropiate above method according to the LivePipe setting

	//start the external process
	void executeBlocking(); ///execute in current thread, blocks until script ends
	void executeInThreadAndNotify(); ///spawns a new thread, executes, and notifies. you must addListener for "eventScriptEnded" event to get notified
									///you must call update() every frame for the notification to work.

	ofEvent<ScriptResult> eventScriptEnded;

protected:

	enum State{
		IDLE,
		RUNNING,
		SLEEPING_AFTER_RUN
	};

	void startThread(){ofThread::startThread();}; //only start threads from our API
	void threadedFunction();

	bool pendingNotification;
	ScriptResult result;

	State state;
	bool isSetup;

	int sleepMSAfterFinished;  //
	int outputPipeReadDelay; //ms
	//int readBufferSize;

	string scriptWorkingDir;
	string scriptCommand;

	vector<string> commandArgs;

	string stdOutput;
	string errOutput;
	string combinedOutput;

	OUT_PIPE liveReadPipe;

	void readStreamWithProgress(Poco::PipeInputStream & input,
								string & output);

};

#endif /* defined(__LP__ofxExternalProcess__) */
