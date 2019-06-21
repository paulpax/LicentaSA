#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


//defineuri ptr min/max E
#define EPv_Min 0
#define EGeo_Min 0
#define EBio_Min 0
#define EBat_Min -3500 //Energy_Bat_Max = Ebat incarcare
#define Es_Min 0
#define EGeo_Max 1500
#define EBio_Max 1500
#define EBat_Max 1000
#define ES_Limit 24000 //limita sistemului de energie

//defineuri ptr costuri
#define CPv 0.05
#define CGeo 0.25
#define CBio 0.3
#define CBat 0.55
#define CExcess 1.5

//definirea memoriei
struct Memory
{
	double objective_function;
	unsigned int EPv[24];
	unsigned int EGeo[24];
	unsigned int EBio[24];
	unsigned int ELoad[24];
	unsigned int EExcess[24];
	int EBat[24];
	int Es[24];
};

//variabile globale
const unsigned int EPv_Max[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 300, 643, 917, 1059, 1038, 859, 562, 217, 0, 0, 0, 0, 0, 0, 0, 0 };//EPV MAX ptr cele 24 ore(doar ptr dec)
const unsigned int ELoad[24] = { 0, 250, 2800, 2700, 0, 300, 600, 3400, 3500, 600, 600, 100, 200, 0, 0, 300, 0, 200, 3500, 3600, 3600, 3700, 3500, 0 };//Eload ptr cele 24ore

unsigned int EPv[24], EGeo[24], EBio[24], EExcess[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int EBat[24], Es[24];
double objective_function = 0.0;

unsigned int randEGeo()
{
	int random;
	random = rand() % ((EGeo_Max + 1 - EGeo_Min) + EGeo_Min);
	return random;
}

unsigned int randEBio()
{
	int random;
	random = rand() % ((EBio_Max + 1 - EBio_Min) + EBio_Min);
	return random;
}

unsigned int randEPv(unsigned int hour)
{
	int random;
	random = rand() % ((EPv_Max[hour] + 1 - EPv_Min) + EPv_Min);
	return random;
}

double randParameter()
{
	double random;
	random = (double)rand() / (double)((unsigned)RAND_MAX + 1);
	return random;
}

//bool battery_validation(int EBat)
//{
//	if ((EBat >= EBat_Min) && (EBat <= EBat_Max))
//		return true;
//	else
//		return false;
//}

bool systemEnergy_validation(int Es)
{
	if ((Es >= Es_Min) && (Es <= ES_Limit))
		return true;
	else
		return false;
}

double calculateTotalCosts(unsigned int EPv[], unsigned int EGeo[], unsigned int EBio[], int EBat[], unsigned int EExcess[])
{
	double sum = 0.0;
	for (unsigned int hour = 0; hour < 24; hour++)
	{
		sum = sum + ((EPv[hour] * CPv) + (EGeo[hour] * CGeo) + (EBio[hour] * CBio) + (EBat[hour] * CBat) + (EExcess[hour] * CExcess));
	}
	return sum;
}

void generateSolution()
{
	bool ok = false;
	Es[0] = 20000;
	for (unsigned int hour = 0; hour < 24; hour++)
	{
		ok = false;
		while (ok == false)
		{
			EPv[hour] = randEPv(hour);
			EGeo[hour] = randEGeo();
			EBio[hour] = randEBio();

			EBat[hour] = ELoad[hour] - EPv[hour] - EGeo[hour] - EBio[hour] - EExcess[hour];

			EExcess[hour] = 0;
			if (EBat[hour] < EBat_Min)
			{
				EExcess[hour] = (EBat[hour] - EBat_Min) * (-1);//ptr a avea valoare pozitiva
				EBat[hour] = EBat_Min;
			}
			if (EBat[hour] > EBat_Max)
			{
				EExcess[hour] = EBat[hour] - EBat_Max;
				EBat[hour] = EBat_Max;
			}

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

unsigned int findMinimum(struct Memory populationMemory[])
{
	double min = populationMemory[0].objective_function;
	unsigned int location = 0;
	for (unsigned int j = 1; j < 500; j++)//caut costul minim din memorie
	{
		if (populationMemory[j].objective_function < min)
		{
			min = populationMemory[j].objective_function;
			location = j;//memorez locatia costului minim din memorie
		}
	}
	return location;
}

unsigned int findMaximum(struct Memory populationMemory[])
{
	double max = populationMemory[0].objective_function;
	unsigned int location = 0;
	for (unsigned int j = 1; j < 500; j++)
	{
		if (populationMemory[j].objective_function > max)
		{
			max = populationMemory[j].objective_function;
			location = j;
		}
	}
	return location;
}

int main()
{
	unsigned int EPv_final[24], EGeo_final[24], EBio_final[24], EExcess_final[24], location_max = 0, location_min = 0, ELoad_final[24];
	int EBat_final[24], Es_final[24];
	double predefined_parameter = 0.5, random_parameter;
	double objective_function_final = 0.0;
	struct Memory populationMemory[500];
	bool Es_ok = false;

	srand(time(NULL));

	FILE *f;
	f = fopen("rezultat.txt", "w");
	if (f == NULL)
	{
		printf("Eroare la deschierea fisierului rezultat.txt!");
		exit(1);
	}

	printf("Incep sa populez memoria cu 500 de solutii valide.\n");
	printf("Am ajuns pe la iteratia: ");
	for (unsigned int i = 0; i < 500; i++)//se populeaza cu 500 de solutii
	{
		printf("%d\n", i);
		generateSolution();
		//se copiaza in memorie 500 de solutii posibile(populatia initiala)
		objective_function = calculateTotalCosts(EPv, EGeo, EBio, EBat, EExcess);
		populationMemory[i].objective_function = objective_function;
		memcpy(populationMemory[i].EPv, EPv, sizeof(EPv));
		memcpy(populationMemory[i].EGeo, EGeo, sizeof(EGeo));
		memcpy(populationMemory[i].EBio, EBio, sizeof(EBio));
		memcpy(populationMemory[i].ELoad, ELoad, sizeof(ELoad));
		memcpy(populationMemory[i].EBat, EBat, sizeof(EBat));
		memcpy(populationMemory[i].Es, Es, sizeof(Es));
		memcpy(populationMemory[i].EExcess, EExcess, sizeof(EExcess));
	}

	printf("Incep sa utilizez SA.\n");
	printf("Am ajuns pe la iteratia: ");
	for (unsigned int i = 0; i < 500; i++)//500 de iteratii
	{
		printf("%d\n", i);
		random_parameter = randParameter();//param random
		if (random_parameter <= predefined_parameter)
		{
			location_max = findMaximum(populationMemory);
			location_min = findMinimum(populationMemory);

			for (unsigned int hour2 = 0; hour2 < 24; hour2++)
			{
				Es_ok = false;
				while (Es_ok == false)
				{
					populationMemory[location_max].EGeo[hour2] = populationMemory[location_max].EGeo[hour2] + (randParameter() - randParameter()) * randParameter() * abs(populationMemory[location_max].EGeo[hour2] - populationMemory[location_min].EGeo[hour2]);
					if (populationMemory[location_max].EGeo[hour2] <= EGeo_Min)
					{
						populationMemory[location_max].EGeo[hour2] = EGeo_Min;
					}
					if (populationMemory[location_max].EGeo[hour2] >= EGeo_Max)
					{
						populationMemory[location_max].EGeo[hour2] = EGeo_Max;
					}

					populationMemory[location_max].EBio[hour2] = populationMemory[location_max].EBio[hour2] + (randParameter() - randParameter()) * randParameter() * abs(populationMemory[location_max].EBio[hour2] - populationMemory[location_min].EBio[hour2]);
					if (populationMemory[location_max].EBio[hour2] <= EBio_Min)
					{
						populationMemory[location_max].EBio[hour2] = EBio_Min;
					}
					if (populationMemory[location_max].EBio[hour2] >= EBio_Max)
					{
						populationMemory[location_max].EBio[hour2] = EBio_Max;
					}

					populationMemory[location_max].EPv[hour2] = populationMemory[location_max].EPv[hour2] + (randParameter() - randParameter()) * randParameter() * abs(populationMemory[location_max].EPv[hour2] - populationMemory[location_min].EPv[hour2]);
					if (populationMemory[location_max].EPv[hour2] <= EPv_Min)
					{
						populationMemory[location_max].EPv[hour2] = EPv_Min;
					}
					if (populationMemory[location_max].EPv[hour2] >= EPv_Max[hour2])
					{
						populationMemory[location_max].EPv[hour2] = EPv_Max[hour2];
					}

					populationMemory[location_max].EBat[hour2] = populationMemory[location_max].ELoad[hour2] - populationMemory[location_max].EPv[hour2] - populationMemory[location_max].EGeo[hour2] - populationMemory[location_max].EBio[hour2] - populationMemory[location_max].EExcess[hour2];

					populationMemory[location_max].EExcess[hour2] = 0;
					if (populationMemory[location_max].EBat[hour2] < EBat_Min)
					{
						populationMemory[location_max].EExcess[hour2] = (populationMemory[location_max].EBat[hour2] - EBat_Min) * (-1);//ptr a avea valoare pozitiva
						populationMemory[location_max].EBat[hour2] = EBat_Min;
					}
					if (populationMemory[location_max].EBat[hour2] > EBat_Max)
					{
						populationMemory[location_max].EExcess[hour2] = populationMemory[location_max].EBat[hour2] - EBat_Max;
						populationMemory[location_max].EBat[hour2] = EBat_Max;
					}

					if (hour2 == 0)
					{
						Es_ok = true;
					}
					else
					{
						populationMemory[location_max].Es[hour2] = populationMemory[location_max].Es[hour2 - 1] - populationMemory[location_max].EBat[hour2];
						if (systemEnergy_validation(populationMemory[location_max].Es[hour2]) == true)
							Es_ok = true;
					}
				}
			}
			populationMemory[location_max].objective_function = calculateTotalCosts(populationMemory[location_max].EPv, populationMemory[location_max].EGeo, populationMemory[location_max].EBio, populationMemory[location_max].EBat, populationMemory[location_max].EExcess);
		}
		else
		{
			location_max = findMaximum(populationMemory);
			generateSolution();
			objective_function = calculateTotalCosts(EPv, EGeo, EBio, EBat, EExcess);
			if (objective_function < populationMemory[location_max].objective_function)//se compara noua solutie generata random cu cea mai "rea" din memorie
			{
				populationMemory[location_max].objective_function = objective_function;
				memcpy(populationMemory[location_max].EPv, EPv, sizeof(EPv));
				memcpy(populationMemory[location_max].EGeo, EGeo, sizeof(EGeo));
				memcpy(populationMemory[location_max].EBio, EBio, sizeof(EBio));
				memcpy(populationMemory[location_max].ELoad, ELoad, sizeof(ELoad));
				memcpy(populationMemory[location_max].EBat, EBat, sizeof(EBat));
				memcpy(populationMemory[location_max].Es, Es, sizeof(Es));
				memcpy(populationMemory[location_max].EExcess, EExcess, sizeof(EExcess));
			}
		}
	}

	printf("Am terminat de executat SA. Memorez si afisez solutia finala.\n");
	//Se memoreaza rezultatul final
	location_min = findMinimum(populationMemory);
	objective_function_final = populationMemory[location_min].objective_function;
	memcpy(EPv_final, populationMemory[location_min].EPv, sizeof(populationMemory[location_min].EPv));
	memcpy(EGeo_final, populationMemory[location_min].EGeo, sizeof(populationMemory[location_min].EGeo));
	memcpy(EBio_final, populationMemory[location_min].EBio, sizeof(populationMemory[location_min].EBio));
	memcpy(ELoad_final, populationMemory[location_min].ELoad, sizeof(populationMemory[location_min].ELoad));
	memcpy(EBat_final, populationMemory[location_min].EBat, sizeof(populationMemory[location_min].EBat));
	memcpy(Es_final, populationMemory[location_min].Es, sizeof(populationMemory[location_min].Es));
	memcpy(EExcess_final, populationMemory[location_min].EExcess, sizeof(populationMemory[location_min].EExcess));

	//Se afiseaza rezultatul final
	printf("Rezultatul final pentru luna decembrie: \n");
	fprintf(f, "Rezultatul final pentru luna decembrie: \n");
	printf("Cost: %f\n", objective_function_final);
	fprintf(f, "Cost: %f\n", objective_function_final);
	printf("Hour:|\tEPv:|\tEGeo:|\tEBio:|\tEBat:|\tEExcess:|\tEs:|\tELoad:|\n");
	fprintf(f, "Hour:|\tEPv:|\tEGeo:|\tEBio:|\tEBat:|\tEExcess:|\tEs:|\tELoad:|\n");
	for (unsigned int i = 0; i < 24; i++)
	{
		printf("%d|\t%d|\t%d|\t%d|\t%d|\t%d|\t%d|\t%d|\t", i + 1, EPv_final[i], EGeo_final[i], EBio_final[i], EBat_final[i], EExcess_final[i], Es_final[i], ELoad_final[i]);
		printf("\n");
		fprintf(f, "%d|\t%d|\t%d|\t%d|\t%d|\t%d|\t%d|\t%d|\t", i + 1, EPv_final[i], EGeo_final[i], EBio_final[i], EBat_final[i], EExcess_final[i], Es_final[i], ELoad_final[i]);
		fprintf(f, "\n");
	}

	fclose(f);
	return (0);
}
/*
Se bloccheaza la iteratia 64 la generarea solutiilor.
*/