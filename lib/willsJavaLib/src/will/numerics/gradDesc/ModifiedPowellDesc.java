package will.numerics.gradDesc;
import will.numerics.*;

/** Powell's Direction Set function minimization.  See section 10.5 of Numerical recipies in C */

public class ModifiedPowellDesc extends GradDesc {
	
	final DErrFun errFun;
	
	final Matrix p;
	final Matrix[] xi;
	final double[] stepsize;
	final Matrix[] exi;
	final double[] estepsize;
	final double ftol;
	final int n;
	
	LinMin lm;
	
	final Matrix dir;
	final Matrix grad;
	final Matrix pt, ptt, xit;

	double fp;
	double fret;
	
	int count;

	/** This utility method is used below */
	private final double SQR(double a) {return (a*a);}

	public ModifiedPowellDesc(DErrFun ef, Matrix[] exi) throws MatrixException {
		this(ef, exi, 1E-3);
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

	public ModifiedPowellDesc(DErrFun ef, Matrix[] exi, double ftol) throws MatrixException {
		this(ef, exi, makexi(ef), ftol);
	}

	public ModifiedPowellDesc(DErrFun ef, Matrix[] exi, Matrix[] xi, double ftol) throws MatrixException {
		super(ef);
		errFun = ef;
		p = ef.getInput();
		grad = ef.getGradient();
		n = p.size();
		this.exi = exi;
		this.xi = xi;
		this.ftol = ftol;
		
		dir = new Matrix(n);
		pt = new Matrix(n);
		ptt = new Matrix(n);
		xit = new Matrix(n);
		
		estepsize = new double[exi.length];
		for (int i=0; i<estepsize.length; i++)
			estepsize[i] = 1;
		stepsize = new double[xi.length];
		for (int i=0; i<stepsize.length; i++)
			stepsize[i] = 1;
		
		fret = errFun.evaluate();
		pt.replace(p);
		
		count = 0;
		
		lm = new LinMin(ef, dir);
	}

	public boolean runOneStep() throws MatrixException {

		int ibig = -1;
		double del = Double.NEGATIVE_INFINITY;
		double fptt;
		
		fptt = fp = fret;
		
		pt.replace(p);
		
		// first minimize along one axis - stops the vectors from becoming linearly dependent
		dir.set(0);
		dir.set(count%n, 1);
		fret = lm.linmin(dir);
		// now minimize along each of the sub-parts of the derivative
		for (int i=0; i<exi.length; i++) {
			errFun.evaluate(true);
			errFun.findGradient();
			dir.replace(grad);
			dir.multEl(exi[i]);
			fret = lm.linmin(dir, estepsize[i]);
			estepsize[i] = (lm.xmin == 0)?1:lm.xmin;
		}
		// now minimize along each basis direction
		for (int i=0; i<xi.length; i++) {
			fptt = fret;
			fret = lm.linmin(xi[i], stepsize[i]);
			stepsize[i] = (lm.xmin == 0)?1:lm.xmin;
			if ((fptt - fret) > del) {
				del = (fptt - fret);
				ibig = i;
			}
		}
		
		count++;
		
		if (del < 0)
			throw new will.UnexpectedException("All line minimizations went uphil!");
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
				double spareStep = (lm.xmin == 0)?1:lm.xmin;
				xi[ibig].replace(xi[n-1]);
				stepsize[ibig] = stepsize[n-1];
				xi[n-1].replace(xit);
				stepsize[n-1] = spareStep;
			}
		}

		return false;
	}


}
