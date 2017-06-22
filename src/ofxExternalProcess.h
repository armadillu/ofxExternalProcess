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
#include "Poco/Process.h"
#include "Poco/PipeStream.h"

class ofxExternalProcess : public ofThread {

public:

	enum OUT_PIPE{
		STDOUT_PIPE,
		STDERR_PIPE,
		STDOUT_AND_STDERR_PIPE,
		IGNORE_OUTPUT
	};

	struct Result{
		ofxExternalProcess * who;
		string commandFullPath;
		int statusCode;
		string stdOutput;
		string errOutput;
		string combinedOutput;
		float runTime;
		Result(){
			statusCode = -1;
			runTime = 0.0f;
		}
	};

	ofxExternalProcess();

	void setup(string workingDir, string scriptCommand, vector<string> args);
	void setSleepTimeAfterDone(int ms){ sleepMSAfterFinished = ms;} //sleep this # of ms after ext process ends
	void setLivePipe(OUT_PIPE pipe);///when running threaded, you can get live stdOut/stdErr output
									///from one of the pipes, or from both at the same time (but mixed, so you dony know whats out and whats err)

	///use this to slow down stdout stderr pipe reading
	///use it if you want a more progressive output from getSmartOutput()
	///in case you are drawing to screen.
	void setLivePipeOutputDelay(int ms){outputPipeReadDelay = ms;}

	//void setLivePipeBufferReadSize(int s){readBufferSize = s;}

	void update(); //must call this if you care about getting notified when ext process ends

	bool isRunning(){return state != IDLE;}

	//these will work on accordance to your choice for setLivePipe().
	//you can call the apropiate method as the process is running to get "live" output on that pipe;
	//but if you call a method that doesnt match your setting for setLivePipe() you will get an empty string
	//until the process is fully finished; at that time, you will get the full output for any pipe.
	string getStdOut();	///returns what the process has spitted out so far
	string getStdErr();
	string getCombinedOutput(); //returns what the process has spitted on both stdout and stderr
	string getSmartOutput(); //returns the appropiate above method according to the chose setLivePipe() setting

	//start the external process
	void executeBlocking(); ///execute in current thread, blocks until script ends
	void executeInThreadAndNotify(); ///spawns a new thread, executes, and notifies. you must addListener for "eventProcessEnded" event to get notified
									///you must call update() every frame for the notification to work.

	void kill();

	ofEvent<Result> eventProcessEnded; //will get triggered when the script is done.

	///get the result of the process execution. includes stdout / err, and exit status code.
	///if you call this ahead of time (ie when no process has been run, or while its still running
	///you will get garbage.
	ofxExternalProcess::Result getLastExecutionResult(){return result;}

protected:

	enum State{
		IDLE,
		RUNNING,
		SLEEPING_AFTER_RUN
	};

	void startThread(){ofThread::startThread();}; //only start threads from our API
	void threadedFunction();

	bool pendingNotification;
	Result result;

	State state;
	bool isSetup;

	int sleepMSAfterFinished = 0;  //
	int outputPipeReadDelay = 0; //ms - used to slow down the reading from pipe in case you want to draw
								//the output on screen - this way you can get a more natural scrolling.

	string scriptWorkingDir;
	string scriptCommand;

	vector<string> commandArgs;

	string stdOutput;
	string errOutput;
	string combinedOutput;

	OUT_PIPE liveReadPipe;

	void readStreamWithProgress(Poco::PipeInputStream & input,
								string & output);

	Poco::ProcessHandle * phPtr = nullptr;
};

#endif /* defined(__LP__ofxExternalProcess__) */
