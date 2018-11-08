/*
 * Simple search&replace color calibration,
 * For example: search=6 (Red)
 *         replace = 4 (Pink)
 * will make the dog see all Red as Pink. 
 * This is useful to simple task like recognize just the Pink beacon, without worrying mis-calibrate as Red
 * Probably be integrated to ManualClassifier sometime
 */
#include <cstdio>
#include <cstdlib>

unsigned char search = 3;
unsigned char replace = 9;

static const int CUBE_SIZE = 128;

int main(int argc, char** argv) {
    if (argc != 2){
        printf("Usage : %s nnmc.cal \n",argv[0]);
        exit(1);
    }
    
    FILE *f = fopen(argv[1], "r");
    
    if (f == NULL){
        printf("Error openning file\n");
        exit(1);
    }

    fprintf(stderr, "Replacing %d by %d\n", search, replace);

    unsigned char colour;
    //int h, s;

    // fill in colour cube
    for (int y = 0; y < CUBE_SIZE; ++y) {
        for (int u = 0; u < CUBE_SIZE; ++u) {
            for (int v = 0; v < CUBE_SIZE; ++v) {

                //h = getH(u, v);
                //s = getS(u, v);
                fscanf(f,"%c",&colour);
                if (colour == search)
                    colour = replace;
                printf("%c", colour);
            }
        }
    }
    
}

