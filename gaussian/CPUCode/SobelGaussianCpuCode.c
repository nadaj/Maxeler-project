/***********************
 * GAUSSIAN NOISE FILTER
 * NADA JANKOVIC 468/12
 * MIPS
 ***********************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "Maxfiles.h"
#include <MaxSLiCInterface.h>
#include "ppmIO.h"

#define CYCLES 1				// Number of sorting repeats on CPU and Maxeler (for statistics)


unsigned long sec(struct timespec t1, struct timespec t2){
    return t2.tv_nsec>t1.tv_nsec?t2.tv_sec-t1.tv_sec:t2.tv_sec-t1.tv_sec-1;
}

unsigned long nsec(struct timespec t1, struct timespec t2){
    return t2.tv_nsec>t1.tv_nsec?t2.tv_nsec-t1.tv_nsec:t2.tv_nsec-t1.tv_nsec + 1000000000;
}

int main(void)
{
		int width = 500, height = 500;
		int dataSize = width * height * sizeof(int);

		struct timespec t1, t2, t3, t4;
		unsigned long s = 0;
		unsigned long ns = 0;
		unsigned long s_cpu_seq[CYCLES];		// time for sequential algorithm on CPU
		unsigned long ns_cpu_seq[CYCLES];
		unsigned long s_maxeler[CYCLES];		// time on Maxeler card
		unsigned long ns_maxeler[CYCLES];
		int k;

		int32_t *inImage = malloc(dataSize);
		int32_t *outImage = malloc(dataSize);
		float *noise = malloc(dataSize);
		float noise_temp = 0;
		float theta = 0;

		for(k = 0 ; k < CYCLES ; k++) {
			clock_gettime(CLOCK_REALTIME, &t1);

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					noise_temp = (float)(rand() % (32767 + 1 - 0) + 0)/32767.1;
					noise_temp = log(1.0 - noise_temp);
					noise_temp = sqrt(-2 * 3000 * noise_temp);
					theta = (float)(rand() % (32767 + 1 - 0) + 0) * 1.9175345E-4 - 3.14159265;
					noise_temp = noise_temp * cos(theta);
					noise[x+height*y] = noise_temp;
				}
			}

			SobelGaussian(width * height, inImage, noise, outImage);
			clock_gettime(CLOCK_REALTIME, &t2);

			s_maxeler[k] = sec(t1, t2);
			ns_maxeler[k] = nsec(t1,t2);

			s = s + sec(t1, t2) + (ns + nsec(t1,t2)) / 1000000000;
			ns = (ns + nsec(t1,t2)) % 1000000000;
			s = 0;
			ns = 0;
			writeImage("gaussianNoiseFilterMAXELER.ppm", outImage, width, height, 1);
		}
		s = 0;
		ns = 0;

		for(k = 0 ; k < CYCLES ; k++)
		{
			s = s + s_maxeler[k];
			ns = ns + ns_maxeler[k];
		}
		s = s + ns / 1000000000;
		ns = ns % 1000000000;

		printf("\nObrada na kartici zavrsena.\nVreme obrade: %lu s, %lu ns \n", s, ns);

		// SEQUENTIAL CODE
		s = 0;
		ns = 0;

		for(k = 0 ; k < CYCLES ; k++) {
			inImage = malloc(dataSize);
			srand(time(0));
			noise_temp = 0;
			theta = 0;

			clock_gettime(CLOCK_REALTIME, &t3);

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					noise_temp = (float)(rand() % (32767 + 1 - 0) + 0)/32767.1;
					noise_temp = log(1.0 - noise_temp);
					noise_temp = sqrt(-2 * 3000 * noise_temp);
					theta = (float)(rand() % (32767 + 1 - 0) + 0) * 1.9175345E-4 - 3.14159265;
					noise_temp = noise_temp * cos(theta);
					//noise = noise + 5;
					if (noise_temp > 255)
						noise_temp = 255;
					if (noise_temp < 0)
						noise_temp = 0;
					inImage[x+height*y] = (unsigned char)(noise_temp + 0.5);
				}
			}
			clock_gettime(CLOCK_REALTIME, &t4);
			s = s + sec(t3, t4) + (ns + nsec(t3,t4)) / 1000000000;
			ns = (ns + nsec(t3,t4)) % 1000000000;
			s_cpu_seq[k] = s;
			ns_cpu_seq[k] = ns;
			s = 0;
			ns = 0;
		}

		s = 0;
		ns = 0;

		for(k = 0 ; k < CYCLES ; k++)
		{
			s = s + s_cpu_seq[k];
			ns = ns + ns_cpu_seq[k];
		}
		s = s + ns / 1000000000;
		ns = ns % 1000000000;

		printf("Gaussian Noise FIlter na CPU zavrsen.\nVreme obrade: %lu s, %lu ns \n\n", s, ns);

		writeImage("gaussianNoiseFilter.ppm", inImage, width, height, 1);

		return 0;
}
