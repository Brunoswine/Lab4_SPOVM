#include <windows.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <stack>
#include <process.h>

#define MAX_COUNT 10 //максильманое кол-во потоков

using namespace std;
unsigned int __stdcall  printString(void* arg);


char strings[10][30] = { {"1) First thread\n"}, {"2) Second thread\n"}, {"3) Third thread\n"}, {"4) Fourth thread\n"}, {"5) Fifth thread\n"}, {"6) Sixth thread\n"}, {"7) Seventh thread\n"}, {"8) Eighth thread\n"}, {"9) Ninth thread\n"}, {"10) Tenth thread\n"} };
/*Объект критическая секция обеспечивает синхронизацию. Этим объектом может владеть только один поток, что и обеспечивает синхронизацию. */
CRITICAL_SECTION cs_print;

stack<HANDLE> threads;
stack<HANDLE> closingThreads;
vector<bool*> quitFlags;

struct threadArg
{
	bool* quitFlag;
	int num;
};


void CloseLastThread()
{
	closingThreads.push(threads.top()); // Добавляем id последнего потока в стек закрывающихся потоков
	*(quitFlags.back()) = true;   // Устанавливаем флаг выхода для последнего потока
	quitFlags.pop_back();         // Удаляем указатели на флаг закрытия последнего потока из массива
	threads.pop();				  // Удаляем id последнего потока
}

void WaitThreads() //Ожидание потоков
{
	while (closingThreads.size() > 0)
	{
		WaitForSingleObject(closingThreads.top(), INFINITE);
		closingThreads.pop();
	}
}

void AddThread()
{
	quitFlags.push_back(new bool(false));

	threadArg* arg = new threadArg(); //создаем новый экземпляр данных о потоке
	(*arg).num = threads.size();              // Номер добавляемого потока
	(*arg).quitFlag = quitFlags.back();		  // Указатель на флаг закрытия для данного потока

	HANDLE thread = (HANDLE)_beginthreadex //создаем поток
	(NULL, //атрибут безопастности потока
		0, //начальный размер стека
		printString, //адрес функции и задачи
		(void *)(arg), //параметры для задачи
		0, //параметры создания задачи
		NULL); //адрес переменной для идентификатора задачи
	threads.push(thread);

}



unsigned int __stdcall  printString(void* arg)
{
	bool *qFlag = (*(threadArg*)arg).quitFlag;   // Указатель на флаг выхода для данного потока 
	int threadNumber = (*(threadArg*)arg).num;   // Номер данного потока
	delete arg;

	while (1)
	{
		if (*qFlag) break;

		EnterCriticalSection(&cs_print); //начало CriticalSection
		for (int i = 0; i < strlen(strings[threadNumber]); i++) //посимвольный вывод
		{

			if (*qFlag) break;

			printf("%c", strings[threadNumber][i]);
			Sleep(100);
		}

		LeaveCriticalSection(&cs_print);//конец CriticalSection
		Sleep(1);
	}

	delete qFlag; //удаляем флаг выхода для данного потока
	return 0;
}


void main()
{
	InitializeCriticalSection(&cs_print); //Перед тем как использовать критическую секцию ее надо проинициализировать
	while (1)
	{
		switch (_getch())
		{
		case '+':
			if (threads.size() < MAX_COUNT) AddThread();
			break;
		case '-':
			if (threads.size() > 0) CloseLastThread();
			break;
		case 'q':
			while (threads.size() > 0)
				CloseLastThread();

			WaitThreads();//ожидание

			DeleteCriticalSection(&cs_print);//удаление CriticalSection
			printf("\n\n");
			system("pause");
			return;
		default:
			break;
		}
	}
}
