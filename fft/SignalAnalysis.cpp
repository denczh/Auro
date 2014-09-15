/*
    File: SignalAnalysis.cpp
*/

#import <Accelerate/Accelerate.h>
#import <stdlib.h>
#import <stdio.h>
#import <string.h>
#import <math.h>
#import "SignalAnalysis.h"
#import "Types.h"

//TEMP
static int count;

struct SIGNAL_ANALYSIS {
    DSPDoubleSplitComplex input;            // Input signal.
    DSPDoubleSplitComplex output;           // Downsampled signal.
	int     size;                           // Size of imput signal.
	double *window;                         // Weighting window.
};

H_SIGNAL_ANALYSIS SignalAnalysisCreate(int32_t size) {
    H_SIGNAL_ANALYSIS p = (SIGNAL_ANALYSIS *)malloc(sizeof(SIGNAL_ANALYSIS));
    
    //TEMP
    count = 0;
    
	if(p) {
        p->size = size;
        int sizeDown = size/DFACTOR;      // Windowing is applied after downsampling by a factor DFACTOR
		p->window = (double *)malloc (sizeof (double)*sizeDown);
        
        /* Hamming window */
        /*
		for(int i = 0; i < size/2; i++) {
			double w = 0.53836 - 0.46164*cosf(2.0*M_PI*i/(double)(sizeDown - 1));
			p->window[i]                = w;
			p->window[sizeDown - i - 1] = w;
		}
         */
        /* Gaussian window */
        double imid = 0.5*(sizeDown + 1), edge = exp (-12.0);
        for (int i = 0; i < sizeDown; i++)
            p->window[i] = (exp (-48.0*(i - imid)*(i - imid)/(sizeDown + 1)/(sizeDown + 1)) - edge)/(1 - edge);
        
        p->input.imagp = (double *)calloc (size, sizeof (double));
        p->input.realp = (double *)malloc (size*sizeof (double));
        p->output.imagp = (double *)calloc (size/DFACTOR, sizeof (double));
        p->output.realp = (double *)malloc ((size/DFACTOR)*sizeof (double));
	}
	return p;
}

void SignalAnalysisDestroy(H_SIGNAL_ANALYSIS p) {
	if(p) {
		if (p->window)
            free(p->window);
        if (p->input.realp)
            free(p->input.realp);
        if (p->input.imagp)
            free(p->input.imagp);
        if (p->output.realp)
            free(p->output.realp);
        if (p->output.imagp)
            free(p->output.imagp);
		free(p);
	}
}

void findLPCoefs (const double x[], double outcoef[]) {
    int N = NSAMPLES/DFACTOR - 1;
    int m = DEG;

    double Ak[DEG + 1] = {0.0};                         // Initialize Ak
    Ak[0] = 1.0;
    double f[NSAMPLES/DFACTOR] = {0.0};                 // Initialize f and b
    double b[NSAMPLES/DFACTOR] = {0.0};
    
    memcpy(f, x, (NSAMPLES/DFACTOR)*sizeof(double));
    memcpy(b, x, (NSAMPLES/DFACTOR)*sizeof(double));
    double Dk = 0.0;                                    // Initialize Dk
    for (int j = 0; j <= N; j++)
        Dk += 2.0*f[j]*f[j];
    Dk -= f[0]*f[0] + b[N]*b[N];
    for (int k = 0; k < m; k++) {                       // Burg recursion
        double lambda = 0.0;                                // Compute lambda
        for (int n = 0; n <= N - k - 1; n++)
            lambda += f[n + k + 1]*b[n];
        lambda *= -2.0/Dk;
        for (int n = 0; n <= (k + 1)/2; n++) {              // Update Ak
            double t1 = Ak[n] + lambda*Ak[k + 1 - n];
            double t2 = Ak[k + 1 - n] + lambda*Ak[n];
            Ak[n] = t1;
            Ak[k + 1 - n] = t2;
        }
        for (int n = 0; n <= N - k - 1; n++) {              // Update f and b
            double t1 = f[n + k + 1] + lambda*b[n];
            double t2 = b[n] + lambda*f[n + k + 1];
            f[n + k + 1] = t1;
            b[n] = t2;
        }
        // Update Dk
        Dk = (1.0 - lambda*lambda)*Dk - f[k + 1]*f[k + 1] - b[N - k - 1]*b[N - k - 1];
    }
    memcpy(outcoef, &Ak[1], m*sizeof(double));            // Assign coefficients
}

int findRoots (const double *pol, int deg, __CLPK_doublecomplex *outr) {
    double leadcoef = pol[deg];
    long n = deg;       // Order of the Hessenberg matrix H. All coefs are divided by Cn, so Cn is 1.0 and is not stored.
    long nn = n*n;      // Number of matrix elements.
    // Allocate storage for Hessenberg matrix (n * n) plus real and imaginary parts of eigenvalues wr and wi (vectors with n els.)
    double H[nn + 2*n];
    memset(H, 0, (nn + 2*n)*sizeof(double));
    double *wr = &H[nn - 1];      // wr points to column storing real part of eigenvalues in matrix H.
    double *wi = &H[nn + n - 1];  // wi points to column storing imag part of eigenvalues in matrix H.
    // Fill the upper Hessenberg matrix (storage is Fortran)
    // C: [i][j] -> Fortran: (j - 1)*n + i
    for(long i = 1; i <= n; i++) {
        H[(i - 1)*n] = -(pol[deg - i]/leadcoef);    // Each coefficient (sign changed) is stored in the first element in each column
        if(i < n)                                   //   (polynomial is normalized)
            H[(i - 1)*n + i] = 1;                   // First element below the diagonal is 1
    }
    char job = 'E';         // Compute eigenvalues only.
    char compz = 'N';       // No Schur vectors are computed.
    long ilo = 1;           // Default value.
    long ihi = n;           // Default value.
    long ldh = n;           // The leading dimension of the array H.
    long ldz = n;           // The leading dimension of the array Z.
    long lwork = -1;        // The dimension of the array work. If lwork = -1, then dhseqr_ does a workspace query. In this case,
    // dhseqr_ checks the input parameters and estimates the optimal workspace size for the given values
    // of n, ilo and ihi. The estimate is returned in work[1].
    double *Z = 0;          // Array of dimensions (ldz, n). If compz == 'N', Z is not referenced.
    double wt[1];           // If info == 0, wt[0] returns an estimate of the optimal value for lwork.
    long info;              // info == 0: successful exit.
    // Find out the working storage needed.
    dhseqr_(&job, &compz, &n, &ilo, &ihi, H, &ldh, &wr[1], &wi[1], Z, &ldz, wt, &lwork, &info);
    if (info < 0)
        printf("Programming error. Argument %ld in dhseqr_ has illegal value.\n", info);
    lwork = wt[0];
    double work[lwork + 1];
    // Find eigenvalues.
    dhseqr_(&job, &compz, &n, &ilo, &ihi, H, &ldh, &wr[1], &wi[1], Z, &ldz, &work[1], &lwork, &info);
    long nroots = n;
    if (info > 0) {
        nroots -= info;
        if (nroots < 1) {
            printf("No roots found.\n");
            return -1;
        }
        // If info == i, dhseqr_ failed to compute all of the eigenvalues. Elements i+1..n of wr and wi contain those eigenvalues
        // which have been successfully computed. This case is rare.
        else {
            printf("Calculated only %ld roots.\n", nroots);
            // Pending: Assign the roots.
            return nroots;
        }
    } else if (info < 0) {
        printf("\nProgramming error. Argument %ld in dhseqr_ has illegal value.\n", -info);
        return -1;
    }
    // Root cleaning: dhseqr_ returns the imaginary part of the first root wrong, the rest is ok. If this is a complex root, its
    // conjugate pair is the last root. If it's real, we assign 0.0 to its imaginary part.
    double ri;
    int nrealroots = 0;
    for (int j = 0; j < nroots; j++) {
        ri = wi[j];
        if (ri == 0.0)
            nrealroots++;
        outr[j].r = wr[j];
        outr[j].i = ri;
    }
    outr[0].i = nrealroots % 2 == 0 ? -ri : 0.0;
    return nroots;
}

double complexMod (__CLPK_doublecomplex c) {
	double a, b, t;
	a = fabs (c.r);
	b = fabs (c.i);
	if (a == 0.0)
        return b;
	if (b == 0.0)
        return a;
	if (a > b) {
		t = b/a;
		return a*sqrt (1.0 + t*t);
	} else {
		t = a/b;
		return b*sqrt (1.0 + t*t);
	}
}

void complexDiv (__CLPK_doublecomplex a, __CLPK_doublecomplex b, __CLPK_doublecomplex *c) {
	double t, d;
	if (fabs (b.r) >= fabs (b.i)) {
		t = b.i/b.r;
		d = b.r + t*b.i;
		(*c).r = (a.r + t*a.i)/d;
		(*c).i = (a.i - t*a.r)/d;
	} else {
		t = b.r/b.i;
		d = b.i + t*b.r;
		(*c).r = (a.r*t + a.i)/d;
		(*c).i = (a.i*t - a.r)/d;
	}
}

void fixIntoUnitCircle (__CLPK_doublecomplex ioroots[], int nroots) {
	for (int j = 0; j < nroots; j++)
		if (complexMod (ioroots[j]) > 1.0) {
            __CLPK_doublecomplex a = {1.0, 0.0}, b, c;
            b.r = ioroots[j].r;
            b.i = -ioroots[j].i;        // Conjugate of the root
			complexDiv (a, b, &c);      // c = a/b
            ioroots[j].r = c.r;         // Return c
            ioroots[j].i = c.i;
        }
}

void SignalAnalysisProcess(H_SIGNAL_ANALYSIS p, const DATATYPE *inSignal, double *outFormants, bool in_dB)
{
	if(p)
	{
        int size = p->size;

        // Converting, filtering and downsampling.
        for (int i = 0; i < size; i++)
            p->input.realp[i] = (float)inSignal[i]/16777216.0;         // From fixed 8.24 to float.
        size /= DFACTOR;
        double freq = SAMPRATE/DFACTOR;
        vDSP_zrdesampD(&(p->input), DFACTOR, filter, &(p->output), size, FTAPS);
        
        // Apply pre-emphasis.
        double preEmphasis = exp (-2.0*M_PI*PREEM/freq);
        for (int i = size; i >= 2; i--)
            p->output.realp[i] -= preEmphasis*p->output.realp[i - 1];

        // Apply weigthing window
		for(int i = 0; i < size; i++)
            p->output.realp[i] *= p->window[i];
        
        // Get linear prediction coefficients.
        double coefs[DEG];                              // The number of the coefficients we need equals the degree of the polynomial
        findLPCoefs (p->output.realp, coefs);

        // Convert LP coefficients to polynomial.
        double pol[DEG + 1];
        for (int i = 0; i < DEG; i++)
            pol[i] = coefs[DEG - i - 1];                            // We put the coefficients in the correct order.
        pol[DEG] = 1.0;                                             // This 1 appears in the all-pole filter polynomial 1 - ∑aᵢ*z⁻ⁱ.
        
        // Find the roots of the polynomial.
        __CLPK_doublecomplex roots[DEG];
        int nroots = findRoots (pol, DEG, roots);
        if (nroots > 0) {
            fixIntoUnitCircle (roots, nroots);                      // Roots to unit circle.

            // Count and fill the formants. The roots come in conjugate pairs, so we need only count those above the real axis.
            double tempF1 = 0.0, tempF2 = 0.0;
            for (int j = 0; j < nroots; j++)
                if (roots[j].i >= 0) {
                    double f = fabs (atan2 (roots[j].i, roots[j].r))*MAXFREQ/M_PI;
                    if (f >= SAFETY && f <= MAXFREQ - SAFETY) {
                        if (tempF1 == 0.0)
                            tempF1 = f;
                        else if (f < tempF1) {
                            tempF2 = tempF1;
                            tempF1 = f;
                        }
                        else if (tempF2 == 0.0 || f < tempF2)
                            tempF2 = f;
                    }
                }
            outFormants[F1] = tempF1;
            outFormants[F2] = tempF2;
        }
	}
}