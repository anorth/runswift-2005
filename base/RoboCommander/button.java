public class button {
    HumanControl hc;

    // constructor
    public button (RoboWirelessBase p) {
        hc = p.getHumanControl();
    }

    public int button() {
    if (hc.data != null && hc.data[20] <= 483.764) {
            if (hc.data != null && hc.data[7] <= 279.86) {
            return 0;
//        } else     if (hc.data != null && hc.data[7] > 279.864) {
//            return 0;
        } else     if (!hc.panStr.equals("?") && Double.parseDouble(hc.panStr) <= 0.369) {
            return 2;
        } else     {
            return 0;
        }
    } else     if (hc.data != null && hc.data[11] <= 796.191) {
        return 0;
    } else     if (!hc.tiltStr.equals("?") && Double.parseDouble(hc.tiltStr) <= 0.334) {
        return 0;
    } else     {
        return 2;
    }
    }
}
