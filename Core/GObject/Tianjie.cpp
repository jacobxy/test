#include "Tianjie.h"
#include "Map.h"

#include "Common/TimeUtil.h"
#include "DB/DBExecutor.h"
#include "DB/DBConnectionMgr.h"
#include "Server/SysMsg.h"
#include "GData/GDataDBExecHelper.h"
#include "GObject/Package.h"
#include "GData/NpcGroup.h"
#include "MsgID.h"

namespace GObject
{
#define START_WITH_59 1 
//事件2捐赠的ID
enum ENUM_ID_TJ2
{
    ID_TJ2_TAEL = 0,
    ID_TJ2_GOLD,
    ID_TJ2_COUPON,
    ID_TJ2_TLYJ,
};
//天劫触发等级
static const int s_tjRoleLevel[] = {59,69,79,89,99,109};
//天劫事件1的小怪ID
static const int s_rate1Npc[][4] = {
{7007, 7008, 7009, 7010},
{7013, 7014, 7015, 7016},
{7019, 7020, 7021, 7022},
{7025, 7026, 7027, 7028},
{7031, 7032, 7033, 7034},
{7037, 7038, 7039, 7040}
};
static const UInt32 s_rate1NpcCount[] = {400, 350, 300, 250}; //怪物数量
static const UInt32 s_rate1NpcScore[] = {70, 80, 90, 100};  //事件中,怪物积分
static const UInt8  s_task1ColorMulti[] = {20, 30, 40, 50}; //绿蓝紫橙
//事件2捐款达成数
static const UInt32 s_rate2DonateScoreMax = 400000;
static const UInt32 s_tj2PlayerMaxScore   = 40000;
static const UInt32 s_rate2DonateScore[] = {200, 200, 200, 200};
static const UInt32 s_rate2DonateExpMulti[] = {30, 30, 30, 30};
//天劫事件3的2种怪
static const int s_tlzNpcId[][2] = {
{7011, 7012},
{7017, 7018},
{7023, 7024},
{7029, 7030},
{7035, 7036},
{7041, 7042}
};
//天劫事件3怪物每层的成长系数
//物攻  法攻  物防  法防  生命  身法  命中   闪避   反击   法术抵抗
//   1     1  0.5    0.5     1     1  0.05%  0.05%  0.05%  0.10%
//                                    200%   100%   100%   200%
static const float  s_rate3NpcBaseModulus[] = {1, 1, 0.5, 0.5, 1, 1};
static const float  s_rate3NpcAdvanceModulus[] = {0.0005, 0.0005, 0.0005, 0.001};
static const float  s_rate3NpcAdvanceModMax[] =  {2, 1, 1, 2};
static const UInt8  s_rate3NpcScore = 100;    //天劫事件3每层积分  
static const UInt8  s_rate3ExpMulti = 10;     //天雷阵的经验倍数
//天劫事件4的分身和真身BOSS
static const UInt32 s_tjBoss[][2] = {
{7043, 7044},
{7045, 7046},
{7047, 7048},
{7049, 7050},
{7051, 7052},
{7053, 7054}
};

static const UInt32 s_tj4Score = 1000;
static const UInt32 s_tjBossScore = 2800;
//天劫宝箱 [绿色,蓝色,紫色,橙色]
static const UInt32 s_tjEventBoxId[] = {9127, 9128, 9129, 9130};
static const UInt32 s_tjTotalBoxId[] = {9134, 9135, 9136, 9137};
static const UInt32 s_tjEventRewardId = 9131;
static const UInt32 s_tjTotalRewardId = 9132;

#define TJ_START_TIME_HOUR 19
#define TIME_60 60 
#define ONE_DAY_SECOND (24*3600)

#define TJ_EVENT_WAIT_TIME 15*60      //天劫事件间隔时间
#define TJ_EVENT_PROCESS_TIME 15*60   //天劫事件持续时间

//#define TJ_EVENT_WAIT_TIME 2*60      //天劫事件间隔时间
//#define TJ_EVENT_PROCESS_TIME 10*60   //天劫事件持续时间
Tianjie::Tianjie()
{
    m_tjTypeId = 0;
    m_isNotRestart = true;

	m_isTjOpened = 0;        
	m_isTjExecute = 0;      
	m_isFinish = 0;         
	m_isOk = 0;             
	m_currOpenedTjLevel = 0;  
	m_currTjRate = 0;         
	m_openTime = 0;      
	m_notifyRate = 0;  

    m_Top1Record = 0;
    m_Top1PlayerId = 0;
    m_eventMaxNumber = 0;
    m_eventCurrNumber = 0;
    m_eventOldPercent = 0;

	memset(m_rate1KilledCount, 0 , sizeof(m_rate1KilledCount)); 

    memset(m_rate2DonateCount, 0, sizeof(m_rate2DonateCount));

    m_bossIndex        = 0;
    m_loc = 0;
    m_bossDay          = 0;

    _hp                = 0;
    m_bossMaxHp        = 0;
    _percent           = 100;
   
    m_tj1Count = 0;
    m_tj2Count = 0;
    m_tj3Count = 0;
    m_tj4BossHp = 0;

    m_nextTjTime = 0;    
}

bool Tianjie::isPlayerInTj(int playerLevel)
{
    return m_isTjOpened && playerLevel >= m_currOpenedTjLevel;
}
bool Tianjie::isOpenTj(Player* pl)
{
    int playerLevel = pl->GetLev();
    //有玩家达到了开启天劫的条件
    if (!m_isTjOpened && playerLevel == m_currOpenedTjLevel)
    {
        SYSMSG_BROADCASTV(5000, pl->getCountry(), pl->getName().c_str(), playerLevel);
        OpenTj();
    }
    return m_isTjOpened;
}
 bool Tianjie::isTjNpc(int npcid, UInt16 loc)
 {
 	if (!m_isTjExecute) return false;
	
 	multimap<int, int>::iterator iter = m_locNpcMap.find(loc);
	//验证怪物和玩家是否在同一个据点
	while (iter != m_locNpcMap.end())
	{
        if (iter->first != loc)
            break;
	    if (iter->second == npcid)
	    {
	        return true;
	    }
        ++iter;
	}
 	return false;
 }
bool  Tianjie::isTjRateNpc(int npcid)
{
    if (!m_isTjExecute)
       return false;
    if (m_currTjRate == 1)
    {
        bool res = (npcid == s_rate1Npc[m_tjTypeId][0] || npcid == s_rate1Npc[m_tjTypeId][1] ||
                    npcid == s_rate1Npc[m_tjTypeId][2] || npcid == s_rate1Npc[m_tjTypeId][3]);
        return res;
    }
    if (m_currTjRate == 3)
    {
        return (npcid == s_tlzNpcId[m_tjTypeId][0] || npcid == s_tlzNpcId[m_tjTypeId][1]); 
    }
    return false;
}
 //开启天劫
void Tianjie::OpenTj()
{
    m_notifyRate = 0;
	m_isTjOpened = 1;
	m_currTjRate = 1;

    struct tm* local;
	time_t t = time(NULL);
	local = localtime(&t);

    if (START_WITH_59)
        m_openTime = TimeUtil::Now()+ TJ_EVENT_WAIT_TIME;
    else
    {        
        int currHour = local->tm_hour;
        if (currHour < TJ_START_TIME_HOUR)
	    	m_openTime = TimeUtil::MkTime(local->tm_year+1900, local->tm_mon+1, local->tm_mday, TJ_START_TIME_HOUR+1,0,0);
	    else
            m_openTime = t + (23-currHour)*3600 + (59-local->tm_min)*60 + (60-local->tm_sec) + TJ_START_TIME_HOUR*3600;
    }
    DB1().PushUpdateData("REPLACE INTO `tianjie`(`is_opened`,`level`,`opentime`,`rate`) VALUES(%d, %d, from_unixtime(%d), %d)", 
		                 m_isTjOpened, m_currOpenedTjLevel, m_openTime, m_currTjRate);

    SYSMSG_BROADCASTV(5001);

    Stream st(REQ::TIANJIE);
    UInt8 type = 5; //通知
    UInt8 status = 1; //开启状态
    UInt8 id = m_tjTypeId;

    st << type << status << id << Stream::eos;
    
    NETWORK()->Broadcast(st);
}
	
bool Tianjie::Init()
{
    LoadFromDB();

    initSortMap();
	return true;
}
bool Tianjie::LoadFromDB()
{
	std::unique_ptr<DB::DBExecutor> execu(DB::gObjectDBConnectionMgr->GetExecutor());
	if (execu.get() == NULL || !execu->isConnected()) return false;

	GData::DBTianjie dbexp;
    if(execu->Prepare("SELECT `id`, `is_opened`,`is_execute`,`is_finish`,`is_ok`,`level`,`rate`,UNIX_TIMESTAMP(opentime),`r1_killed`,`r2_donated`,`r3_copyid`,`r4_day` FROM `tianjie` order by level desc limit 1", dbexp) != DB::DB_OK)
		return false;

	bool isTjDBNull = true;
	if(execu->Next() == DB::DB_OK)
	{
	    isTjDBNull = false;
		m_isTjOpened = dbexp.is_opened;
		m_isTjExecute = dbexp.is_execute;
		m_isFinish = dbexp.is_finish;
		m_isOk = dbexp.is_ok;
		m_currOpenedTjLevel = dbexp.level;
		m_currTjRate = dbexp.rate;
		m_openTime = dbexp.opentime;

        std::string s_r1_killed = dbexp.r1_killed;
        std::string s_r2_donated = dbexp.r2_donated;
        if (m_currTjRate == 3)
        {
            m_eventCurrNumber = dbexp.r3_copyid;
            m_eventMaxNumber = s_maxTlzLevel;
        }
        m_bossDay = dbexp.r4_day;

        size_t i = 0;
        std::string s;
        while ((s = GetNextSection(s_r1_killed, ',')) != "")
        {
            if (i > 3) break;
            m_rate1KilledCount[i] = atoi(s.c_str());
            i++;
        }
        i = 0;
        while ((s = GetNextSection(s_r2_donated, ',')) != "")
        {
            if (i > 3) break;
            m_rate2DonateCount[i] = atoi(s.c_str());
            i++;
        }

        for (i = 0; i < sizeof(s_tjRoleLevel)/sizeof(s_tjRoleLevel[0]); ++i)
		{
			if (s_tjRoleLevel[i] == m_currOpenedTjLevel)
            {
                m_tjTypeId = i;
                break;
            }
        }	
        if (m_isTjExecute) //在事件进行中,程序重启了
        {
            if (m_currTjRate == 1)
            {
                m_eventCurrNumber = m_rate1KilledCount[0]+m_rate1KilledCount[1]+m_rate1KilledCount[2]+m_rate1KilledCount[3];
                m_eventMaxNumber = s_rate1NpcCount[0] + s_rate1NpcCount[1]+ s_rate1NpcCount[2]+s_rate1NpcCount[3];       
            }
            else if (m_currTjRate == 2)
            {
                m_eventCurrNumber = m_rate2DonateCount[0]*s_rate2DonateScore[0] +  m_rate2DonateCount[1]*s_rate2DonateScore[1]+
                                    m_rate2DonateCount[2]*s_rate2DonateScore[2] +  m_rate2DonateCount[3]*s_rate2DonateScore[3];
                m_eventMaxNumber = s_rate2DonateScoreMax;
            }    
            if (m_eventMaxNumber > 0)
                m_eventOldPercent = m_eventCurrNumber*100/m_eventMaxNumber;
            m_oldBroadPercent = m_eventOldPercent%25 ;
         
            if (TimeUtil::Now() - m_openTime < TJ_EVENT_PROCESS_TIME)
            {
                m_isNotRestart = false;
                startTianjie(true);
            }
            else
            {
                closeTianjie();
            }
        }
	}
	if (isTjDBNull) //天劫表无数据,从玩家表中计算
	{
        if (START_WITH_59) //所有天劫都跑一遍
        {
            m_currOpenedTjLevel = s_tjRoleLevel[0];
            m_tjTypeId = 0;

            OpenTj();
        }
        else
        {
	        GData::DBPlayerMaxLevel maxlevel = {0};
	    	if(execu->Prepare("SELECT max(level) from fighter", maxlevel) != DB::DB_OK)
	    	    return false; 

	    	int currMaxPlayerLevel = 0;
	    	if (execu->Next() == DB::DB_OK)
	    	{
	    		currMaxPlayerLevel = maxlevel.level;
	    	}
            int count = sizeof(s_tjRoleLevel)/sizeof(s_tjRoleLevel[0]);
	    	for (int i = 0; i < count; ++i)
	    	{
                if (i == count-1)
                {
                    m_currOpenedTjLevel = s_tjRoleLevel[i];
                }
                else if (currMaxPlayerLevel >= s_tjRoleLevel[i] && currMaxPlayerLevel <  s_tjRoleLevel[i+1])
                {
                    m_currOpenedTjLevel = s_tjRoleLevel[i];
                }
                else
                {
                    continue;
                }
                m_tjTypeId = i;
	    	    
                OpenTj();	
	    		
                break;
	    	}
        }
	}
        
	return true;
}
void Tianjie::onTianjieReq( GameMsgHdr& hdr, const void* data)
{
    MSG_QUERY_PLAYER(pl);
    BinaryReader br(data, hdr.msgHdr.bodyLen);
    UInt8 type = 0;
    br >> type;

    /** 查询状态 **/
    if(0 == type)
    {
        getTianjieData(pl);
    }
    else if (m_isTjExecute && (type == m_currTjRate)) //天劫活动运行
    {
        UInt8 cmd = 0;
        UInt8 id = 0;
        br >> cmd;
        br >> id;
        if (2 == type && 1 == cmd) //捐款
        {
           donate2(pl, id);
        }
        else if (3 == type && 1 == cmd) //破阵
        {
            attack3(pl);
        }
    }
    else if (type < m_currTjRate) //日常任务必须小于当前进度 
    {
        UInt8 cmd = 0;
        UInt8 id = 0;
        br >> cmd;
        br >> id;
        pl->OnDoTianjieTask(type, cmd, id);
    }
}

void Tianjie::getTianjieData(Player* pl, bool isLogin)
{
    if (isLogin)
    {
        Stream st1(REQ::TIANJIE);
        UInt8 type1 = 5; //通知
        UInt8 status1 =  m_isTjOpened;
        UInt8 id1 = m_tjTypeId;
        st1 << type1 << status1 << id1 << Stream::eos;
        pl->send(st1);

        return;
    }
    Stream st(REP::TIANJIE);
    UInt8 status = m_isTjOpened;
    UInt8 type = 0;
    if (m_isTjOpened)
    {
        UInt8 id = getTjTypeId();
        UInt8 s1 = 0; //事件1状态 0:关闭 1:正在运行 2:已渡过
        UInt8 s2 = 0;
        UInt8 s3 = 0;
        UInt8 s4 = 0;
        if (1 == m_currTjRate)
        {
            s1 = m_isTjExecute;
            st << type << status << id << s1 << s2 << s3 << s4;
            if (m_isTjExecute) //事件1正在运行
            {
                getEvent1Data(st, pl);
            }
            else
            {
                st << m_openTime - TimeUtil::Now(); //开启事件
            }
        }
        else if (2 == m_currTjRate)
        {
            s1 = 2;
            s2 = m_isTjExecute;
            
            st << type << status << id << s1 << s2 << s3 << s4;
            //获取日常任务1的信息
            pl->getTjTask1Data(st);
            
            if (m_isTjExecute)
            {
                getEvent2Data(st, pl);
            }
            else
            {
                st << m_openTime - TimeUtil::Now(); //开启事件
            }
        }
        else if (3 == m_currTjRate)
        {
            s1 = s2 = 2;
            s3 = m_isTjExecute;
            
            st << type << status << id << s1 << s2 << s3 << s4;
            pl->getTjTask1Data(st);
            pl->getTjTask2Data(st);

            if (m_isTjExecute)
            {
                getEvent3Data(st, pl);
            }
            else
            {
                st << m_openTime - TimeUtil::Now(); //开启事件
            }
        }
        else if (4 <= m_currTjRate)
        {
            s1 = s2 = s3 = 2;
            s4 = m_isTjExecute;

            st << type << status << id << s1 << s2 << s3 << s4;
            pl->getTjTask1Data(st);
            pl->getTjTask2Data(st);
            pl->getTjTask3Data(st);

            if (m_isTjExecute)
            {
                int time4 = m_openTime + TJ_EVENT_PROCESS_TIME - TimeUtil::Now();

                st << time4;
                if (4 == m_currTjRate)
                {
                    broadBossAppear(s_tjBoss[m_bossIndex][0], m_loc, pl);
                    broadBossCount(m_eventCurrNumber, pl);
                }
                else
                {
                    broadBossCount(10, pl);
                    broadBossAppear(s_tjBoss[m_bossIndex][1], m_loc, pl);
                }
            }
            else
            {
                st << m_openTime - TimeUtil::Now(); //开启事件
            }
            
        }

        st << Stream::eos;
        pl->send(st);

        //获取排行榜信息
        updateRankData(pl);

        if (m_isTjExecute)
        {
            if (1 == m_currTjRate)
                broadEvent1(pl);
            if (2 == m_currTjRate)
                broadEvent2(pl);
            if (3 == m_currTjRate)
                broadEvent3(pl);
        }
    }
    else //天劫未开启
    {
        UInt8 status = 0;
        UInt8 id = m_tjTypeId;
        st << type <<status  << id << Stream::eos;
        pl->send(st);
    }
}
void Tianjie::getEvent1Data(Stream& st, Player* pl)
{
     short k1 = m_rate1KilledCount[0];
     short k2 = m_rate1KilledCount[1];
     short k3 = m_rate1KilledCount[2];
     short k4 = m_rate1KilledCount[3];
     short rank1 = -1;
     int score = -1;
     if (pl != NULL)
     {
         rank1 = getEventRank(pl);
         score = pl->GetVar(VAR_TJ_EVENT_PRESTIGE);
     }
     UInt8 percent1 = m_eventOldPercent;
     int t = m_openTime + TJ_EVENT_PROCESS_TIME - TimeUtil::Now();
    
     st << k1 << k2 << k3 << k4 << rank1 << score << percent1 << t;
}
void Tianjie::getEvent2Data(Stream& st, Player* pl)
{
    short n1 = m_rate2DonateCount[0];
    short n2 = m_rate2DonateCount[1];
    short n3 = m_rate2DonateCount[2];
    short n4 = m_rate2DonateCount[3];
    short rank2 = -1;
    int score = -1;
    if (pl != NULL)
    {
        rank2 = getEventRank(pl);
        score = pl->GetVar(VAR_TJ_EVENT_PRESTIGE);
    }
    UInt8 percent2 = m_eventOldPercent;
    int t = m_openTime + TJ_EVENT_PROCESS_TIME - TimeUtil::Now();
   
    st << n1 << n2 << n3 << n4 << rank2 << score << percent2 << t;
}
void Tianjie::getEvent3Data(Stream& st, Player* pl)
{
    short copyid = m_eventCurrNumber; //当前破阵数
    short rank3  = -1;
    int score = -1;
    if (pl != NULL)
    {
        rank3 = getEventRank(pl);
        score = pl->GetVar(VAR_TJ_EVENT_PRESTIGE);
    }
    UInt8 percent3 = m_eventOldPercent;
    int t = m_openTime + TJ_EVENT_PROCESS_TIME - TimeUtil::Now();

    st << copyid << rank3 << score << percent3 << t;
}
void Tianjie::broadEvent1(Player* pl)
{
     Stream st(REQ::TIANJIE);
     UInt8 type = 6;
     UInt8 event = 1;
     UInt8 cmd = 0;

     st  << type << event << cmd;
     getEvent1Data(st, pl);
     st << Stream::eos;
     
     if (pl != NULL)    
         pl->send(st);
     else
        NETWORK()->Broadcast(st);
}
void Tianjie::broadEvent2(Player* pl)
{
    Stream st(REQ::TIANJIE);
    UInt8 type = 6;
    UInt8 event = 2;
    UInt8 cmd = 0;

    st << type << event << cmd;
    getEvent2Data(st, pl);
    st << Stream::eos;

    if (pl != NULL)
        pl->send(st);
    else
        NETWORK()->Broadcast(st);
}
void Tianjie::broadEvent3(Player* pl)
{
    Stream st(REQ::TIANJIE);
    UInt8 rtype = 6;
    UInt8 event = 3;
    UInt8 cmd = 0;
    st << rtype << event << cmd;
    getEvent3Data(st, pl);
    st << Stream::eos;

    if (pl != NULL)
        pl->send(st);
    else
        NETWORK()->Broadcast(st);
}
void Tianjie::updateEventData(Player* pl)
{
    if (1 == m_currTjRate)
        broadEvent1(pl);
    if (2 == m_currTjRate)
        broadEvent2(pl);
    if (3 == m_currTjRate)
        broadEvent3(pl);
}
void Tianjie::updateRankData(Player* pl)
{
    Stream st(REQ::TIANJIE);
    UInt8 type = 8;
    short rank = getScoreRank(pl);
    int score = pl->GetVar(VAR_TJ_TASK_PRESTIGE);
    
    st << type << rank << score;
    
    int i = 0;
    multimap<int,  Player*>::reverse_iterator iter;
    for (iter=m_scoreSortMap.rbegin(); iter!=m_scoreSortMap.rend(); ++iter)
    {
        i++;
        string name = iter->second->getName();
        st << name;

        if (i == 10)
            break;
    }
    while (i < 10) //少于10个,填空的string
    {
        string name;
        st << name;
        i++;
    }
    st << Stream::eos;
    pl->send(st);
}
void Tianjie::setRatePercent()
{
    UInt8 percent = m_eventCurrNumber * 100 / m_eventMaxNumber;
    if (percent > 100)
        percent = 100;
    if (percent - m_oldBroadPercent >= 25)
    {
        int msgId = 5011;
        if (2 == m_currTjRate) msgId = 5021;
        if (3 == m_currTjRate) msgId = 5031;
        if (percent < 100) 
            SYSMSG_BROADCASTV(msgId, m_tjTypeId, m_currTjRate, percent);

        m_oldBroadPercent = percent;
    }
    m_eventOldPercent = percent; 
}
void Tianjie::broadEventTop1(Player* pl)
{
    int playerScore = pl->GetVar(VAR_TJ_EVENT_PRESTIGE);
    int playerId = pl->getId();

    if (playerScore > m_Top1Record && playerId != m_Top1PlayerId)
    {
        SYSMSG_BROADCASTV(5003, pl->getCountry(), pl->getName().c_str());

        m_Top1Record = playerScore;
        m_Top1PlayerId = playerId;
    }
    else if (playerId == m_Top1PlayerId)
    {
        m_Top1Record = playerScore;
    }
}
void Tianjie::getRate3DailyData(Player* pl, Stream& st)
{
    short copyid = 0;
    UInt8 percent3 = 0;
    int exp3 = 0;
    st << copyid << percent3 << exp3;
}
void Tianjie::process(UInt32 now)
{
    if (m_isTjOpened)
    {
		int tm = m_openTime - now;

		if (0 == m_notifyRate && tm >= 0 && tm <= 15*TIME_60)
		{
			m_notifyRate = 1;
		}
//		printf("process, tm:%d, notifyRate:%d, rate=%d\n", tm, m_notifyRate, m_currTjRate);
    	switch (m_notifyRate)
    	{
	        case 1:
	          if (m_isNotRestart)
                  SYSMSG_BROADCASTV(5002, 15); 
	            m_notifyRate = 2;
	            break;

	        case 2:
				if (tm > 10*TIME_60)
					return;
				
	            if (m_isNotRestart)
			        SYSMSG_BROADCASTV(5002, 10);
	            m_notifyRate = 3; 
	            break;
    
	        case 3:
				if (tm > 5*TIME_60)
					return;
	            if (m_isNotRestart)
				    SYSMSG_BROADCASTV(5002, 5);
	            m_notifyRate = 4;
	            break;

	        case 4:
				if (tm > 5)
					return;
	            m_notifyRate = 5;
	            startTianjie();
	            break;
				
			case 5:
				if (tm > (-1*TJ_EVENT_PROCESS_TIME)) //事件持续时间
//                if (tm > (-1*720))
                    return;
				closeTianjie();
				m_notifyRate = 0;

	        default:
	            break;
	    }
    }
    else if (m_nextTjTime > 0)
    {
		int tm = m_nextTjTime - now;
        if (tm < 5)
        {
            OpenTj();
            m_nextTjTime = 0;
        }
    }
}

void Tianjie::startTianjie(bool isRestart)
{
	m_notifyRate = 5;
	printf("----------------------startTianjie, rate:%d\n", m_currTjRate);
	m_isTjExecute = true;
    m_eventSortMap.clear();

	if (1 == m_currTjRate)
	{
	    start1();
	}
    else if (2 == m_currTjRate)
    {
        start2();
    }
    else if (3 == m_currTjRate)
    {
        start3();
    }
    else if (4 <= m_currTjRate)
    {
        m_currTjRate = 4;
        start4(isRestart);
    }
	DB1().PushUpdateData("update tianjie set is_execute=%d, is_finish=%d, rate=%d, r4_day=%d  where level = %d", 1, 0, m_currTjRate, m_bossDay, m_currOpenedTjLevel);	
    m_isNotRestart = true;
}
void Tianjie::closeTianjie()
{
	printf("----------------------closeTianjie, rate:%d\n", m_currTjRate);
	if (1 == m_currTjRate)
	{
	    close1();
	}
    else if (2 == m_currTjRate)
    {
        close2();
    }
    else if (3 == m_currTjRate)
    {
        close3();
    }
    else if (4 == m_currTjRate)
    {
        close4();
    }
    else if (5 == m_currTjRate)
    {
        closeBoss();
    }
    //发奖品
    if ( m_currTjRate <= 3)
    {
        reward(m_eventSortMap, VAR_TJ_EVENT_PRESTIGE, 0);
    }
    else if (m_currTjRate ==4 && !m_isFinish)
    {
        rewardTask();
    }
    else if (m_currTjRate == 5)
    {
        if (m_isOk)
            rewardBoss();
        rewardTask();
    }
    //下一步
    goNext();

    m_isNotRestart = true;
}
void Tianjie::goNext()
{
    m_notifyRate = 0;
    if ((m_currTjRate == 4 && !m_isFinish) || m_currTjRate == 5)
    {
       broadTianjiePassed();
	   
       DB1().PushUpdateData("update tianjie set is_opened=%d,is_execute=%d,is_finish=%d,is_ok=%d where level=%d", 
	   	                    0, 0, m_isFinish, m_isOk, m_currOpenedTjLevel);
	   
       m_isTjOpened = false;
	   m_isTjExecute = 0;
	   m_isFinish = 0;
	   m_isOk       = false;
	   m_currTjRate = 0;
	   m_openTime = 0;
       m_bossDay =  0;

       memset(m_rate1KilledCount, 0, sizeof(m_rate1KilledCount));
       memset(m_rate2DonateCount, 0, sizeof(m_rate2DonateCount));
 
       m_eventSortMap.clear();
       m_scoreSortMap.clear();

       if (m_tjTypeId+1 < sizeof(s_tjRoleLevel)/sizeof(s_tjRoleLevel[0]))
       {
           m_currOpenedTjLevel = s_tjRoleLevel[++m_tjTypeId];
	   	   DB1().PushUpdateData("INSERT INTO `tianjie`(`level`) VALUES(%d)",m_currOpenedTjLevel);
           if (START_WITH_59)
               m_nextTjTime = TimeUtil::Now() + TJ_EVENT_WAIT_TIME;
           else
               m_openTime += ONE_DAY_SECOND;
       }
	   else
       {
		   m_currOpenedTjLevel = 0;
           m_tjTypeId = 0;
	   }
    }
    else if (4 == m_currTjRate) 
    {
        m_currTjRate += 1;
        startBoss();
    }
	else
    {
		m_isTjExecute = 0;
		m_isFinish = 0;
        if (START_WITH_59)
            m_openTime = TimeUtil::Now() + TJ_EVENT_WAIT_TIME; 
        else
            m_openTime += ONE_DAY_SECOND;
        m_currTjRate += 1;

		DB1().PushUpdateData("update tianjie set is_execute=%d, is_finish=%d, rate=%d, opentime=FROM_UNIXTIME(%d) where level=%d", 
	   	                     0, m_isFinish, m_currTjRate, m_openTime, m_currOpenedTjLevel);
	}
}

void Tianjie::start1()
{	
    m_eventMaxNumber = 0;
    if (m_isNotRestart)
    {
        m_eventCurrNumber = 0;
        m_eventOldPercent = 0;
        m_oldBroadPercent = 0;
    }
    m_Top1Record = 0;
    m_Top1PlayerId = 0;

	for (size_t i = 0; i < 4; ++i)
	{
	    UInt32 npcid = 0;
	    npcid = s_rate1Npc[m_tjTypeId][i];

		//获取npc数据
    	GData::NpcGroups::iterator it = GData::npcGroups.find(npcid);
    	if(it == GData::npcGroups.end())
       	    continue;

    	_ng = it->second;
    	if (!_ng) continue;

        for (size_t j = 0; j < 20; ++j) //一次刷20只怪
        {
            addNpc(npcid);
        }

        //需要消灭的小怪总数
        m_eventMaxNumber += s_rate1NpcCount[i];
	}
	printf("----------------------start1()\n");

    if (m_isNotRestart)
    {
        SYSMSG_BROADCASTV(5010);
        broadEvent1();
    }
}
void Tianjie::attack(Player* pl, UInt16 loc, UInt32 npcid)
{
    if (!m_isTjExecute) return;

    if (1 == m_currTjRate)
    {
        attack1(pl, loc, npcid);
    }
    if (4 == m_currTjRate)
    {
        attack4(pl, loc, npcid);
    }
    else if (5 == m_currTjRate)
    {
        bool res = attackBoss(pl, s_tjBoss[m_bossIndex][1], 1, 1);
        if (res)
        {
            closeTianjie();
        }
    }
}

void Tianjie::attack1(Player* pl, UInt16 loc, UInt32 npcid)
{
    if (!m_isTjExecute || 1 != m_currTjRate)
        return;
    if (m_tj1Count > 0)
        m_eventMaxNumber = m_tj1Count;

    int index = -1;
	for (size_t i = 0; i < 4; ++i)
    {
        if (s_rate1Npc[m_tjTypeId][i] == npcid)
        {
            index = i;
		    break;
        }
    }
	if (index == -1) return;
	
    bool res = false;
    multimap<int, int>::iterator iter = m_locNpcMap.find(loc);
	//验证怪物和玩家是否在同一个据点
	while (iter != m_locNpcMap.end())
	{
        if (iter->first != loc)
            break;
	    if (iter->second == (int)npcid)
	    {
	        res = pl->attackNpc(npcid);
            break;
	    }
        ++iter;
	}
	//玩家赢了
	if (res)
	{
       // printf("--------------------------------------tj1, killed:%d, maxcount:%d\n", m_eventCurrNumber, m_eventMaxNumber);
	    if (m_rate1KilledCount[index] < s_rate1NpcCount[index])
        {
            m_rate1KilledCount[index] += 1;
            m_eventCurrNumber += 1;

            char r1_killed[32] = {0};
            sprintf(r1_killed, "%d,%d,%d,%d", m_rate1KilledCount[0], m_rate1KilledCount[1], m_rate1KilledCount[2], m_rate1KilledCount[3]);

            DB1().PushUpdateData("update tianjie set r1_killed='%s' where level=%d", r1_killed, m_currOpenedTjLevel);
 
            setRatePercent();
            broadEvent1();
        }

        //增加积分
		record1(pl, index);
        broadEvent1(pl);
        
        //删除NPC
        deleteNpc(npcid, loc);

		//判断是否事件已达成
		isFinish();

        //天劫事件还在运行
		if (m_isTjExecute)
		{
		    addNpc(npcid);
		}
        SYSMSG_BROADCASTV(5012, pl->getCountry(), pl->getName().c_str(), loc, npcid);
	}
}
void Tianjie::record1(Player* pl, int npcIndex)
{
    UInt32 score = s_rate1NpcScore[npcIndex];
    
    pl->addExpOrTjScore(0, score, true, true);
}
bool Tianjie::isFinish()
{
    if (!m_isFinish && m_eventCurrNumber >= m_eventMaxNumber)
	{
	    m_isFinish = true;
        DB1().PushUpdateData("update tianjie set is_finish=%d where level=%d", 1, m_currOpenedTjLevel);

        int msgId = 5013;
        if (2 == m_currTjRate) msgId = 5022;
        if (3 == m_currTjRate) msgId = 5032;
        if (msgId == 5022)
        {
            SYSMSG_BROADCASTV(msgId, m_tjTypeId, m_currTjRate, m_tjTypeId, m_currTjRate);
        }
        else
        {
            SYSMSG_BROADCASTV(msgId, m_tjTypeId, m_currTjRate);
        }
    }
	return m_isFinish;
}

void Tianjie::close1()
{
	multimap<int, int>::iterator iter;
	for (iter = m_locNpcMap.begin(); iter != m_locNpcMap.end(); ++iter)
	{
		deleteNpc(iter->second, iter->first);
	}
    m_locNpcMap.clear();
    memset(m_rate1KilledCount, 0, sizeof(m_rate1KilledCount));

    SYSMSG_BROADCASTV(5014);
}
void Tianjie::randomTask1Data(int roleLev,short& npcId, UInt8& color, int& exp)
{
    int idx = uRand(4);
    npcId = s_rate1Npc[m_tjTypeId][idx];

    //绿色 50%，蓝色27%，紫色15%，橙色8%
    int num = uRand(100);
    if (num < 50)
        color = 0;
    else if (num < 77)
        color = 1;
    else if (num < 92)
        color = 2;
    else
        color = 3;

    exp = TIANJIE_EXP(roleLev) * s_task1ColorMulti[color];
}
void Tianjie::start2()
{
    m_eventMaxNumber = 0;
    if (m_isNotRestart)
    {
        m_eventCurrNumber = 0;
        m_eventOldPercent = 0;
        m_oldBroadPercent = 0;
    }
    m_Top1Record = 0;
    m_Top1PlayerId = 0;

    m_eventMaxNumber = s_rate2DonateScoreMax;
   
    printf("----------------------start2()\n");

    if (m_isNotRestart)
    {
        SYSMSG_BROADCASTV(5020);
        broadEvent2();
    }
}
void Tianjie::donate2(Player* pl, UInt8 id)
{
    if (m_currTjRate != 2 || !m_isTjExecute)
        return;

    //GM命令设置最大上限
    if (m_tj2Count > 0)
        m_eventMaxNumber = m_tj2Count;

    Stream st(REQ::TIANJIE);
    UInt8 type = 6; //更新列表
    UInt8 event = 2; //事件2
    UInt8 cmd = 0;   //0:列表 
   
    if (pl->GetVar(VAR_TJ_EVENT_PRESTIGE) >= s_tj2PlayerMaxScore)
    {
        cmd = 5;
        st << type << event << cmd << Stream::eos;
        pl->send(st);
        return;
    }
    int index = -1;
    int score = 0;
    switch (id)
    {
        case ID_TJ2_TAEL:
             if (pl->getTael() >= 1000)
             {
                 ConsumeInfo ci(TianjieDonate,0,0);
                 pl->useTael(1000, &ci);
                 index = 0;
                 score = s_rate2DonateScore[0];
             }
             else
                 cmd = 1;
            break;
       case ID_TJ2_GOLD:
             if (pl->getGold() >= 10)
             {
                 ConsumeInfo ci(TianjieDonate,0,0);
                 pl->useGold(10, &ci);
                 index = 1;
                 score = s_rate2DonateScore[1];
             }
             else
                 cmd = 2;
            break;
       case ID_TJ2_COUPON:
             if (pl->getCoupon() >= 10)
             {
                 ConsumeInfo ci(TianjieDonate,0,0);
                 pl->useCoupon(10, &ci);
                 index = 2;
                 score = s_rate2DonateScore[2];
             }
             else
                 cmd = 3;
            break;
       case ID_TJ2_TLYJ:
             if (pl->GetPackage()->DelItemAny(9138, 1))
             {
                 index = 3;
                 score = s_rate2DonateScore[3];
             }
             else
                 cmd = 4;
            break;
        default:
            break;
    }
    if (cmd > 0) //捐款失败
    {
        st << type << event << cmd << Stream::eos;
        pl->send(st);
        return;
    }
    if (index > -1)
    {
        //printf("--------------------------------------tj2, donated:%d, maxscore:%d\n", m_eventCurrNumber, m_eventMaxNumber);

        int exp = TIANJIE_EXP(pl->GetLev()) * s_rate2DonateExpMulti[index];
        pl->addExpOrTjScore(exp, score, true, false);
        pl->AddVar(id, score);
        
        m_rate2DonateCount[index] += 1;
        //计算积分
        m_eventCurrNumber += score;
        isFinish();
        
        setRatePercent(); 
        broadEvent2();
        broadEvent2(pl);
        
        char r2_donated[32] = {0};
        sprintf(r2_donated, "%d,%d,%d,%d", m_rate2DonateCount[0], m_rate2DonateCount[1], m_rate2DonateCount[2], m_rate2DonateCount[3]);

        DB1().PushUpdateData("update tianjie set r2_donated='%s' where level=%d", r2_donated, m_currOpenedTjLevel);
       /*
        if (m_isFinish)
        {
            closeTianjie();
        }*/
    }
}

void Tianjie::close2()
{
    printf("----------------------close2()\n");
    
    memset(m_rate2DonateCount, 0, sizeof(m_rate2DonateCount));
    SYSMSG_BROADCASTV(5023, m_tjTypeId, m_currTjRate);
}
void Tianjie::start3()
{
    m_eventMaxNumber = 0;
    if (m_isNotRestart)
    {
        m_eventCurrNumber = 0;
        m_eventOldPercent = 0;
        m_oldBroadPercent = 0;
    }
    m_Top1Record = 0;
    m_Top1PlayerId = 0;

    m_eventMaxNumber = s_maxTlzLevel;

    if (m_isNotRestart)
    {
        SYSMSG_BROADCASTV(5030);
        broadEvent3();
    }
    for (int i = 0; i < 2; ++i)
    {
        GData::NpcGroups::iterator it = GData::npcGroups.find(s_tlzNpcId[m_tjTypeId][i]);
        if(it == GData::npcGroups.end())
            break;
        GData::NpcGroup* _ng = it->second;
        m_tlzNpcName[i] = _ng->getName(); 
    }
    printf("----------------------start3()\n");
}
bool getAllFormation(const unsigned int& k, const GData::Formation* f, void* idVec)
{
    std::vector<UInt32>* p = (std::vector<UInt32>*)idVec;
    p->push_back(k);
    return true;
}
void Tianjie::putTlzFighters(BattleSimulator& bsim, int tlzLevel)
{
    static std::vector<UInt32> idVec;
    if (idVec.empty())
        GData::formationManager.enumerate(getAllFormation, (void*)&idVec);
    
    UInt16 fIdx = 0;
    int fCount = idVec.size();
    
    fIdx = uRand(fCount);
    bsim.setFormation(1, idVec[fIdx]);
    
    const GData::Formation* form = GData::formationManager[idVec[fIdx]];

    for (int i = 0; i < 5; i++)
    {
        int idx = uRand(2);

        int npcId = s_tlzNpcId[m_tjTypeId][idx];
   
        GData::NpcGroups::iterator it = GData::npcGroups.find(npcId);
        if(it == GData::npcGroups.end())
           break;
        GData::NpcGroup* _ng = it->second;
        std::vector<GData::NpcFData>& _npcList = _ng->getList();
        if (_npcList.size() == 0)
            break;
        GObject::Fighter* fighter = _npcList[0].fighter;
        GData::AttrExtra baseAttrExtra = *(fighter->getAttrExtraEquip());
        
        int j = tlzLevel;
        GData::AttrExtra attrExtra = baseAttrExtra;
        attrExtra.attack *= (1 + s_rate3NpcBaseModulus[0]*j);
        attrExtra.magatk *= (1 + s_rate3NpcBaseModulus[1]*j);
        attrExtra.defend *= (1 + s_rate3NpcBaseModulus[2]*j);
        attrExtra.magdef *= (1 + s_rate3NpcBaseModulus[3]*j);
        attrExtra.hp     *= (1 + s_rate3NpcBaseModulus[4]*j);
        attrExtra.action *= (1 +s_rate3NpcBaseModulus[5]*j);
        attrExtra.hitrate = min(s_rate3NpcAdvanceModMax[0], attrExtra.hitrate*(1+s_rate3NpcAdvanceModulus[0]*j));
        attrExtra.evade = min(s_rate3NpcAdvanceModMax[1], attrExtra.evade*(1+s_rate3NpcAdvanceModulus[1]*j));
        attrExtra.counter = min(s_rate3NpcAdvanceModMax[2], attrExtra.counter*(1+s_rate3NpcAdvanceModulus[2]*j));
        attrExtra.magres = min(s_rate3NpcAdvanceModMax[3], attrExtra.magres*(1+s_rate3NpcAdvanceModulus[3]*j));
        //增强NPC的属性
        fighter->setAttrExtraEquip(attrExtra);

        bsim.newFighter(1, form->getPos(i), fighter);

        //重置回基础属性
        fighter->setAttrExtraEquip(baseAttrExtra);
    }
}
void Tianjie::attack3(Player* pl)
{
    if (!m_isTjExecute || m_currTjRate != 3)
        return;

    //GM命令设置最大上限
    if (m_tj3Count > 0)
        m_eventMaxNumber = m_tj3Count;

    if (0 == m_eventCurrNumber) 
        m_eventCurrNumber = 1;
   
    bool res = attackTlz(pl, m_eventCurrNumber);
    if (res)
    {
        //printf("--------------------------------------tj3, pozhen:%d, maxlevel:%d\n", m_eventCurrNumber, m_eventMaxNumber);

        int exp = TIANJIE_EXP(pl->GetLev()) * s_rate3ExpMulti;
        pl->addExpOrTjScore(exp, s_rate3NpcScore, true, true);
        
        isFinish();

        broadEventTop1(pl);

        //无限层数
        m_eventCurrNumber += 1;
        if (m_eventCurrNumber <= m_eventMaxNumber)
        {
            setRatePercent();

            DB1().PushUpdateData("update tianjie set r3_copyid=%d where level=%d", m_eventCurrNumber, m_currOpenedTjLevel);
       }
       broadEvent3();
       broadEvent3(pl);
    }
}

void Tianjie::close3()
{
    printf("----------------------close3()\n");

    SYSMSG_BROADCASTV(5033, m_tjTypeId, m_currTjRate);
}

bool Tianjie::attackTlz(Player* pl, UInt16 level)
{
    int r = uRand(2);
    Battle::BattleSimulator bsim(Battle::BS_COPY1, pl, m_tlzNpcName[r], s_tjRoleLevel[m_tjTypeId], false);
    pl->PutFighters(bsim, 0);
    putTlzFighters(bsim, level);

    bsim.start();
    Stream& packet = bsim.getPacket();
    if(packet.size() <= 8)
       return false;

    bool res = bsim.getWinner() == 1;
     
    UInt16 ret = 0x0100;
    if (res) ret = 0x0101;
    Stream st(REP::ATTACK_NPC);
    st << ret << PLAYER_DATA(pl, lastExp) << static_cast<UInt8>(0);
    UInt8 size = pl->_lastLoot.size();
    st << size;
    for (UInt8 i = 0; i < size; ++i)
    {
        st << pl->_lastLoot[i].id << pl->_lastLoot[i].count;
    }
    st.append(&packet[8], packet.size() - 8);
    st << Stream::eos;
    pl->send(st);
    
    return res;
}
void  Tianjie::broadBossAppear(UInt32 bossId, UInt16 loc, Player* pl)
{
    //sendId
    {
        Stream st(REP::DAILY_DATA);
        st << static_cast<UInt8>(16);
        st << static_cast<UInt8>(4);
        st << static_cast<UInt32>(bossId);
        st << Stream::eos;
        if (NULL != pl)
            pl->send(st);
        else
            NETWORK()->Broadcast(st);
    }
    //sendloc
    {
        Stream st(REP::DAILY_DATA);
        st << static_cast<UInt8>(16);
        st << static_cast<UInt8>(5);
        st << static_cast<UInt32>(loc);
        st << Stream::eos;
        if (NULL != pl)
            pl->send(st);
        else 
            NETWORK()->Broadcast(st);
    }
    //sendmaxhp
    if (m_currTjRate == 5)
    {
        Stream st(REP::DAILY_DATA);
        st << static_cast<UInt8>(16);
        st << static_cast<UInt8>(0);
        st << m_bossMaxHp;
        st << Stream::eos;
        if (NULL != pl)
            pl->send(st);
        else
            NETWORK()->Broadcast(st);
 
        sendHp(pl);
    }
}
void Tianjie::broadBossCount(int count, Player* pl)
{
    Stream st(REP::DAILY_DATA);
    st << static_cast<UInt8>(16);
    st << static_cast<UInt8>(3);
    st << static_cast<UInt32>(count);
    st << Stream::eos;
    if (NULL != pl)                                                                                                                                          
        pl->send(st);
    else 
        NETWORK()->Broadcast(st);
}
void Tianjie::start4(bool isRestart)
{
    m_eventCurrNumber = 0;
    m_eventOldPercent = 0;
    m_oldBroadPercent = 0;
    m_Top1Record = 0;
    m_Top1PlayerId = 0;

    m_eventMaxNumber = 10;

    if (!isRestart)
        m_bossDay += 1;
    
    printf("----------------------start4()\n");
    
    m_bossIndex = m_tjTypeId;
    if (m_bossIndex >= 0 )
    {
        addNpc(s_tjBoss[m_bossIndex][0]);
 
        if (m_isNotRestart)
        {
            broadBossAppear(s_tjBoss[m_bossIndex][0], m_loc);
            broadBossCount(m_eventCurrNumber);
        
            SYSMSG_BROADCASTV(5040);
            SYSMSG_BROADCASTV(554, s_tjBoss[m_bossIndex][0], m_loc, s_tjBoss[m_bossIndex][0]);
        }
    }
}
void Tianjie::attack4(Player* pl, UInt16 loc, UInt32 npcid)
{
    if (m_eventCurrNumber >= m_eventMaxNumber)
        return;

     if (npcid == s_tjBoss[m_bossIndex][0])
     {
         bool res = pl->attackNpc(npcid);
         if (res)
         {
             m_eventCurrNumber += 1;
             isFinish();
             broadBossCount(m_eventCurrNumber);
             SYSMSG_BROADCASTV(552, pl->getCountry(), pl->getName().c_str(), m_loc, m_eventCurrNumber, npcid);
           
             pl->addExpOrTjScore(0, s_tj4Score, false, true);

             if (m_isFinish)
             {
                 closeTianjie();
             }
         }
     }
}
        
void Tianjie::close4()
{
    printf("----------------------close4()\n");

    multimap<int, int>::iterator iter = m_locNpcMap.find(m_loc);
    while (iter != m_locNpcMap.end())
    {
       if (iter->first != m_loc)
           break;
       if (iter->second == s_tjBoss[m_bossIndex][0])
       {
           deleteNpc(iter->second, iter->first);
           break;
       }
       ++iter;
    }
    m_locNpcMap.clear();
}

void Tianjie::startBoss()
{
    m_bossIndex = m_tjTypeId;
    printf("----------------------starBoss(), rate=%d\n", m_currTjRate);
    
    addNpc(s_tjBoss[m_bossIndex][1]);
    DB1().PushUpdateData("update tianjie set is_execute=1, rate=5 where level=%d", m_currOpenedTjLevel);

    GData::NpcGroups::iterator it = GData::npcGroups.find(s_tjBoss[m_bossIndex][1]);
    if (it !=  GData::npcGroups.end())
    {
        _ng = it->second;
        std::vector<GData::NpcFData>& nflist = _ng->getList();
        m_bossMaxHp =  nflist[0].fighter->getMaxHP();
        _hp = m_bossMaxHp;

        //printf("-----------------------------------bossHP:%d, maxhp:%d\n", _hp, m_bossMaxHp);
        broadBossAppear(s_tjBoss[m_bossIndex][1], m_loc);

        SYSMSG_BROADCASTV(553, s_tjBoss[m_bossIndex][1]);
    }
    //SYSMSG_BROADCASTV(5001);
}

bool Tianjie::attackBoss(Player* pl, UInt32 npcId, UInt8 expfactor, bool final)
{
   static UInt32 sendflag = 7;

    ++sendflag;

    if (!pl) return false;
    UInt32 now = TimeUtil::Now();
    UInt32 buffLeft = pl->getBuffData(PLAYER_BUFF_ATTACKING, now);
    if(buffLeft > now) 
    {    
        pl->sendMsgCode(0, 1407, buffLeft - now);
        return false;
    }    
    pl->checkLastBattled();

    GData::NpcGroups::iterator it = GData::npcGroups.find(npcId);
    if(it == GData::npcGroups.end())
        return false;

    _ng = it->second;
    Battle::BattleSimulator bsim(Battle::BS_WBOSS, pl, _ng->getName(), _ng->getLevel(), false);
    pl->PutFighters(bsim, 0); 
    _ng->putFighters(bsim);

    std::vector<GData::NpcFData>& nflist = _ng->getList();
    size_t sz = nflist.size();
    if (sz == 0) return false;
    
    GData::NpcFData& nfdata = nflist[0];
    Battle::BattleFighter * bf = bsim.newFighter(1, nfdata.pos, nfdata.fighter);
    bf->setHP(_hp);
    
    //GM命令设置最大上限
    if (m_tj4BossHp > 0)
    {
        m_bossMaxHp = m_tj4BossHp;
        nflist[0].fighter->setBaseHP(m_bossMaxHp);
        nflist[0].fighter->setDirty();
      
        if (bf->getHP() > m_tj4BossHp)
        {
            _hp = m_tj4BossHp;
            _percent = 100;
            bf->setHP(_hp);
        }
        broadBossAppear(s_tjBoss[m_bossIndex][1], m_loc);
    }

    bsim.start();
    Stream& packet = bsim.getPacket();
    if(packet.size() <= 8)
        return false;

    UInt16 ret = 0x0100;
    bool res = bsim.getWinner() == 1;

    UInt32 oldHP = _hp;
    Battle::BattleObject * obj = bsim(1, nfdata.pos);
    if(obj != NULL && obj->isChar())
    {
        Battle::BattleFighter * bfgt = static_cast<Battle::BattleFighter *>(obj);
        UInt32 nHP = bfgt->getHP();

        if(nHP == 0)
            nHP = 0xFFFFFFFF;
        if(_hp != 0xFFFFFFFF && _hp != nHP)
            _hp = nHP;
        if(oldHP != 0xFFFFFFFF)
        {
            if(oldHP == 0)
                oldHP = nflist[0].fighter->getMaxHP();
            UInt32 newHP = (_hp == 0xFFFFFFFF) ? 0 : _hp;
            if(oldHP > newHP)
            {
                UInt32 damage = oldHP - newHP;
                UInt32 exp = ((float)damage / nflist[0].fighter->getMaxHP()) * _ng->getExp() * expfactor;
                if (exp < 1000)
                    exp = 1000;
                pl->addExpOrTjScore(exp, 0, false, true);
                if (!(sendflag % 8))
                    sendDmg(damage);

                AttackInfo info(pl, damage);
                AtkInfoType::iterator i = m_atkinfo.begin();
                while (i != m_atkinfo.end())
                {
                    if ((*i).player == pl)
                    {
                        info += *i;
                        m_atkinfo.erase(i);
                        break;
                    }
                    ++i;
                }
                bool ret = m_atkinfo.insert(info).second;
                TRACE_LOG("WBOSS INSERT ret: %u (pid: %"I64_FMT"u, dmg: %u)", ret, pl->getId(), damage);

               // UInt32 bossMaxHP = nflist[0].fighter->getBaseHP();
                UInt32 bossMaxHP = nflist[0].fighter->getMaxHP();
                UInt8 newPercent = (double)newHP / bossMaxHP * 100;

                if (newPercent > 100)
                    newPercent = 100;
                if (_percent < newPercent)
                    _percent = newPercent;
                if (!newPercent)
                {
                     if (sendflag % 8)
                        sendHp();

                    SYSMSG_BROADCASTV(550, nflist[0].fighter->getId());
                    _percent = 0;
                    _hp = 0;
                    m_isOk = true;
                    res = true;

                    nflist[0].fighter->setExtraAttack(0);
                    nflist[0].fighter->setExtraMagAttack(0);
                    nflist[0].fighter->setWBoss(false);
                }
                else if (newPercent <= 5 && _percent - newPercent >= 5)
                {
                    SYSMSG_BROADCASTV(548, pl->getCountry(), pl->getName().c_str(), nflist[0].fighter->getId(), newPercent);
                    _percent = newPercent;
                }
                else if (_percent - newPercent >= 10)
                {
                    SYSMSG_BROADCASTV(548, pl->getCountry(), pl->getName().c_str(), nflist[0].fighter->getId(), newPercent);
                    _percent = newPercent;
                }
                if (!(sendflag % 8))
                    sendHp();
            }
            else
            {
                TRACE_LOG("WBOSS OLDHP(%u) < NEWHP(%u)", oldHP, newHP);
            }
        }
    }
    if(res)
    {    
        ret = 0x0101;
        if (!final)
        {
            pl->_lastNg = _ng;
            pl->pendExp(_ng->getExp()*expfactor);
            _ng->getLoots(pl, pl->_lastLoot, 1, NULL);
        }
    }

    Stream st(REP::ATTACK_NPC);
    st << ret << PLAYER_DATA(pl, lastExp) << static_cast<UInt8>(0);
    UInt8 size = pl->_lastLoot.size();
    st << size;
    for (UInt8 i = 0; i < size; ++i)
    {
        st << pl->_lastLoot[i].id << pl->_lastLoot[i].count;
    }
    st.append(&packet[8], packet.size() - 8);
    st << Stream::eos;
    pl->send(st);
    bsim.applyFighterHP(0, pl);

    pl->setBuffData(PLAYER_BUFF_ATTACKING, now + 30);
    return res;
}

void Tianjie::closeBoss()
{
    printf("----------------------closeBoss()\n");

    multimap<int, int>::iterator iter = m_locNpcMap.find(m_loc);
    while (iter != m_locNpcMap.end())
    {
       if (iter->first != m_loc)
           break;
       if (iter->second == s_tjBoss[m_bossIndex][1])
       {
           deleteNpc(iter->second, iter->first);
           break;
       }
       ++iter;
    }
    m_locNpcMap.clear();

    {
        Stream st(REP::DAILY_DATA);
        st << static_cast<UInt8>(16);
        st << static_cast<UInt8>(6);
        st << static_cast<UInt32>(0);
        st << Stream::eos;
         
        NETWORK()->Broadcast(st);
    }
}
void Tianjie::broadTianjiePassed()
{
    Stream st(REQ::TIANJIE);
    UInt8 type = 5; //通知
    UInt8 status = 0; //关闭
    UInt8 id  = m_tjTypeId;

    st << type << status << id << Stream::eos;

    NETWORK()->Broadcast(st);
}
int Tianjie::makeTlzKey(UInt8 type, UInt16 level)
{
    int key = 0;
    key = type;
    key <<= 16;
    key += level;
    return key;
}
bool Tianjie::addNpc(int npcid) 
{
    //随即地点
    std::vector<UInt16> spots;
    Map::GetAllSpot(spots);
    if (!spots.size()) return false;
    
    UInt16 spot = spots[uRand(spots.size())];
    if (!spot) return false;
 
    if (m_currTjRate == 5)
        spot = m_loc;
   
    //添加NPC
    Map * p_map = Map::FromSpot(spot);
    if (!p_map) return false;
    
    MOData mo;
    mo.m_ID = npcid;
    mo.m_Hide = false;
    mo.m_Spot = spot;
    mo.m_Type = 6;
    mo.m_ActionType = 0;
    p_map->AddObject(mo);
    p_map->Show(npcid, true, mo.m_Type);

	//保存据点上的怪物
	m_locNpcMap.insert(make_pair(spot, npcid));

    m_loc = spot;

    printf("-------------------------------------------addnpc, id:%d, loc:%d\n", npcid, spot);
	return true;
}

void Tianjie::deleteNpc(int npcid, UInt16 loc)
{
    Map * p_map = Map::FromSpot(loc);
    if (!p_map) return;
    p_map->Hide(npcid, true);
	p_map->DelObject(npcid);

}

void Tianjie::sendDmg(UInt32 damage)
{
    Stream st(REP::DAILY_DATA);
    st << static_cast<UInt8>(16);
    st << static_cast<UInt8>(2);
    st << damage;
    st << Stream::eos;
    NETWORK()->Broadcast(st);
}

void Tianjie::sendHp(Player* player)
{
    Stream st(REP::DAILY_DATA);
    st << static_cast<UInt8>(16);
    st << static_cast<UInt8>(1);
    st << _hp;
    st << Stream::eos;
    if (player)
        player->send(st);
    else
        NETWORK()->Broadcast(st);
}

void Tianjie::insertToEventSortMap(Player* pl, int newScore, int oldScore)
{
    if (!m_isTjExecute || m_currTjRate >= 4)
        return;
    
    multimap<int, Player*>::iterator iter = m_eventSortMap.find(oldScore);

    while (iter != m_eventSortMap.end())
    {
        if (iter->first != oldScore)
            break;
        if (iter->second == pl)
        {
            m_eventSortMap.erase(iter);
            break;
        }
        ++iter;
    }
    m_eventSortMap.insert(make_pair(newScore, pl));
}
void Tianjie::insertToScoreSortMap(Player* pl, int newScore, int oldScore)
{
    if (!m_isTjOpened)
        return;

    multimap<int, Player*>::iterator iter = m_scoreSortMap.find(oldScore);

    while (iter != m_scoreSortMap.end())
    {
        if (iter->first != oldScore)
            break;
        if (iter->second == pl)
        {
            m_scoreSortMap.erase(iter);
            break;
        }
        ++iter;
    }
    m_scoreSortMap.insert(make_pair(newScore, pl));
}
short Tianjie::getEventRank(Player* pl)
{
    short rank = 0;
    multimap<int, Player*>::reverse_iterator iter;
    for (iter=m_eventSortMap.rbegin(); iter!=m_eventSortMap.rend(); ++iter)
    {
        rank++;
        if (iter->second == pl)
        {
            return rank;
        }
    }
    return 0;
}
short Tianjie::getScoreRank(Player* pl)
{
    short rank = 0;
    multimap<int, Player*>::reverse_iterator iter;
    for (iter=m_scoreSortMap.rbegin(); iter!=m_scoreSortMap.rend(); ++iter)
    {
        rank++;
        if (iter->second == pl)
        {
            return rank;
        }
    }
    return 0;
}
void Tianjie::rewardEventBox(Player*pl, int score)
{
    //积分奖励
    const UInt32* pBoxId = s_tjEventBoxId;
    
    int itemCount = score/5000 + 3;
    MailPackage::MailItem* pItems = new MailPackage::MailItem[itemCount];
    int j = 0;
    while (score >= 500)
    {
        if (score >= 5000)
        {
            pItems[j].id = pBoxId[3];
            score -= 5000;
        }
        else if (score >= 2000)
        {
            pItems[j].id = pBoxId[2];
            score -= 2000;
        }
        else if (score >= 1000)
        {
            pItems[j].id = pBoxId[1];
            score -= 1000;
        }
        else 
        {
            pItems[j].id = pBoxId[0];
            score = 0;
        }
        pItems[j].count = 1;
        j++;
    }
    if (j > 0)
        pl->sendMailItem(5054, 5055, pItems, j, false);

    delete [] pItems;
    pItems = NULL;
}
void Tianjie::rewardTotalBox(Player*pl, int score)
{
    //积分奖励
    const UInt32* pBoxId = s_tjTotalBoxId;
    
    int itemCount = score/10000 + 3;
    MailPackage::MailItem* pItems = new MailPackage::MailItem[itemCount];
    int j = 0;
    while (score >= 1000)
    {
        if (score >= 10000)
        {
            pItems[j].id = pBoxId[3];
            score -= 10000;
        }
        else if (score >= 5000)
        {
            pItems[j].id = pBoxId[2];
            score -= 5000;
        }
        else if (score >= 2000)
        {
            pItems[j].id = pBoxId[1];
            score -= 2000;
        }
        else 
        {
            pItems[j].id = pBoxId[0];
            score = 0;
        }
        pItems[j].count = 1;
        j++;
    }
    if (j > 0)
        pl->sendMailItem(5056, 5057, pItems, j, false);

    delete [] pItems;
    pItems = NULL;
}

void Tianjie::reward(multimap<int, Player*>& m, UInt8 varId, UInt8 EventOrTotal)
{
    multimap<int, Player*>::reverse_iterator iter;
    int i = 1;
    for (iter = m.rbegin(); iter != m.rend(); ++iter, ++i)
    {
        MailPackage::MailItem item;
        memset(&item, 0 , sizeof(item));
        if (i <= 10) //1-10名奖励
        {
            if (EventOrTotal == 0)
                item.id =s_tjEventRewardId;
            else if (EventOrTotal == 1)
            {
                item.id = s_tjTotalRewardId;
                if (i == 1)
                SYSMSG_BROADCASTV(5041, iter->second->getCountry(), iter->second->getName().c_str(), iter->second->getCountry(), iter->second->getName().c_str());
            }
            item.count = 11-i;         //个数
            iter->second->sendMailItem(5052, 5053, &item, 1, false);
            int msgId = 5050;
            if (EventOrTotal == 1) msgId = 5051;
            SYSMSG_BROADCASTV(msgId, i, iter->second->getCountry(), iter->second->getName().c_str(), item.id, item.count);
        }
        int score = iter->second->GetVar(varId);
        if (EventOrTotal == 0)
        {
            rewardEventBox(iter->second,score);
        }
        else
        {
            rewardTotalBox(iter->second,score);
        }
        iter->second->SetVar(varId, 0);
    }
    m.clear();
}

void Tianjie::rewardBoss()
{
    size_t sz = m_atkinfo.size();
    if (!sz) return;

    UInt32 j = 1;
    for (AtkInfoType::reverse_iterator i = m_atkinfo.rbegin(); i != m_atkinfo.rend(); ++i, ++j)
    {
        MailPackage::MailItem item;
        memset(&item, 0 , sizeof(item));
        if (j <= 10) //1-10名奖励
        {
            item.id = s_tjEventRewardId;    
            item.count = 11-j;         
            (*i).player->sendMailItem(5052, 5053, &item, 1, true);
            (*i).player->addExpOrTjScore(0, (s_tjBossScore-(j-1)*200), false, false);
            SYSMSG_BROADCASTV(5060, j, (*i).player->getCountry(), (*i).player->getName().c_str(), item.id, item.count, (s_tjBossScore-(j-1)*200));
        }

        if (j == 10)
            break;
    }

    m_atkinfo.clear();
}
void Tianjie::rewardTask()
{
    //达到等级的发奖品
    std::unordered_map<UInt64, Player*>& pm = GObject::globalPlayers.getMap();
    std::unordered_map<UInt64, Player*>::iterator iter;
    for (iter = pm.begin(); iter != pm.end(); ++iter)
    {
        Player* p = iter->second;
        if (p->GetLev() >= m_currOpenedTjLevel)
        {
            MailPackage::MailItem item;
            item.id = 8555;         //天赋术
            item.count = 3;         //
            iter->second->sendMailItem(5058, 5059, &item, 1, true);
        }
        p->SetVar(VAR_TJ_TASK1_NUMBER, 0);
        p->SetVar(VAR_TJ_TASK2_TAEL, 0);
        p->SetVar(VAR_TJ_TASK2_GOLD, 0);
        p->SetVar(VAR_TJ_TASK2_COUPON, 0);
        p->SetVar(VAR_TJ_TASK2_TJYJ, 0);
        p->SetVar(VAR_TJ_TASK2_SCORE, 0);
        p->SetVar(VAR_TJ_TASK3_COPYID, 0);
        p->SetVar(VAR_TJ_EVENT_PRESTIGE, 0);
    }
    //发奖品
    reward(m_scoreSortMap, VAR_TJ_TASK_PRESTIGE, 1);
}
void Tianjie::initSortMap()
{
    m_eventSortMap.clear();
    m_scoreSortMap.clear();

    //遍历所有玩家,将声望插入到m_eventSortMap
    std::unordered_map<UInt64, Player*>& pm = GObject::globalPlayers.getMap();
    std::unordered_map<UInt64, Player*>::iterator iter;
    for (iter = pm.begin(); iter != pm.end(); ++iter)
    {
        Player* p = iter->second;
        int score = p->GetVar(VAR_TJ_TASK_PRESTIGE);
        int eventScore = p->GetVar(VAR_TJ_EVENT_PRESTIGE);
        if (eventScore > 0)
        {
            insertToEventSortMap(p, eventScore, 0 );
            if (m_Top1Record < eventScore)
            {
                m_Top1Record = eventScore;
                m_Top1PlayerId = p->getId();
            }
        }
        if (score > 0)
        {
            insertToScoreSortMap(p, score, 0);
        }
    }
}
std::string Tianjie::GetNextSection(std::string& strString , char cSeperator)
{
    std::string strRet;
    int nIndex=(int)strString.find(cSeperator);
    if(nIndex>=0)
    {
        strRet=strString.substr(0,nIndex);//Section
        strString=strString.substr(nIndex+1);
    }
    else
    {
        strRet=strString;
        strString="";
    }

    return strRet;
}

}