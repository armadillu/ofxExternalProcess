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
#include <future>

class ofxExternalProcess{

public:

	enum OUT_PIPE{
		STDOUT_PIPE,
		STDERR_PIPE,
		STDOUT_AND_STDERR_PIPE,
		IGNORE_OUTPUT
	};

	struct Result{
		ofxExternalProcess * who;
		std::string commandFullPath;
		int statusCode;
		std::string stdOutput;
		std::string errOutput;
		std::string combinedOutput;
		float runTime;
		Result(){
			statusCode = -1;
			runTime = 0.0f;
		}
	};

	ofxExternalProcess();
	~ofxExternalProcess();

	void setup(std::string workingDir, std::string scriptCommand, std::vector<std::string> args);
	void setSleepTimeAfterDone(int ms){ sleepMSAfterFinished = ms;} //sleep this # of ms after ext process ends
	void setLivePipe(OUT_PIPE pipe);///when running threaded, you can get live stdOut/stdErr output
									///from one of the pipes, or from both at the same time (but mixed, so you dony know whats out and whats err)

	void setIoBufferSize(size_t bufferSize);

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
	std::string getStdOut();	///returns what the process has spitted out so far
	std::string getStdErr();
	std::string getCombinedOutput(); //returns what the process has spitted on both stdout and stderr
	std::string getSmartOutput(); //returns the appropiate above method according to the chose setLivePipe() setting

	//start the external process
	void executeBlocking(); ///execute in current thread, blocks until script ends
	void executeInThreadAndNotify(); ///spawns a new thread, executes, and notifies. you must addListener for "eventProcessEnded" event to get notified
									///you must call update() every frame for the notification to work.

	void kill();
	void join(long milliseconds = -1); 	//if u spawned the process with executeInThreadAndNotify(),
										//mostly makes sense for destructors that want to destroy a ofxExternalProcess that is running
										//-1 waits forever

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

	std::mutex mutex;

	void threadedFunction();

	bool pendingNotification;
	Result result;

	State state;
	bool isSetup;

	int sleepMSAfterFinished = 0;  //
	int outputPipeReadDelay = 0; //ms - used to slow down the reading from pipe in case you want to draw
								//the output on screen - this way you can get a more natural scrolling.

	std::string scriptWorkingDir;
	std::string scriptCommand;

	std::vector<std::string> commandArgs;

	std::string stdOutput;
	std::string errOutput;
	std::string combinedOutput;

	size_t ioReadBufferSize = 1024;

	OUT_PIPE liveReadPipe;

	void readStreamWithProgress(Poco::PipeInputStream & input,
								std::string & output);

	Poco::ProcessHandle * phPtr = nullptr;

	std::future<void> future;
};

#endif /* defined(__LP__ofxExternalProcess__) */
