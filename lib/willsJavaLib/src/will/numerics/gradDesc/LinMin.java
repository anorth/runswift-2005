package will.numerics.gradDesc;
import will.numerics.*;

public class LinMin {

	/* These constants are used in the mnbrak method */
	static final double GOLD  =(double)1.618034; //Default ratio by which successive intervals are magnified.
	static final double GLIMIT=(double)100.0;    //Maximum magnification allowed for a parabolic-fit step.
	static final double TINY  =(double)1.0e-20;
	static final double	abscissa_step	= 1;  //step size for the initial abscissas used in the mnbrack 
	
	/* These constants are used in the brent method */
	static final int    ITMAX =100;
	static final double CGOLD =(double)0.3819660;
	static final double ZEPS  =(double)1.0e-10;
	
	double ax;                 //one of the bracketing points used in mnbrak
	double bx;                 //the middle bracketing point used in mnbrak
	double cx;                 //one of the bracketing points used in mnbrak
	double fa=0;               //the output of the errFun evaluated at a
	double fb=0;               //the output of the errFun evaluated at b
	double fc=0;               //the output of the errFun evaluated at c
	double fbx=0;              //the output of the errFun evaluated at bx, the initial error upon
										 //entering the brent method
	double xmin=0;             //the step size used to find the minimum in one direction
	double fx=0;               //
	double tol=3.0e-5;         //the tolerance value used in brent

	/** the function to minimize */
	final ErrFun errFun;

	final Matrix base;
	final Matrix dir;
	final Matrix weights;

	/** This utility method is used in mnbrak and brent:  SIGN(a,b) = |a| * sgn(b)   (where sgn(0)=-1)*/
	private final double SIGN(double a, double b) {return ((b > 0.0) ? Math.abs(a) : -(Math.abs(a)));}

	public LinMin(ErrFun ef) throws MatrixException {
		this(ef, ef.getInput().duplicate(), ef.getInput().duplicate());
	}

	public LinMin(ErrFun ef, Matrix dir) throws MatrixException {
		this(ef, ef.getInput().duplicate(), dir);
	}

	public LinMin(ErrFun ef, Matrix base, Matrix dir) throws MatrixException {
		this(ef, base, dir, 0, abscissa_step);
	}

	public LinMin(ErrFun ef, Matrix base, Matrix dir, double ax, double bx) throws MatrixException {
		errFun = ef;
		this.dir = dir;
		this.base = base;
		this.ax = ax;
		this.bx = bx;
		weights = errFun.getInput();
	}

	public void reset(Matrix dir) throws MatrixException {
		reset(weights, dir);
	}

	public void reset(Matrix base, Matrix dir) throws MatrixException {
		this.base.replace(base);
		this.dir.replace(dir);
		
		ax = 0;
		bx = abscissa_step;
	}

	public void reset(Matrix base, Matrix dir, double abscissa_step) throws MatrixException {
		this.base.replace(base);
		this.dir.replace(dir);
		
		ax = 0;
		bx = abscissa_step;
	}

	/** Calculates the value at a given point on the error surface.
	  */
	private final double eval(double point) throws MatrixException {
		weights.replace(base);
		if (point != 0) {
			weights.addMult(point, dir);
		}
		return errFun.evaluate(); //calculate the error for the given location
	}//end eval

	/** This routine is used to bracket a minimum of a function.
	 *  ax, bx, and cx are points on the function.
	 *  fa, fb, and fc are the value of the function evaluated at those points.
	 * Adapted from "Numerical Recipes in C", 1988.
	 */
	public void mnbrak() throws MatrixException {

		double ulim,u,r,q,fu,dum;
	
		if (ax == bx) {
			bx = bx + 1;
		}
	
		fa=eval(ax);
		fb=eval(bx);
		if (fb>fa) {   //Switch roles of a and b so that we can go downhill in the direction from a to b
			dum=ax; ax=bx; bx=dum;
			dum=fb; fb=fa; fa=dum;
		}
		cx=bx+GOLD*(bx-ax);     //First guess for c.
		fc=eval(cx);
		while (fb>fc) {                 //Keep returning here until we bracket.
			r=(bx-ax)*(fb-fc);      //Compute u by parabolic extrapolation from a,b,c.
			q=(bx-cx)*(fb-fa);              //TINY is used to prevent any possible division by zero.
			u=(double)(bx-((bx-cx)*q-(bx-ax)*r)/(2.0*SIGN(Math.max(Math.abs(q-r),TINY),q-r)));
			ulim=bx+GLIMIT*(cx-bx);
			//We won't go farther than this.  Now to test various possibilites.
			if ((bx-u)*(u-cx) > 0.0) {              //Parabolic u is between b and c: try it.
				fu=eval(u);
				if (fu < fc) {          //Got a minimum between b and c.
					ax=bx;
					bx=u;
					fa=fb;
					fb=fu;
					return;
				} else if (fu > fb) {           //Got a minimum between a and u.
					cx=u;
					fc=fu;
					return;
				}
				u=cx+GOLD*(cx-bx);              //Parabolic fit was no use.  Use default magnification.
				fu=eval(u);
			} else if ((cx-u)*(u-ulim) > 0.0) {     //Parabolic fit is between c and its allowed limit.
				fu=eval(u);
				if (fu < fc) {
					bx=cx; cx=u; u=(cx+GOLD*(cx-bx));
					fb=fc; fc=fu; fu=eval(u);
				}
			} else if ((u-ulim)*(ulim-cx) >= 0.0) { //Limit parabolic u to maximum allowed value.
				u=ulim;
				fu=eval(u);
			} else {                                        //Reject parabolic u, use default magnification.
				u=cx+GOLD*(cx-bx);
				fu=eval(u);
			}
			ax=bx; bx=cx; cx=u;                     //Eliminate oldest point and continue.
			fa=fb; fb=fc; fc=fu;
		}
	} //end mnbrak



  /** Use Brent's method to minimise a function.  This is based on section 10.2 of Numerical
    * Recipies in C, by way of Mance Harmon and Leemon Baird's Conjugate Gradient code.
    * Given a function f, and given a bracketing triplet of abscissas ax, bx, cx (such
    * that bx is between ax and cx, and f(bx) is less than both f(ax) and f(cx)), this
    * routine isolates the minimum to a fractional precision of about tol using Brent's
    * method.  The abscissa of the minimum is returned as xmin, and the minimum function
    * value is returned as brent, the returned function value. */

	public double brent() throws MatrixException {
		int iter;
		double a,b,d=0,etemp,fu,fv,fw,p,q,r,tol1,tol2,u,v,w,x,xm;
		double e=0.0;                   //This will be the distance moved on the step before last.
		
		a=((ax<cx) ? ax : cx);  //a and b must be in ascending order, though the input abscissas need not be.
		b=((ax>cx) ? ax : cx);
		x=w=v=bx;                               //Initializations...
		fbx=fw=fv=fx=eval(x);
		for (iter=1;iter<=ITMAX;iter++) {       //Main program loop
			xm=0.5*(a+b);
			tol1=tol*Math.abs(x)+ZEPS;
			tol2=2.0*tol1;
			if (Math.abs(x-xm) <= (tol2-0.5*(b-a))) {         //Test for done here.
				xmin=x;                 //Arrive here ready to exit with best values.
				weights.replace(base);
				if (xmin != 0) {
					weights.addMult(xmin, dir);
				}
				return fx;
			}
			if (Math.abs(e) > tol1) {         //Construct a trial parabolic fit.
				r=(x-w)*(fx-fv);
				q=(x-v)*(fx-fw);
				p=(x-v)*q-(x-w)*r;
				q=2.0*(q-r);
				if (q > 0.0) p= -p;
				q=Math.abs(q);
				etemp=e;
				e=d;
				if (Math.abs(p) >= Math.abs(0.5*q*etemp) ||
						p <= q*(a-x) || p >= q*(b-x)) {
					e=((x>=xm) ? (a-x) : (b-x));  //The above conditions determine the acceptibility
					d=CGOLD*e;                    //The above conditions determine the acceptibility
				} else {                                          // of the parabolic fit.  Here we take the golden section
					d=p/q;  //Take the parabolic step       // step into the larger of the two segments.
					u=x+d;
					if ((u-a) < tol2 || (b-u) <tol2)
						d=SIGN(tol1,xm-x);
				}
			} else {
				e=((x >= xm) ? (a-x) : (b-x));
				d=CGOLD*e;
			}
			u=((Math.abs(d) >= tol1) ? (x+d) : (x+SIGN(tol1,d)));
			fu=eval(u);                                             //This is the one function evaluation per iteration,
			if (fu <= fx) {                                         //and now we have to decide what to do with our
				if (u >= x)
					a=x;
				else
					b=x;              //function evaluation.  Housekeeping follows:
				v=w; w=x; x=u;
				fv=fw; fw=fx; fx=fu;
			} else {
				if (u < x)
					a=u;
				else
					b=u;
				if ((fu <= fw) || (w == x)) {
					v=w;
					w=u;
					fv=fw;
					fw=fu;
				} else if (fu <= fv || v == x || v == w) {
					v=u;
					fv=fu;
				}
			}                       //Done with housekeeping.  Back for another iteration.
        } //end for ITER
        System.out.println("Too many iterations in BRENT");
        xmin=x;
		weights.replace(base);
		if (xmin != 0) {
			weights.addMult(xmin, dir);
		}
        return fx;
	} //end brent

	public double linmin(Matrix dir) throws MatrixException {
		return linmin(weights, dir);
	}

	public double linmin(Matrix dir, double abscissa_step) throws MatrixException {
		return linmin(weights, dir, abscissa_step);
	}

	public double linmin(Matrix base, Matrix dir) throws MatrixException {
		return linmin(base, dir, abscissa_step);
	}

	public double linmin(Matrix base, Matrix dir, double abscissa_step) throws MatrixException {
		reset(base, dir, abscissa_step);
		mnbrak();
		return brent();
	}

}
