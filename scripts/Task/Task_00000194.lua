--����Ľ�������
function Task_Accept_00000194()
	local player = GetPlayer();
	if player:GetLev() < 95 then
		return false;
	end
	local task =  player:GetTaskMgr();
	if task:HasAcceptedTask(194) or task:HasCompletedTask(194) or task:HasSubmitedTask(194) then
		return false;
	end
	local state = GetPlayerData(6);
	if state == 0 then
		if not task:HasSubmitedTask(193) then
			return false;
		end
	end
	if state == 1 then
		if not task:HasSubmitedTask(193) then
			return false;
		end
	end
	if state == 2 then
		if not task:HasSubmitedTask(193) then
			return false;
		end
	end
	return true;
end




-----�ɽ���������
function Task_Can_Accept_00000194()
	local player = GetPlayer();
	local task =  player:GetTaskMgr();
	if player:GetLev() < 95 then
		return false;
	end
	if task:HasAcceptedTask(194) or task:HasCompletedTask(194) or task:HasSubmitedTask(194) then
		return false;
	end
	local state = GetPlayerData(6);
	if state == 0 then
		if not task:HasSubmitedTask(193) then
			return false;
		end
	end
	if state == 1 then
		if not task:HasSubmitedTask(193) then
			return false;
		end
	end
	if state == 2 then
		if not task:HasSubmitedTask(193) then
			return false;
		end
	end
	return true;
end


--�����������
function Task_Submit_00000194()
	if GetPlayer():GetTaskMgr():HasCompletedTask(194) then
		return true;
	end
	return false;
end


---------------------------------------
------NPC����������ű�
---------------------------------------
function Task_00000194(npcId)
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	local action = ActionTable:Instance();

	if task:GetTaskAcceptNpc(194) == npcId and Task_Accept_00000194 () then
		action.m_ActionType = 0x0001;
		action.m_ActionID = 194
		action.m_ActionToken = 1;
		action.m_ActionStep = 01;
		action.m_ActionMsg = "红鬼谷";
	elseif task:GetTaskSubmitNpc(194) == npcId then
		if Task_Submit_00000194() then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 194
			action.m_ActionToken = 2;
			action.m_ActionStep = 10;
			action.m_ActionMsg = "红鬼谷";
		elseif task:HasAcceptedTask(194) then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 194
			action.m_ActionToken = 0;
			action.m_ActionStep = 0;
			action.m_ActionMsg = "红鬼谷";
		end
	end
	return action;
end

-------------------------------------------------
--------���񽻻�����
-------------------------------------------------
function Task_00000194_step_01()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = "贫尼在此隐居已久，也于那红鬼谷的毒龙尊者井水不犯河水，不过最近毒龙外出，他手下不少弟子在喜马拉雅山为所欲为很是不堪，少侠你出去巡视一下，铲除那些为非作歹的毒龙弟子。";
	action.m_ActionMsg = "我这就去周遭巡逻一下。";
	return action;
end

function Task_00000194_step_10()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = "少侠真是修为深厚，天赋异禀啊。";
	action.m_ActionMsg = "";
	return action;
end

local Task_00000194_step_table = {
		[1] = Task_00000194_step_01,
		[10] = Task_00000194_step_10,
		};

function Task_00000194_step(step)
	if Task_00000194_step_table[step] ~= nil then
		return Task_00000194_step_table[step]();
	end
	return ActionTable:Instance();
end

--��������
function Task_00000194_accept()
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	if not Task_Accept_00000194() then
		return false;
	end
	if not task:AcceptTask(194) then
		return false;
	end
	return true;
end



--�ύ����
function Task_00000194_submit(itemId, itemNum)
	local player = GetPlayer();

	local package = player:GetPackage();

	if not player:GetTaskMgr():SubmitTask(194) then
		return false;
	end


	player:AddExp(180000);
	return true;
end

--��������
function Task_00000194_abandon()
	local package = GetPlayer():GetPackage();
	return GetPlayer():GetTaskMgr():AbandonTask(194);
end