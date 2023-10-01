Auro

===========================================================================

For this project, I took the texture atlas of the main character from the game Prince of Persia 2: The Shadow and the Flame (1993) and animated it, making it respond to three voice commands: 'walk,' 'turn,' and 'jump.'

I didn't use any speech recognition library; all the code is in the project. I relied on the source code of Apple's AurioTouch program, a classic for anyone starting with signal processing, as well as on the C++ helper classes of iPublicUtility, also from Apple.

The core is a radix-4 fast Fourier transform algorithm in assembly language (it's been beautiful learning to program in assembly). For the analysis of input data (obtaining formants, etc.), I used linear prediction, which allowed me to obtain the 5 formants of vowels with fairly good accuracy. Some back closed vowel sounds are challenging for me. It works better with female voices, but the program struggles to detect the low-frequency formants in male voices.

I used an iPhone 4 and, for the core of the algorithm, Apple's Accelerate Framework. I followed Cedrick Collomb's explanation of the Burg algorithm and also studied the source code of the voice analysis software Praat for certain techniques.

I combine formants F1 and F2 to obtain the results. I used the average values for both formants as provided by J. C. Catford in A Practical Introduction to Phonetics to locate the vowels.

I tested the application on the street, and it proves to be quite resilient to noise, even traffic noise.
