package will.numerics;

public final class Stats {

public static void findBestKSSplit(BestKSSplitData data, int attrCol) {
	float splitVal = 0;
	float prob, minprob = 1.1f;
	int i;
	int smallval;
	boolean []needsUpdate = new boolean[data.numSeparateGroups];
	float []thisProb = new float[data.numSeparateGroups];
	int []m = new int[data.numSeparateGroups], n = new int[data.numSeparateGroups];

	// sort the data according to that attribute

	will.algs.Sort.qsort(data.vals, data.lo, data.hi, new will.algs.FloatArrayComparitor(attrCol));

	// mark eveything as on the upper side of the split

	for (i = data.lo; i <= data.hi; i++)
		data.vals[i][data.spareCol] = 1;

	// set up initial state for the individual tests

	for (i=0; i<data.numSeparateGroups; i++) {
		needsUpdate[i] = true;
		m[i] = 0;
		n[i] = data.ksData[i].length;
	}

	// loop through possible split values

	for (smallval = data.lo; smallval < data.hi; smallval++) {
		// mark the value as being on the other side of the split
		data.vals[smallval][data.spareCol] = 0;
		// remember what needs to be recalculated
		int thisGroup = (int)data.vals[smallval][data.seperatorCol];
		needsUpdate[thisGroup] = true;
		m[thisGroup]++;
		n[thisGroup]--;
		// if two points are equal in this attr don't try to split between them
		if (data.vals[smallval][attrCol] == data.vals[smallval+1][attrCol])
			continue;

		// update the separate probabilities and multiply them together
		prob = 1;
		for (i=0; i<data.numSeparateGroups; i++) {
			if (needsUpdate[i]) {
				needsUpdate[i] = false;
				thisProb[i] = will.numerics.Stats.KolmogorovSmirnov(data.ksData[i], m[i], n[i], data.valCol, data.spareCol);
			}
			prob *= thisProb[i];
		}

		// remember the best split
		if (prob < minprob) {
			splitVal = (data.vals[smallval][attrCol] + data.vals[smallval+1][attrCol])/2.0f;	// split pt between neighbouring pts.
			minprob = prob;
		}
	}

	data.splitVal = splitVal;
	data.splitProb = minprob;

	if (data.splitProb < data.bestSplitProb) {
		data.bestSplitVal = splitVal;
		data.bestSplitProb = minprob;
		data.bestSplitAttr = attrCol;
	}
}

public static void findBestSSSplit(BestSSSplitData data, int attrCol) {
	double lochi, hichi;
	double minchi = 1;
	float splitVal = 0;
	int j;
	int smallval;
	double chi;
	
	if ((data.hi-data.lo+1)<2)	// no point even thinking about splitting...
		return;

	System.arraycopy(data.sum, 0, data.hisum, 0, data.numSeparateGroups);
	System.arraycopy(data.sumsq, 0, data.hisumsq, 0, data.numSeparateGroups);

	will.algs.Sort.qsort(data.vals, data.lo, data.hi, new will.algs.FloatArrayComparitor(attrCol));

	for (j=0; j<data.numSeparateGroups; j++) {
		data.lon[j] = 0;
		data.hin[j] = data.groupSize[j];
	}

	for (smallval = data.lo; smallval < data.hi; smallval++) {
		double mean;
		int dir = (int)data.vals[smallval][data.seperatorCol];

		data.lon[dir]++;
		data.hin[dir]--;
		data.losum[dir] += data.vals[smallval][data.valCol];
		data.hisum[dir] -= data.vals[smallval][data.valCol];
		data.losumsq[dir] += data.vals[smallval][data.valCol]*data.vals[smallval][data.valCol];
		data.hisumsq[dir] -= data.vals[smallval][data.valCol]*data.vals[smallval][data.valCol];

		if (data.vals[smallval][attrCol] == data.vals[smallval+1][attrCol])	// if two points are equal in this attr don't try to split between them
			continue;

		chi = 0;

		for (j=0; j<data.numSeparateGroups; j++) {
			mean = data.losum[j]/data.lon[j];
			lochi = data.losumsq[j]/data.lon[j] - mean*mean;

			mean = data.hisum[j]/data.hin[j];
			hichi = data.hisumsq[j]/data.hin[j] - mean*mean;

			chi += lochi*data.lon[j] + hichi*data.hin[j] - data.sumsq[j];
		}

		if (chi < minchi) {
			splitVal = (data.vals[smallval][attrCol] + data.vals[smallval+1][attrCol])/2.0f;	// split pt between neighbouring pts.
			minchi = chi;
		}
	}

	data.splitVal = splitVal;
	data.splitChi = (float)minchi;

	if (data.splitChi < data.bestSplitChi) {
		data.bestSplitVal = splitVal;
		data.bestSplitChi = (float)minchi;
		data.bestSplitAttr = attrCol;
	}
}

static final double ROOT_2_PI = Math.sqrt(2.0 * Math.PI);

public static double Normal(double x)
{
        return Math.exp(-0.5 * x * x) / ROOT_2_PI;
}

public static double Normal(double mean, double stdDev, double x)
{
        return Normal((x-mean)/stdDev)/stdDev;
}

/*
rickr's Matlab code for cumulative normal

function p = alnum(x)
% compute upper tail integral for univariate standard normal.
%

y = x .* x .* 0.5;
i = find(abs(x) > 1.28)';
p = zeros(length(x(:)), 1);
j = length(i);
if (j ~= 0)
  p(i) = 0.398942280385 .* exp(-y(i)) ./ (x(i) - 3.8052e-8 + 1.00000615302 ./ (x
(i) + 3.98064794e-4 + 1.98615381364 ./ (x(i) - 0.151679116635 + 5.29330324926 ./
 (x(i) + 4.8385912808 - 15.1508972451 ./ (x(i) + 0.742380924027 + 30.789933034 .
/ (x(i) + 3.99019417011))))));
end

i = find(abs(x) <= 1.28)';
if (length(i) ~= 0)
  p(i) = 0.5 - x(i) .* (0.398942280444 - 0.399903438504 .* y(i) ./ (y(i) + 5.758
85480458 - 29.8213557808 ./ (y(i) + 2.62433121679 + 48.6959930692 ./ (y(i) + 5.9
2885724438))));
end
*/

public static double cumNorm(double mean, double stdDev, double x) {
	return cumNorm((x-mean)/stdDev);
}

/*
public static double cumNorm(double x) {
// compute upper tail integral for univariate standard normal.

	double y = x * x * 0.5;
	double p;

	if (Math.abs(x) > 1.28) {
		p = 0.398942280385 * Math.exp(-y) /
			(x - 3.8052e-8 + 1.00000615302 /
			(x + 3.98064794e-4 + 1.98615381364 /
			(x - 0.151679116635 + 5.29330324926 /
			(x + 4.8385912808 - 15.1508972451 /
			(x + 0.742380924027 + 30.789933034 /
			(x + 3.99019417011))))));
	} else {
		p = 0.5 - x * (0.398942280444 - 0.399903438504 * y /
		(y + 5.75885480458 - 29.8213557808 /
		(y + 2.62433121679 + 48.6959930692 /
		(y + 5.92885724438))));
	}
	
	return p;
}
*/

// here's sippy's cumNorm code
public static double cumNorm(double x)
{
        if (x >= 0)
        {
                double k = 1.0 / (1.0 + 0.2316419 * x);
                return 1.0 - Normal(x) *
                                (0.31938153 * k -
                                 0.356563782 * k * k +
                                 1.781477937 * k * k * k -
                                 1.821255978 * k * k * k * k +
                                 1.330274429 * k * k * k * k * k);
        }

        return (1.0 - cumNorm(-1.0 * x));
}

// returns the probability that two distributions have equal medians
// given the number of items in each distribution and the sums of the ranks
// in the total ordering (1, 2, ...., N) for each distribution.
public static float wilcoxonRankSum(int m, int n, float mSum, float nSum) {

	if ((n<6) || (m<6)) {	// lower limit on normal approximation
		return 1;
	}
	
	double mean = m*(m+n+1)/2.0;
	double stdDev = Math.sqrt(m*n*(m+n+1)/12.0);
	
	double probLess;
	
	probLess = cumNorm(mean, stdDev, mSum);
	
	if (mSum > mean)
		probLess = (1-probLess);

	return (float) probLess*2;
}

// the Wilcoxon signed rank test for comparing two paired distributions
public static float wilcoxonSignedRank(float []A, float []B) {
	int i, j;
	int num = A.length;
	if (A.length != B.length) {
		throw new will.UnexpectedException();
	}
	
	Float [] diff = new Float[num];
	int count = 0;
	for (i=0; i<num; i++) {
		float thisDiff = A[i] - B[i];
		if (thisDiff != 0) {
			diff[count] = new Float(thisDiff);
			count++;
		}
	}
	
	will.algs.Sort.qsort(diff, 0, count-1, new will.algs.AbsoluteFloatComparitor());
	
	float[] sgndRnk = new float[count];
	
	for (i=0; i<count; i=j) {
		float ival = Math.abs(diff[i].floatValue());
		float avg = i;
		for (j=i+1; ((j<count) && (Math.abs(diff[j].floatValue()) == ival)); j++) {
			avg += j;
		}
		avg /= (j-i);
		for (int k=i; k<j; k++) {
			if (diff[k].floatValue() < 0)
				sgndRnk[k] = -avg;
			else
				sgndRnk[k] = avg;
		}
	}
	
	int Wpos = 0;
	int Wneg = 0;
	
	for (i=0; i<count; i++) {
		if (sgndRnk[i] > 0) {
			Wpos += sgndRnk[i];
		} else {
			Wneg -= sgndRnk[i];
		}
	}
	
	if (count > 20) {
		float eWpos = count*(count+1)/4;
		float vWpos = count*(count+1)*(2*count+1)/24;
		
		throw new will.NotImplementedException();
	} else {
		final float []twoSidedOone = {0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 5, 7, 10, 13, 16, 20, 23,
			28, 32, 38, 43, 49, 55, 61, 68};
		final float []twoSidedOtwo = {0, 0, 0, 0, 0, 0, 0, 0, 2, 3, 5, 7, 10, 13, 16, 20, 24, 28,
			33, 38, 43, 49, 56, 62, 69, 77};
		final float []twoSidedOfive = {0, 0, 0, 0, 0, 0, 0, 2, 4, 6, 8, 11, 14, 17, 21, 25, 30, 35,
			40, 46, 52, 59, 66, 73, 81, 89};
		
		if (count <= 8)
			return (float)0.5;
		
		if ((Wpos < twoSidedOone[count]) || (Wneg < twoSidedOone[count]))
			return (float)0.01;
		else if ((Wpos < twoSidedOtwo[count]) || (Wneg < twoSidedOtwo[count]))
			return (float)0.02;
		else if ((Wpos < twoSidedOfive[count]) || (Wneg < twoSidedOfive[count]))
			return (float)0.05;
		else
			return (float)0.5;
	}
}

public static float KolmogorovSmirnov(float vals[][], int m, int n, int valcol, int mncol) {
	float prev;
	int NumA, NumB;
	float CumA = 0, CumB = 0;
	float Maxd = 0, d;
	boolean Achanged = false, Bchanged = false;
	NumA = NumB = 0;
	float en, en1, en2;
	
	en1 = m;
	en2 = n;
	
	for (int i=0; i<m+n; i++) {
		if (vals[i][mncol] == 0) {	// m
			NumA++;
			Achanged = true;
		} else {	// n
			NumB++;
			Bchanged = true;
		}
		if (i<m+n-1 && vals[i][valcol] > vals[i+1][valcol])
			throw new will.UnexpectedException("Data must be sorted by value");
		if (i<m+n-1 && vals[i][valcol] == vals[i+1][valcol])
			continue;
		if (Achanged) {
			CumA = NumA/en1;
			Achanged = false;
		}
		if (Bchanged) {
			CumB = NumB/en2;
			Bchanged = false;
		}
		d = Math.abs(CumA-CumB);
		if (d > Maxd) {
			Maxd = d;
		}
	}
	if ((NumA != m) || (NumB != n))
		throw new will.UnexpectedException("Class counts incorrect in K-S Test");
	
	en = (float)Math.sqrt(en1*en2/(en1+en2));
	
	if (en < 2.5)
		return 1;	// too little data to tell anything
	
	return will.numerics.NRIJ.probks((en+0.12f+0.11f/en)*Maxd);
}

public static float Kuiper(float vals[][], int m, int n, int valcol, int mncol) {
	float prev;
	int NumA, NumB;
	float CumA = 0, CumB = 0;
	float MaxdU = 0, MaxdD = 0, d;
	boolean Achanged = false, Bchanged = false;
	NumA = NumB = 0;
	float en, en1, en2;
	
	en1 = m;
	en2 = n;
	
	for (int i=0; i<m+n; i++) {
		if (vals[i][mncol] == 0) {	// m
			NumA++;
			Achanged = true;
		} else {	// n
			NumB++;
			Bchanged = true;
		}
		if (i<m+n-1 && vals[i][valcol] > vals[i+1][valcol])
			throw new will.UnexpectedException();
		if (i<m+n-1 && vals[i][valcol] == vals[i+1][valcol])
			continue;
		if (Achanged) {
			CumA = NumA/en1;
			Achanged = false;
		}
		if (Bchanged) {
			CumB = NumB/en2;
			Bchanged = false;
		}
		d = CumA-CumB;
		if ((d < 0) && (-d > MaxdD))
			MaxdD = -d;
		else if ((d > 0) && (d > MaxdU))
			MaxdU = d;
	}
	if ((NumA != m) || (NumB != n))
		throw new will.UnexpectedException();
	
	en = (float)Math.sqrt(en1*en2/(en1+en2));
	
	if (en < 2.5)
		return 1;	// too little data to tell anything
	
	return probKuiper((en+0.155f+0.24f/en)*(MaxdU+MaxdD));
}

public static float probKuiper(float alam) {
	double EPS1 = 0.001;
	double EPS2 = 1.0e-8;
	int j;
	double a2,fac=2.0f,sum=0.0f,term,termbf=0.0f;

	if (alam < 0.4)
		return 1;

	a2 = alam*alam;
	for (j=1;j<=100;j++) {
		double ja2 = 2*a2*j*j;
		term=(2*ja2-1)*Math.exp(-ja2);
		sum += term;
		if (Math.abs(term) <= EPS1*termbf || Math.abs(term) <= EPS2*sum)
			return (float)(2*sum);
		termbf=Math.abs(term);
	}
	return 1.0f;
}

public static float ChitoProb(float df, double chi) {

	if (df == 0) {
		if (chi == 0)
			return 0;
		else
			return 1;
	}

	float prob = will.numerics.NRIJ.gammp(df/2, (float)(chi/2));
	
	return prob;
}

// log*(x) = Log2(x)+log2(log2(x))+log2(log2(log2(x))) + ...
public static double logStar(double n) {
	double result = 0;
	double delta = n;
	
	while(delta > 1) {
		delta = log2(delta);
		result += delta;
	}
	
	return result;
}

/**
 * Calculate ln(n!/k[1]!k[2]!k[]!).
 */

public static float logMulti(int n, int[] k) {
	int sum = 0;
	double sumlnfact = 0;
	
	for (int i=0; i<k.length; i++) {
		sum += k[i];
		sumlnfact += will.numerics.NRIJ.factln(k[i]);
	}
	if (sum != n)
		throw new will.UnexpectedException("bad input in logMulti");
	
	sumlnfact = will.numerics.NRIJ.factln(n)-sumlnfact;
	return (float)(sumlnfact);
}

public static final double log2b10 = Math.log(2);

protected static final int logCacheSize = 4000;

/*
// do NOT use Double here as it is horribly innefficient at .equals
protected static java.util.Hashtable logCache = null;

protected static double cachedLog(double n) {
	if (logCache == null)
		logCache = new java.util.Hashtable(logCacheSize);
	if (logCache.size() >= logCacheSize) {
		// remove a random element
		int elt = (int)(logCacheSize*Math.random());
		java.util.Enumeration enum = logCache.keys();
		while(enum.hasMoreElements()) {
			Double thisKey = (Double)enum.nextElement();
			elt--;
			if (elt == 0) {
				logCache.remove(thisKey);
				break;
			}
		}
	}
	Double thisKey = new Double(n);
	Double result;
	if (!logCache.containsKey(thisKey)) {
		result = new Double(Math.log(n));
		logCache.put(thisKey, result);
	} else {
		result = (Double)logCache.get(thisKey);
	}
	return result.doubleValue();
}

*/

protected static double[] logCacheArray = null;

protected static double cachedLog(int n) {
	if ((n < 2) || (n > logCacheSize+1))
		return Math.log(n);
	if (logCacheArray == null) {
		logCacheArray = new double[logCacheSize];
	}
	double result = logCacheArray[n-2];
	if (result == 0) {
		result = logCacheArray[n-2] = Math.log(n);
	}
	
	return result;
}

public static double log2(int n) {
	return cachedLog(n)/log2b10;
}

public static double log2(double n) {
	return Math.log(n)/log2b10;
}

public static double log(int n) {
	return cachedLog(n);
}

public static double log(double n) {
	return Math.log(n);
}

/** sqrt(a^2 + b^2) without under/overflow. **/

public static double hypot(double a, double b) {
	double r;
	double absa = Math.abs(a);
	double absb = Math.abs(b);
	if (absa > absb) {
		r = b/a;
		r = absa*Math.sqrt(1+r*r);
	} else if (b != 0) {
		r = a/b;
		r = absb*Math.sqrt(1+r*r);
	} else {
		r = 0.0;
	}
	return r;
}

}
