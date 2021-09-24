/*
    Aakash Tyagi
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 09/18/2021
	Original author of the starter code
	
	Please include your name and UIN below
	Name: Benson Nguyen	
	UIN: 628005910
*/

#include "common.h"
#include "FIFOreqchannel.h"
#include <fstream>
#include <ctime>

using namespace std;

int main(int argc, char *argv[]) {
	char *v {"256"};
	int opt;
	int p = 1;
	double t = 0.0;
	int e = 1;
	int c = 0;
	
	string filename = "";
	while ((opt = getopt(argc, argv, "cp:t:e:f:")) != -1) {
		switch (opt) {
			case 'c':
				c = 1;
				break;
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
		}
	}
	

	pid_t id = fork();
	if (!id) {
		char *args[]{"", "-m", "", NULL};
		args[2] = v;
		execvp("./server", args);
	}
	
	else {
		// sending a non-sense message, you need to change this
		FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);
		char buf [MAX_MESSAGE]; // 256

		if (c) {
			srand(time(0));
			MESSAGE_TYPE m = NEWCHANNEL_MSG;
			chan.cwrite(&m, sizeof(MESSAGE_TYPE));
			chan.cread(buf, 256);

			FIFORequestChannel chan2(buf, FIFORequestChannel::CLIENT_SIDE);

			int p1 = rand() % 16, p2 = rand() % 16, p3 = rand() % 16; 
			int t1 = rand() % 59, t2 = rand() % 59, t3 = rand() % 59;
			int e1 = 1 + rand() % 2, e2 = 1 + rand() % 2, e3 = 1 + rand() % 2;

			//message 1
			datamsg m1 (p1, t1, e1);
			chan2.cwrite (&m1, sizeof (datamsg)); // question
			double reply1;
			chan2.cread (&reply1, sizeof(double)); //answer
			cout << "For person " << p1 << ", at time " << t1 << ", the value of ecg "<< e1 <<" is " << reply1 << endl;


			//message 2
			datamsg m2 (p2, t2, e2);
			chan2.cwrite (&m2, sizeof (datamsg)); // question
			double reply2;
			chan2.cread (&reply2, sizeof(double)); //answer
			cout << "For person " << p2 <<", at time " << t2 << ", the value of ecg "<< e2 <<" is " << reply2 << endl;


			//message 3
			datamsg m3 (p3, t3, e3);
			chan2.cwrite (&m3, sizeof (datamsg)); // question
			double reply3;
			chan2.cread (&reply3, sizeof(double)); //answer
			cout << "For person " << p3<<", at time " << t3 << ", the value of ecg "<< e3 <<" is " << reply3 << endl << endl;
			
			m = QUIT_MSG;
			chan2.cwrite(&m, sizeof(MESSAGE_TYPE));
		}


		else if (filename != "") {
			filemsg fm (0,0);
			string fname = filename;
			int len = sizeof(filemsg) + fname.size() + 1;

			char buf2 [len];
			memcpy (&buf2, &fm, sizeof (filemsg));
			strcpy (buf2 + sizeof (filemsg), fname.c_str());
			chan.cwrite (&buf2, len); // I want the file length

			__int64_t filelen;
			chan.cread(&filelen, sizeof(__int64_t));
			char buf [MAX_MESSAGE];
			fm.length = 256;

			ofstream copy;
			copy.open("./received/" + fname, ios::binary | ios::out);
			int nbytes;
			for (int i = 0; i < ceil( (double)filelen / (double)256 ); i++ ) {
				if (filelen - fm.offset < fm.length) {
					fm.length = filelen - fm.offset;
				}
				memcpy(&buf2, &fm, sizeof(filemsg));
				chan.cwrite(&buf2, len);
				nbytes = chan.cread(&buf, 256);
				copy.write(buf, nbytes);
				fm.offset += nbytes;
			}
			copy.close();
		}
		
		else if (t < 0) {
			ofstream output;
			output.open("x1.csv");
			for (double i = 0; i < 2; i+=0.004) {
				datamsg x1 (p, i, 1);
				datamsg x2 (p, i, 2);

				chan.cwrite (&x1, sizeof (datamsg)); // question
				double reply1;
				int nbytes1 = chan.cread (&reply1, sizeof(double)); //answer

				chan.cwrite (&x2, sizeof (datamsg)); // question
				double reply2;
				int nbytes2 = chan.cread (&reply2, sizeof(double)); //answer

				output << i << "," << reply1 << ',' << reply2 << "\n";

			}
			output.close();
		}

		else {
			datamsg x (p, t, e);
			chan.cwrite (&x, sizeof (datamsg)); // question
			double reply;
			int nbytes = chan.cread (&reply, sizeof(double)); //answer
			cout << "For person " << p <<", at time " << t << ", the value of ecg "<< e <<" is " << reply << endl;
		}

		// closing the channel
		MESSAGE_TYPE m = QUIT_MSG;
		chan.cwrite (&m, sizeof (MESSAGE_TYPE));

		return 0;
	}
}
