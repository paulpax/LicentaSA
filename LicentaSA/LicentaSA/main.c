#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>


//defineuri ptr min/max E
#define EPv_Min 0
#define EGeo_Min 0
#define EBio_Min 0
#define EBat_Min 1000 //Energy_Bat_Min = Ebat descarcare
#define EGeo_Max 1500
#define EBio_Max 1500
#define EBat_Max -3500 //Energy_Bat_Max = Ebat incarcare
#define ES_Limit 24000 //limita sistemului de energie

//defineuri ptr costuri
#define CPv 0.05
#define CGeo 0.25
#define CBio 0.3
#define CBat 0.55
#define CExcess 1.5

//variabile globale
const int EPv_Max[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 300, 643, 917, 1059, 1038, 859, 562, 217, 0, 0, 0, 0, 0, 0, 0, 0 };//EPV MAX ptr cele 24 ore(doar ptr dec)
const int ELoad[24] = { 0, 250, 2800, 2700, 0, 300, 600, 3400, 3500, 600, 600, 100, 200, 0, 0, 300, 0, 200, 3500, 3600, 3600, 3700, 3500, 0 };//Eload ptr cele 24ore

int randEGeo()
{
	int random;
	random = rand() % (EGeo_Max + 1 - EGeo_Min) + EGeo_Min;
	return random;
}

int randEBio()
{
	int random;
	random = rand() % (EBio_Max + 1 - EBio_Min) + EBio_Min;
	return random;
}

int randEPv(int hour)
{
	int random;
	random = rand() % (EPv_Max[hour] + 1 - EPv_Min) + EPv_Min;
	return random;
}

bool battery_validation(int EBat)
{
	if ((EBat >= -3500) && (EBat <= 1000))
		return true;
	else
		return false;
}

bool systemEnergy_validation(int Es)
{
	if ((Es >= 0) && (Es <= ES_Limit))
		return true;
	else
		return false;
}

double calculateTotalCosts(int EPv[], int EGeo[], int EBio[], int EBat[], int EExcess)
{
	double sum = 0.0;
	for (int hour = 0; hour < 24; hour++)
	{
		sum = sum + ((EPv[hour] * CPv) + (EGeo[hour] * CGeo) + (EBio[hour] * CBio) + (EBat[hour] * CBat) + (EExcess * CExcess));
	}
	return sum;
}

int main()
{
	int EPv[24], EGeo[24], EBio[24], EBat[24], EExcess = 0, Es[24];
	double objective_function = 0.0;
	bool ok;
	Es[0] = 20000;
	srand(time(NULL));

	for (int hour = 0; hour < 24; hour++)
	{
		ok = false;
		while (ok == false)
		{
			EPv[hour] = randEPv(hour);
			EGeo[hour] = randEGeo();
			EBio[hour] = randEBio();

			EBat[hour] = ELoad[hour] - EPv[hour] - EGeo[hour] - EBio[hour] - EExcess;
			if (battery_validation(EBat[hour] == true))
			{
				if (hour == 0)
				{
					ok = true;
				}
				else
				{
					Es[hour] = Es[hour - 1] - EBat[hour];
					if (systemEnergy_validation(Es[hour]) == true)
						ok = true;
				}
			}
		}
	}
	objective_function = calculateTotalCosts(EPv, EGeo, EBio, EBat, EExcess);

	for (int i = 0; i < 24; i++)
	{
		printf("Epv[%d]: %d", i, EPv[i]);
		printf("\n");
		printf("Egeo[%d]: %d", i, EGeo[i]);
		printf("\n");
		printf("EBio[%d]: %d", i, EBio[i]);
		printf("\n");
		printf("ELoad[%d]: %d", i, ELoad[i]);
		printf("\n");
		printf("EBat[%d]: %d", i, EBat[i]);
		printf("\n");
		printf("Es[%d]: %d", i, Es[i]);
		printf("\n");
	}
	printf("Functia obiectiv: %f", objective_function);

	return (0);
}
