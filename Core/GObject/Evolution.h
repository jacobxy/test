
#ifndef EVOLUTION_H_
#define EVOLUTION_H_
#include "Config.h"
#include "Common/Stream.h" 
#include "GData/AttrExtra.h"
#include "GData/SkillTable.h"
namespace GObject
{
#define EVOLUTION_TASKMAX 9   //编号0-8
#define TASK9_COUNT 3
    enum
    {
        e_xq_bing = 0,
        e_xq_yi = 1,
        e_xq_qi = 2,

        e_xq_max
    };
    class Player ;
    class Fighter;
    class ItemEquip;
    class Package;
    struct task9Player
    { 
        Player * pl;
        UInt8 win;
        UInt32 npcId;
        task9Player():pl(NULL),win(0),npcId(0){}
    } ;
    class Evolution
    {
        public:
            Evolution(Fighter * fgt):_process(0),_award(0),success(0){
                _fighter = fgt;
                memset(&_evolution,0,sizeof(_evolution)) ;
                memset(&_task9,0,sizeof(_task9)) ;
                randomTime = 0;
            }
            UInt8 CompleteTask(UInt8 number,UInt8 task4 = 0);
            UInt8 TryTask0(UInt8 flag = 0,UInt32 npcId = 0);
            UInt8 TryTask1();
            UInt8 TryTask2();
            UInt8 TryTask3();
            UInt8 TryTask9();
            UInt8 TryTask5();
            UInt8 TryTask6();
            UInt8 TryTask7();
            UInt8 TryTask8(UInt8 task4);

            void challenge();
            void UpdateEvolutionToDB();

            //task9
            void RandomTask9Player(UInt8 flag = 0);
            void getTask9Info(Stream& st);

            void SetProcess(UInt16 process){ _process = process;}
            UInt16 GetProcess(){return _process;}
            void SetAward(UInt16 award){ _award = award;}
            UInt16 GetAward(){return _award;}
            UInt8  GetProcess(UInt8 type);
            void SendProcess();
            bool IsComplete();
            ItemEquip * getEquip(UInt8 cnt){if(cnt >=e_xq_max)return NULL ; return _evolution[cnt]; }

            UInt8 GetTaskAward(UInt8 index);


            void SendTaskInfo(UInt8 taskId);
            Player * getOwner();

            void LoadTask9Info(std::string);
            ItemEquip* SetEvolutionEquip(UInt8 pos ,ItemEquip * equip,UInt8 flag = 0);
            void setRandomTime(UInt32 time){ randomTime = time;}
    
            void setSuccessValue(UInt8 val){ success = val;}
            UInt8 FeiSheng();

            void UpdateEquipToDB();
        private:
            UInt16 _process;
            UInt16 _award;
            Fighter * _fighter;
            ItemEquip * _evolution[e_xq_max];// 仙界装备
            struct task9Player _task9[TASK9_COUNT];
            static UInt32 npcIds[EVOLUTION_TASKMAX];
            UInt32 randomTime;
            UInt8 success;
    };
}
#endif // EVOLUTION_H_

/* vim: set ai si nu sm smd hls is ts=4 sm=4 bs=indent,eol,start */
