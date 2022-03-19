#pragma once
#include <stdio.h>
#include <stdlib.h>

void *disseminationBarrier(void *threadid);
void *centralizedBarrier(void *threadid);
void *MCSBarrier(void *threadid);
