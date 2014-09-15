/*
    File: AppDelegate.mm
*/

#import "AppDelegate.h"
#import <Foundation/Foundation.h>
#import "AudioUnit/AudioUnit.h"
#import "CAXException.h"
#import "Types.h"

@implementation AppDelegate

@synthesize window;
@synthesize view;
@synthesize viewController;
@synthesize timer;
@synthesize rioUnit;
@synthesize unitIsRunning;
@synthesize unitHasBeenCreated;
@synthesize bufferManager;
@synthesize inputProc;

#pragma mark-

- (BOOL)getFormants:(double *)outFormants {
    if (formants[0] == 0.0 || formants[1] == 0.0)
        return FALSE;

    outFormants[0] = formants[0];
    outFormants[1] = formants[1];
    return TRUE;
}

// NOS ASEGURAMOS DE DETENER LA AUDIO UNIT AL PRINCIPIO DE LA INTERRUPCIÓN Y DE INICIARLA AL FINAL DE LA INTERRUPCIÓN
void rioInterruptionListener(void *inClientData, UInt32 inInterruption)
{
	printf("Session interrupted! --- %s ---", inInterruption == kAudioSessionBeginInterruption ? "Begin Interruption" : "End Interruption");
	
	AppDelegate *THIS = (AppDelegate*)inClientData;
	
	if (inInterruption == kAudioSessionEndInterruption) {
		// make sure we are again the active session
		XThrowIfError(AudioSessionSetActive(true), "couldn't set audio session active");
		XThrowIfError(AudioOutputUnitStart(THIS->rioUnit), "couldn't start unit");
	}
	
	if (inInterruption == kAudioSessionBeginInterruption) {
		XThrowIfError(AudioOutputUnitStop(THIS->rioUnit), "couldn't stop unit");
    }
}

void propListener(  void *                  inClientData,
					AudioSessionPropertyID	inID,
					UInt32                  inDataSize,
					const void *            inData)
{
	AppDelegate *THIS = (AppDelegate*)inClientData;
	if (inID == kAudioSessionProperty_AudioRouteChange)
	{
		try {
			 UInt32 isAudioInputAvailable; 
			 UInt32 size = sizeof(isAudioInputAvailable);
			 XThrowIfError(AudioSessionGetProperty(kAudioSessionProperty_AudioInputAvailable, &size, &isAudioInputAvailable), "couldn't get AudioSession AudioInputAvailable property value");
			 
			 if(THIS->unitIsRunning && !isAudioInputAvailable)
			 {
				 XThrowIfError(AudioOutputUnitStop(THIS->rioUnit), "couldn't stop unit");
				 THIS->unitIsRunning = false;
			 }
			 else if(!THIS->unitIsRunning && isAudioInputAvailable)
			 {
				 XThrowIfError(AudioSessionSetActive(true), "couldn't set audio session active\n");
			 
				 if (!THIS->unitHasBeenCreated)	// the rio unit is being created for the first time
				 {
					 XThrowIfError(SetupRemoteIO(THIS->rioUnit, THIS->inputProc, THIS->thruFormat), "couldn't setup remote i/o unit");
					 THIS->unitHasBeenCreated = true;

                     THIS->bufferManager = new BufferManager(NSAMPLES);
				 }
				 XThrowIfError(AudioOutputUnitStart(THIS->rioUnit), "couldn't start unit");
				 THIS->unitIsRunning = true;
			 }
		} catch (CAXException e) {
			char buf[256];
			fprintf(stderr, "Error: %s (%s)\n", e.mOperation, e.FormatError(buf));
		}
		
	}
}

static OSStatus	PerformThru(void						*inRefCon, 
							AudioUnitRenderActionFlags 	*ioActionFlags, 
							const AudioTimeStamp 		*inTimeStamp, 
							UInt32 						inBusNumber, 
							UInt32 						inNumberFrames,
							AudioBufferList 			*ioData)
{
    AppDelegate *THIS = (AppDelegate *)inRefCon;
	
    // HINT: Obtiene 1024 (inNumberFrames) samples del micrófono y los coloca en ioData.
    OSStatus err = AudioUnitRender(THIS->rioUnit, ioActionFlags, inTimeStamp, 1, inNumberFrames, ioData);
    
	if (err) {
        printf("PerformThru: error %d\n", (int)err);
        return err;
    }
	
    if (THIS->bufferManager == NULL) return noErr;
    
    if (THIS->bufferManager->NeedsNewAudioData()) {
        THIS->bufferManager->GrabAudioData(ioData);
    }
    
    return err;
}

- (void)applicationDidFinishLaunching:(UIApplication *)application
{	
	// HINT: Se deshabilita el modo de reposo en que la pantalla se apaga.
	application.idleTimerDisabled = YES;
	
	// Initialize our remote i/o unit
	inputProc.inputProc = PerformThru;
	inputProc.inputProcRefCon = self;
    
    formants[0] = 0.0;
    formants[1] = 0.0;

	try {
		// Initialize and configure the audio session
		XThrowIfError(AudioSessionInitialize(NULL, NULL, rioInterruptionListener, self), "couldn't initialize audio session");
			
        UInt32 audioCategory = kAudioSessionCategory_PlayAndRecord;
		XThrowIfError(AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(audioCategory), &audioCategory), "couldn't set audio category");
		XThrowIfError(AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, propListener, self), "couldn't set property listener");

		// HINT: Para el análisis de voz lo apropiado es una ventana de 25 ms. Estableciendo la propiedad
        //       kAudioSessionProperty_PreferredHardwareIOBufferDuration a .025 el sistema crea un buffer de
        //       1024 samples. Comprobamos: 1024 samples / 44110 samp/seg = 0.023 seg.
        Float32 preferredBufferSize = .025;
		XThrowIfError(AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration, sizeof(preferredBufferSize), &preferredBufferSize), "couldn't set i/o buffer duration");
        
		UInt32 size = sizeof(hwSampleRate);
		XThrowIfError(AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareSampleRate, &size, &hwSampleRate), "couldn't get hw sample rate");
		
		XThrowIfError(AudioSessionSetActive(true), "couldn't set audio session active\n");

		XThrowIfError(SetupRemoteIO(rioUnit, inputProc, thruFormat), "couldn't setup remote i/o unit");
		unitHasBeenCreated = true;
		
        bufferManager = new BufferManager(NSAMPLES);
		
		XThrowIfError(AudioOutputUnitStart(rioUnit), "couldn't start remote i/o unit");

		size = sizeof(thruFormat);
		XThrowIfError(AudioUnitGetProperty(rioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &thruFormat, &size), "couldn't get the remote I/O unit's output client format");
		
		unitIsRunning = 1;
	}
	catch (CAXException &e) {
		char buf[256];
		fprintf(stderr, "Error: %s (%s)\n", e.mOperation, e.FormatError(buf));
		unitIsRunning = 0;
	}
	catch (...) {
		fprintf(stderr, "An unknown error occurred\n");
		unitIsRunning = 0;
	}
	
    self.window = [[[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]] autorelease];
    // Override point for customization after application launch.
    self.viewController = [[[ViewController alloc] initWithNibName:@"ViewController" bundle:nil] autorelease];
    self.window.rootViewController = self.viewController;
    [self.window makeKeyAndVisible];
    
    self.view = (FormantView *)viewController.canvas;
    self.timer = [NSTimer scheduledTimerWithTimeInterval:1.0/20.0   // 20 Hz
                                                  target:self
                                                selector:@selector(drawView)
                                                userInfo:nil
                                                 repeats:YES];
}

- (void)drawView {
	if (bufferManager->HasNewAudioData()) {
		if (bufferManager->ComputeData(formants)) {
            [self.view setNeedsDisplay];
        }
        else {
            formants[0] = 0.0;
            formants[1] = 0.0;
        }
    }
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	//start animation now that we're in the foreground
	AudioSessionSetActive(true);
}

- (void)applicationWillResignActive:(UIApplication *)application {
	//stop animation before going into background
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
}

- (void)dealloc
{	
	delete bufferManager;
	[timer invalidate];
    self.timer = nil;
    
	[view release];
	[window release];
	[super dealloc];
}

@end
