package will.containers;

import java.util.*;

public class IteratorObjectFilter extends AbstractIteratorFilter {

protected final ObjectFilter filter;

public IteratorObjectFilter(Iterator it, ObjectFilter filter) {
	super(it);
	this.filter = filter;
}

protected boolean accept(Object obj) {
	return filter.accept(obj);
}

}
