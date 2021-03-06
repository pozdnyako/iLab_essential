/*  14oct2018
        L-1.1.1*/
#define _CRT_SECURE_NO_WARNINGS /*macroses*/
#define _USE_MATH_DEFINES
#define M_PI 3.141592653589793238 /* pi */

#include <stdio.h>/*libraries*/
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "extraMeasurements.h"

FILE* fin; /*file data reservation*/
FILE* fout;
FILE* finD;

/*arrays*/
float** eCurrent = NULL;    /*contain electric current measurements*/
float** ePotential = NULL;  /*contain electric potential measurements*/
float** resists = NULL;     /*contain electric resistance measurements*/
float* diameters = NULL;    /*contain diameter measurements*/
float* eResistivity = NULL; /*contain electric resistivity measurements*/
float* absER =
    NULL;        /*contain absolute errors of electric resistivity measurements*/
float meanRs[3]; /*contain mean values of electric resistance measurements*/
float
    meanRsLSM[3]; /*contain mean values of electric resistance measurements by
                     least squares method*/
float
    accRLSM[3]; /*contain accidental error of electric resistance measurements
                   by least squares method*/
float
    sysRLSM[3];   /*contain systematic error of electric resistance measurements
                     by least squares method*/
float absRLSM[3]; /*contain absolute error of electric resistance measurements
                     by least squares method*/

float observationalError[3] = {
    0.007, 0.001, 0.001}; /*observational errors for used instruments*/
float observationalErrorD[1] = {0.01};
float observationalErrorL[3] = {1, 1, 1};
int wireLength[3] = {200, 300, 500}; /*lengths of wire used in research*/

/*essential variables*/
int number = 0; /*will be filled with length of dynamic arrays*/
int numberD = 0;
int numberER = 0;

void readFile() { /*reading file*/
  int i = 0;
  float eCurrentTemp = 0, ePotentialTemp = 0;
  ePotential = (float**)malloc(3 * sizeof(float*)); /*memory allocation*/
  eCurrent = (float**)malloc(3 * sizeof(float*));
  resists = (float**)malloc(3 * sizeof(float*));
  for (int j = 0; j < 3; j++) {
    eCurrent[j] = (float*)malloc(10 * sizeof(float));
    ePotential[j] = (float*)malloc(10 * sizeof(float));
    resists[j] = (float*)malloc(10 * sizeof(float));
  }
  while (fscanf(fin, "%f %f\n", &ePotentialTemp, &eCurrentTemp) !=
         EOF) { /*arrays filling*/
    ePotential[i / 10][i - 10 * (i / 10)] = ePotentialTemp;
    eCurrent[i / 10][i - 10 * (i / 10)] = eCurrentTemp;
    resists[i / 10][i - 10 * (i / 10)] = ePotentialTemp / eCurrentTemp;
    i++;
  }
  float diameter = 0;
  int j = 0;
  while (fscanf(finD, "%f\n", &diameter) != EOF) {
    diameters = (float*)realloc(diameters, (j + 1) * sizeof(float));
    diameters[j] = diameter;
    j++;
  }
  number = i;
  numberD = j;
}

float absoluteErrorResistivity(
    int index) { /*accidential error for electric resistivity*/
  return (eResistivity[index] *
          sqrt(pow(absRLSM[index] / meanRsLSM[index], 2) +
               pow(absoluteError(diameters, observationalErrorD, numberD, 0) /
                       meanValue(diameters, numberD),
                   2) +
               pow(observationalErrorL[index] / wireLength[index], 2)));
}

void printFile() { /*printing data in file*/
  for (int i = 0; i < sizeof(wireLength) / sizeof(float); i++) {
    fprintf(fout, "For wire length = %i\n", wireLength[i]);
    fprintf(fout,
            "\tMean value of Resistance:%f\n\tMean value of Resistance "
            "according to Least squares method:%f\n\tLSM Accidental "
            "error:%f\n\tLSM systematic error:%f\n\tLSM absolute error:%f\n",
            meanRs[i], meanRsLSM[i], accRLSM[i], sysRLSM[i], absRLSM[i]);
  }
  fprintf(fout,
          "\n\tDiameter: %f\n\tStandard Deviation:%f\n\tAccidential "
          "Error:%f\n\tAbsolute Error:%f\n\n",
          meanValue(diameters, numberD), standardDeviation(diameters, numberD),
          accidentalError(diameters, numberD),
          absoluteError(diameters, observationalErrorD, numberD, 0));
  for (int i = 0; i < sizeof(wireLength) / sizeof(float); i++) {
    fprintf(fout, "For wire length:%i\n\t", wireLength[i]);
    fprintf(fout,
            "Electric Resistivity:%f\n\tAbsolute Error of Electric "
            "Resistivity:%f\n\n",
            eResistivity[i], absER[i]);
  }
  fprintf(
      fout,
      "\t\tConclusion:\n\t\t\tElectric Resistivity = %f +- %f Ohm*Centimetre",
      (meanValue(eResistivity, numberER) * 1000),
      (meanValue(absER, numberER) * 1000));
}

int main() {
  fin = fopen("input.in", "r"); /*file opening*/
  finD = fopen("inputD.in", "r");
  fout = fopen("output.out", "w");
  readFile(); /*file reading*/
  float observationalErrorV = 0.529; /*observational errors for used instruments*/
  float observationalErrorA = 2.5;
  for (int i = 0; i < number / 10; i++) {
    meanRs[i] = meanValue(resists[i], number / 3); /*filling an arrays*/
    meanRsLSM[i] = leastSquaresMethod(eCurrent[i], ePotential[i], number / 3);
    accRLSM[i] =
        meanSquareAccidentalError(eCurrent[i], ePotential[i], number / 3);
    sysRLSM[i] =
        meanSquareSystematicError(eCurrent[i], ePotential[i], number / 3, observationalErrorA, observationalErrorV);
    absRLSM[i] = absoluteErrorLSM(eCurrent[i], ePotential[i], number / 3, observationalErrorA, observationalErrorV);
  }
  for (int i = 0; i < sizeof(meanRsLSM) / sizeof(float*); i++) {
    eResistivity = (float*)realloc(
        eResistivity,
        (i + 1) * sizeof(float)); /*memory reallocation and filling an arrays*/
    eResistivity[i] =
        (M_PI * meanRsLSM[i] * pow(meanValue(diameters, numberD), 2)) /
        (4 * wireLength[i]);
    absER = (float*)realloc(absER, (i + 1) * sizeof(float));
    absER[i] = absoluteErrorResistivity(i);
    numberER++;
  }
  printFile();  // printing
  free(eCurrent);  // free memory dynamic massives
  free(ePotential);
  free(resists);
  free(diameters);
  free(absER);
  free(eResistivity);
  fclose(fin); /*file closing*/
  fclose(finD);
  fclose(fout);
  system("PAUSE");
  return 0;
}
