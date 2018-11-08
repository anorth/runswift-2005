package will.numerics;

/**
 * A Fast Fourier Transform class.  Based on both the 'Numerical Recipies in C' code and
 * an FFT class from http://www.neato.org/~ben/Fft.html.  Modified to use the WebSim matrix
 * class.
 */

public class FFT {

double SampleRate;
int NSamples;
int Power;

private int[] Permute;	// bit reversing permutation table
private double[] Sines;	// pre-computed table of sines

final boolean debug = false;

public FFT(int NSamples, double SampleRate) throws MatrixException {
	this.NSamples = NSamples;
	this.SampleRate = SampleRate;
	
	// Input data array length must be a power of two
	Power = (int)(Math.log((double)NSamples)/Math.log(2.0));
	if ((1 << Power) != NSamples) {
		throw new MatrixException("FFT size must be a power of 2");
	}

	Sines = new double[Power+1];
	int mmax = 1;
	for (int i = 0; i < Sines.length; i++) {
		Sines[i] = (double)Math.sin((double)Math.PI/mmax);
		mmax <<= 1;
	}

	// Build the bit reversal lookup table
	Permute = new int[NSamples];
	int result;
	for (int index = 0; index < NSamples; index++) {
		result = 0;
		for (int loop = 0; loop < Power; loop++) {
			if ((index & (1 << loop)) != 0) {
				result |= 1 << (Power - 1 - loop);
			}
		}
		Permute[index] = result;
	}
}

protected void Compute(Matrix real, Matrix imag, int iSign) throws MatrixException {
	if (real.nCols() != 1)
		throw new MatrixException("Real Matrix must be a vector");
	if (imag.nCols() != 1)
		throw new MatrixException("Imaginary Matrix must be a vector");
	if (real.nRows() != NSamples)
		throw new MatrixException("Incorrect number of rows in Real matrix");
	if (imag.nRows() != NSamples)
		throw new MatrixException("Incorrect number of rows in Imaginary matrix");
	
	// do the permute
	for (int i=0; i<NSamples; i++) {
		int p = Permute[i];
		
		if (p > i) {	// only swap half, otherwise we'll swap them back again...
			if (debug)
				System.out.println("Swapping " + i + ", " + p);
			
			double temp;

			temp = real.val(i);	// swap real components
			real.set(i, real.val(p));
			real.set(p, temp);
			
			temp = imag.val(i);	// swap imaginary components
			imag.set(i, imag.val(p));
			imag.set(p, temp);
		}
	}
	
	int mmax = 1;
	
	for (int p=0; p < Power; p++) {
		if (debug)
			System.out.println("mmax: " + mmax);
		int istep=mmax << 1;
		double wtemp=iSign*Sines[p+1];
		if (debug)
			System.out.println("wtemp: " + wtemp);
		double wpr = -2.0*wtemp*wtemp;
		double wpi=iSign*Sines[p];
		if (debug)
			System.out.println("wpi: " + wpi);
		double wr=1.0;
		double wi=0.0;
		for (int m=0;m<mmax;m+=1) {
			if (debug)
				System.out.println("m: " + m);
			for (int i=m;i<NSamples;i+=istep) {
				int j=i+mmax;
				if (debug)
					System.out.println("i: " + i + " j: " + j);
				double tempr=wr*real.val(j)-wi*imag.val(j);
				double tempi=wr*imag.val(j)+wi*real.val(j);
				real.set(j, real.val(i) - tempr);
				imag.set(j, imag.val(i) - tempi);
				real.set(i, real.val(i) + tempr);
				imag.set(i, imag.val(i) + tempi);
			}
			wtemp=wr;
			wr=wr*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}
}

/**
  * This is a direct translation of the Numerical Recipies in C routine four1.  It is
  * not as efficient in Java as in C as it uses 1 based arrays and interleaves the
  * real and imaginary components.
  */

protected void NRICCompute(Matrix real, Matrix imag, int isign) throws MatrixException {
	int n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	double tempr,tempi;

	if (real.nCols() != 1)
		throw new MatrixException("Real Matrix must be a vector");
	if (imag.nCols() != 1)
		throw new MatrixException("Imaginary Matrix must be a vector");
	if (real.nRows() != NSamples)
		throw new MatrixException("Incorrect number of rows in Real matrix");
	if (imag.nRows() != NSamples)
		throw new MatrixException("Incorrect number of rows in Imaginary matrix");
	
	n=NSamples << 1;
	j=1;
	for (i=1;i<n;i+=2) {
		if (j > i) {
			double temp;
			
			if (debug)
				System.out.println("Swapping " + (int)(i/2) + ", " + (int)(j/2));
			
			temp = real.val(j/2);	// swap real components
			real.set(j/2, real.val(i/2));
			real.set(i/2, temp);
			
			temp = imag.val(j/2);	// swap imaginary components
			imag.set(j/2, imag.val(i/2));
			imag.set(i/2, temp);
		}
		m=n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax=2;
	while (n > mmax) {
		istep=mmax << 1;
		if (debug)
			System.out.println("mmax: " + mmax);
		theta=isign*(2*Math.PI/mmax);
		wtemp=Math.sin(0.5*theta);
		if (debug)
			System.out.println("wtemp: " + wtemp);
		wpr = -2.0*wtemp*wtemp;
		wpi=Math.sin(theta);
		if (debug)
			System.out.println("wpi: " + wpi);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) {
			if (debug)
				System.out.println("m: " + m);
			for (i=m;i<=n;i+=istep) {
				j=i+mmax;
				if (debug)
					System.out.println("i: " + i + " j: " + j + " i': " + (i/2) + " j': " + (j/2));
				tempr=wr*real.val(j/2) - wi*imag.val(j/2);
				tempi=wr*imag.val(j/2) + wi*real.val(j/2);
				real.set(j/2, real.val(i/2) - tempr);
				imag.set(j/2, imag.val(i/2) - tempi);
				real.set(i/2, real.val(i/2) + tempr);
				imag.set(i/2, imag.val(i/2) + tempi);
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}
}

public void ComputeForward(Matrix m) throws MatrixException {
	Matrix real, imag;
	
	real = m.submatrix(0,0,m.nRows(),1);
	imag = m.submatrix(0,1,m.nRows(),1);
	
	ComputeForward(real, imag);
}

public void ComputeForward(Matrix real, Matrix imag) throws MatrixException {
	Compute(real, imag, 1);
}

public void ComputeBackward(Matrix m) throws MatrixException {
	Matrix real, imag;
	
	real = m.submatrix(0,0,m.nRows(),1);
	imag = m.submatrix(0,1,m.nRows(),1);
	
	ComputeBackward(real, imag);
}

public void ComputeBackward(Matrix real, Matrix imag) throws MatrixException {
	Compute(real, imag, -1);
	real.mult(1.0/NSamples);
	imag.mult(1.0/NSamples);
}

void testFFT(Matrix m) throws MatrixException {
	Matrix AReal = m.duplicate();
	Matrix BReal = m.duplicate();
	Matrix AImag = new Matrix(m.nRows(), 1);
	Matrix BImag = new Matrix(m.nRows(), 1);
	m = null;

	System.out.println("NRIC compute");	
	NRICCompute(BReal, BImag, 1);
	System.out.println("My compute");
	Compute(AReal, AImag, 1);
	
	if (AReal.equalEls(BReal))
		System.out.println("Real: Equal");
	else
		System.out.println("Real: Not Equal");

	if (AImag.equalEls(BImag))
		System.out.println("Imag: Equal");
	else
		System.out.println("Imag: Not Equal");
}

}
