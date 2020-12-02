# line_tracking
Line tracking real-time robot with Raspberry Pi and xenomai

- **Hardware**

Raspberry Pi

Pi camera and camera holder (CAO by Sébastien Juishomme and 3D printing by Aline Baudry)

Plateform with 2 motorized wheels (pwm) and 1 idler wheel

Batterry + cable

Wi-Fi key


- **Compilation**

Power the Raspberry

Put the Wi-Fi key, select "STS-Pi-11" network and enter password ("sts_pi_11")

Open a terminal and enter :

```
$ ssh -X pi@10.0.0.1 // Connect to the card
$ raspberry // Raspberry password
$ sudo modprobe bcm2835-v4l2 // Open the Pi camera
$ make // Compilation
$ ./tracking // Executable name
```


- **Required libraries**

opencv // Image processing

WiringPi // Motors monitoring: [wiringpi.com](https://wiringpi.com/)

pthread // Mutex / threads, converted into real-time by compiling with xenomai


- **Code architecture**

**Point binarisation()**

*Binarizes the current image, browses the binarized image (mask), counts the number of black pixels (whose values are less than the chosen threshold) and returns their barycentre (returns [-1,-1] if the image doesn't contain any white pixel)*

**void GPIO_setup()**

*Initializes Raspberry GPIOs to monitor the motors*

**void * tr_image(void * arg)**

*Image processing function on which the 1st thread is running, allocates the returned coordinates of binarisation() to the barycentre point*

**void * moteur(void * arg)**

*Motors pwm monitoring function (left, right, forward, reverse) on which the 2nd thread is running, robot direction is determined according to the x coordinate of the barycentre (reverse if it's -1)*

**int main()**

Retrieves the video stream, opens an "exit" window to use the opencv waitKey() function and stop the code if the "q" key is pressed, creates 2 threads respectively for *tr_image(void *arg) and *moteur(void *arg)
