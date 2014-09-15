/*
    File: BufferManager.h
*/

#import <AudioToolbox/AudioToolbox.h>
#import <libkern/OSAtomic.h>

#import "SignalAnalysis.h"

class BufferManager
{
public:
	BufferManager(UInt32 inNumberFrames);
	~BufferManager();
	volatile int32_t	HasNewAudioData ()	{ return mHasAudioData; }
	volatile int32_t	NeedsNewAudioData () { return mNeedsAudioData; }
	UInt32				GetNumberFrames () { return mNumberFrames; }
	void				GrabAudioData (AudioBufferList *inBL);
	Boolean				ComputeData (double *outFormants);
	
private:
	volatile int32_t	mNeedsAudioData;
	volatile int32_t	mHasAudioData;
	H_SIGNAL_ANALYSIS   mSignalAnalysis;
    DATATYPE*			mAudioBuffer;
	UInt32				mNumberFrames;
	UInt32				mAudioBufferSize;
	int32_t				mAudioBufferCurrentIndex;
};