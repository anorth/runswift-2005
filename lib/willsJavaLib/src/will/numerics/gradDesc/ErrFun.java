package will.numerics.gradDesc;
import will.numerics.*;

/** An ErrFun represents an error function to be minimized,
  * such as mean squared output error for a neural net, or
  * mean squared Bellman residual for a reinforcement-learning system,
  * or mean squared interference for localizing.
  * It is a function f(x) that maps vectors to scalars.
  * At a minimum, the evaluate() method should return something.
  * The getGradient() and getHessian() methods may or may not do
  * anything useful, though they may be needed if gradient descent is to
  * be performed on this function.  The evaluate() method should always
  * be called before calling getGradient() or getHessian();
  * The get*() functions return Matrix  objects for the inputs and outputs.
  * These vectors can be read and set to access the current input/output values.
  * By default, this object contains variables and code for dealing with one
  * function approximator, though these can be ignored or overridden if there
  * are zero or two or more function approximators involved.
  *    <p>This code is (c) 1996 Leemon Baird
  *    <<a href=mailto:leemon@cs.cmu.edu>leemon@cs.cmu.edu</a>>,
  *    <a href=http://www.cs.cmu.edu/~baird>http://www.cs.cmu.edu/~baird</a><br>
  *    The source and object code may be redistributed freely.
  *    If the code is modified, please state so in the comments.
  * @version 1.3, 21 July 97
  * @author Leemon Baird
  */
public abstract class ErrFun {

  /////////////////////////////////////////////////////////////////////

  // the following are called by the gradient descent algorithm.
  // f(x) is the error for a given weight vector x.
  // Depending on the algorithm and settings in the HTML file,
  // this error may be the true error or an unbiased estimate of
  // the true error

  public int getEffectiveDF() {
  	return getInput().size();
  }

  /** The input x sent to the function f(x) (a column vector)*/
  public abstract Matrix getInput();

  /** return the scalar output for the current fInput vector. */
  public abstract double evaluate();

  /** Tell the err fun to squeeze as much out of this one as possible.
    This is called when the minimizer is thinking of stopping and wants to make sure
    that nothing is being lost by approximations. */
  public void setLastDitchEffort() {
  }

  public boolean abort() {
    return false;
  }

}//end ErrFun
