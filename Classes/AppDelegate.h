/*
    File: AppDelegate.h
*/

#import <UIKit/UIKit.h>
#import <libkern/OSAtomic.h>
#import <CoreFoundation/CFURL.h>
#import "ViewController.h"
#import "FormantView.h"
#import "BufferManager.h"
#import "SignalAnalysis.h"
#import "aurio_helper.h"
#import "CAStreamBasicDescription.h"

#define SPECTRUM_BAR_WIDTH 4

#ifndef CLAMP
#define CLAMP(min,x,max) (x < min ? min : (x > max ? max : x))
#endif

typedef struct SpectrumLinkedTexture {
	GLuint							texName; 
	struct SpectrumLinkedTexture	*nextTex;
} SpectrumLinkedTexture;

inline double linearInterp(double valA, double valB, double fract)
{
	return valA + ((valB - valA) * fract);
}

@interface AppDelegate : UIResponder <UIApplicationDelegate> {
	BOOL						unitIsRunning;
	BOOL						unitHasBeenCreated;
	CAStreamBasicDescription	thruFormat;
	Float64						hwSampleRate;
	AURenderCallbackStruct		inputProc;
    double                      formants[2];
}

@property (nonatomic, retain)	UIWindow*				window;
@property (nonatomic, retain)   FormantView*            view;
@property (nonatomic, retain)   NSTimer*                timer;
@property (strong, nonatomic)   ViewController*         viewController;
@property						BufferManager*          bufferManager;
@property (nonatomic, assign)	AudioUnit				rioUnit;
@property (nonatomic, assign)	BOOL					unitIsRunning;
@property (nonatomic, assign)	BOOL					unitHasBeenCreated;
@property (nonatomic, assign)	AURenderCallbackStruct	inputProc;

- (BOOL)getFormants:(double *)outFormants;

@end

