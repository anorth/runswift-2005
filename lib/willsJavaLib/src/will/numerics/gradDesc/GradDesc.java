package will.numerics.gradDesc;
import will.numerics.*;

/** This object performs some form of gradient descent.
  * This code is based on the WebSim code by Leemon Baird.
  * http://www.leemon.com/
  */
public abstract class GradDesc {
	/** the function to minimize */
	ErrFun errFun;

	public GradDesc(ErrFun errFun) {
		this.errFun = errFun;
	}

	public void findMin() throws MatrixException {
		boolean done;
		
		do {
			done = runOneStep();
		} while (!done);
	}

	public void resetState() throws MatrixException {
	}

	/** repeatedly change x until f(x) reaches a local minimum */
	public abstract boolean runOneStep() throws MatrixException;
}
