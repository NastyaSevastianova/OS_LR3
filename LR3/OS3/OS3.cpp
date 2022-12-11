#include <windows.h>
#include <iostream>
#include <iomanip>

using namespace std;

const unsigned CountThread = 16; // 1 2 4 8 12 16 (кол-во потоков)
const unsigned Block = 100; // размер блока
const unsigned Precision = 100000000; // точность вычислений
double pi = 0;

struct Param // структура параметров
{
	double* pi;
	HANDLE* mutex;
	int* startBlock;
};

DWORD WINAPI THREAD(LPVOID lpParam) { // поток 
	Param*  param = (Param*)lpParam; 
	double* pi = (double*)param->pi;
	HANDLE mutex = *(HANDLE*)param->mutex; // mutex не дает использовать одну память одновременно
	int startBlock = *(int*)param->startBlock;

	double accumulator = 0;

	while (startBlock < Precision) {
		for (int i = startBlock; i < startBlock + Block; ++i) {
			accumulator += 4 / (1 + ((i + 0.5) / Precision) * ((i + 0.5) / Precision));
		}
		startBlock += CountThread * Block;
	}

	WaitForSingleObject(mutex, INFINITE);
	*pi += accumulator / Precision;
	ReleaseMutex(mutex);

	return 0;
}

int main()
{
	setlocale(LC_ALL, "Russian");

	int startTime, endTime;
	HANDLE Threads[CountThread], mutex;
	int startingOperations[CountThread];
	mutex = CreateMutex(NULL, FALSE, NULL);
	for (int i = 0; i < CountThread; ++i) { // создание потоков
		startingOperations[i] = i * Block;
		Param * param = new Param{ &pi, &mutex, startingOperations + i};
		Threads[i] = CreateThread(NULL, 0, THREAD, (void*)param, CREATE_SUSPENDED, NULL);
	}

	startTime = GetTickCount();
	for (int i = 0; i < CountThread; ++i){ResumeThread(Threads[i]);} // запуск потоков

	WaitForMultipleObjects(CountThread, Threads, TRUE, INFINITE); // ожидание завершения потоков
	endTime = GetTickCount();

	std::cout << 
		"Потоки: " << CountThread << 
		"\npi: " << fixed << setprecision(20) << pi << 
		"\nВремя: " << endTime - startTime << " мс";

	for (int i = 0; i < CountThread; ++i){CloseHandle(Threads[i]);}
	CloseHandle(mutex);

	return 0;
}


