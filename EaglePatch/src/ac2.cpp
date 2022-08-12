#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <Xinput.h>

#include "patcher.h"

#include "../shared/ini_reader.h"

//#define INCLUDE_CONSOLE // add ability to allocate console window

#ifdef INCLUDE_CONSOLE
#include "../shared/console.h"
#endif

enum eExeVersion
{
	RETAIL_1_01 = 0,
	DIGITAL_UPLAY
};

// these were made static because otherwise it crashes :shrug:
struct sAddresses
{
	static uintptr_t Pad_UpdateTimeStamps;
	static uintptr_t Pad_ScaleStickValues;
	static uintptr_t PadXenon_ctor;
	static uintptr_t PadProxyPC_AddPad;
	static uintptr_t _addXenonJoy_Patch;
	static uintptr_t _addXenonJoy_JumpOut;
	static uintptr_t _PadProxyPC_Patch;
	static uint32_t* _descriptor_var;
	static void** _delete_class;
	static uintptr_t _ps3_controls[4];
	static uintptr_t _ps3_controls_analog[4];
	static uintptr_t _skipIntroVideos;
	static uintptr_t _shadowMapSize;
	static uintptr_t _forceLod0_cloth;
	static uintptr_t _GetLODLevelFromDistance_forceMaxLod;
	static uintptr_t _AddHWGraphicObjectInstances_forceLod0;
	static uintptr_t _AddHWGraphicObjectInstances_checkIsCharacter;
	static uintptr_t _AddHWGraphicObjectInstances_checkIsCharacter_jumpOut;
	static uintptr_t HackPlayerOptionsSaveData;
	static uintptr_t ClassSerializer_EndClass;
};


uintptr_t sAddresses::Pad_UpdateTimeStamps = 0;
uintptr_t sAddresses::Pad_ScaleStickValues = 0;
uintptr_t sAddresses::PadXenon_ctor = 0;
uintptr_t sAddresses::PadProxyPC_AddPad = 0;
uintptr_t sAddresses::_addXenonJoy_Patch = 0;
uintptr_t sAddresses::_addXenonJoy_JumpOut = 0;
uintptr_t sAddresses::_PadProxyPC_Patch = 0;
uint32_t* sAddresses::_descriptor_var = 0;
void** sAddresses::_delete_class = 0;
uintptr_t sAddresses::_ps3_controls[4];
uintptr_t sAddresses::_ps3_controls_analog[4];
uintptr_t sAddresses::_skipIntroVideos = 0;
uintptr_t sAddresses::_shadowMapSize = 0;
uintptr_t sAddresses::_forceLod0_cloth = 0;
uintptr_t sAddresses::_GetLODLevelFromDistance_forceMaxLod = 0;
uintptr_t sAddresses::_AddHWGraphicObjectInstances_forceLod0 = 0;
uintptr_t sAddresses::_AddHWGraphicObjectInstances_checkIsCharacter = 0;
uintptr_t sAddresses::_AddHWGraphicObjectInstances_checkIsCharacter_jumpOut = 0;
uintptr_t sAddresses::HackPlayerOptionsSaveData = 0;
uintptr_t sAddresses::ClassSerializer_EndClass = 0;

int NEEDED_KEYBOARD_SET = 0;

auto ac_getNewDescriptor = (void*(__cdecl*)(uint32_t, uint32_t, uint32_t))0;
auto ac_getDeleteDescriptor = (uint32_t(__thiscall*)(void*, void*))0;

namespace Gear
{
	class MemHook
	{
	public:
		static MemHook*** pRef;

		static MemHook* GetRef()
		{
			return **pRef;
		}

		virtual void _0() = 0; // most likely dtor
		virtual void* Alloc(int, uint32_t, void*, const void*, const char*, const char*, uint32_t, const char*) = 0;
		virtual void _8() = 0;
		virtual void _12() = 0;
		virtual void _16() = 0;
		virtual void Free(int, void*, uint32_t, void*, const char*) = 0;
	};
};

Gear::MemHook*** Gear::MemHook::pRef = nullptr;

void* ac_allocate(int a1, uint32_t a2, void* a3, const void* a4, const char* a5, const char* a6, uint32_t a7, const char* a8)
{
	return Gear::MemHook::GetRef()->Alloc(a1, a2, a3, a4, a5, a6, a7, a8);
}

void ac_delete(void* ptr, void* a2, const char* a3)
{
	if (ptr)
	{
		uint32_t descr = ac_getDeleteDescriptor(*sAddresses::_delete_class, ptr);
		void** vtable = *(void***)ptr;
		auto dtor = (void(__thiscall*)(void*, int))vtable[0];
		dtor(ptr, 0);
		Gear::MemHook::GetRef()->Free(5, ptr, descr, a2, a3);
	}
}

namespace scimitar
{
	struct Object
	{
		void** vtable;
	};

	struct ManagedObject : Object
	{
		int m_Flags;
	};

	struct PropertyDescriptor;

	struct ClassSerializer
	{
		void EndClass()
		{
			((void(__thiscall*)(ClassSerializer*))sAddresses::ClassSerializer_EndClass)(this);
		}
	};

	class InputBindings;

	struct Pad : ManagedObject
	{
		enum PadType
		{
			MouseKeyboardPad = 0,
			PCPad,
			XenonPad,
			PS2Pad,
		};

		enum PadButton
		{
			Button1,
			Button2,
			Button3,
			Button4,
			PadDown,
			PadLeft,
			PadUp,
			PadRight,
			Select,
			Start,
			ShoulderLeft1,
			ShoulderLeft2,
			ShoulderRight1,
			ShoulderRight2,
			StickLeft,
			StickRight,
			NbButtons,
			Button_Invalid = -1,
		};

		struct ButtonStates
		{
			bool state[NbButtons];

			bool IsEmpty() const
			{
				for (int i = 0; i < NbButtons; i++)
					if (state[i])
						return false;
				return true;
			}
		};

		struct AnalogButtonStates
		{
			float state[NbButtons];
		};

		struct __declspec(align(16)) StickState
		{
			float x, y;

			bool IsEmpty() const
			{
				//return fabsf(x) < 0.1f && fabsf(y) < 0.1f;
				return x == 0.0f && y == 0.0f;
			}
		};

		ButtonStates m_LastFrame;
		ButtonStates m_ThisFrame;
		uint64_t m_LastFrameTimeStamp;
		uint64_t m_ThisFrameTimeStamp;
		uint64_t m_ButtonPressTimeStamp[NbButtons];
		uint64_t m_LastFrameEngineTimeStamp;
		uint64_t m_ThisFrameEngineTimeStamp;
		uint64_t m_ButtonPressEngineTimeStamp[NbButtons];
		AnalogButtonStates m_ButtonValues;
		StickState LeftStick;
		StickState RightStick;
		int field_1B0[17];
		float* vibrationData;
		char field_1F8[0x390];

		void UpdatePad(InputBindings* a)
		{
			((void(__thiscall*)(Pad*, InputBindings*))vtable[15])(this, a);
		}

		void UpdateTimeStamps()
		{
			((void(__thiscall*)(Pad*))sAddresses::Pad_UpdateTimeStamps)(this);
		}

		void ScaleStickValues()
		{
			((void(__thiscall*)(Pad*))sAddresses::Pad_ScaleStickValues)(this);
		}

		bool IsEmpty() const
		{
			return m_ThisFrame.IsEmpty() && LeftStick.IsEmpty() && RightStick.IsEmpty();
		}
	};
	static_assert(sizeof(Pad) == 0x590, "Pad");

	struct PadXenon : Pad
	{
		struct PadState
		{
			XINPUT_CAPABILITIES Caps;
			bool Connected;
			bool Inserted;
			bool Removed;
		};

		uint32_t m_PadIndex;
		PadState m_PadState;

		PadXenon* _ctor(uint32_t padId)
		{
			return ((PadXenon * (__thiscall*)(PadXenon*, uint32_t))sAddresses::PadXenon_ctor)(this, padId);
		}

		PadXenon(uint32_t padId)
		{
			_ctor(padId);
		}
		void* operator new(size_t size)
		{
			return ac_allocate(2, sizeof(PadXenon), ac_getNewDescriptor(sizeof(PadXenon), 16, *sAddresses::_descriptor_var), nullptr, nullptr, nullptr, 0, nullptr);
		}
	};

	static_assert(sizeof(PadXenon) == 0x5b0, "PadXenon");

	struct PadData
	{
		scimitar::Pad* pad;
		char field_4[528];
		InputBindings* pInputBindings;
	};

	enum PadSets
	{
		Keyboard1 = 0,
		Keyboard2,
		Keyboard3,
		Keyboard4,
		Joy1,
		Joy2,
		Joy3,
		Joy4,
		NbPadSets,
	};

	struct PadProxyPC : Pad
	{
		int field_590;
		uint32_t selectedPad;
		int field_598;
		scimitar::PadData pads[NbPadSets];

		bool AddPad(scimitar::Pad* a, PadType b, const wchar_t* c, uint16_t d, uint16_t e)
		{
			return ((bool(__thiscall*)(PadProxyPC*, scimitar::Pad*, PadType, const wchar_t*, uint16_t, uint16_t))sAddresses::PadProxyPC_AddPad)(this,a,b,c,d,e);
		}

		void Update()
		{
			if (selectedPad != NEEDED_KEYBOARD_SET && selectedPad != Joy1)
				selectedPad = NEEDED_KEYBOARD_SET;

			pads[NEEDED_KEYBOARD_SET].pad->UpdatePad(pads[NEEDED_KEYBOARD_SET].pInputBindings);
			pads[Joy1].pad->UpdatePad(pads[Joy1].pInputBindings);

			// see if no button was pressed in current pad
			if (pads[selectedPad].pad->IsEmpty())
			{
				uint32_t i = selectedPad == NEEDED_KEYBOARD_SET ? Joy1 : NEEDED_KEYBOARD_SET;
				// if any button was pressed on the other pad then switch to it
				if (pads[i].pad && !pads[i].pad->IsEmpty())
					selectedPad = i;
			}

			m_LastFrame = m_ThisFrame;
			m_ThisFrame = pads[selectedPad].pad->m_ThisFrame;
			LeftStick = pads[selectedPad].pad->LeftStick;
			RightStick = pads[selectedPad].pad->RightStick;

			if (selectedPad >= Joy1) // FIX: Use genuine analog values for gamepads
				m_ButtonValues = pads[selectedPad].pad->m_ButtonValues;
			else
			{
				// This is what original code does with any pad
				// I didn't bother to check if keyboard code fills m_ButtonValues so I'll leave this in just in case
				for (uint32_t i = 0; i < NbButtons; i++)
					m_ButtonValues.state[i] = m_ThisFrame.state[i] ? 1.0f : 0.0f;
			}

			UpdateTimeStamps();
		}
	};
	static_assert(sizeof(PadProxyPC) == 0x1660, "PadProxyPC");
}

ASM(HackPlayerOptionsSaveData)
{
	__asm
	{
		sub esi, 0x3D

		//mov byte ptr [esi + 0x32], 1 // Palazzo Medici
		//mov byte ptr [esi + 0x33], 1 // Basilica di Santa Maria Gloriosa dei Frari
		//mov byte ptr [esi + 0x34], 1 // Arsenale di Venezia

		mov byte ptr [esi + 0x36], 1 // Bonus dye

		mov byte ptr [esi + 0x3B], 1 // knife belt
		mov byte ptr [esi + 0x3C], 1 // altair robes
		mov byte ptr [esi + 0x3D], 1 // auditore crypt

		add esi, 0x3D

		call scimitar::ClassSerializer::EndClass

		retn
	}
}

static scimitar::PadProxyPC* pPad = nullptr;
static scimitar::PadXenon* padXenon = nullptr;

void __cdecl AddXenonPad()
{
	padXenon = new scimitar::PadXenon(0);
	if (!pPad->AddPad(padXenon, scimitar::Pad::PadType::XenonPad, L"XInput Controller 1", 5, 5))
		ac_delete(padXenon, nullptr, nullptr);
}

ASM(_addXenonJoy_Patch)
{
	__asm
	{
		mov eax, [eax+4]
		mov pPad, eax
		call AddXenonPad
	}
	VARJMP(sAddresses::_addXenonJoy_JumpOut)
}

ASM(_AddHWGraphicObjectInstances_forceLod0)
{
	__asm
	{
		// force lod0
		xor eax, eax
		mov [ebp-10h], eax
		jmp sAddresses::_AddHWGraphicObjectInstances_checkIsCharacter
	}
}

ASM(_AddHWGraphicObjectInstances_checkIsCharacter)
{
	__asm
	{
		mov edx, [ebp+8] // get arg1
		mov edx, [edx+84h] // get entity pointer
		test edx, edx
		jz _checkIsCharacter_out

		mov ecx, [edx+60h] // get entity flags
		shr ecx, 12h // check isCharacter flag
		test cl, 1
		jz _checkIsCharacter_out

		// force lod0
		xor eax, eax
		mov [ebp-10h], eax

_checkIsCharacter_out:
		mov cl, [ebx+28h]
		mov edi, [ebx+eax*4+14h]

		jmp sAddresses::_AddHWGraphicObjectInstances_checkIsCharacter_jumpOut
	}
}

void patch()
{
	if (get_private_profile_bool("ImproveShadowMapResolution", TRUE))
	{
		Patch<uint32_t>(sAddresses::_shadowMapSize, 4096);

		// cascade distances? didn't seem to change anything
		//Patch<uint32_t>(0x157CD65 + 3, 45);
		//Patch<uint32_t>(0x157CD6C + 3, 90);
	}

	//PatchByte(0x2210A73, 0); // windowed... doesn't work
	//Patch<uint32_t>(0x15D71F6, 71); // depth format

	if (get_private_profile_bool("ImproveDrawDistance", TRUE))
	{
		BYTE patch[] = {
			0x58, // pop eax
			0x31, 0xC0, // xor eax, eax
			0x90, 0x90 // nop; nop
		};
		PatchBytes(sAddresses::_forceLod0_cloth, patch);

		InjectHook(sAddresses::_AddHWGraphicObjectInstances_forceLod0, _AddHWGraphicObjectInstances_forceLod0, HOOK_JUMP);
		InjectHook(sAddresses::_AddHWGraphicObjectInstances_checkIsCharacter, _AddHWGraphicObjectInstances_checkIsCharacter, HOOK_JUMP);
		PatchJump(sAddresses::_GetLODLevelFromDistance_forceMaxLod, sAddresses::_GetLODLevelFromDistance_forceMaxLod + 0x44);
	}

	if (get_private_profile_bool("UPlayItems", TRUE))
	{
		InjectHook(sAddresses::HackPlayerOptionsSaveData, &HackPlayerOptionsSaveData); // scimitar::ac2::PlayerOptionsSaveData::Serialize()
	}

	NEEDED_KEYBOARD_SET = get_private_profile_int("KeyboardLayout", scimitar::PadSets::Keyboard1);
	if (NEEDED_KEYBOARD_SET < scimitar::PadSets::Keyboard1) NEEDED_KEYBOARD_SET = scimitar::PadSets::Keyboard1;
	else if (NEEDED_KEYBOARD_SET > scimitar::PadSets::Keyboard4) NEEDED_KEYBOARD_SET = scimitar::PadSets::Keyboard4;

	InjectHook(sAddresses::_addXenonJoy_Patch, &_addXenonJoy_Patch, PATCH_JUMP);
	InjectHook(sAddresses::_PadProxyPC_Patch, &scimitar::PadProxyPC::Update, PATCH_JUMP);

	if (get_private_profile_bool("PS3Controls", FALSE))
	{
		PatchByte(sAddresses::_ps3_controls[0], 0x23);
		PatchByte(sAddresses::_ps3_controls[1], 0x25);
		PatchByte(sAddresses::_ps3_controls[2], 0x22);
		PatchByte(sAddresses::_ps3_controls[3], 0x24);
		Patch<uint32_t>(sAddresses::_ps3_controls_analog[0], 0x174);
		Patch<uint32_t>(sAddresses::_ps3_controls_analog[1], 0x17C);
		Patch<uint32_t>(sAddresses::_ps3_controls_analog[2], 0x170);
		Patch<uint32_t>(sAddresses::_ps3_controls_analog[3], 0x178);
	}

	if (get_private_profile_bool("SkipIntroVideos", FALSE))
		PatchByte(sAddresses::_skipIntroVideos, 0xEB);
}

void InitAddresses(eExeVersion exeVersion)
{
	init_private_profile();
#ifdef INCLUDE_CONSOLE
	if (get_private_profile_bool("AllocConsole", FALSE)) init_console();
#endif
	switch (exeVersion)
	{
	case RETAIL_1_01:
		sAddresses::Pad_UpdateTimeStamps = 0x9EE7D0;
		sAddresses::Pad_ScaleStickValues = 0x9EF040;
		sAddresses::PadXenon_ctor = 0x9C9080;
		sAddresses::PadProxyPC_AddPad = 0x9C8860;
		sAddresses::_addXenonJoy_Patch = 0x9C99A5;
		sAddresses::_addXenonJoy_JumpOut = 0x9C99BC;
		sAddresses::_PadProxyPC_Patch = 0x9C71B0;
		sAddresses::_descriptor_var = (uint32_t*)0x2214F20;
		sAddresses::_delete_class = (void**)0x220CA08;
		sAddresses::_ps3_controls[0] = 0xA52C16 + 2;
		sAddresses::_ps3_controls[1] = 0xA52C33 + 2;
		sAddresses::_ps3_controls[2] = 0xA52CDA + 2;
		sAddresses::_ps3_controls[3] = 0xA52CDD + 2;
		sAddresses::_ps3_controls_analog[0] = 0xA52CA0 + 4;
		sAddresses::_ps3_controls_analog[1] = 0xA52CCF + 4;
		sAddresses::_ps3_controls_analog[2] = 0xA52CF3 + 4;
		sAddresses::_ps3_controls_analog[3] = 0xA52D09 + 4;
		sAddresses::_skipIntroVideos = 0x4148EE;
		sAddresses::_shadowMapSize = 0xAB9C13 + 3;
		sAddresses::_forceLod0_cloth = 0xAFCE43;
		sAddresses::_GetLODLevelFromDistance_forceMaxLod = 0x5495AF;
		sAddresses::_AddHWGraphicObjectInstances_forceLod0 = 0xAFCF8C;
		sAddresses::_AddHWGraphicObjectInstances_checkIsCharacter = 0xAFD11F;
		sAddresses::_AddHWGraphicObjectInstances_checkIsCharacter_jumpOut = 0xAFD126;
		sAddresses::HackPlayerOptionsSaveData = 0x10B06B3;
		sAddresses::ClassSerializer_EndClass = 0x9FB540;

		ac_getNewDescriptor = (void* (__cdecl*)(uint32_t, uint32_t, uint32_t))0x9D9030;
		ac_getDeleteDescriptor = (uint32_t(__thiscall*)(void*, void*))0x4236A0;
		Gear::MemHook::pRef = (Gear::MemHook***)0x220CA04;
		break;
	case DIGITAL_UPLAY:
		sAddresses::Pad_UpdateTimeStamps = 0x14B20C0;
		sAddresses::Pad_ScaleStickValues = 0x14B2930;
		sAddresses::PadXenon_ctor = 0x148CF50;
		sAddresses::PadProxyPC_AddPad = 0x148C730;
		sAddresses::_addXenonJoy_Patch = 0x148D885;
		sAddresses::_addXenonJoy_JumpOut = 0x148D89C;
		sAddresses::_PadProxyPC_Patch = 0x148AFA0;
		sAddresses::_descriptor_var = (uint32_t*)0x223DDE8;
		sAddresses::_delete_class = (void**)0x223A3A8;
		sAddresses::_ps3_controls[0] = 0x1515EC6 + 2;
		sAddresses::_ps3_controls[1] = 0x1515EE3 + 2;
		sAddresses::_ps3_controls[2] = 0x1515F8A + 2;
		sAddresses::_ps3_controls[3] = 0x1515F8D + 2;
		sAddresses::_ps3_controls_analog[0] = 0x1515F58 + 4;
		sAddresses::_ps3_controls_analog[1] = 0x1515F7F + 4;
		sAddresses::_ps3_controls_analog[2] = 0x1515FA3 + 4;
		sAddresses::_ps3_controls_analog[3] = 0x1515FB9 + 4;
		sAddresses::_skipIntroVideos = 0x41494E;
		sAddresses::_shadowMapSize = 0x157CD73 + 3;
		sAddresses::_forceLod0_cloth = 0x15BFBB3;
		sAddresses::_GetLODLevelFromDistance_forceMaxLod = 0x51FB1F;
		sAddresses::_AddHWGraphicObjectInstances_forceLod0 = 0x15BFCFC;
		sAddresses::_AddHWGraphicObjectInstances_checkIsCharacter = 0x15BFE8F;
		sAddresses::_AddHWGraphicObjectInstances_checkIsCharacter_jumpOut = 0x15BFE96;
		sAddresses::HackPlayerOptionsSaveData = 0xAD4723;
		sAddresses::ClassSerializer_EndClass = 0x14BEB50;

		ac_getNewDescriptor = (void* (__cdecl*)(uint32_t, uint32_t, uint32_t))0x149CAD0;
		ac_getDeleteDescriptor = (uint32_t(__thiscall*)(void*, void*))0x1466AE0;
		Gear::MemHook::pRef = (Gear::MemHook***)0x223A3A4;
		break;
	default:
		return;
	}
	patch();
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (MEMCMP32(0x00414A54 + 1, 0x01CA6FC8))
			InitAddresses(DIGITAL_UPLAY);
		else if (MEMCMP32(0x004149F4 + 1, 0x01CA4FA0))
			InitAddresses(RETAIL_1_01);
	}

	return TRUE;
}
