package will.containers;

import will.*;
import java.util.*;

public final class Heap_f extends AbstractCollection implements PriorityQueue_f {

static final class location {
	int loc;
	
	location(int loc) {
		this.loc = loc;
	}
	
	int getParentLoc() {
		return getParentLoc(loc);
	}

	static int getParentLoc(int loc) {
		return (loc-1)/2;
	}

	int getLeftChildLoc() {
		return getLeftChildLoc(loc);
	}

	static int getLeftChildLoc(int loc) {
		return loc*2 + 1;
	}
	
	public String toString() {
		return "Loc: " + Integer.toString(loc);
	}
}

protected static final boolean checking = false;
protected static final boolean useLargeDownHeap = true;	// use a downheap 'optimization' (may be less effective when comparisons take lots of time)

protected Object[] elements;
protected float[] priorities;
protected int elementCount;

protected final Map EntryMap; // XXX this should be final, but javac has a bug!
protected location[] elementLocs;

protected int changeCount;

public Heap_f() {
	this(16);
}

public Heap_f(int initialCapacity) {
	this(initialCapacity, true);
}

public Heap_f(int initialCapacity, boolean useFastLookup) {
	elements = new Object[initialCapacity];
	priorities = new float[initialCapacity];
	elementCount = 0;
	changeCount = 0;
	if (useFastLookup) {
		EntryMap = new HashMap(initialCapacity);
		elementLocs = new location[initialCapacity];
	} else {
		EntryMap = null;
	}
}

public int size() {
	return elementCount;
}

public boolean isEmpty() {
	return (elementCount == 0);
}

// Note: This is slightly different to Vector.ensureCapacity() in that if capacity is already enough nothing happens
public void ensureCapacity(int minCapacity) {
	if (elements.length < minCapacity+1) {
		int newCapacity = 2*elements.length;
		
		if (newCapacity < minCapacity) {	// if we're still too small use minCapacity
			newCapacity = minCapacity+1;
		}
	
		Object[] newElements = new Object[newCapacity];
		System.arraycopy(elements, 0, newElements, 0, elementCount);
		elements = newElements;
		
		float[] newPriorities = new float[newCapacity];
		System.arraycopy(priorities, 0, newPriorities, 0, elementCount);
		priorities = newPriorities;
		
		if (elementLocs != null) {
			location[] newLocs = new location[newCapacity];
			System.arraycopy(elementLocs, 0, newLocs, 0, elementCount);
			elementLocs = newLocs;
		}
	}
}

public void trimToSize() {
	if (elements.length > size()+2) {
		int newCapacity = size()+2;
		
		Object[] newElements = new Object[newCapacity];
		System.arraycopy(elements, 0, newElements, 0, elementCount);
		elements = newElements;

		float[] newPriorities = new float[newCapacity];
		System.arraycopy(priorities, 0, newPriorities, 0, elementCount);
		priorities = newPriorities;
		
		if (elementLocs != null) {
			location[] newLocs = new location[newCapacity];
			System.arraycopy(elementLocs, 0, newLocs, 0, elementCount);
			elementLocs = newLocs;
		}
	}
}

public Comparator comparator() {
	throw new UnsupportedOperationException();
}

protected boolean eq(Object a, Object b) {
	if ((a == null) && (b == null))
		return true;
	if (a == null)
		return false;
	if (b == null)
		return false;
	return a.equals(b);
}

protected location getLocation(Object ob) {
	return getLocation(ob, -1);
}

protected location getLocation(int loc) {
	if (checking && ((loc < 0) || (loc >= elementCount)))
		throw new will.UnexpectedException();
	if (elementLocs != null) {
		return elementLocs[loc];
	}
	return getLocation(elements[loc], loc);
}

protected location getLocation(Object ob, int hint) {
	if (hint >= 0) {	// check the hint
		if (checking && (hint >= elementCount))
			throw new will.UnexpectedException();
		if (eq(elements[hint],ob)) {	// hint correct!
			if (EntryMap != null)
				return elementLocs[hint];
			else
				return new location(hint);
		}
	}
	// hint incorrect or missing
	if (EntryMap != null) {
		return (location)EntryMap.get(ob);
	} else {
		for (int i=0; i<elementCount; i++) {
			if (eq(elements[i],ob))
				return new location(i);
		}
		return null;
	}
}

public boolean contains(Object elem) {
	if (EntryMap != null) {
		return EntryMap.containsKey(elem);
	}
	for (int i=0; i<elementCount; i++) {
		if (eq(elements[i],elem))
			return true;
	}
	return false;
}

public Iterator iterator() {
	return new Iterator() {
		int location = 0;
		final int changeCountCopy = changeCount;
		
		public boolean hasNext() {
			if (changeCount != changeCountCopy) {
				throw new ConcurrentModificationException();
			}
			return location < elementCount;
		}
		
		public Object next() {
			if (!hasNext())
				throw new NoSuchElementException();
			return elements[location];
		}
		
		public void remove() {
			throw new UnsupportedOperationException();
		}
	};
}

public boolean add(Object o) {
	throw new UnsupportedOperationException("Must have a priority to add to a Priority Queue!");
}

public boolean add(Object o, float priority) {
	if (contains(o))	// don't add something twice
		return false;
	changeCount++;
	ensureCapacity(elementCount+1);
	location l = new location(elementCount);
	if (EntryMap != null) {
		EntryMap.put(o, l);
		elementLocs[elementCount] = l;
	}
	elements[elementCount] = o;
	priorities[elementCount] = priority;
	elementCount++;
	upHeap(l);
	return true;
}

public boolean remove(Object o) {
	location l = getLocation(o);
	if (l == null)
		return false;
	changeCount++;
	if (EntryMap != null) {
		EntryMap.remove(o);
	}
	if (elementCount != 1) {
		Object lastOb = elements[elementCount-1];
		location ol = getLocation(lastOb, elementCount-1);
		elements[l.loc] = lastOb;
		priorities[l.loc] = priorities[elementCount-1];
		if (elementLocs != null)
			elementLocs[l.loc] = ol;
		ol.loc = l.loc;
		elementCount--;
		if (useLargeDownHeap)
			largeDownHeap(ol);
		else
			smallDownHeap(ol);
	} else {
		elementCount--;
	}
	elements[elementCount] = null;
	priorities[elementCount] = 0;
	if (elementLocs != null)
		elementLocs[elementCount] = null;
	
	if (checking)
		Check();
	
	return true;
}

public void clear() {
	if (elementCount == 0)
		return;
	changeCount++;
	for (int i=0; i<elementCount; i++) {
		elements[i] = null;
		priorities[i] = 0;
	}
	if (EntryMap != null) {
		EntryMap.clear();
		for (int i=0; i<elementCount; i++)
			elementLocs[i] = null;
	}
	elementCount = 0;
}

public Object peekMin() {
	if (elementCount > 0) {
		return elements[0];
	} else
		return null;
}

public float peekMinPriority() {
	if (elementCount > 0) {
		return priorities[0];
	} else
		throw new NoSuchElementException("No objects in heap");
}

public Object removeMin() {
	if (elementCount > 0) {
		Object result = elements[0];
		remove(result);
		return result;
	} else
		return null;
}

public boolean setPriority(Object o, float priority) {
	location l = getLocation(o);
	if (l == null)
		throw new NoSuchElementException();
	priorities[l.loc] = priority;
	return alteredKey(l);
}

public float getPriority(Object o) {
	location l = getLocation(o);
	if (l == null)
		throw new NoSuchElementException();
	return priorities[l.loc];
}

public boolean alteredKey(Object o) {
	throw new UnsupportedOperationException("Use setPriority for a PriorityQueue_d.");
}

protected boolean alteredKey(location l) {
	changeCount++;
	if (l.loc == 0) {
		smallDownHeap(l);
	} else {
		int parent = l.getParentLoc();
		if (priorities[parent] > priorities[l.loc]) {
			upHeap(l);
		} else {
			smallDownHeap(l);
		}
	}
	return true;
}

protected void upHeap(location hole) {
	if (elementCount < 1)
		return;
	
	Object origElement = elements[hole.loc];	// store this elt away - create the hole
	float origPriority = priorities[hole.loc];
	if (checking && (elementLocs != null) && (hole != elementLocs[hole.loc]))
		throw new will.UnexpectedException();
	int parentLoc = hole.getParentLoc();

	while ((hole.loc > 0) && (priorities[parentLoc] > origPriority)) {

		// System.out.println("swapping: " + parentLoc + " and " + hole.loc);

		elements[hole.loc] = elements[parentLoc];	// move the item
		priorities[hole.loc] = priorities[parentLoc];
		if (elementLocs != null)
			elementLocs[hole.loc] = elementLocs[parentLoc];
		
		location parent = getLocation(parentLoc);
		parent.loc = hole.loc;
		hole.loc = parentLoc;

		parentLoc = hole.getParentLoc();
	}
/*
	System.out.println("hole at: " + hole.loc + " parent: " + parentLoc);
	System.out.println("comparison: " + comparator.compare(elements[parentLoc], origElement));
*/
	elements[hole.loc] = origElement;	// put the elt back in the hole
	priorities[hole.loc] = origPriority;
	if (elementLocs != null)
		elementLocs[hole.loc] = hole;

	if (checking)
		Check();
	
	return;
}

/**
 * This method moves an element down the heap to where it fits.  It does this by moving it
 * down to the bottom of the heap then calling upHeap.  This is efficient for removals where
 * the thing is being moved down the heap came from the bottom of the heap.  For small increases
 * of key this will be inefficient.
 */

protected void largeDownHeap(location hole) {
	if (elementCount <= 1)
		return;

	Object origElement = elements[hole.loc];	// store this elt away - create the hole
	float origPriority = priorities[hole.loc];
	if (checking && (elementLocs != null) && (hole != elementLocs[hole.loc]))
		throw new will.UnexpectedException();
	int heapBottomParentLoc = location.getParentLoc(elementCount-1);

	// move hole down till it's at the bottom, or parent of the last element
	// this way we don't need to check if the right child exists all the way down
	while (hole.loc < heapBottomParentLoc) {
		int childLoc = hole.getLeftChildLoc();
		if (priorities[childLoc] > priorities[childLoc+1]) {
			childLoc++;
		}
		
		elements[hole.loc] = elements[childLoc];
		priorities[hole.loc] = priorities[childLoc];
		if (elementLocs != null)
			elementLocs[hole.loc] = elementLocs[childLoc];
		
		location child = getLocation(childLoc);
		child.loc = hole.loc;
		hole.loc = childLoc;
	}
	
	// handle case of it being parent of last element seperately
	if (hole.loc == heapBottomParentLoc) {
		int childLoc = hole.getLeftChildLoc();
		if ((childLoc+1 < elementCount) &&
			(priorities[childLoc] > priorities[childLoc+1])) {
			childLoc++;
		}
		
		elements[hole.loc] = elements[childLoc];
		priorities[hole.loc] = priorities[childLoc];
		if (elementLocs != null)
			elementLocs[hole.loc] = elementLocs[childLoc];
		
		location child = getLocation(childLoc);
		child.loc = hole.loc;
		hole.loc = childLoc;
	}
	
	// put the element back into the hole
	elements[hole.loc] = origElement;
	priorities[hole.loc] = origPriority;
	if (elementLocs != null)
		elementLocs[hole.loc] = hole;
	
	upHeap(hole);

	return;
}

/**
 * This method moves an element down the heap to where it fits.
 * It does this by bubbling the element down the heap till it is in the correct spot.
 */

protected void smallDownHeap(location hole) {
	if (elementCount <= 1)
		return;

	Object origElement = elements[hole.loc];	// store this elt away - create the hole
	float origPriority = priorities[hole.loc];
	if (checking && (elementLocs != null) && (hole != elementLocs[hole.loc]))
		throw new will.UnexpectedException();
	int heapBottomParentLoc = location.getParentLoc(elementCount-1);

	// move hole down till it's at the bottom, or parent of the last element
	// this way we don't need to check if the right child exists all the way down
	while (hole.loc < heapBottomParentLoc) {
		int childLoc = hole.getLeftChildLoc();
		if (priorities[childLoc] > priorities[childLoc+1]) {
			childLoc++;
		}
		
		if (origPriority <= priorities[childLoc])
			break;

		elements[hole.loc] = elements[childLoc];
		priorities[hole.loc] = priorities[childLoc];
		if (elementLocs != null)
			elementLocs[hole.loc] = elementLocs[childLoc];
		
		location child = getLocation(childLoc);
		child.loc = hole.loc;
		hole.loc = childLoc;
	}
	
	// handle case of it being parent of last element seperately
	if (hole.loc == heapBottomParentLoc) {
		int childLoc = hole.getLeftChildLoc();
		if ((childLoc+1 < elementCount) &&
			(priorities[childLoc] > priorities[childLoc+1])) {
			childLoc++;
		}
		
		if (origPriority > priorities[childLoc]) {
			elements[hole.loc] = elements[childLoc];
			priorities[hole.loc] = priorities[childLoc];
			if (elementLocs != null)
				elementLocs[hole.loc] = elementLocs[childLoc];
			
			location child = getLocation(childLoc);
			child.loc = hole.loc;
			hole.loc = childLoc;
		}
	}
	
	// put the element back into the hole
	elements[hole.loc] = origElement;
	priorities[hole.loc] = origPriority;
	if (elementLocs != null)
		elementLocs[hole.loc] = hole;
	
	if (checking)
		Check();
	
	return;
}

protected void Check() {
	if (elementCount > elements.length) {
		throw new will.UnexpectedException("bad element count: " + this);
	}
	
	if (elements.length != priorities.length) {
		throw new will.UnexpectedException("priorities and elements arrays are different lengths!");
	}
	
	if (EntryMap != null) {
		if (EntryMap.size() != elementCount) {
			throw new will.UnexpectedException("Incorrect Entry Map Size:\nHeap:" + this + "\n\nEntryMap: " + EntryMap + "\n");
		}
		if (elementLocs.length != elements.length) {
			throw new will.UnexpectedException("elementLocs and elements arrays are different lengths!");
		}
	}
	
	for (int i=0; i<elementCount; i++) {
		if ((i > 0) &&	// check heap condition on priorities
				(priorities[i] < priorities[location.getParentLoc(i)])) {
			throw new will.UnexpectedException("not heap ordered: " + this);
		}
		if (EntryMap != null) {
			location thisLoc = elementLocs[i];
			if (thisLoc.loc != i) {
				throw new will.UnexpectedException("Incorrect Location Map Entry: " + this + "\n\nEntryMap: " + EntryMap + "\n");
			}
			if (thisLoc != EntryMap.get(elements[i])) {
				throw new will.UnexpectedException("Incorrect Location Map Entry: " + this + "\n\nEntryMap: " + EntryMap + "\n");
			}
		}
	}
}

public String toString() {
	StringBuffer result = new StringBuffer("Heap: ");
	
	result.append("ID: ");
	result.append(System.identityHashCode(this));
	result.append(" elementCount: ");
	result.append(elementCount);
	result.append("\n");
	
	for (int i=0; i<elementCount; i++) {
		result.append("Elt ");
		result.append(i);
		result.append(": ");
		result.append(elements[i]);
		result.append(", ");
		result.append(priorities[i]);
		result.append("\n");
	}
	
	return result.toString();
}

}
