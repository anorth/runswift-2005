package will.algs;

import java.util.*;

final class FloatComparitor implements Comparator {

public int compare(Object a, Object b) {
	float A, B;
	
	A = ((Float)a).floatValue();
	B = ((Float)b).floatValue();
	
	if (A < B)
		return -1;
	else if (A == B)
		return 0;
	else
		return 1;
}

}

