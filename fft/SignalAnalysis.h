/*
    File: SignalAnalysis.h
*/

#if !defined __SIGNAL_ANALYSIS_H__
#define __SIGNAL_ANALYSIS_H__

#import <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif
    
// typedef int16_t         DATATYPE;
typedef int32_t         DATATYPE;
    
/*
 * Forward declarations
 */
struct SIGNAL_ANALYSIS;
typedef struct SIGNAL_ANALYSIS* H_SIGNAL_ANALYSIS;
	
/* 
 * Create a SignalAnalysis object. The block size argument must be a power of 2
 */
H_SIGNAL_ANALYSIS SignalAnalysisCreate(int32_t blockSize);
void SignalAnalysisDestroy(H_SIGNAL_ANALYSIS p);
void SignalAnalysisProcess(H_SIGNAL_ANALYSIS p, const DATATYPE* inSignal, double *outFormants, bool in_dB);

#ifdef __cplusplus
}
#endif
		
#endif
