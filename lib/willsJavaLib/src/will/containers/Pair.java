package will.containers;

public class Pair {
	private static final int prime = 65521;	// 2^15 - 19 or 2^16 - 15
	protected final Object A;
	protected final Object B;
	
	public Pair(Object A, Object B) {
		this.A = A;
		this.B = B;
	}
	
	public Object getA() {
		return A;
	}
	
	public Object getB() {
		return B;
	}
	
	public boolean equals(Pair obj) {
		if (A == null) {
			if (obj.A != null) {
				if (!obj.B.equals(A)) {
					return false;
				}
			}
		} else if (!A.equals(obj.A)) {
			return false;
		}
		
		if (B == null) {
			if (obj.B != null) {
				if (!obj.B.equals(B)) {
					return false;
				}
			}
		} else if (!B.equals(obj.B)) {
			return false;
		}
		
		return true;
	}
	
	public boolean equals(Object obj) {
		if (!(obj instanceof Pair))
			return false;
		return this.equals((Pair)obj);
	}
	
	public int hashCode() {
		final int Acode = (A==null)?0:A.hashCode();
		final int Bcode = (B==null)?0:B.hashCode();
		
		int result = ((Acode%prime) << 16) | (Bcode % prime);
		
		return result;
	}
}
