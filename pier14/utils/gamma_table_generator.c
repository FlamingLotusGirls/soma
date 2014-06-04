/* This code was written once to produce a gamma table, it wasn't really meant to be reused, but now it's clear how it was generated */

#include<stdio.h>
#include<math.h>

/* A constant that keeps values within 8 bits */
#define START 2*2*2*2*2*2*2 /* 127 */
#define GDELTA 0.04302 /* Curated to produce a wide gamut without overflowing */

/* Functions, sometimes we used them */
void genlows(int i);
void printarrays();

/* Everything is global */
/* (Sung to the tune of "Everything is Awesome") */
float a, b;
int on[256];
int off[256];

/* A big jumble of logic that generates a zero point, lows to the mid, mid, and then to up from there and prints */
int main()
{
    int i;
    on[0] = 0;
    off[0] = 32;
    a = START;
    b = START;
    genlows(1);
    a = START;
    b = START;
    on[127] = 32;
    off[127] = 32;
    for(i=128;i<=255;i++)
    {
        a*=1+GDELTA;
        on[i] = lround(a/(exp2(floor(log2f(a/b)))));
        off[i] = lround(b/(exp2(floor(log2f(a/b)))));
        //printf("%f,%f\n", a/(exp2(floor(log2f(a/b)))), b/(exp2(floor(log2f(a/b)))));
        //printf("%ld,%ld\n", lround(a/(exp2(floor(log2f(a/b))))), lround(b/(exp2(floor(log2f(a/b))))));
    }
    printarrays();
    return 0;
}

/* Generate lows, backwards so that the printfs can also be used and come out in the right order for creation of alternate formats */
void genlows(int i)
{
    float aa, bb;
    if(i == 127)
        return;
    a = a - (a * GDELTA);
    aa=a;
    bb=b;
    on[127-i] = lround((aa/aa)*(exp2(floor(log2f(aa))+1)));
    off[127-i] = lround((bb/aa)*(exp2(floor(log2f(aa))+1)));
    genlows(i+1);
    //printf("%ld,%ld\n", lround((aa/aa)*(exp2(floor(log2f(aa))+1))), lround((bb/aa)*(exp2(floor(log2f(aa))+1))));
    //printf("%f,%f\n", (aa/aa)*(exp2(floor(log2f(aa)))), (bb/aa)*(exp2(floor(log2f(aa)))));
    return;
}

/* Print arrays out in a C-like format */
void printarrays()
{
    int i;
    printf("uint8_t on[256] = { ");
    for(i=0;i<256;i++)
        if(i != 255)
            printf(" %d,", on[i]);
        else
            printf(" %d", on[i]);
    printf("};\n");
    printf("\n");
    printf("uint8_t off[256] = { ");
    for(i=0;i<256;i++)
        if(i != 255)
            printf(" %d,", off[i]);
        else
            printf(" %d", off[i]);
    printf("};\n");
}
