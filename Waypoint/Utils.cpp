#include <algorithm>
#include "Utils.h"
#include "meta_api.h"

bool UTIL_IsInViewCone(const Vector &origin, const Vector &angle, const Vector& point, float fov, bool use_2d)
{
	Vector vecLOS, vecForward;
	float flDot;

	MAKE_VECTORS(angle);
	vecForward = gpGlobals->v_forward;
	vecLOS = point - origin;

	if (use_2d)
	{
		vecForward.z = 0;
		vecLOS.z = 0;
	}

	vecLOS = vecLOS.Normalize();

	flDot = DotProduct(vecLOS, vecForward);

	if (flDot >= cos(fov * (M_PI / 360)))
		return true;

	return false;
}

bool UTIL_IsVisible(const Vector& start, const Vector& end, edict_t* pEntity, int flags)
{
	TraceResult tr;
	TRACE_LINE(start, end, flags, pEntity, &tr);

	if (tr.flFraction == 1.0)
		return true;

	return false;
}

Vector UTIL_GetGroundPos(const Vector& origin, int hull)
{
	TraceResult tr;
	TRACE_HULL(origin, Vector(origin.x, origin.y, origin.z - 4096), ignore_monsters, hull, NULL, &tr);

	Vector pos = tr.vecEndPos;
	return pos;
}

char* UTIL_VarArgs(const char* format, ...)
{
	va_list argptr;
	static char string[1024];

	va_start(argptr, format);
	vsprintf(string, format, argptr);
	va_end(argptr);

	return string;
}

void UTIL_BeamPoints(edict_t* pEntity, const Vector& pos1, const Vector& pos2, short sprite, int startFrame, int frameRate, int life, int width, int noise, int r, int g, int b, int brightness, int speed)
{
	MESSAGE_BEGIN(pEntity == nullptr ? MSG_BROADCAST : MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pEntity);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(pos1.x);
	WRITE_COORD(pos1.y);
	WRITE_COORD(pos1.z);
	WRITE_COORD(pos2.x);
	WRITE_COORD(pos2.y);
	WRITE_COORD(pos2.z);
	WRITE_SHORT(sprite);
	WRITE_BYTE(startFrame);		// startframe
	WRITE_BYTE(frameRate);		// framerate
	WRITE_BYTE(life);		// life
	WRITE_BYTE(width);		// width
	WRITE_BYTE(noise);		// noise
	WRITE_BYTE(r);	// r
	WRITE_BYTE(g);		// g
	WRITE_BYTE(b);		// b
	WRITE_BYTE(brightness);	// brightness
	WRITE_BYTE(speed);		// speed
	MESSAGE_END();
}

float UTIL_DistLineSegments(const Vector& s1p0, const Vector& s1p1, const Vector& s2p0, const Vector& s2p1)
{
	static const float SMALL_NUM = 0.00000001;

	Vector u = s1p1 - s1p0;
	Vector v = s2p1 - s2p0;
	Vector w = s1p0 - s2p0;

	float a = DotProduct(u, u); // always >= 0
	float b = DotProduct(u, v);
	float c = DotProduct(v, v); // always >= 0
	float d = DotProduct(u, w);
	float e = DotProduct(v, w);
	float D = a * c - b * b; // always >= 0
	float sc, sN, sD = D; // sc = sN / sD, default sD = D >= 0
	float tc, tN, tD = D; // tc = tN / tD, default tD = D >= 0

	// compute the line parameters of the two closest points
	if (D < SMALL_NUM) // the lines are almost parallel
	{
		sN = 0.0; // force using point P0 on segment S1
		sD = 1.0; // to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else // get the closest points on the infinite lines
	{
		sN = (b * e - c * d);
		tN = (a * e - b * d);
		if (sN < 0.0) // sc < 0 => the s=0 edge is visible
		{
			sN = 0.0;
			tN = e;
			tD = c;
		}
		else if (sN > sD) // sc > 1  => the s=1 edge is visible
		{
			sN = sD;
			tN = e + b;
			tD = c;
		}
	}

	if (tN < 0.0) // tc < 0 => the t=0 edge is visible
	{
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
			sN = 0.0;
		else if (-d > a)
			sN = sD;
		else
		{
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD) // tc > 1  => the t=1 edge is visible
	{
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
			sN = 0;
		else if ((-d + b) > a)
			sN = sD;
		else
		{
			sN = (-d + b);
			sD = a;
		}
	}

	// finally do the division to get sc and tc
	sc = (fabs(sN) < SMALL_NUM ? 0.0 : sN / sD);
	tc = (fabs(tN) < SMALL_NUM ? 0.0 : tN / tD);

	Vector dP = w + (sc * u) - (tc * v);  // =  S1(sc) - S2(tc)
	return dP.Length();
}