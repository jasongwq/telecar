#ifndef __SYS_OS_H
#define __SYS_OS_H
#define MAXTASKS 3
volatile unsigned char timers[MAXTASKS];
#define _SS static unsigned char _lc=0; switch(_lc){default: 
#define _EE ;}; _lc=0; return 255;
#define WaitX(tickets)  do {_lc=(__LINE__+((__LINE__%256)==0))%256; return tickets ;} while(0); case (__LINE__+((__LINE__%256)==0))%256: 

#define RunTask(TaskName,TaskID) do { if (timers[TaskID]==0) timers[TaskID]=TaskName(); }  while(0); 
#define RunTaskA(TaskName,TaskID) { if (timers[TaskID]==0) {timers[TaskID]=TaskName(); continue;} }   //ǰ����������ȱ�ִ֤��

#define CallSub(SubTaskName) do {unsigned char currdt; _lc=(__LINE__+((__LINE__%256)==0))%256; return 0; case (__LINE__+((__LINE__%256)==0))%256:  currdt=SubTaskName(); if(currdt!=255) return currdt;} while(0);
#define InitTasks() {unsigned char i; for(i=MAXTASKS;i>0 ;i--) timers[i-1]=0; }
#define UpdateTimers() {unsigned char i; for(i=MAXTASKS;i>0 ;i--){if((timers[i-1]!=0)&&(timers[i-1]!=255)) timers[i-1]--;}}

#define SEM unsigned int 
//��ʼ���ź���
#define InitSem(sem) sem=0;
//�ȴ��ź���
#define WaitSem(sem) do{ sem=1; WaitX(0); if (sem>0) return 1;} while(0);
//�ȴ��ź�����ʱ������� ��ʱ��tickets ���Ϊ0xFFFE
#define WaitSemX(sem,tickets)  do { sem=tickets+1; WaitX(0); if(sem>1){ sem--;  return 1;} } while(0);
//�����ź���
#define SendSem(sem)  do {sem=0;} while(0);
#endif
