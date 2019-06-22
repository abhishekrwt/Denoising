#include<iostream>
#include<stdio.h>
#include<string>
#include<pthread.h>
#include<time.h>
#define thrd 4
using namespace std;
struct BMP {
    int width;
    int height;
    unsigned char header[54];
    unsigned char *pixels;
    int sz;
};

BMP image,Nimage;

void readBMP(string filename) {
    int i;
    string fileName = filename;
    FILE *f = fopen(fileName.c_str(), "rb");
    fread(image.header, sizeof(unsigned char), 54, f); // read the 54-byte header

    image.width = *(int *) &image.header[18];
    image.height = *(int *) &image.header[22];

    image.sz = 3 * image.width * image.height;
    image.pixels = new unsigned char[image.sz]; // allocate 3 bytes per pixel
    fread(image.pixels, sizeof(unsigned char), image.sz, f); // read the rest of the data at once
    fclose(f);

    for (i = 0; i < image.sz; i += 3) {
        unsigned char tmp = image.pixels[i];
        image.pixels[i] = image.pixels[i + 2];
        image.pixels[i + 2] = tmp;
    }
}

void writeBMP(string filename) {
    string fileName = filename;
    FILE *out = fopen(fileName.c_str(), "wb");
    fwrite(Nimage.header, sizeof(unsigned char), 54, out);
    int i;
    unsigned char tmp;
    for (i = 0; i < Nimage.sz; i += 3) {
        tmp = Nimage.pixels[i];
        Nimage.pixels[i] = Nimage.pixels[i + 2];
        Nimage.pixels[i + 2] = tmp;
    }
    fwrite(Nimage.pixels, sizeof(unsigned char), image.sz, out); // read the rest of the data at once
    fclose(out);
}

int average(int x,int y,int f){
    int sum=0,i,j;
    for(i=0;i<3;i++){
        for(j=0;j<3;j++){
            sum+=image.pixels[((y-1+j) * image.height + (x+i-1)) * 3 + f];
        }
    }
    return (sum/9);
}

void *denoise(void *arg) {
    int value=*(int*)arg;
    for (int i = (value-1)*(image.width/thrd); i < value*(image.width/thrd); i++) {
        for (int j = 1; j < image.height-1; j++) {
            Nimage.pixels[(j * image.height + i) * 3 + 0] = average(i,j,0);
            Nimage.pixels[(j * image.height + i) * 3 + 1] = average(i,j,1);
            Nimage.pixels[(j * image.height + i) * 3 + 2] = average(i,j,2);
        }
    }
    return(NULL);
}


int main() {
    pthread_t tid[thrd];
    int i,x[]={1,2,3,4,5,6};
    clock_t t;
	double tt;
	t=clock();
    readBMP("lenna.bmp");
    Nimage = image;
    Nimage.pixels = new unsigned char[image.sz];
    for(i=0;i<thrd;i++){
		pthread_create(&tid[i],NULL,denoise,&x[i]);
	}
	for(i=0;i<thrd;i++){
		pthread_join(tid[i],NULL);
	}
    writeBMP("secondout.bmp");
    t=clock()-t;
	tt=((double)t)/CLOCKS_PER_SEC;
	cout<<"Time taken using 4 threads is "<<tt<<endl;
    return 0;
}
