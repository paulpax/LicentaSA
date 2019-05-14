#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


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

//definirea memoriei
struct Memory
{
	double objective_function;
	unsigned int EPv[24];
	unsigned int EGeo[24];
	unsigned int EBio[24];
	unsigned int ELoad[24];
	unsigned int EExcess;
	int EBat[24];
	int Es[24];
};

//variabile globale
const unsigned int EPv_Max[24] = { 0, 0, 0, 0, 0, 0, 0, 0, 300, 643, 917, 1059, 1038, 859, 562, 217, 0, 0, 0, 0, 0, 0, 0, 0 };//EPV MAX ptr cele 24 ore(doar ptr dec)
const unsigned int ELoad[24] = { 0, 250, 2800, 2700, 0, 300, 600, 3400, 3500, 600, 600, 100, 200, 0, 0, 300, 0, 200, 3500, 3600, 3600, 3700, 3500, 0 };//Eload ptr cele 24ore

unsigned int EPv[24], EGeo[24], EBio[24], EExcess = 0;
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

double calculateTotalCosts(unsigned int EPv[], unsigned int EGeo[], unsigned int EBio[], int EBat[], unsigned int EExcess)
{
	double sum = 0.0;
	for (unsigned int hour = 0; hour < 24; hour++)
	{
		sum = sum + ((EPv[hour] * CPv) + (EGeo[hour] * CGeo) + (EBio[hour] * CBio) + (EBat[hour] * CBat) + (EExcess * CExcess));
	}
	return sum;
}

void generateSolution()
{
	bool ok;
	for (unsigned int hour = 0; hour < 24; hour++)
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

int main()
{
	unsigned int EPv_final[24], EGeo_final[24], EBio_final[24], EExcess_final = 0, location, ELoad_final[24];
	int EBat_final[24], Es_final[24];
	double predefined_parameter = 0.5, random_parameter;
	double objective_function_final = 0.0;
	struct Memory populationMemory[500];

	Es[0] = 20000;
	srand(time(NULL));

	for (unsigned int i = 0; i < 500; i++)
	{
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
		populationMemory[i].EExcess = EExcess;
	}
	
	random_parameter = randParameter();//parametul random ptr comparare

	for (unsigned int i = 0; i < 500; i++)//500 de iteratii
	{
		if (random_parameter < predefined_parameter)//Q: daca o sa imi dea nr random = parametru predefinit?
		{
			location = findMinimum(populationMemory);
			objective_function_final = populationMemory[location].objective_function;
			memcpy(EPv_final, populationMemory[location].EPv, sizeof(populationMemory[location].EPv));
			memcpy(EGeo_final, populationMemory[location].EGeo, sizeof(populationMemory[location].EGeo));
			memcpy(EBio_final, populationMemory[location].EBio, sizeof(populationMemory[location].EBio));
			memcpy(ELoad_final, populationMemory[location].ELoad, sizeof(populationMemory[location].ELoad));
			memcpy(EBat_final, populationMemory[location].EBat, sizeof(populationMemory[location].EBat));
			memcpy(Es_final, populationMemory[location].Es, sizeof(populationMemory[location].Es));
			EExcess = populationMemory[location].EExcess;
		}
		else
		{
			generateSolution();//creaza o solutie noua si o memoreaza
			objective_function = calculateTotalCosts(EPv, EGeo, EBio, EBat, EExcess);
			populationMemory[i].objective_function = objective_function;
			memcpy(populationMemory[0].EPv, EPv, sizeof(EPv));
			memcpy(populationMemory[0].EGeo, EGeo, sizeof(EGeo));
			memcpy(populationMemory[0].EBio, EBio, sizeof(EBio));
			memcpy(populationMemory[0].ELoad, ELoad, sizeof(ELoad));
			memcpy(populationMemory[0].EBat, EBat, sizeof(EBat));
			memcpy(populationMemory[0].Es, Es, sizeof(Es));
			populationMemory[0].EExcess = EExcess;

		}
	}

	//for (unsigned int i = 0; i < 24; i++)
	//{
	//	printf("Epv[%d]: %d", i, populationMemory[30].EPv[i]);
	//	printf("\n");
	//	printf("Egeo[%d]: %d", i, populationMemory[30].EGeo[i]);
	//	printf("\n");
	//	printf("EBio[%d]: %d", i, populationMemory[30].EBio[i]);
	//	printf("\n");
	//	printf("ELoad[%d]: %d", i, populationMemory[30].ELoad[i]);
	//	printf("\n");
	//	printf("EBat[%d]: %d", i, populationMemory[30].EBat[i]);
	//	printf("\n");
	//	printf("Es[%d]: %d", i, populationMemory[30].Es[i]);
	//	printf("\n");
	//}
	printf("Functia obiectiv: %f\n", populationMemory[30].objective_function);
	printf("param random: %f", random_parameter);

	return (0);
}
/* 1. Am memorat 500 de solutii initiale. Din pacate daca depasesc 20 de iteratii consola imi ramane blocata cu
semnul _(ca si cum ar cauta solutii valide in continuare). Tin sa mentionez ca momentan am un laptop cu 4gb ram 
si HDD cu RPM specifice unui laptop
2. Parametrul random ce v a fi comparat cu parametrul predefinit(= 0.5) este setat doar o singura data
3. Memorarea solutiei finale se realizeaza incepand de la linia 178.
*Oare ar trebui sa verific unde se afla costul minim in memorie, iar mai apoi solutia noua generata sa 
NU o memorez unde se afla costul minim din memorie?
*Ar trebui sa mai verific inca odata costul minim din memorie si sa o memorez in variabila solutiei finale dupa ce
se realizeaza cele 500 de iteratii?
*/