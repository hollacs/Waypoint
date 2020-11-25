#pragma once

#include <string>

#include "extdll.h"

#include "Node.h"

class Editor
{
public:
	Editor();

	void Init();
	void Reset();
	void HandleCommands(edict_t* pClient);
	void CmdStart(const edict_t* player, const struct usercmd_s* cmd);
	void Think(edict_t* pEntity);
	void Disconnect(edict_t* pEntity);

private:
	void ToggleEditMode(edict_t *pEntity, bool enabled);
	void ToggleAutoMode(bool enabled);
	void CreateNode(const Vector& origin, const char *pszFlags);
	void CreatePath(const char* pszFlags);
	void RemoveNode();
	void RemoveAllNodes();
	void RemovePath();
	bool CheckFlags(int flags);
	void AddNodeFlag(const char* pszFlag);
	void AddPathFlag(const char *pszFlag);
	void RemoveNodeFlag(const char *pszFlag);
	void RemovePathFlag(const char* pszFlag);
	void SetNodeFlag(const char* pszFlag);
	void SetPathFlag(const char* pszFlag);
	void SetAutoNode(float dist);
	void SetAutoPath(float dist);
	void SaveFile();
	int GetFlagsByName(const char* pszName);
	std::string GetNameByFlags(int flags);

	bool CheckNodes(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd, bool path);
	bool CheckNode(std::shared_ptr<Node> pNode);
	bool IsConnectable(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd, float distance);

	void AutoCreateNode(edict_t* pEntity, const Vector& origin, float min_dist, int flags, bool check_dist, bool off_node);

	std::shared_ptr<Node> GetCurrentNode() const;
	std::shared_ptr<Node> GetAimNode(std::shared_ptr<Node> pSkip) const;

	void DrawNode(std::shared_ptr<Node> pNode, bool is_current, bool is_aiming) const;
	void DrawPath(std::shared_ptr<Node> pBegin, std::shared_ptr<Node> pEnd) const;

	edict_t* m_pEditor;
	bool m_AutoMode;
	float m_AutoNodeDist, m_AutoPathDist;
	float m_EditTime, m_AutoTime;
	char m_szFilePath[256];

	bool m_IsJumping;
	bool m_IsOnGround;
	bool m_IsStillOnGround;
	bool m_IsNewNode;
	Vector m_LastOrigin;
	std::weak_ptr<Node> m_pLastNode;
	std::weak_ptr<Node> m_pBegin, m_pEnd;
};

Editor& GetEditor();