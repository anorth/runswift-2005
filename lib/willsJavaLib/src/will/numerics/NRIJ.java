package will.numerics;

import pointer.*;

public final class NRIJ {

public static final class KSTwoRetVal {
	public float d;
	public float prob;
}

public static float bico(int n, int k)
{
	return (float)Math.floor(0.5+Math.exp(factln(n)-factln(k)-factln(n-k)));
}

private static float factlnTable[] = null;
private static final int factlnTableSize = 101;

/**
 * Returns ln(n!)
 */

public static float factln(int n)
{
	if (factlnTable == null) {
		factlnTable = new float[factlnTableSize];
		for (int i=0; i<factlnTableSize; i++)
			factlnTable[i] = -1f;
	}

	if (n < 0) throw new NRIJRuntimeException("Negative factorial in routine factln");
	if (n <= 1) return 0.0f;
	if (n < factlnTableSize) return (factlnTable[n] >= 0) ? factlnTable[n] : (factlnTable[n]=gammln(n+1.0f));
	else return gammln(n+1.0f);
}

public static float gammln(float xx)
{
	double x,y,tmp,ser;
	double cof[] = {76.18009172947146,-86.50532032941677,
		24.01409824083091,-1.231739572450155,
		0.1208650973866179e-2,-0.5395239384953e-5};
	int j;

	y=x=xx;
	tmp=x+5.5;
	tmp -= (x+0.5)*Math.log(tmp);
	ser=1.000000000190015;
	for (j=0;j<=5;j++) ser += cof[j]/++y;
	return (float)(-tmp+Math.log(2.5066282746310005*ser/x));
}

public static float gammp(float a, float x)
{
	PFloat gamser = new PFloat(0), gammcf = new PFloat(0), gln = new PFloat(0);

	if (x < 0.0 || a <= 0.0) throw new NRIJRuntimeException("Invalid arguments in routine gammp");
	if (x < (a+1.0)) {
		gser(gamser,a,x,gln);
		return gamser.val;
	} else {
		gcf(gammcf,a,x,gln);
		return 1.0f-gammcf.val;
	}
}

public static void gcf(PFloat gammcf, float a, float x, PFloat gln)
{
	int ITMAX = 100;
	float EPS = 3.0e-7f;
	float FPMIN = 1.0e-30f;

	int i;
	float an,b,c,d,del,h;

	gln.val=gammln(a);
	b=x+1.0f-a;
	c=1.0f/FPMIN;
	d=1.0f/b;
	h=d;
	for (i=1;i<=ITMAX;i++) {
		an = -i*(i-a);
		b += 2.0f;
		d=an*d+b;
		if (Math.abs(d) < FPMIN) d=FPMIN;
		c=b+an/c;
		if (Math.abs(c) < FPMIN) c=FPMIN;
		d=1.0f/d;
		del=d*c;
		h *= del;
		if (Math.abs(del-1.0) < EPS) break;
	}
	if (i > ITMAX) throw new NRIJRuntimeException("a too large, ITMAX too small in gcf");
	gammcf.val=(float)(Math.exp(-x+a*Math.log(x)-(gln.val))*h);
}

public static void gser(PFloat gamser, float a, float x, PFloat gln)
{
	int ITMAX = 100;
	float EPS = 3.0e-7f;

	int n;
	float sum,del,ap;

	gln.val=gammln(a);
	if (x <= 0.0) {
		if (x < 0.0) throw new NRIJRuntimeException("x less than 0 in routine gser");
		gamser.val=0.0f;
		return;
	} else {
		ap=a;
		del=sum=1.0f/a;
		for (n=1;n<=ITMAX;n++) {
			++ap;
			del *= x/ap;
			sum += del;
			if (Math.abs(del) < Math.abs(sum)*EPS) {
				gamser.val=(float)(sum*Math.exp(-x+a*Math.log(x)-(gln.val)));
				return;
			}
		}
		throw new NRIJRuntimeException("a too large, ITMAX too small in routine gser");
	}
}

public static KSTwoRetVal kstwo(float data1[], int n1, float data2[], int n2)
{
	int j1=1,j2=1;
	float d1,d2,dt,en1,en2,en,fn1=0,fn2=0;
	KSTwoRetVal result = new KSTwoRetVal();
	float d;

	sort(n1,data1);
	sort(n2,data2);
	en1=n1;
	en2=n2;
	d=0;
	while (j1 <= n1 && j2 <= n2) {
		if ((d1=data1[j1]) <= (d2=data2[j2])) fn1=j1++/en1;
		if (d2 <= d1) fn2=j2++/en2;
		if ((dt=Math.abs(fn2-fn1)) > d) d=dt;
	}
	en=(float)Math.sqrt(en1*en2/(en1+en2));
	result.d = d;
	result.prob=probks((en+0.12f+0.11f/en)*(d));
	return result;
}

public static float probks(float alam)
{
	double EPS1 = 0.001;
	double EPS2 = 1.0e-8;
	int j;
	double a2,fac=2.0f,sum=0.0f,term,termbf=0.0f;

	a2 = -2.0*alam*alam;
	for (j=1;j<=100;j++) {
		term=fac*Math.exp(a2*j*j);
		sum += term;
		if (Math.abs(term) <= EPS1*termbf || Math.abs(term) <= EPS2*sum) return (float)sum;
		fac = -fac;
		termbf=Math.abs(term);
	}
	return 1.0f;
}

// #define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;

public static void sort(int n, float arr[])
{
	int i,ir=n,j,k,l=1;
	int jstack=0,istack[];
	float a,temp;
	int NSTACK = 50;
	int M = 7;

	//istack=ivector(1,NSTACK);
	istack=new int[NSTACK+1];
	for (;;) {
		if (ir-l < M) {
			for (j=l+1;j<=ir;j++) {
				a=arr[j];
				for (i=j-1;i>=l;i--) {
					if (arr[i] <= a) break;
					arr[i+1]=arr[i];
				}
				arr[i+1]=a;
			}
			if (jstack == 0) break;
			ir=istack[jstack--];
			l=istack[jstack--];
		} else {
			k=(l+ir) >> 1;
			temp = arr[k]; arr[k] = arr[l+1]; arr[l+1] = temp;
			//SWAP(arr[k],arr[l+1])
			if (arr[l] > arr[ir]) {
				temp = arr[l]; arr[l] = arr[ir]; arr[ir] = temp;
				//SWAP(arr[l],arr[ir])
			}
			if (arr[l+1] > arr[ir]) {
				temp = arr[l+1]; arr[l+1] = arr[ir]; arr[ir] = temp;
				//SWAP(arr[l+1],arr[ir])
			}
			if (arr[l] > arr[l+1]) {
				temp = arr[l]; arr[l] = arr[l+1]; arr[l+1] = temp;
				//SWAP(arr[l],arr[l+1])
			}
			i=l+1;
			j=ir;
			a=arr[l+1];
			for (;;) {
				do i++; while (arr[i] < a);
				do j--; while (arr[j] > a);
				if (j < i) break;
				temp = arr[i]; arr[i] = arr[j]; arr[j] = temp;
				//SWAP(arr[i],arr[j]);
			}
			arr[l+1]=arr[j];
			arr[j]=a;
			jstack += 2;
			if (jstack > NSTACK) throw new NRIJRuntimeException("NSTACK too small in sort.");
			if (ir-i+1 >= j-l) {
				istack[jstack]=ir;
				istack[jstack-1]=i;
				ir=j-1;
			} else {
				istack[jstack]=j-1;
				istack[jstack-1]=l;
				l=i;
			}
		}
	}
}

}
