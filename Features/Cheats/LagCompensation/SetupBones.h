#pragma once
#include "..\..\includes.hpp"

class CIKContext
{
public:
	void Construct()
	{
		using IKConstruct = void(__thiscall*)(void*);
		static auto ik_ctor = (IKConstruct)util::FindSignature(CLIENT_DLL, crypt_str("53 8B D9 F6 C3 03 74 0B FF 15 ?? ?? ?? ?? 84 C0 74 01 CC C7 83 ?? ?? ?? ?? ?? ?? ?? ?? 8B CB"));

		ik_ctor(this);
	}

	void Destructor()
	{
		using IKDestructor = void(__thiscall*)(CIKContext*);
		static auto ik_dector = (IKDestructor)util::FindSignature(CLIENT_DLL, crypt_str("56 8B F1 57 8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 BE ?? ?? ?? ?? ??"));

		ik_dector(this);
	}

	void ClearTargets()
	{
		auto i = 0;
		auto count = *reinterpret_cast <int*> ((uintptr_t)this + 0xFF0);

		if (count > 0)
		{
			auto target = reinterpret_cast <int*> ((uintptr_t)this + 0xD0);

			do
			{
				*target = -9999;
				target += 85;
				++i;
			} while (i < count);
		}
	}

	void Init(CStudioHdr* hdr, Vector* angles, Vector* origin, float currentTime, int frames, int boneMask)
	{
		using Init_t = void(__thiscall*)(void*, CStudioHdr*, QAngle*, Vector*, float, int, int);

		static auto ik_init = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D 8F"));
		((Init_t)ik_init)(this, hdr, (QAngle*)angles, origin, currentTime, frames, boneMask);
	}

	void UpdateTargets(Vector* pos, Quaternion* qua, matrix3x4_t* matrix, uint8_t* boneComputed)
	{
		using UpdateTargets_t = void(__thiscall*)(void*, Vector*, Quaternion*, matrix3x4_t*, uint8_t*);

		static auto ik_update_targets = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F0 81 EC ?? ?? ?? ?? 33 D2"));
		((UpdateTargets_t)ik_update_targets)(this, pos, qua, matrix, boneComputed);
	}

	void SolveDependencies(Vector* pos, Quaternion* qua, matrix3x4_t* matrix, uint8_t* boneComputed)
	{
		using SolveDependencies_t = void(__thiscall*)(void*, Vector*, Quaternion*, matrix3x4_t*, uint8_t*);

		static auto ik_solve_dependencies = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F0 81 EC ?? ?? ?? ?? 8B 81"));
		((SolveDependencies_t)ik_solve_dependencies)(this, pos, qua, matrix, boneComputed);
	}
};

struct CBoneSetup
{
	CStudioHdr* m_pStudioHdr;
	int m_boneMask;
	float* m_flPoseParameter;
	void* m_pPoseDebugger;

	void InitPose(Vector pos[], Quaternion q[], CStudioHdr* hdr)
	{
		static auto init_pose = util::FindSignature("client.dll", "55 8B EC 83 EC 10 53 8B D9 89 55 F8 56 57 89 5D F4 8B 0B 89 4D F0");

		__asm
		{
			mov eax, this
			mov esi, q
			mov edx, pos
			push dword ptr[hdr + 4]
			mov ecx, [eax]
			push esi
			call init_pose
			add esp, 8
		}
	}

	void AccumulatePose(Vector pos[], Quaternion q[], int sequence, float cycle, float flWeight, float flTime, CIKContext* pIKContext)
	{
		using AccumulatePoseFn = void(__thiscall*)(CBoneSetup*, Vector* a2, Quaternion* a3, int a4, float a5, float a6, float a7, CIKContext* a8);
		static auto accumulate_pose = (AccumulatePoseFn)util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? A1"));

		return accumulate_pose(this, pos, q, sequence, cycle, flWeight, flTime, pIKContext);
	}

	void CalcAutoplaySequences(Vector pos[], Quaternion q[], float flRealTime, CIKContext* pIKContext)
	{
		static auto calc_autoplay_sequences = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 EC 10 53 56 57 8B 7D 10 8B D9 F3 0F 11 5D ??"));

		__asm
		{
			movss   xmm3, flRealTime
			mov eax, pIKContext
			mov ecx, this
			push eax
			push q
			push pos
			call calc_autoplay_sequences
		}
	}

	void CalcBoneAdj(Vector pos[], Quaternion q[], float* controllers, int boneMask)
	{
		static auto calc_bone_adj = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F8 81 EC ?? ?? ?? ?? 8B C1 89 54 24 04 89 44 24 2C 56 57 8B 00"));

		__asm
		{
			mov     eax, controllers
			mov     ecx, this
			mov     edx, pos; a2
			push    dword ptr[ecx + 4]; a5
			mov     ecx, [ecx]; a1
			push    eax; a4
			push    q; a3
			call    calc_bone_adj
			add     esp, 0xC
		}
	}
};

struct mstudioposeparamdesc_t
{
	int sznameindex;

	inline char* const pszName(void) const
	{
		return ((char*)this) + sznameindex;
	}

	int flags;
	float start;
	float end;
	float loop;
};

class CSetupBones
{
public:
	void setup();
	void get_skeleton();
	void studio_build_matrices(CStudioHdr* hdr, const matrix3x4_t& worldTransform, Vector* pos, Quaternion* q, int boneMask, matrix3x4_t* out, uint32_t* boneComputed);
	void attachment_helper();
	void fix_bones_rotations();

	matrix3x4_t* m_boneMatrix = nullptr;
	Vector m_vecOrigin = ZERO;
	Vector m_angAngles = ZERO;
	CStudioHdr* m_pHdr = nullptr;
	Vector* m_vecBones = nullptr;
	Quaternion* m_quatBones = nullptr;
	bool m_bShouldDoIK = false;
	bool m_bShouldAttachment = true;
	bool m_bShouldDispatch = true;
	int m_boneMask = 0;
	float m_flPoseParameters[24];
	float m_flWorldPoses[24];
	int m_nAnimOverlayCount = 0;
	AnimationLayer* m_animLayers = nullptr;
	float m_flCurtime = 0.0f;
	player_t* m_animating = nullptr;
};