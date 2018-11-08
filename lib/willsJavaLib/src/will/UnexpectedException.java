package will;

public class UnexpectedException extends RuntimeException {

	private final boolean printMessage = true;
	
	public UnexpectedException() {
		this("");
	}

	// take another exception and rethrow it
	// the message includes the original exception class and message
	public UnexpectedException(java.lang.Exception t) {
		this(makeStringFromThrowable(t, null));
	}
	
	public UnexpectedException(java.lang.Throwable t, String label) {
		this(makeStringFromThrowable(t, label));
	}
	
	public UnexpectedException(String s) {
		super(s);
		System.out.flush();
		if (printMessage) {
			System.err.println(getMessage());
		}
		System.err.flush();
	}

	public static final String makeStringFromThrowable(java.lang.Throwable t, String label) {
		if (t == null) {
			if (label == null)
				return "Null passed as Exception";
			else
				return label + ": null";
		} else if (t instanceof UnexpectedException) {
			if (label == null)
				return "Rethrowing: " + t.getMessage();
			else
				return "Rethrowing (" + label + "): " + t.getMessage();
		} else {
			java.io.CharArrayWriter myStream = new java.io.CharArrayWriter();
			java.io.PrintWriter pStream = new java.io.PrintWriter(myStream);

			t.printStackTrace(pStream);
			pStream.close();
		
			if (label == null) {
				return "Exception: " + t.getClass().getName() + "\n" +
					"Message: " + t.getMessage() + "\n" +
					"at: " + myStream.toString() + "\n";
			} else {
				return "Exception: " + t.getClass().getName() + "\n" +
					"Label: " + label + 
					"Message: " + t.getMessage() + "\n" +
					"at: " + myStream.toString() + "\n";
			}
		}
	}
	
	public final boolean isPrintingMessages() {
		return printMessage;
	}
}
