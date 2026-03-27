#include "v3math.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>

//homogenous, tests, and dynamic/static
//double check: v3_from_points and subtraction  

//calculates new vector based on two points 
//input: float* distance, *point a, *point b 
void v3_from_points(float *dst, float *a, float *b){
    //from a to b therefore b-a
    dst[0]=b[0]-a[0];
    dst[1]=b[1]-a[1];
    dst[2]=b[2]-a[2];
}

//performs vector addition 
//input: float* dst, *point a, *point b
void v3_add(float *dst, float *a, float *b){
    dst[0]=a[0]+b[0];
    dst[1]=a[1]+b[1];
    dst[2]=a[2]+b[2];
}

//subtract the two vectors (a-b)
//input: float* dst, *point a, *point b
void v3_subtract(float *dst, float *a, float *b){
    dst[0]=a[0]-b[0];
    dst[1]=a[1]-b[1];
    dst[2]=a[2]-b[2];
}

//get the dot product of two vectors 
//input:float *point a, *point b
//output: 
float v3_dot_product(float *a, float *b){
    return (a[0]*b[0]+a[1]*b[1]+a[2]*b[2]);
}

//get the cross product (normal) of two vectors 
//input: float* dst, *point a, *point b
void v3_cross_product(float *dst, float *a, float *b){
    dst[0]=a[1]*b[2]-a[2]*b[1];
    dst[1]=a[2]*b[0]-a[0]*b[2];
    dst[2]=a[0]*b[1]-a[1]*b[0];
}

//scales a vector
//input: float* destination, float scale
void v3_scale(float *dst, float s){
    dst[0]=dst[0]*s;
    dst[1]=dst[1]*s;
    dst[2]=dst[2]*s;
}


//gets the angle between a and b 
//input: float* a and b 
float v3_angle(float *a, float *b){
    return acos(v3_dot_product(a,b)/(v3_length(a)*v3_length(b)));
}

//gets an angle equavilent (no acos)
//input: float* a and b 
float v3_angle_quick(float *a, float *b){
    return v3_dot_product(a,b)/(v3_length(a)*v3_length(b));
}

//reflect a vector across a normal
//input: float *destination, float *vector, float *normal 
void v3_reflect(float *dst, float *v, float *n){
    float temp[3];
    v3_normalize(temp, n);
    v3_scale(temp,2*v3_dot_product(v,temp));
    v3_subtract(dst,v,temp);
}

//get the length of a vector 
//input: float *vectora
//output float length
float v3_length(float *a)
{
    //distance equation
    return (sqrt((a[0] * a[0]+a[1] * a[1])+a[2] * a[2]));
}

//normalizes a vector 
//input: float *destination, float *vectora
void v3_normalize(float *dst, float *a){
    float length=v3_length(a);
    dst[0]=a[0]/length; 
    dst[1]=a[1]/length; 
    dst[2]=a[2]/length; 
}

bool v3_equals(float *a, float *b, float tolerance){
    for(int i=0; i<3; i++){
        if(a[i]>b[i]+tolerance||a[i]<b[i]-tolerance){
            printf("index %d is not equal\n",i);
            return 0;
        }
    }
    return 1; 
}