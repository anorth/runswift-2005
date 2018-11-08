/* rlog2bfl.cc
 * Parses rlog files and creates BFL files for each YUV image present.
 * This also serves as an example program for parsing robolink rlog streams
 */

#include <iostream>
#include <fstream>
#include <sstream>

#include <stdlib.h>
#include <sys/time.h>

#define OFFLINE

#include "../robolink.h"

using namespace std;

int main(int argc, char* argv[]) {

    srand((unsigned)time(0));

    /* Parse command line args */
    if (argc != 3) {
      cerr << "Usage: " << argv[0] << " <robolink_logfile> <datafile>" << endl;
        exit(1);
    }

    /* Open the rlog file */
    ifstream fin(argv[1]);
    if (! fin.is_open()) {
        perror("error opening rlog file");
        exit(1);
    }

    /* And prepare a file for the action data */
    ofstream fout(argv[2], ifstream::out);
    if (! fout.is_open()) {
      perror("error opening BFL");
      exit(1);
    }

    robolink_header header;
    struct rlnk_atomicaction action;

    fout << "#Forward   Left   Turn" << endl;
    fout << "#Atomic actions" << endl;

    /* Read each chunk in the file */
    while (true) {
        // read the header
        fin.read((char*)&header, sizeof(header));
        if (! fin.good()) { // stop on EOF
            break;
        }

        // skip over non-yuv data
        if (header.data_type != RLNK_ATOMIC_ACTION) {
            fin.seekg(header.data_len, ios_base::cur);
            continue;
        }

        // copy the infomation from the log file to the out file
	fin.read((char*)&action, sizeof(rlnk_atomicaction));
	action.forward += rand() / (double) RAND_MAX / 5.0;
	action.left += rand() / (double) RAND_MAX / 5.0;
	action.turn += rand() / (double) RAND_MAX / 5.0;
	//cout << rand() / (double) RAND_MAX /5.0 << endl;
	 
	fout << action.forward << " " << action.left << " " << action.turn << endl;
    }

    fout.close();
    fin.close();
    return 0;
}
