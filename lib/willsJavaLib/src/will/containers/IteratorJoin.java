package will.containers;

import java.util.*;

public class IteratorJoin implements Iterator {

final Iterator itA;
final Iterator itB;

boolean finishedFirst;

public IteratorJoin(Iterator itA, Iterator itB) {
	this.itA = itA;
	this.itB = itB;
	finishedFirst = false;
}

public boolean hasNext() {
	if (!finishedFirst) {
		if (itA.hasNext())
			return true;
		finishedFirst = true;
	}
	return itB.hasNext();
}

public Object next() {
	if (!hasNext())
		throw new NoSuchElementException();
	
	if (!finishedFirst)
		return itA.next();
	else
		return itB.next();
}

public void remove() {
	throw new UnsupportedOperationException();
}

}
