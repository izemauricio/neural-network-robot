#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <math.h>

#define EPOCAS 100000

#define BIAS 1.0
#define COEF 0.35

#define ENTRADAS 3
#define INTERMEDIARIOS 18
#define SAIDAS 4

#define LINHAS 579

double input[LINHAS][ENTRADAS];
double output[LINHAS][SAIDAS];

double w1[ENTRADAS+1][INTERMEDIARIOS];
double w2[INTERMEDIARIOS+1][SAIDAS];

void salvarPesos()
{
	FILE *arquivo;
	int i, j;

	arquivo = fopen("pesos.txt", "w");
	if (arquivo == NULL)
	{
		printf("fopen error\n");
	}

	for (i = 0; i<1 + ENTRADAS; i++)
		for (j = 0; j<INTERMEDIARIOS; j++)
			fprintf(arquivo, "rede.pesosEntradaInter[%d][%d] = %lf;\n", i, j, w1[i][j]);

	for (i = 0; i<1 + INTERMEDIARIOS; i++)
		for (j = 0; j<SAIDAS; j++)
			fprintf(arquivo, "rede.pesosInterSaida[%d][%d] = %lf;\n", i, j, w2[i][j]);

	for (i = 0; i<1 + ENTRADAS; i++)
		for (j = 0; j<INTERMEDIARIOS; j++)
			fprintf(arquivo, "w1[%d][%d]=%lf;\n", i, j, w1[i][j]);

	for (i = 0; i<1 + INTERMEDIARIOS; i++)
		for (j = 0; j<SAIDAS; j++)
			fprintf(arquivo, "w2[%d][%d]=%lf;\n", i, j, w2[i][j]);

	fclose(arquivo);
}

void lerArquivo()
{
	int x;
	FILE *ptr_file;

	ptr_file = fopen("dados.txt", "r");
	if (!ptr_file)
	{
		printf("fopen error\n");
	}

	x = 0;
	while (!feof(ptr_file) && x <= LINHAS)
	{
		fscanf(ptr_file, "%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t", &input[x][0], &input[x][1], &input[x][2], &output[x][0], &output[x][1], &output[x][2], &output[x][3]);

		output[x][0] = output[x][0] / 1000;
		output[x][1] = output[x][1] / 1000;
		output[x][2] = output[x][2] / 1000;
		output[x][3] = output[x][3] / 1000;

		x++;
	}

	fclose(ptr_file);
}

void fimprimirPesos()
{
	int l, c;

	for (l = 0; l<1 + ENTRADAS; l++)
		for (c = 0; c<INTERMEDIARIOS; c++)
			printf("w1[%d][%d]=%lf\n", l, c, w1[l][c]);

	for (l = 0; l<1 + INTERMEDIARIOS; l++)
		for (c = 0; c<SAIDAS; c++)
			printf("w2[%d][%d]=%lf\n", l, c, w2[l][c]);
}

double fgerarNumeroAleatorio()
{
	int MIN = -0.1;
	int MAX = +0.1;
	double n;

	n = (double)rand() / RAND_MAX;

	return(MIN + n*(MAX - MIN));
}

void fcolocarPesosAleatorios()
{
	int l, c;

	for (l = 0; l<1 + ENTRADAS; l++)
		for (c = 0; c<INTERMEDIARIOS; c++)
			w1[l][c] = fgerarNumeroAleatorio();

	for (l = 0; l<1 + INTERMEDIARIOS; l++)
		for (c = 0; c<SAIDAS; c++)
			w2[l][c] = fgerarNumeroAleatorio();
}

double fsigmoide(double sinal)
{
	//return(1/(1+(pow(M_E,-sinal))));
	return(1 / (1 + (exp(-sinal))));
}

void ftreinar()
{
	int epoca, linha;
	int i, n, l, c;

	double in_intermediaria[1 + ENTRADAS], in_saida[1 + INTERMEDIARIOS];
	double net, gradiente;

	double sigmoide_intermediaria[INTERMEDIARIOS];
	double sigmoide_saida[SAIDAS];
	double erro_intermediaria[INTERMEDIARIOS];
	double erro_saida[SAIDAS];

	printf("Treinando...\n");

	for (epoca = 0; epoca<EPOCAS; epoca++)
	{
		for (linha = 0; linha<LINHAS; linha++)
		{
			in_intermediaria[0] = BIAS;
			for (i = 1; i<1 + ENTRADAS; i++)
				in_intermediaria[i] = input[linha][i - 1];

			for (n = 0; n<INTERMEDIARIOS; n++)
			{
				net = 0;
				for (i = 0; i<1 + ENTRADAS; i++)
					net = net + in_intermediaria[i] * w1[i][n];
				sigmoide_intermediaria[n] = fsigmoide(net);
			}

			in_saida[0] = BIAS;
			for (i = 1; i<1 + INTERMEDIARIOS; i++)
				in_saida[i] = sigmoide_intermediaria[i - 1];

			for (n = 0; n<SAIDAS; n++)
			{
				net = 0;
				for (i = 0; i<1 + INTERMEDIARIOS; i++)
					net = net + in_saida[i] * w2[i][n];
				sigmoide_saida[n] = fsigmoide(net);
			}

			for (n = 0; n<SAIDAS; n++)
				erro_saida[n] = sigmoide_saida[n] * (1 - sigmoide_saida[n]) * (output[linha][n] - sigmoide_saida[n]);

			for (n = 0; n<INTERMEDIARIOS; n++)
			{
				gradiente = 0;
				for (i = 0; i<SAIDAS; i++)
					gradiente = gradiente + erro_saida[i] * w2[n + 1][i];
				erro_intermediaria[n] = sigmoide_intermediaria[n] * (1 - sigmoide_intermediaria[n])*(gradiente);
			}

			for (l = 0; l<1 + INTERMEDIARIOS; l++)
				for (c = 0; c<SAIDAS; c++)
					w2[l][c] = w2[l][c] + (COEF * erro_saida[c] * in_saida[l]);

			for (l = 0; l<1 + ENTRADAS; l++)
				for (c = 0; c<INTERMEDIARIOS; c++)
					w1[l][c] = w1[l][c] + (COEF * erro_intermediaria[c] * in_intermediaria[l]);

		}

		if ((epoca % (EPOCAS / 100)) == 0)
		{
			printf("%i concluido.\n", epoca / (EPOCAS / 100));
			for (i = 0; i<SAIDAS; i++)
				printf("%lf\n", erro_saida[i]);

		}
	}
}

void fcalcular(double minhasEntradas[ENTRADAS])
{
	int i, n;
	double in_intermediaria[1 + ENTRADAS], in_saida[1 + INTERMEDIARIOS];
	double net, gradiente;
	double sigmoide_intermediaria[INTERMEDIARIOS];
	double sigmoide_saida[SAIDAS];

	printf("Calculando...\n");

	in_intermediaria[0] = BIAS;
	for (i = 1; i<1 + ENTRADAS; i++)
		in_intermediaria[i] = minhasEntradas[i - 1];

	for (n = 0; n<INTERMEDIARIOS; n++)
	{
		net = 0;
		for (i = 0; i<1 + ENTRADAS; i++)
			net = net + in_intermediaria[i] * w1[i][n];
		sigmoide_intermediaria[n] = fsigmoide(net);
	}

	in_saida[0] = BIAS;
	for (i = 1; i<1 + INTERMEDIARIOS; i++)
		in_saida[i] = sigmoide_intermediaria[i - 1];

	for (n = 0; n<SAIDAS; n++)
	{
		net = 0;
		for (i = 0; i<1 + INTERMEDIARIOS; i++)
			net = net + in_saida[i] * w2[i][n];
		sigmoide_saida[n] = fsigmoide(net);
	}

	for (n = 0; n<SAIDAS; n++)
	{
		printf("sigmoide_saida[%d]=%lf\n", n, sigmoide_saida[n]);
	}
}

void mamama()
{
	// Dados de treinamento
	// Entrada=3 Intermediario=18 Saida=4 Linhas=579
	w1[0][0] = -0.099750;
	w1[0][1] = 0.012717;
	w1[0][2] = -0.061339;
	w1[0][3] = 0.061748;
	w1[0][4] = 0.017002;
	w1[0][5] = -0.004025;
	w1[0][6] = -0.029942;
	w1[0][7] = 0.079192;
	w1[0][8] = 0.064568;
	w1[0][9] = 0.049321;
	w1[0][10] = -0.065178;
	w1[0][11] = 0.071789;
	w1[0][12] = 0.042100;
	w1[0][13] = 0.002707;
	w1[0][14] = -0.039201;
	w1[0][15] = -0.097003;
	w1[0][16] = -0.081719;
	w1[0][17] = -0.027110;
	w1[1][0] = -0.070537;
	w1[1][1] = -0.066820;
	w1[1][2] = 0.097705;
	w1[1][3] = -0.010862;
	w1[1][4] = -0.076183;
	w1[1][5] = -0.099066;
	w1[1][6] = -0.098218;
	w1[1][7] = -0.024424;
	w1[1][8] = 0.006333;
	w1[1][9] = 0.014237;
	w1[1][10] = 0.020353;
	w1[1][11] = 0.021433;
	w1[1][12] = -0.066753;
	w1[1][13] = 0.032609;
	w1[1][14] = -0.009842;
	w1[1][15] = -0.029575;
	w1[1][16] = -0.088592;
	w1[1][17] = 0.021537;
	w1[2][0] = 0.056664;
	w1[2][1] = 0.060521;
	w1[2][2] = 0.003977;
	w1[2][3] = -0.039610;
	w1[2][4] = 0.075195;
	w1[2][5] = 0.045335;
	w1[2][6] = 0.091180;
	w1[2][7] = 0.085144;
	w1[2][8] = 0.007871;
	w1[2][9] = -0.071532;
	w1[2][10] = -0.007584;
	w1[2][11] = -0.052934;
	w1[2][12] = 0.072448;
	w1[2][13] = -0.058080;
	w1[2][14] = 0.055931;
	w1[2][15] = 0.068731;
	w1[2][16] = 0.099359;
	w1[2][17] = 0.099939;
	w1[3][0] = 0.022300;
	w1[3][1] = -0.021512;
	w1[3][2] = -0.046757;
	w1[3][3] = -0.040544;
	w1[3][4] = 0.068029;
	w1[3][5] = -0.095251;
	w1[3][6] = -0.024827;
	w1[3][7] = -0.081475;
	w1[3][8] = 0.035441;
	w1[3][9] = -0.088757;
	w1[3][10] = -0.098242;
	w1[3][11] = 0.083758;
	w1[3][12] = -0.044823;
	w1[3][13] = -0.045421;
	w1[3][14] = 0.017582;
	w1[3][15] = 0.038237;
	w1[3][16] = 0.067522;
	w1[3][17] = 0.045299;
	w2[0][0] = -0.003012;
	w2[0][1] = -0.058928;
	w2[0][2] = 0.048747;
	w2[0][3] = -0.006308;
	w2[1][0] = -0.008408;
	w2[1][1] = 0.089831;
	w2[1][2] = 0.048888;
	w2[1][3] = -0.078344;
	w2[2][0] = 0.019810;
	w2[2][1] = -0.022953;
	w2[2][2] = 0.047002;
	w2[2][3] = 0.021793;
	w2[3][0] = 0.014481;
	w2[3][1] = -0.027732;
	w2[3][2] = -0.069689;
	w2[3][3] = -0.054979;
	w2[4][0] = -0.014969;
	w2[4][1] = 0.060576;
	w2[4][2] = 0.003421;
	w2[4][3] = 0.097998;
	w2[5][0] = 0.050310;
	w2[5][1] = -0.030888;
	w2[5][2] = -0.066204;
	w2[5][3] = 0.031462;
	w2[6][0] = -0.001621;
	w2[6][1] = -0.087292;
	w2[6][2] = 0.039952;
	w2[6][3] = 0.000961;
	w2[7][0] = -0.070501;
	w2[7][1] = 0.089917;
	w2[7][2] = -0.071685;
	w2[7][3] = 0.081024;
	w2[8][0] = 0.038578;
	w2[8][1] = -0.039390;
	w2[8][2] = -0.014689;
	w2[8][3] = -0.085925;
	w2[9][0] = 0.093323;
	w2[9][1] = 0.036637;
	w2[9][2] = -0.069353;
	w2[9][3] = 0.075451;
	w2[10][0] = 0.064336;
	w2[10][1] = 0.016410;
	w2[10][2] = -0.061730;
	w2[10][3] = -0.064422;
	w2[11][0] = 0.063439;
	w2[11][1] = -0.004947;
	w2[11][2] = -0.068889;
	w2[11][3] = 0.000784;
	w2[12][0] = 0.046403;
	w2[12][1] = -0.018882;
	w2[12][2] = -0.044084;
	w2[12][3] = 0.013749;
	w2[13][0] = 0.036448;
	w2[13][1] = 0.051170;
	w2[13][2] = 0.044383;
	w2[13][3] = -0.004941;
	w2[14][0] = -0.075396;
	w2[14][1] = -0.026438;
	w2[14][2] = 0.066936;
	w2[14][3] = -0.092981;
	w2[15][0] = 0.003403;
	w2[15][1] = 0.032597;
	w2[15][2] = -0.014756;
	w2[15][3] = -0.079064;
	w2[16][0] = 0.089868;
	w2[16][1] = 0.084277;
	w2[16][2] = 0.009909;
	w2[16][3] = -0.030802;
	w2[17][0] = -0.005655;
	w2[17][1] = -0.025004;
	w2[17][2] = 0.069396;
	w2[17][3] = -0.036625;
	w2[18][0] = -0.008780;
	w2[18][1] = -0.045622;
	w2[18][2] = 0.096594;
	w2[18][3] = -0.040440;
}

int main()
{
	int i;
	double minhasEntradas[ENTRADAS];

	//fcolocarPesosAleatorios();
	//salvarPesos();
	mamama();
	lerArquivo();
	fimprimirPesos();
	ftreinar();
	fimprimirPesos();

	while (1)
	{
		for (i = 0; i<ENTRADAS; i++)
		{
			printf("Digite a entrada %d: ", i + 1);
			scanf("%lf", &minhasEntradas[i]);
		}

		fcalcular(minhasEntradas);
	}

	return 0;
}
