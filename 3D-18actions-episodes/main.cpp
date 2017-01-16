#include <iostream>
#include <stdio.h>
#include<string.h>
#include <Windows.h> // gettickcount
#include <stdlib.h> // rand 
#include <time.h>
#include <omp.h> // openmp头文件

using namespace std;

const int dim=4; //5*5%
const int obs = 30; //zhang ai wu
const double o_r = -1.0; //zhang ai wu
const double w_r = -10; //wall
const double g_r = 10.0; // goal


//int map[dim][dim][dim] = {0};
int map[dim*dim*dim]={0};

int obs_index[obs] = { 78, 24, 19, 117, 18, 23, 42, 9, 103, 52, 45, 100, 59, 41, 43, 120, 55, 114, 25, 62, 10, 57, 39, 33, 89, 88, 99, 119, 48, 46 };//enough
//int obs_index[obs]={0};

int opa[18] = { 1, 0, 3, 2, 5, 4, 9, 8, 7, 6, 15, 14, 17, 16, 11, 10, 13, 12 };

double reward[dim*dim*dim][18] ;
//up,down,front,back,left,right
double q[dim*dim*dim][18] ;

double pq[dim*dim*dim][18];
double qvalue[10000] = {0};
void sumqvalue(int e);

void generate_obs();
void generate_map();
void print_obs();
void print_map();
void generate_reward();
void generate_q();
void qlearning(int n,double e,double alpha,double gammma);

int maxaction(int n);

int currentstate(int s, int a);

int main(int argc, char*argv[])
{
	//print_obs();
	//print_map();

	int i, j, k, t,n;
	srand((unsigned)time(NULL));
	//generate_obs();
	generate_map();

//	print_obs();
//	print_map();

	generate_reward();
	generate_q();//initialize q


	FILE *file0; //reward, Q
	fopen_s(&file0, "0initialr&q.txt", "wt");
	
	for (i = 0; i < dim*dim*dim; i++)
	{
		fprintf(file0, "%d  ", i);
		for (j = 0; j < 18; j++)
		{
			fprintf(file0, "%.2lf ", q[i][j]);
		}
		if (i % (dim*dim) == dim*dim - 1)
			fprintf(file0, "\n");
		fprintf(file0, "\n");
	}
	fclose(file0);

    int episodes =30000;//10000
	double epsilon = 0.8;
	double alpha = 0.1;//learning rate
	double gamma = 0.9;//discount_factor

	printf("\nstart q learning\n");
	long long start, finish;
	long long total;
	start = GetTickCount();
	qlearning(episodes,epsilon,alpha,gamma);
	finish = GetTickCount();
	total = finish - start;	
	cout << total <<  endl;
	printf("\nq learning done!\n");


	FILE *file1; //new q value 
	fopen_s(&file1, "1newqvalue.txt", "wt");
	
	for (i = 0; i < dim*dim*dim; i++)
	{
		fprintf(file1,"%d  ", i);
		for (j = 0; j < 18; j++)
		{
			fprintf(file1 ,"%.2lf ", q[i][j]);
		}
		if (i % (dim*dim) == dim*dim - 1)
			fprintf(file1,"\n");
		fprintf(file1,"\n");
	}
	fclose(file1);

	FILE *file2; //path
	fopen_s(&file2, "2path.txt", "wt");
	int cs = 0,ns=0, p[1000] = { 0 }, pn = 0;
	int a;
	while (cs != dim*dim*dim - 1)
	{
		a = maxaction(cs);
		ns = currentstate(cs, a);
		p[pn++] = ns;
		cs = ns;

		i = cs / (dim*dim);
		t = cs - i*(dim*dim);
		j = t / dim;
		k = t - dim*j;
		
		printf("%d ", cs);
		fprintf(file2, "%d %d %d;\n",i,j,k);
	}
	fprintf(file2, "\n");
	for (n = 0; n < pn; n++)
	{
		fprintf(file2, "%d ", p[n]);
	}
	fclose(file2);
	printf("\n%d \n", pn);

	FILE *file3;//obs
	fopen_s(&file3, "3obsticale.txt", "wt");
	for (n = 0; n < obs;n++)
	{
		i = obs_index[n] / (dim*dim);
		t = obs_index[n] - i*(dim*dim);
		j = t / dim;
		k = t - dim*j;
		printf("%d ", obs_index[n]);
		fprintf(file3, "%d %d %d;\n", i, j, k);
	}
	fprintf(file3, "\n");
	for (n = 0; n < obs; n++)
	{
		fprintf(file3, "%d ", obs_index[n]);
	}
	fclose(file3);

	FILE *file4;//obs
	fopen_s(&file4, "4episode.csv", "wt");
	for (i = 1; i < episodes; i++)
	{
		fprintf(file4,"%.2f\n", qvalue[i]);
	}



	return 0;
}

void qlearning(int episodes,double epsilon, double alpha, double gamma)
{
	int e, rs, ra, flag = 0,flag1=0;
	double re;
	int cs;
	int i;

	int oa, pa=-1;

	//srand((unsigned)time(NULL));

	for (e = 0; e < episodes; e++)
	{
	//	printf("\n episode:  %d\n", e);
		//srand((unsigned)time(NULL));
		rs = 1 + rand() % (dim * dim * dim - 2);
		flag = 0;
		for (i = 0; i < obs; i++)
		{
			if (rs == obs_index[i])
				flag = 1;
		}
		if (flag)
		{
			//printf("%d obs\n", rs);
			continue;
		}
		pa = -1;
		while (rs != dim*dim*dim - 1)
		{
			//srand((unsigned)time(NULL));
			re = rand() % 1000 / 1000.0; //0~1
			if (re < epsilon)
			
			{
				//srand((unsigned)time(NULL));
				ra = rand() % 18; //0~5

///////////////////////////////////////////////////////////	
				if (opa[ra] == pa)
					continue;
				else
					pa = ra;
///////////////////////////////////////////////////////////

			}
			else
				ra = maxaction(rs);


			cs = currentstate(rs, ra);
			if (cs == -1)
			{
				//q[rs][ra] += w_r;////////////////////////////////去掉
			//	printf("%d %d\n", rs,ra);
				continue;
			}

			q[rs][ra] = q[rs][ra] + alpha*(reward[rs][ra] + gamma*q[cs][maxaction(cs)] - q[rs][ra]);
			//printf("%d %d\n", rs, ra);
			
			flag1 = 0;
	
			for (i = 0; i < obs; i++)
			{
				if (cs == obs_index[i])
					flag1 = 1;
			}
 			if (flag1)
			{
				//q[rs][ra] += o_r;
				continue;
			}
			else
				rs = cs;
		}
		//printf("%d\n", e);
		sumqvalue(e);
	}
}
void sumqvalue(int e)
{
	int i,j;
	double sum = 0;
	

	if (e != 0)
	{
		for (i = 0; i < dim*dim*dim; i++)
		for (j = 0; j < 18;j++)
		{
			//if (q[i][j]>0)
				sum = sum + abs(q[i][j]-pq[i][j]);
		}
		memcpy(pq, q, sizeof(double)*dim*dim*dim * 18);
	}
	
	qvalue[e] = sum;
}
int currentstate(int s, int a)
{
	int i, j, k, t;
	i = s / (dim*dim);
	t = s - i*(dim*dim);
	j = t / dim;
	k = t - dim*j;

	//up,down,front,back,left,right
	//0, 1,   2,   3,    4,    5

	switch (a)
	{
	case 0:
		if (i + 1 >= dim)
			return -1;
		else
			return  (i + 1)*dim*dim + j*dim + k;
	case 1:
		if (i - 1 < 0)
			return -1;
		else
			return  (i - 1)*dim*dim + j*dim + k;
	case 2:
		if (j - 1 < 0)
			return -1;
		else
			return i*dim*dim + (j-1)*dim + k;

	case 3:
		if (j + 1 >= dim)
			return -1;
		else
			return i*dim*dim + (j +1 )*dim + k;

	case 4:
		if (k - 1 < 0)
			return -1;
		else 
			return i*dim*dim + j *dim + k-1;

	case 5:
		if (k + 1 >= dim)
			return -1;
		else
			return i*dim*dim + j *dim + k +1;

		//6frontleft,7frontright,8backleft,9backright
		//10front,   11back,     12left,   13right  - up
		//14front,   15back,     16left,   17right  - down

	case 6:
		if (j - 1 < 0 || k - 1 < 0)
			return -1;
		else
			return i*dim*dim + (j - 1)*dim + k-1;
	case 7:
		if (j - 1 < 0 || k + 1 >= dim)
			return -1;
		else
			return i*dim*dim + (j - 1)*dim + k + 1;
	case 8:
		if (j + 1 >=dim || k - 1 < 0)
			return -1;
		else
			return  i*dim*dim + (j + 1)*dim + k - 1;
	case 9:
		if (j + 1 >= dim || k + 1 >= dim)
			return -1;
		else
			return i*dim*dim + (j + 1)*dim + k + 1;


	case 10:
		if (j - 1 < 0 || i + 1 >= dim)
			return -1;
		else
			return (i+1)*dim*dim + (j - 1)*dim + k;

	case 11:
		if (j + 1 >= dim || i + 1 >= dim)
			return -1;
		else
			return (i + 1)*dim*dim + (j + 1)*dim + k;

	case 12:
		if (k - 1 < 0 || i + 1 >= dim)
			return -1;
		else
			return (i + 1)*dim*dim + j *dim + k - 1;

	case 13:
		if (k + 1 >= dim || i + 1 >= dim)
			return -1;
		else
			return (i + 1)*dim*dim + j *dim + k + 1;


	case 14:
		if (j - 1 < 0 || i - 1 < 0)
			return -1;
		else
			return (i-1)*dim*dim + (j - 1)*dim + k;

	case 15:
		if (j + 1 >= dim || i - 1 < 0)
			return -1;
		else
			return (i - 1)*dim*dim + (j + 1)*dim + k;

	case 16:
		if (k - 1 < 0 || i - 1 < 0)
			return -1;
		else
			return (i - 1)*dim*dim + j *dim + k - 1;

	case 17:
		if (k + 1 >= dim || i - 1 < 0)
			return -1;
		else
			return (i - 1)*dim*dim + j *dim + k + 1;
	}
}


int maxaction(int s)
{
	//srand((unsigned)time(NULL));
	int i;
	double av = -10000000;
	int ai = -1;
	int n = 0, a[18] ;
	for (i = 0; i < 18; i++)
	{
		if (q[s][i]>av)
		{
			ai = i; av = q[s][i];
		}
	}

	for (i = 0; i < 18; i++)
	{
		if (q[s][i] == av)
		{
			a[n++] = i;
		}
	}
	if (n==1)
		return ai;

	else
	{
		
		//srand((unsigned)time(NULL));
		n = rand() % n; //1~18
		return a[n];
	}
}


void generate_obs()
{
	//srand((unsigned)time(NULL));
	int r; int n = 0;
	int i;
	int flag = 0;

	while (1)
	{
		r = 1 + rand() % (dim * dim * dim - 2);//1~23
		flag = 0;
		for (i = 0;i < obs; i++)
		{
			if (r == obs_index[i])
				flag = 1;
		}
		if (flag == 0)
			obs_index[n++] = r;
		if (n == obs)
			break;
	}
	//for (i = 0; i < obs; i++)
	//	printf("%d ", obs_index[i]);
}

void generate_map()
{
	int n;
	for (n = 0; n < obs; n++)
	{
		//int i, j, k, t;
		//i = obs_index[n] / (dim*dim);
		//t = obs_index[n] - i*(dim*dim);
		//j = t / dim;
		//k = t - dim*j;
		map[obs_index[n]] = 1;
	}	
}

void print_obs()
{
	int i;
	printf("\n print obs \n");
	for (i = 0; i < obs; i++)
		printf("%d ", obs_index[i]);
	printf("\n");
}

void print_map()
{
	printf("\n print map\n");
	int i;
	for (i = 0; i < dim*dim*dim; i++)
	{
		printf("%d ", map[i]);
		if (i % (dim*dim) == dim*dim - 1)
			printf("\n");
	}	
	printf("\n");
}

void generate_reward()
{
//	map[dim*dim*dim - 2] = g_r;
	int n;
	int i, j, k, t;

	int flag;

	for (i = 0; i < dim*dim*dim;i++)
	for (j = 0; j < 18; j++)
		reward[i][j] = 0;

	for (n = 0; n < obs; n++)
	{
		i = obs_index[n] / (dim*dim);
		t = obs_index[n] - i*(dim*dim);
		j = t / dim;
		k = t - dim*j;

		//obs_index[n]=i*dim*dim+j*dim+k
		//up,down,front,back,left,right
		//0, 1,   2,   3,    4,    5
		//6frontleft,7frontright,8backleft,9backright
		//10front,   11back,     12left,   13right  - up
		//14front,   15back,     16left,   17right  - down

		//check up
		if (i + 1 < dim)
			reward[obs_index[n] + dim*dim][1] = o_r;//down=1
		//check down
		if (i - 1 >= 0)
			reward[obs_index[n]-dim*dim][0] = o_r;//up=0
		//check front
		if (j - 1 >=0)
			reward[obs_index[n] - dim][3] = o_r;//back=3
		//check back
		if (j + 1 < dim)
			reward[obs_index[n] + dim][2] = o_r;//front=2
		//check left
		if (k - 1 >= 0)
			reward[obs_index[n] - 1][5] = o_r;//right=5
		//check right
		if (k + 1 < dim)
			reward[obs_index[n] + 1][4] = o_r;//left=4


		//check frontleft
		if (j - 1 >= 0 && k - 1 >= 0)
			reward[obs_index[n] - dim - 1][9] = o_r;//backright=9
		//check frontright
		if (j - 1 >= 0 && k + 1 < dim)
			reward[obs_index[n] - dim + 1][8] = o_r;//backleft=8
		//check backleft
		if (j + 1 < dim &&k - 1 >= 0)
			reward[obs_index[n] + dim - 1][7] = o_r;//frontright=7
		//check backright
		if (j + 1 < dim && k + 1 < dim)
			reward[obs_index[n] + dim + 1][6] = o_r;//frontleft=6;


		//6frontleft,7frontright,8backleft,9backright
		//10front,   11back,     12left,   13right  - up
		//14front,   15back,     16left,   17right  - down


		//check frontup
		if (j - 1 >= 0 && i + 1 < dim)
			reward[obs_index[n] + dim*dim - dim][15] = o_r;//backdown=15
		//check backup
		if (j + 1 < dim && i + 1 < dim)
			reward[obs_index[n] + dim*dim + dim][14] = o_r;//frontdown=14
		//check leftup
		if (k - 1 >= 0 && i + 1 < dim)
			reward[obs_index[n] + dim*dim - 1][17] = o_r;//rightdown=17
		//check rightup
		if (k + 1 < dim && i + 1 < dim)
			reward[obs_index[n] + dim*dim + 1][16] = o_r;//leftdown=16
	

		//check frontdown
		if (j - 1 >= 0 && i - 1 >= 0)
			reward[obs_index[n] - dim*dim - dim][11] = o_r;//backup=15
		//check backdown
		if (j + 1 < dim && i - 1 >= 0)
			reward[obs_index[n] - dim*dim + dim][10] = o_r;//frontup=14
		//check leftdown
		if (k - 1 >= 0 && i - 1 >= 0)
			reward[obs_index[n] - dim*dim - 1][13] = o_r;//rightup=17
		//check rightdown
		if (k + 1 < dim && i - 1 >= 0)
			reward[obs_index[n] - dim*dim + 1][12] = o_r;//leftup=16
	}

	//6frontleft,7frontright,8backleft,9backright
	//10front,   11back,     12left,   13right  - up
	//14front,   15back,     16left,   17right  - down

	//check up
	for (i = dim*dim*dim - dim*dim; i < dim*dim*dim; i++)
	{
		reward[i][0] = w_r;
		reward[i][10] = w_r;
		reward[i][11] = w_r;
		reward[i][12] = w_r;
		reward[i][13] = w_r;
	}

	//check down
	for (i = 0; i < dim*dim; i++)
	{
		reward[i][1] = w_r;
		reward[i][14] = w_r;
		reward[i][15] = w_r;
		reward[i][16] = w_r;
		reward[i][17] = w_r;
	}

	//check front
	for (i = 0; i < dim;i++)
	for (k = 0; k < dim; k++)
	{
		reward[i*dim*dim + k][2] = w_r;
		reward[i*dim*dim + k][6] = w_r;
		reward[i*dim*dim + k][7] = w_r;
		reward[i*dim*dim + k][10] = w_r;
		reward[i*dim*dim + k][14] = w_r;
	}
	//check back
	for (i = 0; i < dim; i++)
	for (k = 0; k < dim; k++)
	{
		reward[i*dim*dim +dim*(dim-1)+ k][3] = w_r;
		reward[i*dim*dim + dim*(dim - 1) + k][8] = w_r;
		reward[i*dim*dim + dim*(dim - 1) + k][9] = w_r;
		reward[i*dim*dim + dim*(dim - 1) + k][11] = w_r;
		reward[i*dim*dim + dim*(dim - 1) + k][15] = w_r;
	}

	// check left
	for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	{
		reward[i*dim*dim + dim*j ][4] = w_r;
		reward[i*dim*dim + dim*j][6] = w_r;
		reward[i*dim*dim + dim*j][8] = w_r;
		reward[i*dim*dim + dim*j][12] = w_r;
		reward[i*dim*dim + dim*j][16] = w_r;
	}

	// check right
	for (i = 0; i < dim; i++)
	for (j = 0; j < dim; j++)
	{
		reward[i*dim*dim + dim*j+dim-1][5] = w_r;
		reward[i*dim*dim + dim*j + dim - 1][7] = w_r;
		reward[i*dim*dim + dim*j + dim - 1][9] = w_r;
		reward[i*dim*dim + dim*j + dim - 1][13] = w_r;
		reward[i*dim*dim + dim*j + dim - 1][17] = w_r;
	}

	//flag = 0;
	//for (i = 0; i < obs; i++)
	//{
	//	if (dim*dim*dim - 2==obs_index[i])
	//		flag = 1;
	//}
	//if (!flag)
	//	reward[dim*dim*dim - 2][5] = g_r;//right=5  dim*dim*dim - 2

	//flag = 0;
	//for (i = 0; i < obs; i++)
	//{
	//	if (dim*dim*dim - 1 - dim*dim == obs_index[i])
	//		flag = 1;
	//}
	//if (!flag)
	//	reward[dim*dim*dim - 1 - dim*dim][0] = g_r;//up=0 dim*dim*dim - 1 - dim*dim

	//flag = 0;
	//for (i = 0; i < obs; i++)
	//{
	//	if (dim*dim*dim - 1 - dim == obs_index[i])
	//		flag = 1;
	//}
	//if (!flag)
	//	reward[dim*dim*dim - 1 - dim][3] = g_r;//back=3 dim*dim*dim - 1 - dim

	//flag = 0;
	//for (i = 0; i < obs; i++)
	//{
	//	if (dim*dim*dim - 1 - dim*dim - 1 == obs_index[i])
	//		flag = 1;
	//}
	//if (!flag) 
	//	reward[dim*dim*dim - 1 - dim*dim - 1][13] = g_r;//rightup=13 dim*dim*dim - 1 - dim*dim-1

	//flag = 0;
	//for (i = 0; i < obs; i++)
	//{
	//	if (dim*dim*dim - 1 - dim*dim - dim == obs_index[i])
	//		flag = 1;
	//}
	//if (!flag)
	//	reward[dim*dim*dim - 1 - dim*dim - dim][11] = g_r;//backtup=11

	//flag = 0;
	//for (i = 0; i < obs; i++)
	//{
	//	if (dim*dim*dim - dim - 1 - dim - 1 == obs_index[i])
	//		flag = 1;
	//}
	//if (!flag)
	//	reward[dim*dim*dim - dim - 1 - dim - 1][9] = g_r;//backright=9


	//if (reward[dim*dim*dim - 2][5] != o_r)//right=5
	//	reward[dim*dim*dim - 2][5] = g_r;

	//if (reward[dim*dim*dim - 1 - dim*dim][0] != o_r)//up=0
	//	reward[dim*dim*dim - 1 - dim*dim][0] = g_r;

	//if (reward[dim*dim*dim - 1 - dim][3] != o_r)//back=3
	//	reward[dim*dim*dim - 1 - dim][3] = g_r;

	//if (reward[dim*dim*dim - 1 - dim*dim-1][13] != o_r)//rightup=13
	//	reward[dim*dim*dim - 1 - dim*dim-1][13] = g_r;

	//if (reward[dim*dim*dim - 1 - dim*dim-dim][11] != o_r)//backtup=11
	//	reward[dim*dim*dim - 1 - dim*dim-dim][11] = g_r;

	//if (reward[dim*dim*dim - dim - 1 - dim - 1][9] != o_r)//backright=9
	//	reward[dim*dim*dim - dim - 1 - dim - 1][9] = o_r;

	reward[dim*dim*dim - 2][5] = g_r;
	reward[dim*dim*dim - 1 - dim*dim][0] = g_r;
	reward[dim*dim*dim - 1 - dim][3] = g_r;
	reward[dim*dim*dim - 1 - dim*dim-1][13] = g_r;
	reward[dim*dim*dim - 1 - dim*dim-dim][11] = g_r;
	reward[dim*dim*dim - dim - 1 - dim - 1][9] = o_r;

	//printf("\n print reward \n");

	//for (i = 0; i < dim*dim*dim; i++)
	//{
	//	printf("%d  ", i);
	//	for (j = 0; j < 18;j++)
	//	{
	//		printf("%.2lf ", reward[i][j]);
	//	}
	//	if (i % (dim*dim) == dim*dim - 1)
	//		printf("\n");
	//	printf("\n");
	//}
	//printf("\n");
}


void generate_q()
{
	//or initialize matrix Q to zero
	memcpy(q, reward, sizeof(double)*dim*dim*dim * 18);
	memcpy(pq, q, sizeof(double)*dim*dim*dim * 18);
	//int i,j;
	//printf("\n print q \n");

	//for (i = 0; i < dim*dim*dim; i++)
	//{
	//	printf("%d  ", i);
	//	for (j = 0; j < 18; j++)
	//	{
	//		printf("%.2lf ", q[i][j]);
	//	}
	//	if (i % (dim*dim) == dim*dim - 1)
	//		printf("\n");
	//	printf("\n");
	//}
	//printf("\n");
}