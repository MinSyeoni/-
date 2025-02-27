#include "InputDev.h"

USING(Engine)
IMPLEMENT_SINGLETON(CInputDev)

Engine::CInputDev::CInputDev(void)
{

}

Engine::CInputDev::~CInputDev(void)
{
	Free();
}

HRESULT Engine::CInputDev::Ready_InputDev(HINSTANCE hInst, HWND hWnd)
{

	// DInput 컴객체를 생성하는 함수
	FAILED_CHECK_RETURN(DirectInput8Create(hInst, 
											DIRECTINPUT_VERSION, 
											IID_IDirectInput8, 
											(void**)&m_pInputSDK, 
											NULL), E_FAIL);

	// 키보드 객체 생성
	FAILED_CHECK_RETURN(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyBoard, nullptr), E_FAIL);

	// 생성된 키보드 객체의 대한 정보를 컴 객체에게 전달하는 함수
	m_pKeyBoard->SetDataFormat(&c_dfDIKeyboard);
	
	// 장치에 대한 독점권을 설정해주는 함수, 클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수
	m_pKeyBoard->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// 장치에 대한 access 버전을 받아오는 함수
	m_pKeyBoard->Acquire();


	// 마우스 객체 생성
	FAILED_CHECK_RETURN(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr), E_FAIL);

	// 생성된 마우스 객체의 대한 정보를 컴 객체에게 전달하는 함수
	m_pMouse->SetDataFormat(&c_dfDIMouse);

	// 장치에 대한 독점권을 설정해주는 함수, 클라이언트가 떠있는 상태에서 키 입력을 받을지 말지를 결정하는 함수
	m_pMouse->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

	// 장치에 대한 access 버전을 받아오는 함수
	m_pMouse->Acquire();


	return S_OK;
}

void Engine::CInputDev::Set_InputDev(void)
{
	m_pKeyBoard->GetDeviceState(256, m_byKeyState);
	m_pMouse->GetDeviceState(sizeof(m_tMouseState), &m_tMouseState);
}

HRESULT CInputDev::Inquire_Input_State()
{
	if (nullptr == m_pKeyBoard ||
		nullptr == m_pMouse)
		return E_FAIL;

	// m_byKeyState배열 중, 누른 키에 해당하는 인덱스배열에 음수를 채운다. 
	// 단, 눌리지않은 키들에대해서는 항상 0의 값이 담긴다.
	m_pKeyBoard->GetDeviceState(256, m_byKeyState);

	m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE), &m_tMouseState);

	return NOERROR;
}

_bool CInputDev::Get_DIKeyUp(_ubyte byKeyID)
{
	if (m_byKeyState[byKeyID] & 0x80)
	{
		m_bIsKeyPressed[byKeyID] = true;
		return false;
	}
	else if (m_bIsKeyPressed[byKeyID])
	{
		m_bIsKeyPressed[byKeyID] = false;
		return true;
	}

	return false;
}

_bool CInputDev::Get_DIKeyDown(_ubyte byKeyID)
{
	// 이전에 누른적 없고 현재 눌렸을 때
	if (!(m_bIsKeyDown[byKeyID]) && (m_byKeyState[byKeyID] & 0x80))
	{
		m_bIsKeyDown[byKeyID] = true;
		return true;
	}
	// 현재 누른적 없으나 기존에 눌렸을 때
	else if (!(m_byKeyState[byKeyID] & 0x80) && (m_bIsKeyDown[byKeyID]))
	{
		m_bIsKeyDown[byKeyID] = false;
		return false;
	}
	return false;
}

_bool CInputDev::Get_DIKeyPressing(_ubyte byKeyID)
{
	if (m_byKeyState[byKeyID] & 0x80)
		return true;

	return false;
}

_bool CInputDev::Get_DIKeyCombined(_ubyte byFirst, _ubyte bySecond)
{
	if (Get_DIKeyDown(bySecond) && (m_byKeyState[byFirst] & 0x80))
		return true;

	return false;
}

void Engine::CInputDev::Free(void)
{
	Engine::Safe_Release(m_pKeyBoard);
	Engine::Safe_Release(m_pMouse);
	Engine::Safe_Release(m_pInputSDK);
}

