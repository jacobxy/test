
#ifndef BATTLEACTION_H_
#define BATTLEACTION_H_

#include "Config.h"

namespace Battle
{
    class BattleFighter;
    class BattleObject;
    //行为动作
    class BattleAction
    {
        public:
            BattleAction(){};
            virtual  UInt32 GetAttack(){return 0;}
            virtual  UInt32 GetHit(){return 0;}
            virtual  UInt32 GetWreck() {return 0;}
            virtual  UInt32 GetCritical() {return 0;}
            virtual  BattleFighter * GetBattleFighter(){return NULL;}
            virtual  bool CanCounter() {return true;}
    };
    class ActionPackage  : public BattleAction
    {
        public:
            ActionPackage(UInt32 attack, UInt32 hit, UInt32 wreck, UInt32 critical,/* BattleObject * bo,*/ BattleFighter * bf): _attack(attack),_hit(hit),_wreck(wreck),_critical(critical),_bf(bf)
        {
            vec_bo.clear();
        }
            ~ActionPackage(){}
            UInt32 GetAttack(){return _attack;}
            UInt32 GetHit(){return _hit;}
            UInt32 GetWreck() {return _wreck;}
            UInt32 GetCritical() {return _critical;}
            //UInt32 GetBattleObject() {return _bo;}
            BattleFighter * GetBattleFighter() {return _bf;}
            void PushObject(BattleObject* bo){ vec_bo.push_back(bo);}
        private: 
            UInt32 _attack;   //攻击
            UInt32 _hit;      //命中
            UInt32 _wreck;    //破击
            UInt32 _critical; //暴击
            std::vector<BattleObject *> vec_bo;
            UInt16 skillScopeId;
            BattleFighter * _bf; //攻击发起者
    };
    class ImagePackage : public BattleAction
    {
        private:
    };
}
#endif // BATTLEACTION_H_

/* vim: set ai si nu sm smd hls is ts=4 sm=4 bs=indent,eol,start */
