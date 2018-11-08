package will.numerics.gradDesc;
import will.numerics.*;

/** Powell's Direction Set function minimization.  See section 10.5 of Numerical recipies in C */

public class PowellDesc extends GradDesc {
	
	final Matrix p;
	final Matrix[] xi;
	final double ftol;
	final int n;
	
	LinMin lm;
	
	final Matrix pt, ptt, xit;

	double fp;
	double fret;

	/** This utility method is used below */
	private final double SQR(double a) {return (a*a);}

	public PowellDesc(ErrFun ef) throws MatrixException {
		this(ef, 1E-3);
	}

	private static Matrix[] makexi(ErrFun ef) throws MatrixException {
		Matrix x = ef.getInput();
		
		final int n = x.size();
		
		Matrix[] xi = new Matrix[n];
		
		for (int i=0; i<n; i++) {
			xi[i] = new Matrix(n);
			xi[i].set(i, 1);
		}
		
		return xi;
	}

	public PowellDesc(ErrFun ef, double ftol) throws MatrixException {
		this(ef, ftol, makexi(ef));
	}

	public PowellDesc(ErrFun ef, double ftol, Matrix[] xi) throws MatrixException {
		super(ef);
		p = ef.getInput();
		n = p.size();
		if (xi.length != n)
			throw new IllegalArgumentException();
		this.xi = xi;
		this.ftol = ftol;
		
		pt = new Matrix(n);
		ptt = new Matrix(n);
		xit = new Matrix(n);
		
		fret = errFun.evaluate();
		pt.replace(p);
		
		lm = new LinMin(ef, xi[0]);
	}

	public boolean runOneStep() throws MatrixException {

		int ibig = -1;
		double del = Double.NEGATIVE_INFINITY;
		double fptt;
		
		fptt = fp = fret;
		for (int i=0; i<n; i++) {
			fptt = fret;
			fret = lm.linmin(xi[i]);
			if ((fptt - fret) > del) {
				del = (fptt - fret);
				ibig = i;
			}
		}
		if (del < 0)
			throw new will.UnexpectedException("All line minimizations went uphill!");
		if (2.0*Math.abs(fp - fret) <= ftol*(Math.abs(fp)+Math.abs(fret))) {
			return true;
		}
		if (errFun.abort())
			return true;
		
		ptt.multK(2,p);
		ptt.sub(pt);
		xit.replace(p);
		xit.sub(pt);
		pt.replace(p);

		p.replace(ptt);
		fptt = errFun.evaluate();
		p.replace(pt);
		
		if (fptt < fp) {
			double t = 2.0*(fp - 2.0*fret + fptt)*SQR(fp - fret - del) - del*SQR(fp - fptt);
			if (t < 0) {
				fret = lm.linmin(xit);
				xi[ibig].replace(xi[n-1]);
				xi[n-1].replace(xit);
			}
		}

		return false;
	}


}
