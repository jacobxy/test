--����Ľ�������
function Task_Accept_00000565()
	if GetPlayerData(6) ~= 1 then
		return false;
	end
	local player = GetPlayer();
	if player:GetLev() < 30 then
		return false;
	end
	local task =  player:GetTaskMgr();
	if task:HasAcceptedTask(565) or task:HasCompletedTask(565) or task:HasSubmitedTask(565) then
		return false;
	end
	return true;
end




-----�ɽ���������
function Task_Can_Accept_00000565()
	local player = GetPlayer();
	local task =  player:GetTaskMgr();
	if GetPlayerData(6) ~= 1 then
		return false;
	end
	if player:GetLev() < 30 then
		return false;
	end
	if task:HasAcceptedTask(565) or task:HasCompletedTask(565) or task:HasSubmitedTask(565) then
		return false;
	end
	return true;
end


--�����������
function Task_Submit_00000565()
	if GetPlayer():GetTaskMgr():HasCompletedTask(565) then
		return true;
	end
	return false;
end


---------------------------------------
------NPC����������ű�
---------------------------------------
function Task_00000565(npcId)
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	local action = ActionTable:Instance();

	if task:GetTaskAcceptNpc(565) == npcId and Task_Accept_00000565 () then
		action.m_ActionType = 0x0001;
		action.m_ActionID = 565
		action.m_ActionToken = 1;
		action.m_ActionStep = 01;
		action.m_ActionMsg = "玉清大师";
	elseif task:GetTaskSubmitNpc(565) == npcId then
		if Task_Submit_00000565() then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 565
			action.m_ActionToken = 2;
			action.m_ActionStep = 10;
			action.m_ActionMsg = "玉清大师";
		elseif task:HasAcceptedTask(565) then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 565
			action.m_ActionToken = 0;
			action.m_ActionStep = 0;
			action.m_ActionMsg = "玉清大师";
		end
	end
	return action;
end

-------------------------------------------------
--------���񽻻�����
-------------------------------------------------
function Task_00000565_step_01()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = "玉清大师本是邪道毒龙尊者的同门，后来遇见神尼优昙点化弃暗投明，拜入神尼门下，如今正在成都郊外辟邪村隐居，你去拜访一下她，象她请教一下剑术上的疑问，对你大有好处啊。";
	action.m_ActionMsg = "弟子正有些疑问呢。";
	return action;
end

function Task_00000565_step_10()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = "有劳小友带来齐掌教的问候。";
	action.m_ActionMsg = "";
	return action;
end

local Task_00000565_step_table = {
		[1] = Task_00000565_step_01,
		[10] = Task_00000565_step_10,
		};

function Task_00000565_step(step)
	if Task_00000565_step_table[step] ~= nil then
		return Task_00000565_step_table[step]();
	end
	return ActionTable:Instance();
end

--��������
function Task_00000565_accept()
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	if not Task_Accept_00000565() then
		return false;
	end
	if not task:AcceptTask(565) then
		return false;
	end
	task:AddTaskStep(565);
	return true;
end



--�ύ����
function Task_00000565_submit(itemId, itemNum)
	local player = GetPlayer();

	local package = player:GetPackage();

	if not player:GetTaskMgr():SubmitTask(565) then
		return false;
	end


	player:AddExp(8000);
	return true;
end

--��������
function Task_00000565_abandon()
	local package = GetPlayer():GetPackage();
	return GetPlayer():GetTaskMgr():AbandonTask(565);
end