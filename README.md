# vehiclecounting
A simple car counting system based on background subtraction using gaussian mixture models and cvTracks.

#Compiling the source
$cd vehiclecounting
$mkdir build
$cd build
$cmake ..
$make 

#Running 
$cd ..
$./bsgmmtracker video.avi -d 10 -hv 0 -m 50

#Command Line Usage
-d 10: parameter used to set the size of structuring element used for dilation.
-hv 0: hv=0 for horizontal orientation and hv = 1 for vertical orientation.
-m 50: margin (virtual line position from the right or from the bottom.)
