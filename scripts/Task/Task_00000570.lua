--����Ľ�������
function Task_Accept_00000570()
	if GetPlayerData(6) ~= 1 then
		return false;
	end
	local player = GetPlayer();
	if player:GetLev() < 30 then
		return false;
	end
	local task =  player:GetTaskMgr();
	if task:HasAcceptedTask(570) or task:HasCompletedTask(570) or task:HasSubmitedTask(570) then
		return false;
	end
	return true;
end




-----�ɽ���������
function Task_Can_Accept_00000570()
	local player = GetPlayer();
	local task =  player:GetTaskMgr();
	if GetPlayerData(6) ~= 1 then
		return false;
	end
	if player:GetLev() < 30 then
		return false;
	end
	if task:HasAcceptedTask(570) or task:HasCompletedTask(570) or task:HasSubmitedTask(570) then
		return false;
	end
	return true;
end


--�����������
function Task_Submit_00000570()
	if GetPlayer():GetTaskMgr():HasCompletedTask(570) then
		return true;
	end
	return false;
end


---------------------------------------
------NPC����������ű�
---------------------------------------
function Task_00000570(npcId)
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	local action = ActionTable:Instance();

	if task:GetTaskAcceptNpc(570) == npcId and Task_Accept_00000570 () then
		action.m_ActionType = 0x0001;
		action.m_ActionID = 570
		action.m_ActionToken = 1;
		action.m_ActionStep = 01;
		action.m_ActionMsg = "寒泉恶人";
	elseif task:GetTaskSubmitNpc(570) == npcId then
		if Task_Submit_00000570() then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 570
			action.m_ActionToken = 2;
			action.m_ActionStep = 10;
			action.m_ActionMsg = "寒泉恶人";
		elseif task:HasAcceptedTask(570) then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 570
			action.m_ActionToken = 0;
			action.m_ActionStep = 0;
			action.m_ActionMsg = "寒泉恶人";
		end
	end
	return action;
end

-------------------------------------------------
--------���񽻻�����
-------------------------------------------------
function Task_00000570_step_01()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = "成都不远处的桂花山是一处美景，不过山中有五色云雾，寻常人轻易不得入内，前不久飞龙师太的徒弟金驼在桂花山杀死了我派弟子数人，这种邪恶之徒人人得而诛之，你去跑一趟吧。";
	action.m_ActionMsg = "弟子遵命，我去去就回。";
	return action;
end

function Task_00000570_step_10()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = GetPlayerName(GetPlayer()).."你真是身手不凡啊。 ";
	action.m_ActionMsg = "";
	return action;
end

local Task_00000570_step_table = {
		[1] = Task_00000570_step_01,
		[10] = Task_00000570_step_10,
		};

function Task_00000570_step(step)
	if Task_00000570_step_table[step] ~= nil then
		return Task_00000570_step_table[step]();
	end
	return ActionTable:Instance();
end

--��������
function Task_00000570_accept()
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	if not Task_Accept_00000570() then
		return false;
	end
	if not task:AcceptTask(570) then
		return false;
	end
	return true;
end



--�ύ����
function Task_00000570_submit(itemId, itemNum)
	local player = GetPlayer();

	local package = player:GetPackage();

	if not player:GetTaskMgr():SubmitTask(570) then
		return false;
	end


	return true;
end

--��������
function Task_00000570_abandon()
	local package = GetPlayer():GetPackage();
	return GetPlayer():GetTaskMgr():AbandonTask(570);
end