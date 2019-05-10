#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


const int PS = 256;
const int VMS = 256;
const int MMS = 128;
const int TS = 16;


int main(int argc, char *argv[]) {

	int physMem[128];
	int virMem[256][2];
	int tlb[16][2];

	for (int i = 0; i < VMS; i++) {
		virMem[i][0] = (i > MMS - 1) ? -1 : i;
		virMem[i][1] = (i > MMS - 1) ? -1 : MMS - i;
	}
	for (int i = 0; i < MMS; i++) {
		physMem[i] = i;
	}

	for (int i = 0; i < TS; i++) {
		tlb[i][0] = (i > TS - 1) ? -1 : i;
		tlb[i][1] = (i > TS - 1) ? -1 : TS - i;
	}

	if (argc != 2) {
		printf("Incorrect Arguments.\n");
		return 1;
	}

	FILE *pFile;
	pFile = fopen(argv[1], "r");

	if (pFile == NULL) {
		printf("Error opening a file %s: %s\n", argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	char *line = NULL;
	size_t len = 0;
	size_t read;

	int pgNum = 0;
	int physAdd = 0;
	int count = 0;

	double freq = 0.0;

	printf("Translating 1000 Logical Addresses: \n\n");

	while ((read = getline(&line, &len, pFile)) != -1) {
		int offset = atoi(line) & 255;
		int page = atoi(line) & 65280;
		int tblNum = page >> 8;
		int hit = 0;

		for (int i = 0; i < TS; i++) {
			if (tlb[i][0] == tblNum) {
				hit = 1;
				printf("TABLE HIT\n");
				break;
			}
		}

		if (virMem[tblNum][0] < 0 && !hit) {
			count++;
			srand(time(NULL));
			int r = rand();
			int largest = 0;
			int remove = 0;
			for (int i = 0; i < VMS; i++) {
				if (virMem[i][1] > largest) {
					largest = virMem[i][1];
					remove = i;
				}
			}

			int replace = r % 15;
			tlb[replace][0] = tblNum;
			tlb[replace][1] = virMem[remove][0];
			virMem[tblNum][0] = virMem[remove][0];
			virMem[tblNum][1] = 0;
			virMem[remove][0] = -1;
			virMem[remove][1] = 0;
		}

		if (page < 100) {
			printf("Virtual Address = %d     \t", page);
		}
		else if (page < 1000) {
			printf("Virtual Address = %d   \t", page);
		}
		else {
			printf("Virtual Address = %d  \t", page);
		}
		physAdd = (physMem[virMem[tblNum][0]] * PS) + offset;


		printf("Physical Address: %d\n", physAdd);


		pgNum++;

		for (int i = 0; i < VMS; i++) {
			virMem[i][1]++;
		}
	}

	freq = (double)count / 1000 * 100;
	printf("\nPage Fault freq: %.2f%% \n", freq);

	free(line);
	fclose(pFile);

	exit(EXIT_SUCCESS);

}
