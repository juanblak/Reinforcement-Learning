#include <iostream>
#include <stdio.h>
#include<string.h>
#include <Windows.h> // gettickcount
#include <stdlib.h> // rand 
#include <time.h>
#include <omp.h> // openmp头文件

using namespace std;


const int episodes = 10000;
const double epsilon = 0.9;
const double alpha = 0.1; //learning rate
const double gamma = 0.9;//discount_factor


const int dim = 5; //5*5 7*7
const int obs = 3; //障碍物个数
const double o_r = -1.0; //障碍物_reward
const double w_r = -2.0; //墙壁_reward
const double g_r = 1.0; //目标_reward

int obs_index[obs] = { 8, 11, 22 };//enough
int oval = 18;
int opa[4] = { 1, 0, 3, 2 };

double reward[dim*dim][4];


double q[dim*dim][4];
double pq[dim*dim][4];
double qvalue[20000] = { 0 };
void sumqvalue(int e);

//0up, 1down, 2right, 3left

void generate_q();
void sarsa();
int choose_action(int s, int pa);
int max_action(int s);
int get_state(int s, int a);
int get_reward(int s_);
int main(int argc, char*argv[])
{
	int i, j, n;
	srand((unsigned)time(NULL));

	generate_q();//initialize q


	printf("\nstart sarsa learning\n");
	sarsa();
	printf("\nsarsa learning done!\n");

	FILE *file1; //new q value 
	fopen_s(&file1, "1newqvalue.txt", "wt");

	for (i = 0; i < dim*dim; i++)
	{
		fprintf(file1, "%d  ", i);
		for (j = 0; j < 4; j++)
		{
			fprintf(file1, "%.2lf ", q[i][j]);
		}
		if (i % (dim*dim) == dim*dim - 1)
			fprintf(file1, "\n");
		fprintf(file1, "\n");
	}
	fclose(file1);

	FILE *file2; //path
	fopen_s(&file2, "2path.txt", "wt");
	int cs = 0, ns = 0, p[1000] = { 0 }, pn = 0;
	int a;
	while (cs != oval)
	{
		do
		{
			a = max_action(cs);
			ns = get_state(cs, a);
		} while (ns == -2);

		p[pn++] = ns;
		cs = ns;

		i = cs / dim;
		j = cs - dim*i;

		printf("%d ", cs);

		fprintf(file2, "%d %d;\n", i, j);
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
	for (n = 0; n < obs; n++)
	{
		i = cs / dim;
		j = cs - dim*i;

		printf("%d ", obs_index[n]);
		fprintf(file3, "%d %d;\n", i, j);
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
		fprintf(file4, "%.2f\n", qvalue[i]);
	}
	fclose(file4);

	return 0;
}
void sarsa()
{
	int observation, action, observation_, action_, reward;
	double q_target;

	int e;
	int pa;
	for (e = 0; e < episodes; e++)
	{
		printf("%d\n", e);
		observation = 0;
		pa = -1;//
		action = choose_action(observation, pa);
		
		while (1)
		{
			pa = action;//
			observation_ = get_state(observation, action);
			reward = get_reward(observation_);

			if (reward == -2)
				observation_ = observation;

			action_ = choose_action(observation_, pa);

			if (reward == 1)
				q_target = reward;
			else
				q_target = reward + gamma*q[observation_][action_];

			q[observation][action] += alpha*(q_target - q[observation][action]);

			if (reward == -1 || reward == 1)
				break;

			observation = observation_;
			action = action_;
			printf("%d ", observation);

		}
		sumqvalue(e);
	}
}

int get_reward(int s_)
{
	int i;
	if (s_ == oval)
		return 1;
	if (s_ == -2)
		return -2;

	for (i = 0; i < obs; i++)
	{
		if (s_ == obs_index[i])
			return -1;
	}

	return 0;
}
int get_state(int s, int a)
{
	int i, j;
	i = s / dim;
	j = s - i*dim;

	switch (a)
	{
	case 0://up
		if (i - 1 >= 0)
			return (i - 1)*dim + j;
		else
			return  -2;
	case 1://down
		if (i + 1 < dim)
			return (i + 1)*dim + j;
		else
			return  -2;

	case 2://right
		if (j + 1 < dim)
			return i*dim + j + 1;
		else
			return -2;
	case 3://left
		if (j - 1 >= 0)
			return i*dim + j - 1;
		else
			return -2;
	}
}


int choose_action(int s, int pa)
{
	double re;
	int action;
	re = rand() % 10 / 10.0; //0~1

	if (re < epsilon)
		action = max_action(s); //0~3

	else
		action = rand() % 4;
	//else
	//{
	//	do
	//		action = rand() % 4;
	//	while (opa[action] == pa);
	//		
	//}

	return action;
}

int max_action(int s)
{
	int i;
	double av = -10000000;
	int ai = -1;
	int n = 0, a[4];
	for (i = 0; i < 4; i++)
	{
		if (q[s][i]>av)
		{
			ai = i; av = q[s][i];
		}
	}

	for (i = 0; i < 4; i++)
	{
		if (q[s][i] == av)
		{
			a[n++] = i;
		}
	}
	if (n == 1)
		return ai;

	else
	{
		n = rand() % n; //0~3
		return a[n];
	}
}
void sumqvalue(int e)
{
	int i;
	double sum = 0;

	if (e != 0)
	{
		for (i = 0; i < dim*dim; i++)
		{
			//if (q[i][j]>0)
			sum = sum + abs(q[i][0] - pq[i][0]) + abs(q[i][1] - pq[i][1]) + abs(q[i][2] - pq[i][2]) + abs(q[i][3] - pq[i][3]);
		}
		memcpy(pq, q, sizeof(double)*dim*dim * 4);
	}

	qvalue[e] = sum;
}
void generate_q()
{
	int i;
	printf("\n print q \n");

	for (i = 0; i < dim*dim; i++)
	{
		q[i][0] = 0.0; q[i][1] = 0; q[i][2] = 0; q[i][3] = 0;
		pq[i][0] = 0.0; pq[i][1] = 0; pq[i][2] = 0; pq[i][3] = 0;

	}
}