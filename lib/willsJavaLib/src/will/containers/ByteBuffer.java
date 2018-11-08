package will.containers;

public class ByteBuffer {

byte[] buff;
int size;

public ByteBuffer() {
	buff = new byte[10];
	size = 0;
}

public ByteBuffer(int size) {
	buff = new byte[size];
}

public byte[] getByteArray() {
	byte[] result = new byte[size];
	System.arraycopy(buff, 0, result, 0, size);
	return result;
}

public int length() {
	return size;
}

public void setSize(int newSize) {

	if (newSize < size)
		return;
	else if (newSize < buff.length) {
		if (size*2+10 > newSize) {
			if (size*5+10 > buff.length)
				return;
			newSize = size*2;
		}
	} else if (newSize < buff.length*2) {
		newSize = buff.length*2;
	}

	byte[] newBuff = new byte[newSize];
	System.arraycopy(buff, 0, newBuff, 0, size);
	
	buff = newBuff;
}

public void append(byte b) {
	setSize(size+1);
	
	buff[size++] = b;
}

public void append(byte[] b) {
	setSize(size + b.length + 1);

	System.arraycopy(b, 0, buff, size, b.length);
}

public void truncate(int newSize) {
	if (newSize < 0)
		return;
	if (newSize >= size)
		return;
	
	size = newSize;
}

}
