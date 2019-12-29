#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <malloc.h>
#include <signal.h>
#include <mpi.h>
#include <time.h>
/******************************************************************************
  Displays two grey scale images. On the left is an image that has come from an
  image processing pipeline, just after colour thresholding. On the right is
  the result of applying an edge detection convolution operator to the left
  image. This program performs that convolution.
   
  Things to note:
	- A single unsigned char stores a pixel intensity value. 0 is black, 256 is
  	white.
	- The colour mode used is GL_LUMINANCE. This uses a single number to
  	represent a pixel's intensity. In this case we want 256 shades of grey,
  	which is best stored in eight bits, so GL_UNSIGNED_BYTE is specified as
  	the pixel data type.

    
  To compile adapt the code below wo match your filenames:  
	mpicc -o ipcoursework ipcoursework.c -lglut -lGL -lm -lcrypt

	mpirun -n 5  -quiet ./ipcoursework
   
  
******************************************************************************/
#define width 100
#define height 72

unsigned char image[], results[width * height];
int startIndex, endIndex;

int time_difference(struct timespec *start,
                	struct timespec *finish,
                	long long int *difference) {
  long long int ds =  finish->tv_sec - start->tv_sec;
  long long int dn =  finish->tv_nsec - start->tv_nsec;

  if(dn < 0 ) {
	ds--;
	dn += 1000000000;
  }
  *difference = ds * 1000000000 + dn;
  return !(*difference > 0);
}

void detect_edges(unsigned char *in, unsigned char *out) {
  int i;
  int n_pixels = (width * height);

  for(i=0;i<n_pixels;i++) {
	int x, y; // the pixel of interest
	int b, d, f, h; // the pixels adjacent to x,y used for the calculation
	int r; // the result of calculate
    
	y = i / width;
	x = i - (width * y);

	if (x == 0 || y == 0 || x == width - 1 || y == height - 1) {
  	results[i] = 0;
	} else {
  	b = i + width;
  	d = i - 1;
  	f = i + 1;
  	h = i - width;

  	r = (in[i] * 4) + (in[b] * -1) + (in[d] * -1) + (in[f] * -1)
      	+ (in[h] * -1);

  	if (r > 0) { // if the result is positive this is an edge pixel
    	out[i] = 255;
  	} else {
    	out[i] = 0;
  	}
	}
  }
}

void tidy_and_exit() {
  exit(0);
}

void sigint_callback(int signal_number){
  printf("\nInterrupt from keyboard\n");
  tidy_and_exit();
}

static void display() {
  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos4i(-1, -1, 0, 1);
  glDrawPixels(width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, image);
  glRasterPos4i(0, -1, 0, 1);
  glDrawPixels(width, height, GL_LUMINANCE, GL_UNSIGNED_BYTE, results);
  glFlush();
}

static void key_pressed(unsigned char key, int x, int y) {
  switch(key){
	case 27: // escape
  	tidy_and_exit();
  	break;
	default:
  	printf("\nPress escape to exit\n");
  	break;
  }
}

int main(int argc, char **argv) {
  signal(SIGINT, sigint_callback);
  // printf("image dimensions %dx%d\n", width, height);
  // struct timespec start, finish;
  // long long int difference;   
  int account = 0;
 
  int size, rank;
  // clock_gettime(CLOCK_MONOTONIC, &start);
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if(size != 5) {
	if(rank == 0) {
  	printf("This program needs 5 processes\n");
	}
  } else {
	if(rank ==0){
     	struct timespec start, finish;
   	long long int difference;  
   	clock_gettime(CLOCK_MONOTONIC, &start);
   	  	MPI_Send(&results[0], 1800, MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD);
     	MPI_Send(&results[1800], 1800, MPI_UNSIGNED_CHAR, 2, 0, MPI_COMM_WORLD);
     	MPI_Send(&results[3600], 1800, MPI_UNSIGNED_CHAR, 3, 0, MPI_COMM_WORLD);
     	MPI_Send(&results[5400], 1800, MPI_UNSIGNED_CHAR, 4, 0, MPI_COMM_WORLD);
   	 
     	MPI_Recv(&results[0], 1800, MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  
  		  	MPI_Recv(&results[1800], 1800,MPI_UNSIGNED_CHAR, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
     	MPI_Recv(&results[3600], 1800,MPI_UNSIGNED_CHAR, 3, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
     	MPI_Recv(&results[5400], 1800,MPI_UNSIGNED_CHAR, 4, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
 	 
  		  	clock_gettime(CLOCK_MONOTONIC, &finish);
         	time_difference(&start, &finish, &difference);
         	printf("run lasted %9.5lfs\n", difference/1000000000.0);
         	glutInit(&argc, argv);
         	glutInitWindowSize(width * 2,height);
         	glutInitDisplayMode(GLUT_SINGLE | GLUT_LUMINANCE);
 	 
         	glutCreateWindow("6CS005 Image Progessing Courework");
         	glutDisplayFunc(display);
         	glutKeyboardFunc(key_pressed);
  		  	glClearColor(0.0, 1.0, 0.0, 1.0);

  		  	glutMainLoop();

  		  	tidy_and_exit();
  	 
    
	} else {
  	if(rank == 1){
    
     	startIndex = 0;
      	endIndex = 1799;
      	MPI_Recv(&results[0], 1800, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      	detect_edges(image, results);
 		   	MPI_Send(&results[0], 1800, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
  	}
    else if(rank == 2){
      	startIndex = 1800;
      	endIndex = 3599;
   	 
 		   	MPI_Recv(&results[1800], 1800, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      	detect_edges(image, results);
 		   	MPI_Send(&results[1800], 1800, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    }
    else if(rank == 3){
   	 startIndex = 3600;
   	 endIndex = 5399;
 
   	 MPI_Recv(&results[3600], 1800, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   	 detect_edges(image, results);
 			 MPI_Send(&results[3600], 1800, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
   	    
    }
    else if(rank == 4){
     	 startIndex = 5400;
     	 endIndex = 7199;
    
     	 MPI_Recv(&results[5400], 1800, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
   	 detect_edges(image, results);
 			 MPI_Send(&results[5400], 1800, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
    }
	}
  }
  MPI_Finalize();
  // clock_gettime(CLOCK_MONOTONIC, &finish);
  // time_difference(&start, &finish, &difference);
  // printf("run lasted %9.5lfs\n", difference/1000000000.0);
  return 0;
}

unsigned char image[] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,0,0,255,255,0,0,255,255,
  0,255,255,255,0,0,0,0,255,255,0,0,0,0,0,0,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,
  255,0,0,255,255,0,0,255,255,0,255,255,0,0,255,0,0,255,255,
  0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,0,0,255,255,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,255,
  255,255,255,0,0,255,255,0,0,255,255,0,255,0,0,255,255,0,0,
  255,255,0,0,255,255,255,0,0,0,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,0,0,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,255,255,
  255,255,0,0,255,255,255,255,255,0,0,255,0,0,255,255,0,0,0,
  0,255,255,255,0,255,255,0,0,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,255,
  255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,0,255,255,255,255,255,0,0,255,255,255,0,0,0,255,255,0,
  0,255,0,0,0,0,255,255,255,0,0,255,255,0,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,0,0,255,255,255,255,0,0,0,0,0,0,
  0,0,0,255,0,0,255,255,0,0,0,255,255,255,255,0,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,0,0,255,255,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,
  255,0,0,255,255,255,255,255,255,255,255,255,0,0,255,255,255,255,255,
  0,0,0,255,255,255,0,0,255,255,0,0,255,0,0,0,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,255,255,255,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,
  255,255,255,255,255,255,0,0,255,255,255,255,255,255,255,255,255,255,0,
  0,255,255,255,255,0,0,255,255,255,255,255,0,0,255,0,0,255,255,
  0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,
  0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,255,255,0,255,255,255,255,0,0,0,255,255,255,255,255,
  255,255,255,255,255,0,255,255,255,255,255,0,255,255,255,255,255,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,255,255,0,0,255,255,255,0,0,0,
  255,255,255,255,255,255,255,255,255,255,0,0,255,255,255,255,0,0,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,255,
  255,255,0,0,0,0,255,255,255,255,255,255,255,255,255,0,0,255,255,
  255,255,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,
  255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,
  255,255,255,0,0,255,0,0,255,255,0,255,255,255,255,255,255,255,255,
  255,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,255,255,255,255,255,0,255,0,0,255,255,0,0,255,255,
  255,255,255,255,255,0,0,0,0,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,255,0,0,255,255,0,0,0,0,255,
  255,255,0,255,255,255,255,255,255,255,0,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,
  255,0,0,0,255,255,255,0,0,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,0,0,0,255,255,255,255,0,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,255,255,255,255,255,255,0,0,0,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,
  255,255,255,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,255,255,0,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,255,255,255,255,255,255,255,255,255,0,0,0,0,0,
  0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,255,255,0,0,0,255,255,255,255,255,0,0,255,255,255,
  255,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,255,255,255,255,0,0,255,255,255,255,0,
  0,0,255,255,255,255,0,0,0,0,0,0,0,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,255,255,255,0,0,0,0,255,255,255,255,0,0,
  255,255,255,255,0,0,255,255,255,255,255,0,0,0,0,0,0,0,255,
  255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,255,255,0,0,255,255,255,255,0,0,255,255,
  255,255,255,0,0,255,255,255,255,0,0,255,255,255,255,255,255,0,0,
  0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,255,255,255,
  255,0,0,255,255,255,255,255,255,0,255,255,255,255,0,0,255,255,255,
  255,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,255,255,255,
  255,0,0,255,255,255,0,0,255,255,255,255,255,255,0,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,
  0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,
  255,0,255,255,255,255,0,0,255,255,255,255,0,255,255,255,255,255,255,
  255,255,255,255,255,0,0,0,0,0,255,255,255,0,0,0,0,0,0,
  0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,
  0,0,255,255,255,255,0,0,255,255,255,255,0,255,255,255,255,255,255,
  255,255,255,255,255,255,0,255,255,255,255,0,0,0,255,255,255,255,0,
  0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,
  0,0,0,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,
  255,255,255,255,0,0,0,255,255,255,255,0,0,255,255,255,255,0,0,
  255,255,255,255,255,255,255,255,255,255,255,0,255,255,255,255,255,255,255,
  255,255,255,255,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,
  0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,0,255,255,255,255,0,0,0,255,255,255,0,0,0,255,
  255,255,255,0,0,255,255,255,255,255,255,255,0,255,255,255,0,0,255,
  255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,255,
  255,255,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,0,0,0,255,255,255,255,0,255,255,255,255,255,0,255,255,255,
  255,0,0,0,0,255,255,255,255,0,255,255,255,255,255,255,0,0,255,
  255,255,255,0,0,255,255,255,255,255,255,255,255,0,0,0,0,0,0,
  0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,0,255,255,255,255,0,0,0,255,255,255,255,0,0,255,255,255,
  255,255,255,255,255,255,0,0,0,0,255,255,255,255,0,255,255,255,255,
  255,255,0,0,255,255,255,255,0,0,0,255,255,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,255,255,255,255,0,255,255,255,0,0,0,255,255,255,0,
  0,0,255,255,255,255,255,255,255,255,255,0,0,0,0,255,255,255,255,
  0,0,255,255,255,255,255,0,0,0,255,255,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,255,255,255,255,0,255,255,255,255,0,
  0,255,255,255,0,0,0,255,255,255,255,255,255,255,255,255,0,0,0,
  0,0,255,255,255,0,0,255,255,255,255,255,0,0,0,255,255,255,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,255,255,255,0,0,0,0,255,255,255,255,0,
  255,255,255,255,0,255,255,255,255,0,0,0,0,255,255,255,255,255,255,
  255,255,0,0,0,0,0,255,255,255,255,0,255,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,
  255,255,255,255,0,0,255,255,255,255,255,255,255,255,255,0,0,0,255,
  255,255,255,255,255,255,255,255,0,0,0,0,255,255,255,255,0,0,255,
  255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,255,255,255,255,255,255,255,255,255,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,255,255,
  255,255,255,255,255,255,255,255,255,0,0,255,255,255,255,255,255,255,255,
  255,255,0,0,255,255,255,255,255,255,255,255,255,255,255,0,0,255,255,
  255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,
  0,0,0,0,255,255,255,255,255,255,255,255,255,255,0,0,255,255,255,
  255,255,255,255,255,255,255,255,0,0,255,255,255,0,0,255,255,255,255,
  255,255,0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,
  255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,
  0,0,255,255,255,255,255,0,0,255,255,255,255,0,0,255,255,255,255,
  0,0,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,
  255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,255,255,255,255,0,0,0,0,0,0,255,255,255,255,
  0,255,255,255,255,0,0,0,255,255,255,255,0,0,255,255,255,255,0,
  0,255,255,255,255,0,0,0,255,255,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,
  255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,255,255,255,255,0,0,255,255,0,
  0,255,255,255,255,0,255,255,255,255,0,0,0,255,255,255,255,0,0,
  255,255,255,255,0,0,255,255,255,255,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,
  255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,
  255,255,255,255,255,255,0,255,255,255,255,255,255,255,255,0,0,0,255,
  255,255,255,255,255,255,255,255,255,0,0,0,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,
  255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,255,255,255,255,255,255,255,255,255,0,255,255,255,255,255,255,
  255,0,0,0,0,255,255,255,255,255,255,255,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,
  255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,0,255,
  255,255,255,255,255,255,0,0,0,0,255,255,255,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,
  255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,0,255,
  255,255,255,0,0,255,255,255,255,255,255,0,0,0,0,255,255,255,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,
  255,255,0,0,0,255,255,255,0,0,0,255,255,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,255,255,255,255,0,255,255,255,255,0,0,0,255,255,255,
  255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,
  255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,
  255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,
  255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,
  255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,
  255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,
  255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,
  255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,
  255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,255,255,255,255,255,255,255,255,255,255,255,255,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,255,255,255,255,255,255,255,255,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,
  255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


