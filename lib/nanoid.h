#pragma once
#include<unistd.h>
#define NANOIDLEN 21

/* symbols (A-Za-z0-9_-) */
static inline int nanoidgen(char*b,size_t n){int r=getentropy(b,n);if(!r)for(char c;n--;b[n]+=c?c<2?44:c<12?46:c<38?53:59:95)c=b[n]&=63;return r;}

/* symbols (A-Za-z0-9), that without (_-) */
static inline int nanoidgen2(char*b,size_t n){int r=getentropy(b,n);if(!r)for(char c;n--;b[n]+=c?c<2?47:c<12?46:c<38?53:59:90)c=b[n]&=63;return r;}
