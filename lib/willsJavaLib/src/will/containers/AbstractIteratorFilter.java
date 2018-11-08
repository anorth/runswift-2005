package will.containers;

import java.util.*;

public abstract class AbstractIteratorFilter implements Iterator {

protected final Iterator it;

private boolean nextObjValid;
private Object nextObj;

public AbstractIteratorFilter(Iterator it) {
	this.it = it;
	nextObjValid = false;
}

protected abstract boolean accept(Object obj);

public boolean hasNext() {
	if (nextObjValid)
		return true;
	
	while (!nextObjValid) {
		if (!it.hasNext())
			return false;
		
		try {
			nextObj = it.next();
		} catch (NoSuchElementException e) {
			throw new will.UnexpectedException(e);
		}
		
		nextObjValid = accept(nextObj);
	}
	return true;
}

public Object next() {
	if (!hasNext())
		throw new NoSuchElementException();
	if (!nextObjValid)
		throw new will.UnexpectedException();
	
	nextObjValid = false;
	return nextObj;
}

public void remove() {
	throw new UnsupportedOperationException();
}

}
