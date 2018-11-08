package will.containers;

import java.util.*;

public interface PriorityQueue_f extends PriorityQueue {
	boolean add(Object obj, float priority);
	float peekMinPriority();
	float getPriority(Object o);
	boolean setPriority(Object o, float priority);
}
