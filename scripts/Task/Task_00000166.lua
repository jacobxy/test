--����Ľ�������
function Task_Accept_00000166()
	local player = GetPlayer();
	if player:GetLev() < 80 then
		return false;
	end
	local task =  player:GetTaskMgr();
	if task:HasAcceptedTask(166) or task:HasCompletedTask(166) or task:HasSubmitedTask(166) then
		return false;
	end
	local state = GetPlayerData(6);
	if state == 0 then
		if not task:HasSubmitedTask(165) then
			return false;
		end
	end
	if state == 1 then
		if not task:HasSubmitedTask(165) then
			return false;
		end
	end
	if state == 2 then
		if not task:HasSubmitedTask(165) then
			return false;
		end
	end
	return true;
end




-----�ɽ���������
function Task_Can_Accept_00000166()
	local player = GetPlayer();
	local task =  player:GetTaskMgr();
	if player:GetLev() < 80 then
		return false;
	end
	if task:HasAcceptedTask(166) or task:HasCompletedTask(166) or task:HasSubmitedTask(166) then
		return false;
	end
	local state = GetPlayerData(6);
	if state == 0 then
		if not task:HasSubmitedTask(165) then
			return false;
		end
	end
	if state == 1 then
		if not task:HasSubmitedTask(165) then
			return false;
		end
	end
	if state == 2 then
		if not task:HasSubmitedTask(165) then
			return false;
		end
	end
	return true;
end


--�����������
function Task_Submit_00000166()
	if GetPlayer():GetTaskMgr():HasCompletedTask(166) then
		return true;
	end
	return false;
end


---------------------------------------
------NPC����������ű�
---------------------------------------
function Task_00000166(npcId)
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	local action = ActionTable:Instance();

	if task:GetTaskAcceptNpc(166) == npcId and Task_Accept_00000166 () then
		action.m_ActionType = 0x0001;
		action.m_ActionID = 166
		action.m_ActionToken = 1;
		action.m_ActionStep = 01;
		action.m_ActionMsg = "玄冥界";
	elseif task:GetTaskSubmitNpc(166) == npcId then
		if Task_Submit_00000166() then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 166
			action.m_ActionToken = 2;
			action.m_ActionStep = 10;
			action.m_ActionMsg = "玄冥界";
		elseif task:HasAcceptedTask(166) then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 166
			action.m_ActionToken = 0;
			action.m_ActionStep = 0;
			action.m_ActionMsg = "玄冥界";
		end
	end
	return action;
end

-------------------------------------------------
--------���񽻻�����
-------------------------------------------------
function Task_00000166_step_01()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = "这元磁真力玄妙异常，不过因为地处北极，元磁真力结合阴寒之力竟然沟通到玄冥异界，引得不少玄冥异民降临，此类异界生灵是我等之大敌，只是我首菁英骚扰在先，还无法完全化为人形，希望小友你可以出手除掉此害。";
	action.m_ActionMsg = "是吗，我这就去除掉玄冥异民。";
	return action;
end

function Task_00000166_step_10()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = "老朽修炼多年，终于可以化身成人了。";
	action.m_ActionMsg = "";
	return action;
end

local Task_00000166_step_table = {
		[1] = Task_00000166_step_01,
		[10] = Task_00000166_step_10,
		};

function Task_00000166_step(step)
	if Task_00000166_step_table[step] ~= nil then
		return Task_00000166_step_table[step]();
	end
	return ActionTable:Instance();
end

--��������
function Task_00000166_accept()
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	if not Task_Accept_00000166() then
		return false;
	end
	if not task:AcceptTask(166) then
		return false;
	end
	return true;
end



--�ύ����
function Task_00000166_submit(itemId, itemNum)
	local player = GetPlayer();

	local package = player:GetPackage();

	if not player:GetTaskMgr():SubmitTask(166) then
		return false;
	end


	player:AddExp(150000);
	return true;
end

--��������
function Task_00000166_abandon()
	local package = GetPlayer():GetPackage();
	return GetPlayer():GetTaskMgr():AbandonTask(166);
end