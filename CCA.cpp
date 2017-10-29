#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

using namespace std;


/** Read the JPEG image at `filename` as an array of bytes.
  Data is returned through the out pointers, while the return
  value indicates success or failure.
  NOTE: 1) if image is RGB, then the bytes are concatenated in R-G-B order
        2) `image` should be freed by the user
 */
static inline int
read_JPEG_file(char *filename,
               int *width, int *height, int *channels, unsigned char *(image[]))
{
  FILE *infile;
  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  struct jpeg_error_mgr jerr;
  struct jpeg_decompress_struct cinfo;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, infile);
  (void) jpeg_read_header(&cinfo, TRUE);
  (void) jpeg_start_decompress(&cinfo);

  *width = cinfo.output_width, *height = cinfo.output_height;
  *channels = cinfo.num_components;
  // printf("width=%d height=%d c=%d\n", *width, *height, *channels);
  *image = malloc(*width * *height * *channels * sizeof(*image));
  JSAMPROW rowptr[1];
  int row_stride = *width * *channels;

  while (cinfo.output_scanline < cinfo.output_height) {
    rowptr[0] = *image + row_stride * cinfo.output_scanline;
    jpeg_read_scanlines(&cinfo, rowptr, 1);
  }
  jpeg_finish_decompress(&cinfo);

  jpeg_destroy_decompress(&cinfo);
  fclose(infile);
  return 1;
}


/** Writes the image in the specified file.
  NOTE: works with Grayscale or RGB modes only (based on number of channels)
 */
static inline void
write_JPEG_file(char *filename, int width, int height, int channels,
                unsigned char image[], int quality)
{
  FILE *outfile;
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }

  struct jpeg_error_mgr jerr;
  struct jpeg_compress_struct cinfo;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  jpeg_stdio_dest(&cinfo,outfile);

  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = channels;
  cinfo.in_color_space = channels == 1 ? JCS_GRAYSCALE : JCS_RGB;
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, quality, TRUE);

  jpeg_start_compress(&cinfo, TRUE);
  JSAMPROW rowptr[1];
  int row_stride = width * channels;
  while (cinfo.next_scanline < cinfo.image_height) {
    rowptr[0] = & image[cinfo.next_scanline * row_stride];
    jpeg_write_scanlines(&cinfo, rowptr, 1);
  }
  jpeg_finish_compress(&cinfo);

  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
}

vector < vector <int> > table;

void replaceCalc (int x, int y, int replace[]) {
  for (int v=0; v<table[y].size(); v++) {
    if (replace[table[y][v]]==-1){
      replace[table[y][v]]=x;
      replaceCalc(x, table[y][v], replace);
    }
  }
}


int main(int argc, char *argv[])
{
  if (argc < 3) {
    printf("Usage: ./ex1 image_in.jpg image_out.jpg\n");
    return 1;
  }

  unsigned char *image;
  int width, height, channels;
  read_JPEG_file(argv[1], &width, &height, &channels, &image);
  int threshold = 0;
  int histogram[256];

  vector < int > init;
  table.push_back( init );
  table[0].push_back(0);

  for (int i=0; i<255; i++) {
    histogram[i]=0;
  }

  for (int i=0; i<width*height; i++) {
    int intensity = image[i];
    histogram[intensity]+=1;
  }


  int cumulate=0, numerator=0, calculating = 1;
  double uh, ul;
  while (calculating){
    for (int j=0; j<=threshold; j++) {
      cumulate += histogram[j];
      numerator += j*histogram[j];
    }
    if (cumulate==0) ul=0;
    else ul = numerator/cumulate;
    numerator=0; cumulate=0;
    for (int j=threshold+1; j<255; j++) {
      cumulate += histogram[j];
      numerator += j*histogram[j];
    }
    if (cumulate==0) uh=0;
    else uh = numerator/cumulate;
    numerator=0; cumulate=0;
    int checkT = (int)((ul+uh)/2.0);
    if (checkT<=threshold) {
      calculating=0;
      threshold = checkT;
    } else {
      threshold += 1;
    }
  }

  for (int i=0; i<width*height; i++) {
    if (image[i]>threshold) image[i]=255;
    else image[i]=0;
  }

  int img2d[height][width];
  for (int i=0; i<height; i++){
    for (int j=0; j<width; j++) {
      img2d[i][j] = image[i*width+j];
    }
  }

  int neighbours[9];
  for (int i=0; i<9; i++) {
    neighbours[i]=0;
  }
  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {
      int count = 0;
      for (int i=0; i<9; i++) {
        neighbours[i]=0;
      }
      for (int k=i-1; k<=i+1; k++) {
        if (k>=0 && k<height) {
          for (int l=j-1; l<=j+1; l++) {
            if (l>=0 && l<width) {
              neighbours[count]=img2d[k][l];
              count += 1;
            }
          }
        }
      }
      for (int c=0; c<8; c++){
        for (int d=0; d<8-c; d++){
          if (neighbours[d]>neighbours[d+1]) {
            int swap = neighbours[d];
            neighbours[d] = neighbours[d+1];
            neighbours[d+1] = swap;
          }
        }
      }
      img2d[i][j]=neighbours[4];
    }
  }




  int labelled[height][width];
  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {
      labelled[i][j]=-1;
    }
  }
  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {
      if (img2d[i][j]==255) labelled[i][j] = 0;
      else {
        vector <int> nelabels;
        for (int k=i-1; k<=i; k++) {
          if (k>=0 && k<height) {
            for (int l=j-1; l<=j+1; l++) {
              if (l>=0 && l<width && !( (k==i && l==j) || (k==i && l==j+1) ) ) {
                if (img2d[k][l]==0) {
                  labelled[i][j]=labelled[k][l];
                  int inLabels = 0;
                  for (int v=0; v<nelabels.size(); v++) {
                    if (nelabels[v]==labelled[k][l]){
                      inLabels = 1;
                      break;
                    }
                  }
                  if (!inLabels) {
                    nelabels.push_back(labelled[k][l]);
                  }
                }
              }
            }
          }
        }
        if (labelled[i][j]==-1) {
          int s = table.size();
          labelled[i][j]=s;
          table.push_back(init);
          table[s].push_back(s);
        } else {
          for (int v=0; v<nelabels.size(); v++) {
            int lab = nelabels[v];
            table[lab].insert(table[lab].end(), nelabels.begin(), nelabels.end());
          }
        }
      }
    }
  }

  const int Size = table.size();
  int replace[Size];
  for (int i=0; i<Size; i++) {
    replace[i]=-1;
  }

  for (int v=0; v<table.size(); v++) {
    if (replace[v]==-1) {
      replaceCalc(v, v, replace);
    }
  }

  int replaceCopy [Size];
  for (int i=0; i<Size; i++) {
    replaceCopy[i]=replace[i];
  }
  sort(replaceCopy, replaceCopy+Size);
  int previous = replaceCopy[1];
  int change = 1;
  int count = 0;
  for (int i=1; i<Size; i++) {
    if (change) {
      count++;
      change = 0;
      for (int j=0; j<Size; j++) {
        if (replace[j]==previous) {
          replace[j]=count;
        }
      }
    }
    if (replaceCopy[i+1]>replaceCopy[i]) {
      previous = replaceCopy[i+1];
      change = 1;
    }
  }

  for (int v=0; v<height; v++) {
    for (int w=0; w<width; w++) {
      labelled[v][w] = replace[labelled[v][w]];
    }
  }

  ofstream fout;
  fout.open("labels.txt");
  for (int v=0; v<height; v++) {
    for (int w=0; w<width; w++) {
      if (labelled[v][w]<10) fout<<labelled[v][w];
      fout << labelled[v][w];
    }
    fout << "\n";
  }
  fout.close();

  for (int i=0; i<height; i++) {
    for (int j=0; j<width; j++) {
      image[i*width+j]=labelled[i][j];
    }
  }

  write_JPEG_file(argv[2], width, height, channels, image, 95);
  free(image);
  return 0;
}
