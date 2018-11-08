package will.numerics;

import will.algs.*;

/**
	vals contains a 2D array of data.  Each row is a datapoint.
	The data in that array from lo to hi will be searched for the best split
	based on the attribute in column attrCol.
	The best split is defined as that split which maximises the statistical difference
	of the data on each side of the split according to the Kolmogorov-Smirnov test.
	The data are grouped into numSeparateGroups groups numbered from 0 to numSeparateGroups-1.
	Column seperatorCol indicates which group a datapoint belongs to.  The distibutions are
	compared separately for each group and the differences combined.
	The rows may be rearranged in the array.
	The column spareCol is used by the algorithm.
*/

public final class BestKSSplitData {
	public float splitVal;
	public float splitProb;

	public float bestSplitVal;
	public float bestSplitProb;
	public int bestSplitAttr;

	float vals[][];
	int lo;
	int hi;
	int valCol;
	int seperatorCol;
	int spareCol;
	int numSeparateGroups;
	float ksData[][][];
	
	public BestKSSplitData(float vals[][], int lo, int hi, int valCol
		, int seperatorCol, int spareCol, int numSeparateGroups) {
		int i;
		
		// split the data into groups by action and put it in groupVals

		Sort.qsort(vals, lo, hi, new FloatArrayComparitor(seperatorCol));
		
		int [] groupCounts = new int[numSeparateGroups];

		for (i=lo; i<=hi; i++)
			groupCounts[(int)(vals[i][seperatorCol])]++;
		
		ksData = new float[numSeparateGroups][][];

		int startLoc = lo;

		for (i=0; i<numSeparateGroups; i++) {
			ksData[i] = new float[groupCounts[i]][];
			System.arraycopy(vals, startLoc, ksData[i], 0, groupCounts[i]);
			Sort.qsort(ksData[i], 0, groupCounts[i]-1, new FloatArrayComparitor(valCol));
			startLoc += groupCounts[i];
		}
		
		this.vals = vals;
		this.lo = lo;
		this.hi = hi;
		this.valCol = valCol;
		this.seperatorCol = seperatorCol;
		this.spareCol = spareCol;
		this.numSeparateGroups = numSeparateGroups;
		bestSplitVal = 0;
		bestSplitProb = 1.1f;
		bestSplitAttr = -1;
	}
}

