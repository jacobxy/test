#include "BattleWalkFighter.h"
#include "GData/SkillTable.h"
namespace Battle
{ 
    bool BattleWalkFighter::PreGetObject()
    { 
        if(!_target || !_target->getHP())
        {
            if(!_count)
                _target = GetField()->GetTargetForRide(!GetSideInBS(), getPosX(),getPosY(), 2);
            else
            {
                if(_target)
                {
                    //COUT << "战将编号"  << static_cast<UInt32>(_target->GetBSNumber()) << "死亡。 ";
                _target = GetField()->GetTarget(!GetSideInBS(),getPosX(),getPosY());
                }
                if(_target)
                {
                    //COUT << "战将编号"  << static_cast<UInt32>(GetBSNumber()) << "锁定目标" << static_cast<UInt32>(_target->GetBSNumber()) << std::endl;
                }
            }
            SetMove(true);
            ++_count;
            BuildLocalStream(e_run);
        }
        if(_target)
            SetBattleTargetPos(_target->getPosX(),_target->getPosY());
        return true;
    } 

    UInt16 BattleWalkFighter::GetTargetDistance()
    { 
        if(!_target || !GetField())
            return 0;
        UInt16 advance = GetField()->getDistance(this,_target);
        return advance;
    } 

    void BattleWalkFighter::BuildLocalStream(UInt8 wait, UInt8 param)
    {
        //_st.reset();
        //_st << static_cast<UInt8>(ACTION_HAPPEN); //即使起作用
        //_st << static_cast<UInt8>(getPosX());
        //_st << static_cast<UInt8>(getPosY());
        //InsertFighterInfo(_st);
        //TODO 被击
        //_st << _actionType;
        switch(wait)
        {
            case e_run:
                if(_target)
                {
                    _st << static_cast<UInt16>(GetNowTime());
                    _st << GetBSNumber();
                    _st << static_cast<UInt8>(4);
                    _st << _target->GetBSNumber();
                    //COUT << "时间点" << static_cast<UInt32>(GetNowTime()) << std::endl;
                    //COUT << "战将编号" << static_cast<UInt32>(GetBSNumber()) << std::endl;
                    //COUT << "目标编号" << static_cast<UInt32>(_target->GetBSNumber()) << std::endl;
                }
                break;
            case e_attack_near:
            case e_attack_middle:
            case e_attack_distant:
            case e_image_attack:
            case e_image_therapy: //弃用
                {
                    //_st << static_cast<UInt8>(wait);  //是否延迟
                    //_st << static_cast<UInt8>(_actionLast); //动作持续
                    //for(;it!=targetList.end();++it)
                    { 
                        //_st << (_target)->getPosX();
                        //_st << (_target)->getPosY();
                    } 
                }
            case e_be_attacked://弃用
                _st << static_cast<UInt32>(param);
                break;
            default :
                break;
        }
    }

    UInt16 BattleWalkFighter::GetSpeed()
    { 
        PreGetObject();
        if(_count < 2 && _isChild && _target && _target->GetTypeId() == 1)
        {
            if((GetSideInBS() == 0 && GetHighSpeed()) || (GetSideInBS() == 1 && !GetHighSpeed()))             
                return 75;//return GetBaseSpeed()*4/10;
            else 
                return 225;// GetBaseSpeed()*12/10;
        }
        return GetBaseSpeed();
    } 

    void BattleWalkFighter::resetBattleStatue()
    { 
        _canMove = true;
        _isHighSpeed = false;
        _count = 0;
        BattleFighter::resetBattleStatue();
    } 
} 

