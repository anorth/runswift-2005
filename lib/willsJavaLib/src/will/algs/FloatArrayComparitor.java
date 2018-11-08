package will.algs;

import java.util.*;

public final class FloatArrayComparitor implements Comparator {

int compAttr;

public FloatArrayComparitor(int attribute) {
	compAttr = attribute;
}

public int compare(Object a, Object b) {
	float[] vectA, vectB;
	
	vectA = (float[])a;
	vectB = (float[])b;
	
	if (vectA[compAttr] < vectB[compAttr])
		return -1;
	else if (vectA[compAttr] == vectB[compAttr])
		return 0;
	else
		return 1;
}

}

