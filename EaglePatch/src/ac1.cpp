#include <Windows.h>
#include <stdio.h>
#include <stdint.h>
#include <Xinput.h>
#include <dxgi.h>

#include "patcher.h"

#include "../shared/ini_reader.h"

//#define INCLUDE_CONSOLE // add ability to allocate console window

#ifdef INCLUDE_CONSOLE
#include "../shared/console.h"
#endif

enum eExeVersion
{
	DIGITAL_DX9 = 0,
	DIGITAL_DX10
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
	static uintptr_t _multisampling1;
	static uintptr_t _multisampling2;
	static uintptr_t _multisampling3;
	static uint32_t* _descriptor_var;
	static uintptr_t _ps3_controls[4];
	static uintptr_t _ps3_controls_analog[4];
	static uintptr_t _skipIntroVideos;
};

uintptr_t sAddresses::Pad_UpdateTimeStamps = 0;
uintptr_t sAddresses::Pad_ScaleStickValues = 0;
uintptr_t sAddresses::PadXenon_ctor = 0;
uintptr_t sAddresses::PadProxyPC_AddPad = 0;
uintptr_t sAddresses::_addXenonJoy_Patch = 0;
uintptr_t sAddresses::_addXenonJoy_JumpOut = 0;
uintptr_t sAddresses::_PadProxyPC_Patch = 0;
uintptr_t sAddresses::_multisampling1 = 0;
uintptr_t sAddresses::_multisampling2 = 0;
uintptr_t sAddresses::_multisampling3 = 0;
uint32_t* sAddresses::_descriptor_var = 0;
uintptr_t sAddresses::_ps3_controls[4];
uintptr_t sAddresses::_ps3_controls_analog[4];
uintptr_t sAddresses::_skipIntroVideos = 0;


auto ac_getNewDescriptor = (void* (__cdecl*)(uint32_t, uint32_t, uint32_t))0;
auto ac_allocate = (void*(__cdecl*)(int, uint32_t, void*, const void*, const char*, const char*, uint32_t, const char*))0;
auto ac_delete = (void (__cdecl*)(void*, void*, const char*))0;

static int NEEDED_KEYBOARD_SET = 0;

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
		AnalogButtonStates m_ButtonValues;
		StickState LeftStick;
		StickState RightStick;
		int field_1B0[17];
		float* vibrationData;
		char field_1F8[0x390];

		void UpdatePad(InputBindings*a)
		{
			((void(__thiscall*)(Pad*, InputBindings*))vtable[10])(this, a);
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
	static_assert(sizeof(Pad) == 0x500, "Pad");

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
			return ((PadXenon *(__thiscall*)(PadXenon*, uint32_t))sAddresses::PadXenon_ctor)(this, padId);
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

	static_assert(sizeof(PadXenon) == 0x520, "PadXenon");

	struct PadData
	{
		scimitar::Pad* pad;
		char field_4[528];
		InputBindings* pInputBindings;
	};
	static_assert(sizeof(PadData) == 536, "PadData");

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
	static_assert(sizeof(PadProxyPC) == 0x15D0, "PadProxyPC");
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

struct D3D10ResolutionContainer
{
	IDXGIOutput* DXGIOutput;
	DXGI_MODE_DESC* modes;
	uint32_t m_width, m_height, m_refreshRate, _5, _6;
	uint32_t modesNum;
	uint32_t _8;
	// there's probably more fields in here but we don't need them

	void GetDisplayModes(IDXGIOutput* a1)
	{
		((void(__thiscall*)(D3D10ResolutionContainer*, IDXGIOutput*))0x7BAD20)(this, a1);
	}

	void FindCurrentResolutionMode(uint32_t width, uint32_t height, uint32_t refreshRate)
	{
		((void(__thiscall*)(D3D10ResolutionContainer*, uint32_t, uint32_t, uint32_t))0x7BA770)(this, width, height, refreshRate);
	}

	static bool IsDisplayModeAlreadyAdded(const DXGI_MODE_DESC& mode, const DXGI_MODE_DESC* newModes, const uint32_t newModesNum)
	{
		for (uint32_t j = newModesNum; j > 0; j--)
		{
			if (newModes[j - 1].Width == mode.Width && newModes[j - 1].Height == mode.Height
				&& newModes[j - 1].RefreshRate.Numerator == mode.RefreshRate.Numerator
				&& newModes[j - 1].Format == mode.Format && newModes[j - 1].ScanlineOrdering == mode.ScanlineOrdering)
				return true;
		}
		return false;
	}

	void GetDisplayModes_hook(IDXGIOutput* a1)
	{
		GetDisplayModes(a1);
		DXGI_MODE_DESC* newModes = new DXGI_MODE_DESC[modesNum];
		uint32_t newModesNum = 0;
		for (uint32_t i = 0; i < modesNum; i++)
		{
			if (!IsDisplayModeAlreadyAdded(modes[i], newModes, newModesNum))
			{
				newModes[newModesNum++] = modes[i];
//#ifdef INCLUDE_CONSOLE
//				printf("Mode %i - Width %i Height %i RefreshRate %i %i Format %i ScanlineOrdering %i Scaling %i\n", i,
//					modes[i].Width, modes[i].Height, modes[i].RefreshRate.Numerator, modes[i].RefreshRate.Denominator, modes[i].Format, modes[i].ScanlineOrdering, modes[i].Scaling);
//#endif
			}
		}
		memset(modes, 0, sizeof(DXGI_MODE_DESC) * modesNum); // just to have cleaner memory
		memcpy(modes, newModes, sizeof(DXGI_MODE_DESC) * newModesNum);
		modesNum = newModesNum;
		delete[]newModes;
		FindCurrentResolutionMode(m_width, m_height, m_refreshRate);
	}
};

void patch()
{
	NEEDED_KEYBOARD_SET = get_private_profile_int("KeyboardLayout", scimitar::PadSets::Keyboard1);
	if (NEEDED_KEYBOARD_SET < scimitar::PadSets::Keyboard1) NEEDED_KEYBOARD_SET = scimitar::PadSets::Keyboard1;
	else if (NEEDED_KEYBOARD_SET > scimitar::PadSets::Keyboard4) NEEDED_KEYBOARD_SET = scimitar::PadSets::Keyboard4;

	InjectHook(sAddresses::_addXenonJoy_Patch, &_addXenonJoy_Patch, PATCH_JUMP);
	InjectHook(sAddresses::_PadProxyPC_Patch, &scimitar::PadProxyPC::Update, PATCH_JUMP);

	// fix multisampling
	PatchByte(sAddresses::_multisampling1, 0xEB);
	PatchBytes(sAddresses::_multisampling2, (unsigned char*)"\xb9\x01\x00\x00\x00\x90", 6);
	Nop(sAddresses::_multisampling3, 3);

	if (get_private_profile_bool("PS3Controls", FALSE))
	{
		PatchByte(sAddresses::_ps3_controls[0], 0x23);
		PatchByte(sAddresses::_ps3_controls[1], 0x25);
		PatchByte(sAddresses::_ps3_controls[2], 0x22);
		PatchByte(sAddresses::_ps3_controls[3], 0x24);
		Patch<uint32_t>(sAddresses::_ps3_controls_analog[0], 0xE4);
		Patch<uint32_t>(sAddresses::_ps3_controls_analog[1], 0xEC);
		Patch<uint32_t>(sAddresses::_ps3_controls_analog[2], 0xE0);
		Patch<uint32_t>(sAddresses::_ps3_controls_analog[3], 0xE8);
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
	case DIGITAL_DX9:
		sAddresses::Pad_UpdateTimeStamps = 0x93F990;
		sAddresses::Pad_ScaleStickValues = 0x93FC80;
		sAddresses::PadXenon_ctor = 0x9161A0;
		sAddresses::PadProxyPC_AddPad = 0x90B2F0;
		sAddresses::_addXenonJoy_Patch = 0x916979;
		sAddresses::_addXenonJoy_JumpOut = 0x916990;
		sAddresses::_PadProxyPC_Patch = 0x909C90;
		sAddresses::_multisampling1 = 0xE91422;
		sAddresses::_multisampling2 = 0xE9116D;
		sAddresses::_multisampling3 = 0xE91178;
		sAddresses::_descriptor_var = (uint32_t*)0x1A1E680;
		ac_getNewDescriptor = (void*(__cdecl*)(uint32_t, uint32_t, uint32_t))0x924070;
		ac_allocate = (void* (__cdecl*)(int, uint32_t, void*, const void*, const char*, const char*, uint32_t, const char*))0x7A4510;
		ac_delete = (void(__cdecl*)(void*, void*, const char*))0x916440;

		sAddresses::_ps3_controls[0] = 0x98CF98 + 2;
		sAddresses::_ps3_controls[1] = 0x98CFB5 + 2;
		sAddresses::_ps3_controls[2] = 0x98D061 + 2;
		sAddresses::_ps3_controls[3] = 0x98D064 + 2;
		sAddresses::_ps3_controls_analog[0] = 0x98D02E + 4;
		sAddresses::_ps3_controls_analog[1] = 0x98D056 + 4;
		sAddresses::_ps3_controls_analog[2] = 0x98D07B + 4;
		sAddresses::_ps3_controls_analog[3] = 0x98D092 + 4;
		sAddresses::_skipIntroVideos = 0x405495;
		break;
	case DIGITAL_DX10:
		sAddresses::Pad_UpdateTimeStamps = 0x912620;
		sAddresses::Pad_ScaleStickValues = 0x912910;
		sAddresses::PadXenon_ctor = 0x8F5E30;
		sAddresses::PadProxyPC_AddPad = 0x8EB7F0;
		sAddresses::_addXenonJoy_Patch = 0x8F6609;
		sAddresses::_addXenonJoy_JumpOut = 0x8F6620;
		sAddresses::_PadProxyPC_Patch = 0x8EA190;
		sAddresses::_multisampling1 = 0x1064252;
		sAddresses::_multisampling2 = 0x1063F9D;
		sAddresses::_multisampling3 = 0x1063FA8;
		sAddresses::_descriptor_var = (uint32_t*)0x29A3710;
		ac_getNewDescriptor = (void*(__cdecl*)(uint32_t, uint32_t, uint32_t))0x903AB0;
		ac_allocate = (void* (__cdecl*)(int, uint32_t, void*, const void*, const char*, const char*, uint32_t, const char*))0x415BD0;
		ac_delete = (void(__cdecl*)(void*, void*, const char*))0x8F60D0;

		sAddresses::_ps3_controls[0] = 0x96D7A8 + 2;
		sAddresses::_ps3_controls[1] = 0x96D7C5 + 2;
		sAddresses::_ps3_controls[2] = 0x96D871 + 2;
		sAddresses::_ps3_controls[3] = 0x96D874 + 2;
		sAddresses::_ps3_controls_analog[0] = 0x96D83E + 4;
		sAddresses::_ps3_controls_analog[1] = 0x96D866 + 4;
		sAddresses::_ps3_controls_analog[2] = 0x96D88B + 4;
		sAddresses::_ps3_controls_analog[3] = 0x96D8A2 + 4;
		sAddresses::_skipIntroVideos = 0x4054B5;

		if (get_private_profile_bool("D3D10_RemoveDuplicateResolutions", TRUE))
		{
			// remove interlaced resolutions
			PatchByte(0x7BAD2E + 1, 0);
			PatchByte(0x7BAD70 + 1, 0);
			InjectHook(0x7F343D, &D3D10ResolutionContainer::GetDisplayModes_hook);
		}
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
		if (MEMCMP32(0x00401375 + 1, 0x42d6)) // dx9
		{
			InitAddresses(DIGITAL_DX9);
		}
		else if (MEMCMP32(0x004013DE + 1, 0x428d)) // dx10
		{
			InitAddresses(DIGITAL_DX10);
		}
	}

	return TRUE;
}
