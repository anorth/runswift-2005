package will.algs;

import java.util.*;

public final class Sort {

static final int InsertionSortLimit = 15;

public static boolean checkSort(Object[] array) {
	return checkSort(array, new HashCompare());
}

public static boolean checkSort(Object[] array, Comparator comparitor) {
	return checkSort(array, 0, array.length-1, comparitor);
}

public static boolean checkSort(Object[] array, int lo, int hi, Comparator comparitor) {
	int i;
	
	for (i=lo; i<hi; i++) {
		if (comparitor.compare(array[i], array[i+1]) > 0)
			return false;
	}
	
	return true;
}

public static void qsort(Object[] array) {
	qsort(array, new HashCompare());
}

public static void qsort(Object[] array, Comparator comparitor) {
	qsort(array, 0, array.length-1, comparitor);
}

public static void qsort(Object[] array, int lo, int hi, Comparator comparitor) {
	// qsortA(array, lo, hi, comparitor, new java.util.Random());	// standard qsort
	qsortB(array, lo, hi, comparitor, (int)(2*will.numerics.Stats.log2(hi-lo+1)));	// introspective qsort
}

// standard qsort
private static void qsortA(Object[] array, int lo, int hi, Comparator comparitor, java.util.Random myRand) {
	while ((hi - lo) > InsertionSortLimit) {
		int i, j, k, l;
		Object pivot;
		Object spare;	// used for swapping
		// find 2 random elements and the middle element - pivot is median
		// first random element is taken from first half of the array
		// second random element is taken from the second half of the array
		// if the array is already sorted this guarantees that the median is picked as pivot
		// incorporated both random and median of three pivot selection schemes
		// I have no idea if this is a good idea or not
		{
			int randRange = (hi-lo-1)/2;
			int randA = lo+(Math.abs(myRand.nextInt())%randRange);
			int randB = hi-(Math.abs(myRand.nextInt())%randRange);
			int mid = (lo+hi)/2;
			
			if (comparitor.compare(array[randA], array[randB]) < 0) {
				// randA < randB
				if (comparitor.compare(array[mid], array[randA]) < 0) {
					// mid < randA < randB
					pivot = array[randA];
				} else {
					// randA < randB && randA <= mid
					if (comparitor.compare(array[mid], array[randB]) < 0) {
						pivot = array[mid];	// randA <= mid < randB
					} else {
						pivot = array[randB];	// randA < randB <= mid
					}
				}
			} else {
				// randB >= randA
				if (comparitor.compare(array[mid], array[randA]) < 0) {
					// randB >= randA > mid
					pivot = array[randA];
				} else {
					//  randB >= randA && mid >= randA
					if (comparitor.compare(array[mid], array[randB]) < 0) {
						// randA <= mid < randB
						pivot = array[mid];
					} else {
						// randA <= randB < mid
						pivot = array[randB];
					}
				}
			}
		}	// pivot found
		
		// partition the elements
		// lo <= x < i:	equal to pivot
		// i <= x < j:	less than pivot
		// j <= x <= k:	unknown
		// k < x <= l:	greater than pivot
		// l < x <= hi:	equal to pivot
		// Note that its better to copy the equal elements to the ends and then have to
		// recopy them to the center than to 'tank-tread' them to the middle directly with lots
		// of copies
		{
			int comparison;
			i = j = lo;
			k = l = hi;
			
			while (j <= k) {
				// move lower boundary up
				while (j <= k) {
					comparison = comparitor.compare(array[j], pivot);
					if (comparison < 0) {
						// move unknown element into < region
						// just increment j
						j++;
					} else if (comparison == 0) {
						// move unkown element into lower = region
						// swap with < element at edge of = region
						// increment i and j
						spare = array[i];
						array[i] = array[j];
						array[j] = spare;
						i++;
						j++;
					} else
						break;
				}
				// move upper boundary down
				while (j <= k) {
					comparison = comparitor.compare(array[k], pivot);
					if (comparison > 0) {
						// move unknown element into > region
						// just decrement k
						k--;
					} else if (comparison == 0) {
						// move unkown element into upper = region
						// swap with > element at edge of = region
						// increment k and l
						spare = array[l];
						array[l] = array[k];
						array[k] = spare;
						k--;
						l--;
					} else
						break;
				}
				
				// need to swap blocking elements if any
				if (j < k) {
					spare = array[j];
					array[j] = array[k];
					array[k] = spare;
					j++;
					k--;
				} else if (j == k) {	// TODO: comment this out once finished testing
					throw new will.UnexpectedException();
				}
			}
		}
		// now swap equal elements back into the middle
		// at the end, lo <= x < j: less than pivot
		// k < x <= hi: greater than pivot
		{
			if (j > i) {	// if there are any < elements to swap with
				if ((i-lo) < (j - i)) {	// copy the smaller amount - don't tank tread here either
					while (i > lo) {
						i--;
						j--;
						spare = array[i];
						array[i] = array[j];
						array[j] = spare;
					}
				} else {
					int top = j;
					j = lo;
					while (i < top) {
						spare = array[j];
						array[j] = array[i];
						array[i] = spare;
						i++;
						j++;
					}
				}
			} else {
				// j <= i
				j = i = lo;
			}
			if (k < l) {	// if there are any > elements to swap with
				if ((hi-l) < (l-k)) {	// copy the smaller amount - don't tank tread here either
					while (l < hi) {
						l++;
						k++;
						spare = array[l];
						array[l] = array[k];
						array[k] = spare;
					}
				} else {
					int bot = k;
					k = hi;
					while (l > bot) {
						spare = array[k];
						array[k] = array[l];
						array[l] = spare;
						k--;
						l--;
					}
				}
			} else {
				// l <= k
				k = l = hi;
			}
		}
		
		// now sort two halves
		// do smaller one recursivly first
		// then iterate for the larger
		if ((j-lo) < (hi-k)) {
			// lower half is smaller
			qsortA(array, lo, j-1, comparitor, myRand);
			lo = k+1;
		} else {
			// upper half is smaller
			qsortA(array, k+1, hi, comparitor, myRand);
			hi = j-1;
		}
	}
	// perform insertionSort
	{
		int i, j;
		Object spare;
		
		for (i=lo+1; i<=hi; i++) {
			spare = array[i];	// make hole at top
			for (j=i; (j>lo) && (comparitor.compare(array[j-1], spare) > 0); j--)	// shuffle up till hole in right spot
				array[j] = array[j-1];	// put this item in hole
			array[j] = spare;
		}
	}
}

// 'introspective sort'
// a depth-limited qsort - switches to mergesort below the given depth
private static void qsortB(Object[] array, int lo, int hi, Comparator comparitor, int depthLimit) {
	while (((hi - lo) > InsertionSortLimit) && (depthLimit > 0)) {
		int i, j, k, l;
		Object pivot;
		Object spare;	// used for swapping

		// use median of 3 pivot scheme
		{
			int mid = (lo+hi)/2;
			
			if (comparitor.compare(array[lo], array[hi]) < 0) {
				// lo < hi
				if (comparitor.compare(array[mid], array[lo]) < 0) {
					// mid < lo < hi
					pivot = array[lo];
				} else {
					// lo < hi && lo <= mid
					if (comparitor.compare(array[mid], array[hi]) < 0) {
						pivot = array[mid];	// lo <= mid < hi
					} else {
						pivot = array[hi];	// lo < hi <= mid
					}
				}
			} else {
				// hi >= lo
				if (comparitor.compare(array[mid], array[lo]) < 0) {
					// hi >= lo > mid
					pivot = array[lo];
				} else {
					//  hi >= lo && mid >= lo
					if (comparitor.compare(array[mid], array[hi]) < 0) {
						// lo <= mid < hi
						pivot = array[mid];
					} else {
						// lo <= hi < mid
						pivot = array[hi];
					}
				}
			}
		}	// pivot found
		
		// partition the elements
		// lo <= x < i:	equal to pivot
		// i <= x < j:	less than pivot
		// j <= x <= k:	unknown
		// k < x <= l:	greater than pivot
		// l < x <= hi:	equal to pivot
		// Note that its better to copy the equal elements to the ends and then have to
		// recopy them to the center than to 'tank-tread' them to the middle directly with lots
		// of copies
		{
			int comparison;
			i = j = lo;
			k = l = hi;
			
			while (j <= k) {
				// move lower boundary up
				while (j <= k) {
					comparison = comparitor.compare(array[j], pivot);
					if (comparison < 0) {
						// move unknown element into < region
						// just increment j
						j++;
					} else if (comparison == 0) {
						// move unkown element into lower = region
						// swap with < element at edge of = region
						// increment i and j
						spare = array[i];
						array[i] = array[j];
						array[j] = spare;
						i++;
						j++;
					} else
						break;
				}
				// move upper boundary down
				while (j <= k) {
					comparison = comparitor.compare(array[k], pivot);
					if (comparison > 0) {
						// move unknown element into > region
						// just decrement k
						k--;
					} else if (comparison == 0) {
						// move unkown element into upper = region
						// swap with > element at edge of = region
						// increment k and l
						spare = array[l];
						array[l] = array[k];
						array[k] = spare;
						k--;
						l--;
					} else
						break;
				}
				
				// need to swap blocking elements if any
				if (j < k) {
					spare = array[j];
					array[j] = array[k];
					array[k] = spare;
					j++;
					k--;
				} else if (j == k) {	// TODO: comment this out once finished testing
					throw new will.UnexpectedException();
				}
			}
		}
		// now swap equal elements back into the middle
		// at the end, lo <= x < j: less than pivot
		// k < x <= hi: greater than pivot
		{
			if (j > i) {	// if there are any < elements to swap with
				if ((i-lo) < (j - i)) {	// copy the smaller amount - don't tank tread here either
					while (i > lo) {
						i--;
						j--;
						spare = array[i];
						array[i] = array[j];
						array[j] = spare;
					}
				} else {
					int top = j;
					j = lo;
					while (i < top) {
						spare = array[j];
						array[j] = array[i];
						array[i] = spare;
						i++;
						j++;
					}
				}
			} else {
				// j <= i
				j = i = lo;
			}
			if (k < l) {	// if there are any > elements to swap with
				if ((hi-l) < (l-k)) {	// copy the smaller amount - don't tank tread here either
					while (l < hi) {
						l++;
						k++;
						spare = array[l];
						array[l] = array[k];
						array[k] = spare;
					}
				} else {
					int bot = k;
					k = hi;
					while (l > bot) {
						spare = array[k];
						array[k] = array[l];
						array[l] = spare;
						k--;
						l--;
					}
				}
			} else {
				// l <= k
				k = l = hi;
			}
		}
		
		// we just went down another level
		depthLimit--;
		
		// now sort thw two sides
		// do one recursivly first
		// then iterate for the other
		qsortB(array, lo, j-1, comparitor, depthLimit);
		lo = k+1;
	}
	
	if (depthLimit > 0) {
		// perform insertionSort
		int i, j;
		Object spare;
		
		for (i=lo+1; i<=hi; i++) {
			spare = array[i];	// make hole at top
			for (j=i; (j>lo) && (comparitor.compare(array[j-1], spare) > 0); j--)	// shuffle up till hole in right spot
				array[j] = array[j-1];	// put this item in hole
			array[j] = spare;
		}
	} else	// we are in a worse case - switch to an asyptotically optimal sort
		heapsort(array, lo, hi, comparitor);
}

public static int bsearch(Object[] array, Object key) {
	return bsearch(array, new HashCompare(), key);
}

public static int bsearch(Object[] array, Comparator comparitor, Object key) {
	return bsearch(array, 0, array.length-1, comparitor, key);
}

// returns the lowest index of the equal elements or -1 if no equal elements
public static int bsearch(Object[] array, int lo, int hi, Comparator comparitor, Object key) {
	int lowestEqual = -1;	// if we don't find it, return an invalid array address
	int mid;
	int comparison;
	
	while (lo < hi) {
		mid = (lo+hi)/2;
		comparison = comparitor.compare(key, array[mid]);
		if (comparison > 0) {
			lo = mid+1;
		} else if (comparison == 0) {
			hi = mid-1;
			lowestEqual = mid;
		} else {
			hi = mid-1;
		}
	}
	
	return lowestEqual;
}

public static int partition(Object[] array, Object key) {
	return partition(array, new HashCompare(), key);
}

public static int partition(Object[] array, Comparator comparitor, Object key) {
	return partition(array, 0, array.length-1, comparitor, key);
}

// breaks array into < and >= sets
// returns the lowest index of the >= elements
public static int partition(Object[] array, int lo, int hi, Comparator comparitor, Object key) {
	int i, j;
	Object spare;
	
	i = lo; j = hi;
	
	while (i <= j) {
		while ((i <= j) && (comparitor.compare(key, array[i]) > 0)) i++;
		while ((i <= j) && (comparitor.compare(key, array[j]) <= 0)) j--;
		
		if (i < j) {
			spare = array[i];
			array[i] = array[j];
			array[j] = spare;
			i++;
			j--;
		} else if (i == j)
			throw new will.UnexpectedException();
	}
	return i;
}

public static void
heapsort(Object[] array) {
	heapsort(array, new HashCompare());
}

public static void heapsort(Object[] array, Comparator comparitor) {
	heapsort(array, 0, array.length-1, comparitor);
}

public static void heapsort(Object[] array, int lo, int hi, Comparator comparitor) {
	// first make the array into a heap
	for (int i=hi; i>=lo; i--)
		downheap(array, lo, hi, i, comparitor);
	
	// now extract the maximum and move it to the end of the array repeatedly
	for (int i=hi; i>lo; i--) {
		Object spare = array[i];
		array[i] = array[lo];
		array[lo] = spare;
		downheap(array, lo, i-1, lo, comparitor);
	}
}

static void downheap(Object[] array, int lo, int hi, int node, Comparator comparitor) {
	int leftchild, rightchild;
	int diff = node-lo;
	int largest;
	
	leftchild = diff*2+1+lo;
	rightchild = diff*2+2+lo;
	
	if ((leftchild <= hi) && (comparitor.compare(array[leftchild], array[node]) > 0))
		largest = leftchild;
	else
		largest = node;
	if ((rightchild <= hi) && (comparitor.compare(array[rightchild], array[largest]) > 0))
		largest = rightchild;
	if (largest != node) {
		Object spare;
		
		spare = array[node];
		array[node] = array[largest];
		array[largest] = spare;
		
		downheap(array, lo, hi, largest, comparitor);
	}
}

public static void insertionSort(Object[] array, int lo, int hi, Comparator comparitor) {
	int i, j;
	Object spare;
	
	for (i=lo+1; i<=hi; i++) {
		spare = array[i];	// make hole at top
		for (j=i; (j>lo) && (comparitor.compare(array[j-1], spare) > 0); j--)	// shuffle up till hole in right spot
			array[j] = array[j-1];	// put this item in hole
		array[j] = spare;
	}
}

public static void countingSort(Object[] array, CountingValue val) {
	countingSort(array, 0, array.length-1, val, 20);
}

public static void countingSort(Object[] array, int lo, int hi, CountingValue val) {
	countingSort(array, lo, hi, val, 20);
}

public static void countingSort(Object[] array, int lo, int hi, CountingValue val, int numValues) {
	countingSort(array, lo, hi, val, 0, numValues);
}

public static void countingSort(Object[] array, int lo, int hi, CountingValue val, int loValue, int hiValue) {
	Object[] spareArray;
	int[] counts = new int[hiValue - loValue + 1];
	
	for (int i=lo; i<=hi; i++) {
		int thisVal = val.value(array[i]);
		thisVal -= loValue;
		if (thisVal < 0) {
			if (true) {
				throw new will.UnexpectedException("incorrect counting sort range");
			} else {
				counts = null;
				countingSort(array, lo, hi, val, thisVal+loValue, hiValue);
				return;
			}
		}
		if (thisVal >= counts.length) {
			hiValue = thisVal+loValue+5;
			int[] newCounts = new int[hiValue - loValue + 1];
			System.arraycopy(counts, 0, newCounts, 0, counts.length);
			counts = newCounts;
		}
		counts[thisVal]++;
	}
	
	for (int i=1; i<counts.length; i++) {
		counts[i] += counts[i-1];
	}
	
	spareArray = new Object[hi-lo+1];

	for (int i=spareArray.length-1; i>=0; i--) {
		Object thisObj = array[i+lo];
		int thisVal = val.value(thisObj);
		thisVal -= loValue;
		int storeLoc = --counts[thisVal];
		spareArray[storeLoc] = thisObj;
	}
	
	System.arraycopy(spareArray, 0, array, lo, spareArray.length);
}

}
