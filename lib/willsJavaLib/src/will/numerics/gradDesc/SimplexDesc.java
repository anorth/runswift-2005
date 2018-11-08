package will.numerics.gradDesc;
import will.numerics.*;

/** Downhill simplex minimization.  See section 10.4 of Numerical recipies in C */

public class SimplexDesc extends GradDesc {
	
	final Matrix[] p;
	final Matrix x;
	final double[] y;
	final double ftol;
	final int ndim;
	final int mpts;

	final Matrix psum;
	
	public SimplexDesc(ErrFun ef) throws MatrixException {
		this(ef, 1E-3);
	}

	private static Matrix[] makep(ErrFun ef) throws MatrixException {
		Matrix x = ef.getInput();
		
		Matrix[] p = new Matrix[x.size()+1];
		
		p[0] = x.duplicate();
		
		for (int i=1; i<p.length; i++) {
			p[i] = x.duplicate();
			p[i].set(i-1, x.val(i-1) + 1);
		}
		
		return p;
	}

	public SimplexDesc(ErrFun ef, double ftol) throws MatrixException {
		this(ef, ftol, makep(ef));
	}

	private static double[] makey(ErrFun ef, Matrix[] p) throws MatrixException {
		double[] y = new double[p.length];
		
		Matrix x = ef.getInput();
		
		for (int i=0; i<y.length; i++) {
			x.replace(p[i]);
			y[i] = ef.evaluate();
		}
		
		return y;
	}

	public SimplexDesc(ErrFun ef, double ftol, Matrix[] p) throws MatrixException {
		this(ef, ftol, p, makey(ef, p));
	}

	public SimplexDesc(ErrFun ef, double ftol, Matrix[] p, double[] y) throws MatrixException {
		super(ef);
		x = ef.getInput();
		ndim = x.size();
		mpts = ndim+1;
		if (p.length != mpts)
			throw new IllegalArgumentException();
		if (y.length != mpts)
			throw new IllegalArgumentException();
		this.ftol = ftol;
		this.p = p;
		this.y = y;
		psum = new Matrix(ndim);
		
		getPsum();
	}

	void getPsum() throws MatrixException {
		psum.replace(p[0]);
		for (int i=1; i<mpts; i++) {
			psum.add(p[i]);
		}
	}

	void swapPt(int i, int j) {
		double spare = y[i];
		y[i] = y[j];
		y[j] = spare;
		Matrix spareMat = p[i];
		p[i] = p[j];
		p[j] = spareMat;
	}

	public boolean runOneStep() throws MatrixException {
		int ilo = 0;
		int ihi, inhi;
		double rtol;
		double ytry;
		
		// find the worst, second-worst, and best points (ihi, inhi, ilo)
		
		if (y[0]>y[1]) {
			inhi = 1;
			ihi = 0;
		} else {
			inhi = 0;
			ihi = 1;
		}
		
		for (int i=0; i<mpts; i++) {
			if (y[i] <= y[ilo])
				ilo = i;
			if (y[i] > y[ihi]) {
				inhi = ihi;
				ihi = i;
			} else if (y[i] > y[inhi] && i != ihi) {
				inhi = i;
			}
		}
		
		// check to see if we're done
		
		rtol = 2.0*Math.abs(y[ihi]-y[ilo])/(Math.abs(y[ihi])+Math.abs(y[ilo]));
		if (rtol < ftol) {
			if (ilo != 0)
				swapPt(0, ilo);
			x.replace(p[0]);
			return true;
		}
		
		// do the iteration
		
		ytry = amotry(ihi, -1.0);
		
		if (ytry <= y[ilo]) {
			// gives a great result - try extrapolating further in this direction
			ytry = amotry(ihi, 2.0);
		} else if (ytry >= y[inhi]) {
			// the reflected point is worse than the second-worst point.  Look for an intermediate lower point, i.e., do a 1D contraction.
			double ysave = y[ihi];
			ytry = amotry(ihi, 0.5);
			if (ytry >= ysave) {
				// can't seem to get rid of this high point.  Contract around lowest point.
				for (int i=0; i<mpts; i++) {
					if (i != ilo) {
						x.multK(0.5, p[i]);
						x.addMult(0.5, p[ilo]);
						p[i].replace(x);
						y[i] = errFun.evaluate();
					}
				}
				getPsum();
			}
		}
		
		if (errFun.abort())
			return true;
		
		return false;
	}

	/** extrapolates by a factor of fac through the face of the simplex across from the point ihi,
	  * tries it, and replaces the point if the new point is better. */

	double amotry(int ihi, double fac) throws MatrixException {
		double fac1, fac2, ytry;
		
		fac1 = (1.0-fac)/ndim;
		fac2 = fac1-fac;
		
		x.multK(fac1,psum);
		x.addMult(-fac2, p[ihi]);
		
		ytry = errFun.evaluate();
		
		if (ytry < y[ihi]) {
			y[ihi] = ytry;
			psum.sub(p[ihi]);
			psum.add(x);
			p[ihi].replace(x);
		}
		
		return ytry;
	}
}
