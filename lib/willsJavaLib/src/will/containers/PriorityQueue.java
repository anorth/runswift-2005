package will.containers;

import java.util.*;

public interface PriorityQueue extends Collection {
	Comparator comparator();
	Object peekMin();
	Object removeMin();
	boolean alteredKey(Object o);
}
