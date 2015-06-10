#ifndef __SYS_OS_H
#define __SYS_OS_H
#define MAXTASKS 3
volatile unsigned int timers[MAXTASKS];

#define _SS static unsigned int _lc=0; switch(_lc){default:
#define _EE ;}_lc=0; return 65536;
#define _LOOP_SS _lc=(__LINE__+((__LINE__%65536)==0))%65536;for(;;){case (__LINE__+((__LINE__%65536)==0))%65536:
#define WaitX(tickets)  do {_lc=(__LINE__+((__LINE__%65536)==0))%65536; return tickets ;} while(0); case (__LINE__+((__LINE__%65536)==0))%65536:
#define LoopX(tickets); return tickets;}

#define RunTask(TaskName,TaskID) do { if (timers[TaskID]==0) timers[TaskID]=TaskName(); }  while(0);
#define RunTaskA(TaskName,TaskID) { if (timers[TaskID]==0) {timers[TaskID]=TaskName(); continue;} }   //前面的任务优先保证执行

#define UpdateTimers() {char i; for(i=MAXTASKS;i>0 ;i--){if((timers[i-1]!=0)&&(timers[i-1]!=0xffff)) timers[i-1]--;}}
//////////////////////////////////////////////////////////////////////////////////

#define CallSub(SubTaskName) do {unsigned char currdt; _lc=(__LINE__+((__LINE__%65536)==0))%65536; return 0; case (__LINE__+((__LINE__%65536)==0))%65536:  currdt=SubTaskName(); if(currdt!=65536) return currdt;} while(0);

#define SEM unsigned int
//初始化信号量
#define InitSem(sem) sem=0;
//等待信号量
#define WaitSem(sem) do{ sem=1; WaitX(0); if (sem>0) return 1;} while(0);
//等待信号量或定时器溢出， 定时器tickets 最大为0xFFFE
#define WaitSemX(sem,tickets)  do { sem=tickets+1; WaitX(0); if(sem>1){ sem--;  return 1;} } while(0);
//发送信号量
#define SendSem(sem)  do {sem=0;} while(0);
#endif
