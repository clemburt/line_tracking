/* Libraries */
#include <iostream>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <math.h>
#include <vector>
// Image processing
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <curses.h>
// GPIO
#include <wiringPi.h>
#include <softPwm.h>
// Real-time
#include <pthread.h>
#include <unistd.h>

using namespace cv;
using namespace std;

/* Real-time */
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; // Creation d'un mutex
#define NB_THREAD 2

/* Binarisation */
Point Barycentre = Point(-1, -1); // Barycentre des pixels noirs
Mat image, mask; // Matrices pour stocker l'image courante et l'image binarisée
int seuil = 100; // Seuil pour la binarisation

/* GPIO */
int leftMot = 23; // Phyisical 33
int rightMot = 26; // Physical 32
int dirleftMot = 22; // Physical 29
int dirrightMot = 21; // Physical 31


/*
 * Binarize image and return barycentre of black pixels
 */

Point binarisation() {

    int x, y;
    int sommeX = 0, sommeY = 0;
    int nbPixels = 0;


    // Binarization and creation of bicolor mask (black and white)
    image.copyTo(mask);
    cvtColor(image, mask, CV_RGB2GRAY);
    threshold(mask, mask, seuil, 255, CV_THRESH_BINARY);


    // Browse the mask and count the number of black pixels (whose values are less than chosen threshold)
    for(x = 0; x < mask.cols; x++) {
        for(y = 0; y < mask.rows; y++) {
            // If black pixel, count it into barycentre computation
            if((int)image.at<uchar>(y,x)<seuil) {
                sommeX += x;
                sommeY += y;
                nbPixels++;
            }
        }
    }

    Point Centre(-1,-1);
    // If no pixel detected, return (-1,-1)
    if (nbPixels > 0)
        Centre = Point((int)(sommeX / nbPixels), (int)(sommeY / nbPixels));

    return Centre;
}


/*
* GPIO initialization
*/

void GPIO_setup() {

  wiringPiSetup();

  pinMode(leftMot,OUTPUT);
  pinMode(rightMot,OUTPUT);
  pinMode(dirleftMot,OUTPUT);
  pinMode(dirrightMot,OUTPUT);

  // Monitor motors speed with pwm in the interval [0;100]
  softPwmCreate(leftMot, 0, 100);
  softPwmCreate(rightMot, 0, 100);

  digitalWrite(rightMot,LOW);
  digitalWrite(dirrightMot,LOW);

}


/*
* Image processing function on which the 1st thread is running
*/

void *tr_image(void *arg) {

  Barycentre = binarisation();

}

/*
* Motors pwm monitoring function on which the 2nd thread is running
*/

void *moteur(void *arg) {

  // Return barycentre coordinates
  printf("x = %d\n",Barycentre.x);
  //printf("y = %d\n",baryCentre.y);
  // x = [0;640]
  // y = [0;480]

  if ((Barycentre.x>=0) && (Barycentre.x<=315)) { // LEFT
    softPwmWrite(leftMot,10);
    softPwmWrite(rightMot,20);
    digitalWrite(dirleftMot,HIGH);
    digitalWrite(dirrightMot,LOW);
  }

  if ((Barycentre.x>315) && (Barycentre.x<325)) { // FORWARD
    softPwmWrite(leftMot,20);
    softPwmWrite(rightMot,20);
    digitalWrite(dirleftMot,HIGH);
    digitalWrite(dirrightMot,LOW);
  }

  if (Barycentre.x>=325) { // RIGHT
    softPwmWrite(leftMot,20);
    softPwmWrite(rightMot,10);
    digitalWrite(dirleftMot,HIGH);
    digitalWrite(dirrightMot,LOW);
  }

  if (Barycentre.x==-1) { // BACK
    softPwmWrite(leftMot,10);
    softPwmWrite(rightMot,10);
    digitalWrite(dirleftMot,LOW);
    digitalWrite(dirrightMot,HIGH);
  }

}



int main() {

    GPIO_setup(); // GPIO initialization
    pthread_t id[NB_THREAD]; // Declare threads

    std::string filename = "0"; // Camera number (between 0 and 2 according to its type: webcam, Pi camera, etc.)
  	VideoCapture m_cap; // Declare the capture
  	int m_fps, devid;
  	std::istringstream iss(filename.c_str()); // Open video stream and convert the filename to a number
  	bool isOpen;

  	if(!(iss >> devid))
  	{
  		isOpen = m_cap.open(filename.c_str());
  	}
  	else
  	{
  		isOpen = m_cap.open(devid);
  	}

  	if(!isOpen)
  	{
  		std::cerr << "Unable to open video file." << std::endl;
  		return false;
  	}

  	// Set framerate to 30 if it can't be read
  	m_fps = m_cap.get(CV_CAP_PROP_FPS);
  	if(m_fps == 0)
  		m_fps = 30;

    bool isReading = true;

  	// Compute time to wait for reading the framerate
  	int timeToWait = 30; // 1000/m_fps;

    // Create a window for using the waitkey, which allows to stop the code by pressing the "q" key
    char key;
    Mat exit_screen;
    exit_screen = imread("exit.jpg", CV_LOAD_IMAGE_COLOR);
    namedWindow("Exit", CV_WINDOW_AUTOSIZE);
    imshow("Exit", exit_screen);

    // While we won't quit
    while(key != 'Q' && key != 'q') {

        // Retrieve current image
        isReading = m_cap.read(image);
        // If no retrieved image, quit the loop
        if(!isReading)
            continue;

        pthread_mutex_lock(&m);
        pthread_create(&id[0], NULL, tr_image, NULL);
        pthread_create(&id[1], NULL, moteur, NULL);
        pthread_join(id[0], NULL);
        pthread_join(id[1], NULL);
        pthread_mutex_unlock(&m);


        // Wait 10 ms
        key = waitKey(10);

    }

    // Stop motors
    digitalWrite(leftMot,LOW);
    digitalWrite(rightMot,LOW);

    // Close windows
    destroyAllWindows();
    // Close capture
    m_cap.release();

    return 0;

}
