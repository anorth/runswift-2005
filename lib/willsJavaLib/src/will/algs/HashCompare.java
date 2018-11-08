package will.algs;

import java.util.*;

public class HashCompare implements Comparator {

public int compare(Object a, Object b) {
	return a.hashCode() - b.hashCode();
}

}

