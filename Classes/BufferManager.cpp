/*
    File: BufferManager.cpp
*/

#include "BufferManager.h"

#define min(x,y) (x < y) ? x : y

BufferManager::BufferManager(UInt32 inNumberFrames) :
	mNeedsAudioData(0),
	mHasAudioData(0),
	mNumberFrames(inNumberFrames),
    mAudioBufferSize(inNumberFrames * sizeof(DATATYPE)),
    mAudioBufferCurrentIndex(0)
{
    mAudioBuffer = (DATATYPE *)malloc(mAudioBufferSize);
	mSignalAnalysis = SignalAnalysisCreate(mNumberFrames);
	OSAtomicIncrement32Barrier(&mNeedsAudioData);
}

BufferManager::~BufferManager()
{
	free(mAudioBuffer);
	SignalAnalysisDestroy(mSignalAnalysis);
}

void BufferManager::GrabAudioData(AudioBufferList *inBL)
{
    // HINT: Si el número de datos de entrada (1024) supera al tamaño del buffer (1024), fuera.
    //       En principio ambos valores son iguales porque así se han configurado.
	if (mAudioBufferSize < inBL->mBuffers[0].mDataByteSize)	return;
    
    // HINT: Hay un índice en el buffer, hay que ver si desde el índice hasta el final hay sitio
    //        para los datos de entrada
	UInt32 bytesToCopy = min(inBL->mBuffers[0].mDataByteSize, mAudioBufferSize - mAudioBufferCurrentIndex);
	
    // HINT: Copia los datos independientemente de su tamaño, bytesToCopy son bytes, no datos. 
    memcpy(mAudioBuffer + mAudioBufferCurrentIndex, inBL->mBuffers[0].mData, bytesToCopy);
	
    // HINT: Se actualiza el índice
    mAudioBufferCurrentIndex += bytesToCopy / sizeof(DATATYPE);
    
    // HINT: Si el buffer está lleno, avisar
	if (mAudioBufferCurrentIndex >= mAudioBufferSize / sizeof(DATATYPE))
	{
		// HINT: mHasAudioData es una variable que puede estar compartida por varios procesadores/hilos.
        //        Esta función la incrementa de forma segura, indicando que hay datos listos.
        OSAtomicIncrement32Barrier(&mHasAudioData);
        
		// HINT: mNeedsAudioData es una variable que puede estar compartida por varios procesadores/hilos.
        //        Esta función la decrementa de forma segura, indicando que no se necesitan datos nuevos.
		OSAtomicDecrement32Barrier(&mNeedsAudioData);
	}
}

Boolean	BufferManager::ComputeData(double *outFormants)
{
	// HINT: Si mHasAudioData == 1 el buffer está lleno y se procesan los datos
    if (HasNewAudioData())
	{
		SignalAnalysisProcess(mSignalAnalysis, mAudioBuffer, outFormants, true);		
		
        // HINT: mHasAudioData es una variable que puede estar compartida por varios procesadores/hilos.
        //        Esta función la decrementa de forma segura, indicando que se han procesado los datos.
        OSAtomicDecrement32Barrier(&mHasAudioData);
        
		// HINT: mNeedsAudioData es una variable que puede estar compartida por varios procesadores/hilos.
        //        Esta función la incrementa de forma segura, indicando que se necesitan datos nuevos.
		OSAtomicIncrement32Barrier(&mNeedsAudioData);
        
		mAudioBufferCurrentIndex = 0;
		return true;
	}
	else if (mNeedsAudioData == 0)
		// HINT: mNeedsAudioData es una variable que puede estar compartida por varios procesadores/hilos.
        //        Esta función la incrementa de forma segura, indicando que se necesitan datos nuevos.
        OSAtomicIncrement32Barrier(&mNeedsAudioData);
	
	return false;
}
