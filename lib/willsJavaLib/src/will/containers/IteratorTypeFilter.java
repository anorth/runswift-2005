package will.containers;

import java.util.*;

public class IteratorTypeFilter extends AbstractIteratorFilter {

protected Class type;	// final
protected boolean include;	// final

public IteratorTypeFilter(Iterator it, Class type) {
	this(it, type, true);
}

public IteratorTypeFilter(Iterator it, Class type, boolean include) {
	super(it);
	this.type = type;
	this.include = include;
}

protected boolean accept(Object nextObj) {
	return ((nextObj == null) ||
			(include && (type.isInstance(nextObj))) ||
			((!include) && (!type.isInstance(nextObj))));
}

}
