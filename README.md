# C++ Implementation of Connected Components Algorithm

#### This program implements the two-pass connected components algorithm (CCA) in C++ using the vectors standard library.  
The program also creates a ```labels.txt``` file which shows the labels as well.  
The program first uses histogram manipulation for thresholding and then the CCA is used.
#### Please note that the program only works on monochromatic images.

### To compile and then run:

```bash
g++ -O3 CCA.cpp -o CCA -ljpeg -fpermissive
./CCA <input image file name> <output image file name>
```
##### My Example:

```bash
./CCA exampleinput.jpg exampleoutput.jpg
```
