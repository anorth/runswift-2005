package will.algs;

import java.util.*;

public final class AbsoluteFloatComparitor implements Comparator {

public int compare(Object a, Object b) {
	float A, B;
	
	A = Math.abs(((Float)a).floatValue());
	B = Math.abs(((Float)b).floatValue());
	
	if (A < B)
		return -1;
	else if (A == B)
		return 0;
	else
		return 1;
}

}

