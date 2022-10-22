// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "SetupBones.h"

uintptr_t& GetBoneMerge(player_t* player)
{
	static auto bone_merge = util::FindSignature(CLIENT_DLL, crypt_str("89 86 ?? ?? ?? ?? E8 ?? ?? ?? ?? FF 75 08"));
	return *(uintptr_t*)((uintptr_t)player + *(uintptr_t*)(bone_merge + 0x2));
}

mstudioposeparamdesc_t* pPoseParameter(CStudioHdr* hdr, int index)
{
	using poseParametorFN = mstudioposeparamdesc_t * (__thiscall*)(CStudioHdr*, int);
	static poseParametorFN pose_parameter = (poseParametorFN)util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 8B 45 08 57 8B F9 8B 4F 04 85 C9 75 15 8B"));

	return pose_parameter(hdr, index);
}

__forceinline uintptr_t rel32_fix(uintptr_t ptr)
{
	auto offset = *(uintptr_t*)(ptr + 0x1);
	return (uintptr_t)(ptr + offset + 0x5);
}

void UpdateCache(uintptr_t bonemerge)
{
	static auto bone_merge_update_cache = (void(__thiscall*)(uintptr_t))rel32_fix(util::FindSignature(CLIENT_DLL, crypt_str("E8 ?? ?? ?? ?? 83 7E 10 00 74 64")));
	bone_merge_update_cache(bonemerge);
}

float GetPoseParamValue(CStudioHdr* hdr, int index, float flValue)
{
	if (index < 0 || index > 24)
		return 0.0f;

	auto pose_param = pPoseParameter(hdr, index);

	if (!pose_param)
		return 0.0f;

	auto PoseParam = *pose_param;

	if (PoseParam.loop)
	{
		auto wrap = (PoseParam.start + PoseParam.end) / 2.0f + PoseParam.loop / 2.0f;
		auto shift = PoseParam.loop - wrap;

		flValue = flValue - PoseParam.loop * floor((flValue + shift) / PoseParam.loop);
	}

	return (flValue - PoseParam.start) / (PoseParam.end - PoseParam.start);
}

void MergeMatchingPoseParams(uintptr_t bonemerge, float* poses, float* target_poses)
{
	UpdateCache(bonemerge);

	if (*(DWORD*)(bonemerge + 0x10) && *(DWORD*)(bonemerge + 0x8C))
	{
		auto index = (int*)(bonemerge + 0x20);
		auto counter = 0;

		do
		{
			if (*index != -1)
			{
				auto target = *(player_t**)(bonemerge + 0x4);
				auto hdr = target->m_pStudioHdr();
				auto pose_param_value = 0.0f;

				if (hdr && *(studiohdr_t**)hdr && counter >= 0)
				{
					auto pose = target_poses[counter];
					auto pose_param = pPoseParameter(hdr, counter);

					pose_param_value = pose * (pose_param->end - pose_param->start) + pose_param->start;
				}

				auto target2 = *(player_t**)(bonemerge);
				auto hdr2 = target2->m_pStudioHdr();

				poses[*index] = GetPoseParamValue(hdr2, *index, pose_param_value);
			}

			++counter;
			++index;
		} while (counter < 24);
	}
}

void CSetupBones::setup()
{
	static auto ik = util::FindSignature(CLIENT_DLL, crypt_str("8B 8F ?? ?? ?? ?? 89 4C 24 1C"));
	static auto m_pIk = *(CIKContext**)((uintptr_t)m_animating + 0x99C);

	if (!m_bShouldDoIK)
		m_pIk = nullptr;

	m_pHdr = m_animating->m_pStudioHdr();

	uint32_t bone_computed[8];
	memset(bone_computed, 0, 8 * sizeof(uint32_t));

	auto sequences_available = !*(int*)(*(uintptr_t*)m_pHdr + 0x150) || *(int*)((uintptr_t)m_pHdr + 0x4);

	if (m_pIk)
	{
		m_pIk->Init(m_pHdr, &m_angAngles, &m_vecOrigin, m_flCurtime, TIME_TO_TICKS(m_flCurtime), m_boneMask);

		if (sequences_available)
			get_skeleton();

		m_pIk->UpdateTargets(m_vecBones, m_quatBones, m_boneMatrix, (uint8_t*)bone_computed);

		using oCalculateIKLocks = void(__thiscall*)(void*, float);
		call_virtual <oCalculateIKLocks>(m_animating, 192 + 1)(m_animating, m_flCurtime);

		m_pIk->SolveDependencies(m_vecBones, m_quatBones, m_boneMatrix, (uint8_t*)bone_computed);
	}
	else if (sequences_available)
		get_skeleton();

	matrix3x4_t transform;
	transform.AngleMatrix(m_angAngles, m_vecOrigin);

	studio_build_matrices(m_pHdr, transform, m_vecBones, m_quatBones, m_boneMask, m_boneMatrix, bone_computed);

	if (m_boneMask & BONE_USED_BY_ATTACHMENT && m_bShouldAttachment)
		attachment_helper();

	m_animating->m_flLastBoneSetupTime() = m_flCurtime;

	m_animating->m_BoneAccessor().m_ReadableBones |= m_boneMask;
	m_animating->m_BoneAccessor().m_WritableBones |= m_boneMask;

	static auto invalidate_bone_cache = util::FindSignature(CLIENT_DLL, crypt_str("80 3D ?? ?? ?? ?? ?? 74 16 A1 ?? ?? ?? ?? 48 C7 81"));
	static auto model_bone_counter = *(uintptr_t*)(invalidate_bone_cache + 0xA);

	m_animating->m_iMostRecentModelBoneCounter() = *(uint32_t*)model_bone_counter;
}

bool CanBeAnimated(player_t* player)
{
	static auto custom_player = *(int*)(util::FindSignature(CLIENT_DLL, crypt_str("80 BF ?? ?? ?? ?? ?? 0F 84 ?? ?? ?? ?? 83 BF ?? ?? ?? ?? ?? 74 7A")) + 2);

	if (!*(bool*)((uintptr_t)player + custom_player) || !player->get_animation_state())
		return false;

	auto weapon = player->m_hActiveWeapon().Get();

	if (!weapon)
		return false;

	auto world_model = (player_t*)weapon->m_hWeaponWorldModel().Get();

	if (!world_model || *(short*)((uintptr_t)world_model + 0x26E) == -1)
		return player == g_ctx.local();

	return true;
}

void CSetupBones::get_skeleton()
{
	alignas(16) Vector position[256];
	alignas(16) Quaternion rotation[256];

	static auto ik = util::FindSignature(CLIENT_DLL, crypt_str("8B 8F ?? ?? ?? ?? 89 4C 24 1C"));
	static auto m_pIk = *(CIKContext**)((uintptr_t)m_animating + 0x99C);

	if (!m_bShouldDoIK)
		m_pIk = nullptr;

	alignas(16) char buffer[32];
	alignas(16) auto bone_setup = (CBoneSetup*)&buffer;

	bone_setup->m_pStudioHdr = m_pHdr;
	bone_setup->m_boneMask = m_boneMask;
	bone_setup->m_flPoseParameter = m_flPoseParameters;
	bone_setup->m_pPoseDebugger = nullptr;

	bone_setup->InitPose(m_vecBones, m_quatBones, m_pHdr);
	bone_setup->AccumulatePose(m_vecBones, m_quatBones, *(int*)((uintptr_t)m_animating + 0x28BC), *(float*)((uintptr_t)m_animating + 0xA14), 1.0f, m_flCurtime, m_pIk);

	int layer[15] =
	{
		m_nAnimOverlayCount
	};

	for (auto i = 0; i < m_nAnimOverlayCount; ++i)
	{
		auto& final_layer = m_animLayers[i];

		if (final_layer.m_flWeight > 0.0f && final_layer.m_nOrder != 15 && final_layer.m_nOrder >= 0 && final_layer.m_nOrder < m_nAnimOverlayCount)
			layer[final_layer.m_nOrder] = i;
	}

	static auto copy_to_follow = rel32_fix(util::FindSignature(CLIENT_DLL, crypt_str("E8 ?? ?? ?? ?? 8B 87 ?? ?? ?? ?? 8D 8C 24 ?? ?? ?? ?? 8B 7C 24 18")));
	static auto bone_merge_copy_to_follow = (void(__thiscall*)(uintptr_t, Vector*, Quaternion*, int, Vector*, Quaternion*))copy_to_follow;

	static auto copy_from_follow = rel32_fix(util::FindSignature(CLIENT_DLL, crypt_str("E8 ?? ?? ?? ?? F3 0F 10 45 ?? 8D 84 24 ?? ?? ?? ??")));
	static auto bone_merge_copy_from_follow = (void(__thiscall*)(uintptr_t, Vector*, Quaternion*, int, Vector*, Quaternion*))copy_from_follow;

	char tmp_buffer[4208];
	auto world_ik = (CIKContext*)tmp_buffer;

	auto weapon = m_animating->m_hActiveWeapon().Get();

	if (CanBeAnimated(m_animating) && weapon)
	{
		auto weaponWorldModel = (player_t*)weapon->m_hWeaponWorldModel().Get();

		if (weaponWorldModel)
		{
			auto bone_merge = GetBoneMerge(weaponWorldModel);

			if (bone_merge)
			{
				MergeMatchingPoseParams(bone_merge, m_flWorldPoses, m_flPoseParameters);
				auto world_hdr = weaponWorldModel->m_pStudioHdr();

				world_ik->Construct();
				world_ik->Init(world_hdr, &m_angAngles, &m_vecOrigin, m_flCurtime, 0, BONE_USED_BY_BONE_MERGE);

				alignas(16) char buffer2[32];
				alignas(16) auto world_setup = (CBoneSetup*)&buffer2;

				world_setup->m_pStudioHdr = world_hdr;
				world_setup->m_boneMask = BONE_USED_BY_BONE_MERGE;
				world_setup->m_flPoseParameter = m_flWorldPoses;
				world_setup->m_pPoseDebugger = nullptr;

				world_setup->InitPose(position, rotation, world_hdr);

				for (auto i = 0; i < m_nAnimOverlayCount; ++i)
				{
					auto layer = &m_animLayers[i];

					if (layer && layer->m_nSequence > 1 && layer->m_flWeight > 0.0f)
					{
						if (m_bShouldDispatch && m_animating == g_ctx.local())
						{
							using UpdateDispatchLayer = void(__thiscall*)(void*, AnimationLayer*, CStudioHdr*, int);
							call_virtual <UpdateDispatchLayer>(m_animating, 246 + 1)(m_animating, layer, world_hdr, layer->m_nSequence);
						}

						if (!m_bShouldDispatch || layer->m_nDispatchSequence_2 <= 0 || layer->m_nDispatchSequence_2 >= (*(studiohdr_t**)world_hdr)->numlocalseq)
							bone_setup->AccumulatePose(m_vecBones, m_quatBones, layer->m_nSequence, layer->m_flCycle, layer->m_flWeight, m_flCurtime, m_pIk);
						else if (m_bShouldDispatch)
						{
							bone_merge_copy_from_follow(bone_merge, m_vecBones, m_quatBones, BONE_USED_BY_BONE_MERGE, position, rotation);

							using oIKAddDependencies = void(__thiscall*)(CIKContext*, float, int, int, float, float);
							static auto add_dependencies = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 81 EC BC 00 00 00 53 56 57"));

							if (m_pIk)
								(oIKAddDependencies(add_dependencies) (m_pIk, *(float*)((uintptr_t)m_animating + 0xA14), layer->m_nSequence, layer->m_flCycle, bone_setup->m_flPoseParameter[2], layer->m_flWeight));

							world_setup->AccumulatePose(position, rotation, layer->m_nDispatchSequence_2, layer->m_flCycle, layer->m_flWeight, m_flCurtime, world_ik);
							bone_merge_copy_to_follow(bone_merge, position, rotation, BONE_USED_BY_BONE_MERGE, m_vecBones, m_quatBones);
						}
					}
				}

				world_ik->Destructor();
			}
		}
	}
	else
	{
		for (auto i = 0; i < m_nAnimOverlayCount; ++i)
		{
			auto layer_count = layer[i];

			if (layer_count >= 0 && layer_count < m_nAnimOverlayCount)
			{
				auto final_layer = &m_animLayers[i];
				bone_setup->AccumulatePose(position, rotation, final_layer->m_nSequence, final_layer->m_flCycle, final_layer->m_flWeight, m_flCurtime, m_pIk);
			}
		}
	}

	if (m_pIk)
	{
		world_ik->Construct();
		world_ik->Init(m_pHdr, &m_angAngles, &m_vecOrigin, m_flCurtime, 0, m_boneMask);
		bone_setup->CalcAutoplaySequences(m_vecBones, m_quatBones, m_flCurtime, world_ik);
		world_ik->Destructor();
	}
	else
		bone_setup->CalcAutoplaySequences(m_vecBones, m_quatBones, m_flCurtime, nullptr);

	bone_setup->CalcBoneAdj(m_vecBones, m_quatBones, (float*)((uintptr_t)m_animating + 0xA54), m_boneMask);
}

void CSetupBones::studio_build_matrices(CStudioHdr* hdr, const matrix3x4_t& rotationmatrix, Vector* pos, Quaternion* q, int boneMask, matrix3x4_t* bonetoworld, uint32_t* boneComputed)
{
	auto i = 0;
	auto chain_length = 0;
	auto bone = -1;
	auto studio_hdr = *(studiohdr_t**)hdr;

	if (bone < -1 || bone >= studio_hdr->numbones)
		bone = 0;

	auto bone_parent = (CUtlVector <int>*) ((uintptr_t)hdr + 0x44);
	auto bone_flags = (CUtlVector <int>*) ((uintptr_t)hdr + 0x30);

	int chain[MAXSTUDIOBONES];

	if (bone <= -1)
	{
		chain_length = studio_hdr->numbones;

		for (i = 0; i < studio_hdr->numbones; ++i)
			chain[chain_length - i - 1] = i;
	}
	else
	{
		i = bone;

		do
		{
			chain[chain_length++] = i;
			i = bone_parent->m_Memory.Element(i);
		} while (i != -1);
	}

	matrix3x4_t bone_matrix;

	for (auto j = chain_length - 1; j >= 0; --j)
	{
		i = chain[j];

		if ((1 << (i & 0x1F)) & boneComputed[i >> 5])
			continue;

		auto flag = bone_flags->m_Memory.Element(i);
		auto parent = bone_parent->m_Memory.Element(i);

		if (flag & boneMask && q)
		{
			bone_matrix.QuaternionMatrix(q[i], pos[i]);

			if (parent == -1)
				bonetoworld[i] = rotationmatrix.ConcatTransforms(bone_matrix);
			else
				bonetoworld[i] = bonetoworld[parent].ConcatTransforms(bone_matrix);
		}
	}
}

void CSetupBones::attachment_helper()
{
	using AttachmentHelperFn = void(__thiscall*)(player_t*, CStudioHdr*);
	static auto m_AttachmentHelper = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4"));

	static auto attachment = (AttachmentHelperFn)m_AttachmentHelper;
	attachment(m_animating, m_pHdr);
}

void CSetupBones::fix_bones_rotations()
{
	auto studio_hdr = m_animating->m_pStudioHdr();

	if (studio_hdr)
	{
		auto hdr = *(studiohdr_t**)studio_hdr;

		if (hdr)
		{
			auto hitbox_set = hdr->pHitboxSet(m_animating->m_nHitboxSet());

			for (auto i = 0; i < hitbox_set->numhitboxes; ++i)
			{
				auto hitbox = hitbox_set->pHitbox(i);

				if (hitbox->rotation.IsZero())
					continue;

				matrix3x4_t hitbox_transform;
				hitbox_transform.AngleMatrix(hitbox->rotation);

				m_boneMatrix[hitbox->bone] = m_boneMatrix[hitbox->bone].ConcatTransforms(hitbox_transform);
			}
		}
	}
}