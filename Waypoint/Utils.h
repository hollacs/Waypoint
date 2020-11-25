#pragma once

#include "extdll.h"
#include "meta_api.h"

void UTIL_HudMessage(edict_t* pEntity, const hudtextparms_t& textparms, const char* pMessage);

char* UTIL_VarArgs(const char* format, ...);

bool UTIL_IsInViewCone(const Vector& origin, const Vector& angle, const Vector& point, float fov = 90.0f, bool use_2d = false);

bool UTIL_IsVisible(const Vector& start, const Vector& end, edict_t* pEntity, int flags);

bool UTIL_CheckHull(const Vector& start, const Vector& end, int flags, int hull, edict_t* pEntity);

void UTIL_BeamPoints(edict_t* pEntity, const Vector& pos1, const Vector& pos2, short sprite, int startFrame, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed);

float UTIL_DistLineSegments(const Vector& s1p0, const Vector& s1p1, const Vector& s2p0, const Vector& s2p1);