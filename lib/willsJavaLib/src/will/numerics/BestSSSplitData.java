package will.numerics;

public final class BestSSSplitData {
	public float splitVal;
	public float splitChi;

	public float bestSplitVal;
	public float bestSplitChi;
	public int bestSplitAttr;

	double []losum, losumsq;
	double []hisum, hisumsq;
	int []lon, hin;

	float vals[][];
	int lo;
	int hi;
	int valCol;
	int seperatorCol;
	int numSeparateGroups;
	double []sum;
	double []sumsq;
	int []groupSize;
	
	public BestSSSplitData(float vals[][], int lo, int hi, int valCol
		, int seperatorCol, int numSeparateGroups) {
		int i;
		
		sum = new double[numSeparateGroups];
		sumsq = new double[numSeparateGroups];

		losum = new double[numSeparateGroups];
		losumsq = new double[numSeparateGroups];
		hisum = new double[numSeparateGroups];
		hisumsq = new double[numSeparateGroups];

		lon = new int[numSeparateGroups];
		hin = new int[numSeparateGroups];
		groupSize = new int[numSeparateGroups];

		// calculate sums and sum of squares of errors
		for (i = lo; i <= hi; i++) {
			int dir = (int)vals[i][seperatorCol];
			sum[dir] += vals[i][valCol];
			sumsq[dir] += vals[i][valCol]*vals[i][valCol];
			groupSize[dir]++;
		}

		this.vals = vals;
		this.lo = lo;
		this.hi = hi;
		this.valCol = valCol;
		this.seperatorCol = seperatorCol;
		this.numSeparateGroups = numSeparateGroups;
		bestSplitVal = 0;
		bestSplitChi = 1.1f;
		bestSplitAttr = -1;
	}
}

