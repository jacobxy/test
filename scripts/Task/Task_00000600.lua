--任务的接受条件
function Task_Accept_00000600()
	local player = GetPlayer();
	if player:GetLev() < 30 then
		return false;
	end
	local task =  player:GetTaskMgr();
	if task:HasAcceptedTask(600) or task:HasCompletedTask(600) or task:HasSubmitedTask(600) then
		return false;
	end
	return true;
end




-----可接任务条件
function Task_Can_Accept_00000600()
	local player = GetPlayer();
	local task =  player:GetTaskMgr();
	if player:GetLev() < 30 then
		return false;
	end
	if task:HasAcceptedTask(600) or task:HasCompletedTask(600) or task:HasSubmitedTask(600) then
		return false;
	end
	return true;
end


--任务完成条件
function Task_Submit_00000600()
	if GetPlayer():GetTaskMgr():HasCompletedTask(600) then
		return true;
	end
	return false;
end


---------------------------------------
------NPC交互的任务脚本
---------------------------------------
function Task_00000600(npcId)
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	local action = ActionTable:Instance();

	if task:GetTaskAcceptNpc(600) == npcId and Task_Accept_00000600 () then
		action.m_ActionType = 0x0001;
		action.m_ActionID = 600
		action.m_ActionToken = 1;
		action.m_ActionStep = 01;
		action.m_ActionMsg = task_msg_1012;
	elseif task:GetTaskSubmitNpc(600) == npcId then
		if Task_Submit_00000600() then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 600
			action.m_ActionToken = 2;
			action.m_ActionStep = 10;
			action.m_ActionMsg = task_msg_1013;
		elseif task:HasAcceptedTask(600) then
			action.m_ActionType = 0x0001;
			action.m_ActionID = 600
			action.m_ActionToken = 0;
			action.m_ActionStep = 0;
			action.m_ActionMsg = task_msg_1014;
		end
	end
	return action;
end

-------------------------------------------------
--------任务交互步骤
-------------------------------------------------
function Task_00000600_step_01()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = task_msg_1015..GetPlayerName(GetPlayer())..task_msg_1016;
	action.m_ActionMsg = task_msg_1017;
	return action;
end

function Task_00000600_step_10()
	local action = ActionTable:Instance();
	action.m_ActionType = 0x0001;
	action.m_ActionToken = 3;
	action.m_ActionStep = 0;
	action.m_NpcMsg = GetPlayerName(GetPlayer())..task_msg_1018;
	action.m_ActionMsg = "";
	return action;
end

local Task_00000600_step_table = {
		[1] = Task_00000600_step_01,
		[10] = Task_00000600_step_10,
		};

function Task_00000600_step(step)
	if Task_00000600_step_table[step] ~= nil then
		return Task_00000600_step_table[step]();
	end
	return ActionTable:Instance();
end

--接受任务
function Task_00000600_accept()
	local player = GetPlayer();
	local task = player:GetTaskMgr();
	if not Task_Accept_00000600() then
		return false;
	end
	if not task:AcceptTask(600) then
		return false;
	end
	return true;
end



--提交任务
function Task_00000600_submit(itemId, itemNum)
	local player = GetPlayer();

	local package = player:GetPackage();

	if not player:GetTaskMgr():SubmitTask(600) then
		return false;
	end


	player:AddExp(3000);
	return true;
end

--放弃任务
function Task_00000600_abandon()
	local package = GetPlayer():GetPackage();
	return GetPlayer():GetTaskMgr():AbandonTask(600);
end
