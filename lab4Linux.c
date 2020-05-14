#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <stack>
#include <unistd.h>
#include <termios.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>

#define MAX_COUNT 10

using namespace std;

void* printString(void* arg);
void CloseLastThread();
void WaitThreads();
void AddThread();

char strings[10][30] = {{"1) First thread\n"}, {"2) Second thread\n"}, {"3) Third thread\n"}, {"4) Fourth thread\n"},
                        {"5) Fifth thread\n"}, {"6) Sixth thread\n"}, {"7) Seventh thread\n"}, {"8) Eighth thread\n"}, {"9) Ninth thread\n"},
                        {"10) Tenth thread\n"}};

pthread_mutex_t printMutex; //определяем мьютексы для печати

stack<pthread_t> threads;
stack<pthread_t> closingThreads;
vector<bool*> quitFlags;

struct threadArg
{
    bool* quitFlag;
    int num;
};


int main()
{
    //иницализируем мьютекс
    if(pthread_mutex_init(&printMutex, NULL) != 0)
    {
        printw("Initialize mutex error...\n");
        endwin();
        return 0;
    }

    initscr(); //перевод терминала в curses-режим
    clear();
    noecho();//вводимые симоволы не отображаются на экран
    refresh();//проверка буфера
    nodelay(stdscr, TRUE);

    while(1)
    {
        usleep(10000);
        switch(getch())
        {
        case '+':
            if(threads.size() < MAX_COUNT) AddThread();
            break;

        case '-':
            if(threads.size() > 0) CloseLastThread();
            break;

        case 'q':
            while(threads.size() > 0)
            CloseLastThread();
            WaitThreads();

            pthread_mutex_destroy(&printMutex);
            clear();
            endwin();
            return 0;

        default:
            break;
        }
    }
}


void CloseLastThread()
{
    closingThreads.push(threads.top()); // Добавляем id последнего потока в стек закрывающихся потоков
    *(quitFlags.back()) = true;   // Устанавливаем флаг выхода для последнего потока
    quitFlags.pop_back();         // Удаляем указатель на флаг закрытия последнего потока из массива
    threads.pop();				  // Удаляем id последнего потока
}

void WaitThreads()
{
    while(closingThreads.size() > 0)
    {
        pthread_join(closingThreads.top(), NULL); // Ожидаем завершения последнего потока
        closingThreads.pop();
    }
}

void AddThread()
{
    quitFlags.push_back(new bool(false));

    threadArg* arg = new threadArg();
    (*arg).num = threads.size();              // Номер добавляемого потока
    (*arg).quitFlag = quitFlags.back();		  // Указатель на флаг закрытия для данного потока

    pthread_t thread;
    //создаем новый поток
    if(pthread_create(&thread, //id потока
                      NULL,     //использование атрибутов по умолчанию
                      printString, //функция, которая и будет выполняться в новом потоке
                      arg)!= 0) //переданные аргументы
    {
        printw("Creating new thread error...\n");
        endwin();
        return;
    }
    //и добавляем его в наш вектор
    threads.push(thread);
    
}



void* printString(void* arg)
{
    bool *qFlag = (*(threadArg*)arg).quitFlag;   // Указатель на флаг выхода для данного потока
    int threadNumber = (*(threadArg*)arg).num;   // Номер данного потока
    delete (threadArg*)arg;

    while(1)
    {
        if(*qFlag) break;
        //захватываем мьютекс
        pthread_mutex_lock(&printMutex);
        for(int i = 0; i < strlen(strings[threadNumber]); i++)//производим посимвольную печать
        {
            if(*qFlag) break;
            printw("%c",strings[threadNumber][i]);
            usleep(100000);
        }
        //освобожаем мьюеткс
        pthread_mutex_unlock(&printMutex);

        usleep(100);
    }

    delete qFlag;
    return NULL;
}

