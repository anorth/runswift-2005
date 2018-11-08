package will.numerics.gradDesc;
import will.numerics.*;

public class ConjGrad extends GradDesc {

	static final double EPS  =(double)1.0e-20;

	final DErrFun errFun;         //the error function to minimize
	final LinMin lm;

	final double tolerance;  //stop learning when smoothed error < tolerance
	final double FTOL;
	
	final int nWeights;           //this is the number of degree of freedom in the function approximator

	/* These are the weights of the function approximator as well as the initial gradient */
	final Matrix grad;
	final Matrix old_grad;
	final Matrix h; // direction vector

	/* used in the main loop */
	int count;
	double fx;		// the function value at the current best point
	double fb;		// the best evaluation last time through the loop

	boolean almost_done = false;
	
	public ConjGrad(DErrFun ef) {
		this(EPS, -1, ef);
	}
	
	public ConjGrad(double tol, DErrFun ef) {
		this(tol, -1, ef);
	}
	
	public ConjGrad(double tol, double dtol, DErrFun ef) {
		super(ef);
		count = 0;
	
		errFun = ef;
		nWeights = errFun.getEffectiveDF();      //used for determining when to reset the search direction
		tolerance = tol;
		if (dtol > EPS)
			FTOL = dtol;
		else
			FTOL = EPS;
		grad = errFun.getGradient(); /* current gradient direction */
		fb = errFun.evaluate(true);     //Initialize the gradient vector
		errFun.findGradient();
		
		grad.mult(-1);               // set the gradient vector to the downhill direction
		old_grad = grad.duplicate();   // the next dir to minimize
		h = grad.duplicate();         // Trace of past gradients
		try {
			lm = new LinMin(ef, h);
		} catch (MatrixException e) {
			throw new will.UnexpectedException();
		}
	}
	
	public void resetState() throws MatrixException {
		count = 0;
		fb = errFun.evaluate(true);     //Initialize the gradient vector
		errFun.findGradient();
		grad.mult(-1);                  //set the gradient vector to the downhill direction
		old_grad.replace(grad);
		h.replace(grad);
	}
	
	  /** repeatedly change x until f(x) reaches a local minimum.
		* This runs one step of the simulation.  The function returns true when the simulation
		* is completely done.  As the simulation is running, it should call
		* the watchManager.update() function when varaibles change so all the display
		* windows can be updated.
		*/
	public boolean runOneStep() throws MatrixException {
		fx = lm.linmin(h);
		
		if (errFun.abort())
			return true;
		// Check if error<tolerance, if so, quit.  The normal exit criteria is the second half
		// of this conditional statement.  The convergence tolerance is FTOL.
		boolean done_check = (2.0*Math.abs(fb-fx)) <= (FTOL*(Math.abs(fx)+Math.abs(fb)+EPS));
		fb = fx;
		if((fx<tolerance) || (almost_done && done_check))
			return true;
		almost_done = false;
		if (done_check) {	// didn't move far
			almost_done = true;
			errFun.setLastDitchEffort();
			count = nWeights+1;
			// System.out.println("Resetting conjugate");
		}
		
		// calculate new gradient and new conjugate direction
		errFun.evaluate(true);
		errFun.findGradient();

		double gg = old_grad.dot(old_grad);
		old_grad.add(grad);
		double dgg = old_grad.dot(grad);

		if (gg == 0) {
			return true;
		}

		double gam = dgg/gg;

		old_grad.multK(-1, grad);	// old_grad = -grad (to make it point downhill)
		h.mult(gam);
		h.add(old_grad);

		// if num of iterations > degrees of freedom then reset and start again
		if ((count++)>nWeights) {
			count=0;
			h.replace(old_grad);
			grad.replace(old_grad);
		}
		return false; //the minimisation isn't finished so return false
	  }
	
	
}
