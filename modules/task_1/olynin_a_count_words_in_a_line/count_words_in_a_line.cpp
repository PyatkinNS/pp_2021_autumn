// Copyright 2021 Olynin Alexandr
# include <string.h>
#include <malloc.h>
#include <mpi.h>
#include "../../../modules/task_1/olynin_a_count_words_in_a_line/count_words_in_a_line.h"

char tokens[8] = {'\n', '-',',','.','!', ':', '?', ';'};

int SequentialCountWordsInALine(const char* tmp)
{
    char* str = (char*)malloc((strlen(tmp) + 1) * sizeof(char));
	memcpy(str, tmp, strlen(tmp) + 1);
    str[strlen(tmp)] = ' ';
	str[strlen(tmp) + 1] = '\0';
    int l = 0; 
	int count = 0;
    char* is_token = nullptr;
    int len = strlen(str) + 1;
	for (int i = 0; i < len; i++)
	{
		is_token = strchr(tokens, (int)(str[i]));
		if (is_token == nullptr)
		{
			if (str[i] != ' ')
				l++;
			else if (l > 0)
			{
				count++;
				l = 0;
			}
		}
	}
    return count;
}

int ParallelCountWordsInALine(const char* tmp)
{
    char* str = nullptr;
	char* local_str = nullptr;
	int inaccuracy = 0;

	int* send_counts = nullptr;
	int* offset = nullptr;
	int ProcNum;
	int ProcRank; 

	MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);

	send_counts = (int*)malloc(ProcNum * sizeof(int));
	offset = (int*)malloc(ProcNum * sizeof(int));

	if (ProcRank == 0)
	{
		str = (char*)malloc((strlen(tmp) + 1) * sizeof(char));
		memcpy(str, tmp, strlen(tmp) + 1);
		int sum = 0;
		int ost = strlen(str) % ProcNum;
		for (int i = 0; i < ProcNum; i++)
		{
			send_counts[i] = (strlen(str) / ProcNum);
			if (ost > 0)
			{
				send_counts[i]++;
				ost--;
			}
			offset[i] = sum;
			sum += send_counts[i];
		}
		sum = 0;
		for (int i = 1; i < ProcNum; i++)
		{
			sum += send_counts[i - 1];
			char* is_token_char_one = strchr(tokens, (int)(str[sum - 1]));
			char* is_token_char_two = strchr(tokens, (int)(str[sum]));
			if (is_token_char_one == nullptr && is_token_char_two == nullptr
				&& str[sum] != ' ' && str[sum - 1] != ' ')
				inaccuracy++;
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Bcast(send_counts, ProcNum, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(offset, ProcNum, MPI_INT, 0, MPI_COMM_WORLD);

	local_str = (char*)malloc((send_counts[ProcRank] + 2) * sizeof(char));

	MPI_Scatterv(str, send_counts, offset, MPI_CHAR, local_str, send_counts[ProcRank], MPI_CHAR, 0, MPI_COMM_WORLD);
	local_str[send_counts[ProcRank]] = ' ';
	local_str[send_counts[ProcRank] + 1] = '\0';
	int l = 0;
	int count = 0;
    char* is_token = nullptr;
	for (int i = 0; i < send_counts[ProcRank] + 1; i++)
	{
		is_token = strchr(tokens, (int)(local_str[i]));
		if (is_token == nullptr)
		{
			if (local_str[i] != ' ')
				l++;
			else if (l > 0)
			{
				count++;
				l = 0;
			}
		}
	}
    int total_count;
	MPI_Reduce(&count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if (ProcRank == 0)
	{
		total_count -= inaccuracy;
        free(str);
		return total_count;
	}
    free(local_str);
	return ProcRank;
}

const char* GetReadyText(int key)
{
    if (key == 0)
        return "Amidst a wild flat meadow encircled by an Edenic lush forest, a couple have cocooned themselves in a secluded mansion that was not so long ago burned to the ground, devotedly restored by the supportive wife. Within this safe environment, the once famous middle-aged poet husband is desirous of creating his magnum opus; however, he seems unable to break out of the persistent creative rut that haunts him. Then, unexpectedly, a knock at the door, the sudden arrival of a cryptic late-night visitor and his intrusive wife will stimulate the writer's stagnant imagination. Little by little, much to the perplexed wife's surprise, the more chaos he lets in their haven, the better for his punctured male ego. In the end, will this incremental mess blemish, irreparably, the couple's inviolable sanctuary?";
    if (key == 1)
        return "The Emergency Code for a plane hijacking is 7500: a tense, intense thriller, told from the cockpit. A flight from Berlin to Paris. Everyday routine in the cockpit of an Airbus A319. Co-pilot Tobias Ellis is preparing the plane for take-off, which then follows without incident. Then we hear shouting in the passenger cabin. A group of young men try and storm the cockpit, among them 18-year old Vedat. A fight begins between crew and attackers, with the desire on the one hand to save individual lives and on the other to avert an even bigger catastrophe. The cockpit door becomes a battleground - and Tobias ends up being the arbiter over life and death.";
    return "error";
}