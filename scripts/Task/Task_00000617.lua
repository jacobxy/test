--����Ľ�������
function Task_Accept_00000617()
	local player = GetPlayer();
	if player:GetLev() < 30 then
		return false;
	end
	local task =  player:GetTaskMgr();
	if task:HasAcceptedTask(617) or task:HasCompletedTask(617) or task:HasSubmitedTask(617) then
		return false;
	end
	return true;
end




-----�ɽ���������
function Task_Can_Accept_00000617()
	local player = GetPlayer();
	local task =  player:GetTaskMgr();
	if player:GetLev() < 30 then
		return false;
	end
	if task:HasAcceptedTask(617) or task:HasCompletedTask(617) or task:HasSubmitedTask(617) then
		return false;
	end
	return true;
end


--�����������
function Task_Submit_00000617()
	if GetPlayer():GetTaskMgr():HasCompletedTask(617) then
		return true;
	end
	return false;
end


---------------------------------------
------NPC����������ű�
---------------------------------------
function Task_00000617(npcId)
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	local action = ActionTable:Instance();

	if task:GetTaskAcceptNpc(617) == npcId and Task_Accept_00000617 () then
		action.m_ActionType = 0x0001;
		action.m_ActionID = 617
		action.m_ActionToken = 1;
		action.m_ActionStep = 01;
		action.m_ActionMsg = "邪恶道人";
	elseif task:GetTaskSubmitNpc(617) == npcId then
		if Task_Submit_00000617() then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 617
			action.m_ActionToken = 2;
			action.m_ActionStep = 10;
			action.m_ActionMsg = "邪恶道人";
		elseif task:HasAcceptedTask(617) then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 617
			action.m_ActionToken = 0;
			action.m_ActionStep = 0;
			action.m_ActionMsg = "邪恶道人";
		end
	end
	return action;
end

-------------------------------------------------
--------���񽻻�����
-------------------------------------------------
function Task_00000617_step_01()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = "桂花山有一座极阴洞，听乡民们说那里不知什么时候来了一个邪恶的道人，四处掳掠年青的少男男女回去修炼邪功，你速速去将他除掉。";
	action.m_ActionMsg = "好的，除恶扬善乃是我们正道本色。";
	return action;
end

function Task_00000617_step_10()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = GetPlayerName(GetPlayer()).."你果然是身手不凡。";
	action.m_ActionMsg = "";
	return action;
end

local Task_00000617_step_table = {
		[1] = Task_00000617_step_01,
		[10] = Task_00000617_step_10,
		};

function Task_00000617_step(step)
	if Task_00000617_step_table[step] ~= nil then
		return Task_00000617_step_table[step]();
	end
	return ActionTable:Instance();
end

--��������
function Task_00000617_accept()
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	if not Task_Accept_00000617() then
		return false;
	end
	if not task:AcceptTask(617) then
		return false;
	end
	return true;
end



--�ύ����
function Task_00000617_submit(itemId, itemNum)
	local player = GetPlayer();

	local package = player:GetPackage();

	if not player:GetTaskMgr():SubmitTask(617) then
		return false;
	end


	player:AddExp(3000);
	return true;
end

--��������
function Task_00000617_abandon()
	local package = GetPlayer():GetPackage();
	return GetPlayer():GetTaskMgr():AbandonTask(617);
end