package will.numerics.gradDesc;

import will.numerics.*;

/* this class is the abstract type for a differentiable error function */

public abstract class DErrFun extends ErrFun {
	/** The gradient of f(x) with respect to x (a column vector)*/
	public abstract Matrix getGradient();

	/**
	 * A simple implementation of the ErrFun evaluation method that calls the
	 * evaluate(false) method from DErrFun.
	 */
	public double evaluate() {
		return evaluate(false);
	}

	/** return the scalar output for the current fInput vector. */
	public abstract double evaluate(boolean willFindGradient);

	/** update the fGradient vector based on the current fInput vector */
	public abstract void findGradient();
}
