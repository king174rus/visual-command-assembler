#include "gui.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

#include <iostream>
#include "string"
#include <chrono>
#include <algorithm> // для std::transform
#include <cctype>    // для std::toupper

using namespace std::chrono;
using namespace std;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); // set click points
	}return 0;
	/**	case WM_ERASEBKGND:
	{
		HDC hdc = (HDC)wideParameter;

		// Задайте цвет фона
		HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0)); // Красный цвет фона

		RECT rect;
		GetClientRect(window, &rect);
		FillRect(hdc, &rect, hBrush);

		DeleteObject(hBrush);

		return 1;
	}
	break;	**/
	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\courbd.ttf", 16.5f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

auto start = high_resolution_clock::now();


char buf[255];
char buf1[255];
char bufznach[9];
char bufaddress[9];

string byte1 = "00";
string byte2 = "00";
string byte3 = "00";
string byte4 = "00";
string adresbyte1 = "00";
string adresbyte2 = "05";
string adresbyte3 = "A4";
string adresbyte4 = "20";
string znachbyte1 = "20";
string znachbyte2 = "11";
string znachbyte3 = "5A";
string znachbyte4 = "4B";
string znachperemenbyte1 = "20";
string znacperemenhbyte2 = "11";
string znacperemenhbyte3 = "5A";
string znacperemenhbyte4 = "4B";
string registr = " ";
string peremennai = "4B5A1120";
string adres = "0005A420";
string resultString = "";
string glavregistr = "00000000";
string byteregistrs[6][4] = { {"00","00","00","00"},{"00","00","00","00"} ,{"00","00","00","00"} ,{"00","00","00","00"},{"00","40","10","00"},{"00","40","10","00"} };


string xchg_temp_val1;
string xchg_temp_val2;
string xchg_temp_val3;
string xchg_temp_val4;
string xchg_reg_val1;
string xchg_reg_val2;
string xchg_reg_val3;
string xchg_reg_val4;
int xchg_operand_reg_index = -1;

float greenznach = 1;
float blueznach = 1;
float redznach = 1;
float greenadres = 1;
float blueadres = 1;
float redadres = 1;
float green = 1;
float blue = 1;
float red = 1;
float green1 = 0;
float blue1 = 0;
float red1 = 0;
float redbutton = 0.1216;
float greenbutton = 0.6745;
float bluebutton = 0.8667;
float timer = 0.0f;
float chet = 0.0f;
bool DF = false;
bool isBlinking = false;


const char* Registres[] = { "EAX", "AX", "AH", "AL","EBX", "BX", "BH", "BL" ,"ECX", "CX", "CH", "CL" ,"EDX", "DX", "DH", "DL", "ESI", "SI", "EDI", "DI"};
static int NumberRegistr = 0;
const char* TypePeremennoi[] = { "dd", "dw", "db"};
static int NumberTypePeremennoi = 0;
const char* Elements[] = { "x1", "[x1]", "EAX", "AX", "AH", "AL","EBX", "BX", "BH", "BL" ,"ECX", "CX", "CH", "CL" ,"EDX", "DX", "DH", "DL", "ESI", "SI", " "," ",  "EDI", "DI"," ", " ",
						  "[EAX]", "[AX]", "[AH]", "[AL]","[EBX]", "[BX]", "[BH]", "[BL]" ,"[ECX]", "[CX]", "[CH]", "[CL]" ,"[EDX]", "[DX]", "[DH]", "[DL]", "[ESI]", "[SI]"," ", " ", "[EDI]", "[DI]"," ", " " };
const char* FakeElements[] = { "x1", "[x1]", "EAX", "AX", "AH", "AL","EBX", "BX", "BH", "BL" ,"ECX", "CX", "CH", "CL" ,"EDX", "DX", "DH", "DL", "ESI", "SI", "EDI", "DI",
						  "[EAX]", "[AX]", "[AH]", "[AL]","[EBX]", "[BX]", "[BH]", "[BL]" ,"[ECX]", "[CX]", "[CH]", "[CL]" ,"[EDX]", "[DX]", "[DH]", "[DL]", "[ESI]", "[SI]", "[EDI]", "[DI]" };
static int FakeNumberElements = 0;
static int NumberElements = 0;
string ElementsCommands[2][5] = { {"0005A420", "00", "05", "A4", "20"}, {"00401000", "00", "40", "10","00"} };
string predZnach[4] = { "00", "40", "10","00" };
static int NumberElementCommands = 0;

const char* Commads[] = {"LEA", "MOV", "LODSB", "LODSW", "LODSD", "STOSB", "STOSW", "STOSD", "CLD", "STD", "XCHG", "MOVZX", "MOVSX"};
static int NumberCommand =0;

int error = 0;
int otchet = 0;
int bitysInRegistr = 4;
int Registr = 0;
long long AdresHex = 0;


void BlinkingText(const char* text)
{
	if (isBlinking)
	{
		timer += ImGui::GetIO().DeltaTime;

		if (timer > 3.0f)
		{
			timer = 0.0f;
		}
		else if (timer > 1.5f)
		{
			chet += ImGui::GetIO().DeltaTime;
			ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 0.0), text);
			return;
		}
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), text);
}

void UnderlineText(const char* text, float start, float size, float tolshina)
{
	ImVec2 textSize = ImGui::CalcTextSize(text);
	ImVec2 textPos = ImGui::GetCursorScreenPos();

	ImGui::GetWindowDrawList()->AddLine(
		ImVec2(textPos.x + start, textPos.y + textSize.y + tolshina),
		ImVec2(textPos.x + start + textSize.x + size, textPos.y + textSize.y + tolshina),
		IM_COL32(red1 * 255, green1 * 255, blue1 * 255, 255)
	);

	return;
}

void ByteText(string byte[4])
{
	ImGuiID id;
	for (int i = 0; i < 4; i++)
	{
		id = i;
		ImGui::BeginChild(id, ImVec2(40, 40), true);
		if (byte[i] == "Blink")
		{
			BlinkingText((const char*)byteregistrs[Registr][i].c_str());
            ImGui::SameLine();
            BlinkingText("");
            
		}
		else if (byte[i] == "White")
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)byteregistrs[Registr][i].c_str());
		}
		else if (byte[i] == "Red")
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[Registr][i].c_str());
		}
		else if (byte[i] == "Gray")
		{
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[Registr][i].c_str());
		}
		ImGui::EndChild();
		if (i != 3) ImGui::SameLine(0, 0);
	}
	return;
}

unsigned long long convertStringtoHex(string str)
{
	unsigned long long hex;
	std::stringstream ss;
	ss << std::hex << str;
	ss >> hex;
	return hex;
}

string convertHextoString(unsigned long long hex)
{
	std::stringstream ss;
	ss << std::hex << hex;
	return ss.str();
}

// Функция для получения индекса регистра по номеру элемента
int GetRegisterIndexFromElement(int elementNumber) {
	if (elementNumber >= 2 && elementNumber < 22) {
		return (elementNumber - 2) / 4;
	}
	return -1;
}

// Функция для получения типа регистра по номеру элемента
int GetRegisterTypeFromElement(int elementNumber) {
	if (elementNumber >= 2 && elementNumber < 22) {
		return (elementNumber - 2) % 4; // 0 - полный, 1 - 2-байтный, 2 - AH, 3 - AL
	}
	return -1;
}

// Функция для получения значений операнда
void GetOperandValues(int elementNumber, string& val1, string& val2, string& val3, string& val4, int& sizeType) {
	val1 = val2 = val3 = val4 = "00";
	sizeType = 0;

	if (elementNumber <= 1) {
		// Переменная x1
		if (NumberTypePeremennoi == 0) { // dd
			if (peremennai.length() >= 8) {
				val1 = peremennai.substr(6, 2);
				val2 = peremennai.substr(4, 2);
				val3 = peremennai.substr(2, 2);
				val4 = peremennai.substr(0, 2);
			}
			sizeType = 4;
		}
		else if (NumberTypePeremennoi == 1) { // dw
			if (peremennai.length() >= 4) {
				val1 = peremennai.substr(2, 2);
				val2 = peremennai.substr(0, 2);
			}
			sizeType = 2;
		}
		else if (NumberTypePeremennoi == 2) { // db
			if (peremennai.length() >= 2) {
				val1 = peremennai.substr(0, 2);
			}
			sizeType = 1;
		}
	}
	else if (elementNumber >= 2 && elementNumber < 22) {
		// Регистр
		int regIndex = (elementNumber - 2) / 4;
		int regType = (elementNumber - 2) % 4;

		if (regType == 0) { // Полный 4-байтный регистр
			val1 = byteregistrs[regIndex][3];
			val2 = byteregistrs[regIndex][2];
			val3 = byteregistrs[regIndex][1];
			val4 = byteregistrs[regIndex][0];
			sizeType = 4;
		}
		else if (regType == 1) { // 2-байтный регистр
			val1 = byteregistrs[regIndex][3];
			val2 = byteregistrs[regIndex][2];
			sizeType = 2;
		}
		else if (regType == 2) { // AH
			val1 = byteregistrs[regIndex][1];
			sizeType = 1;
		}
		else if (regType == 3) { // AL
			val1 = byteregistrs[regIndex][3];
			sizeType = 1;
		}
	}
	else if (elementNumber >= 22) {
		// Память - пока используем нули
		sizeType = 4;
	}
}

// Функция для установки значений операнда
void SetOperandValues(int elementNumber, string val1, string val2, string val3, string val4, int sizeType) {
	if (elementNumber <= 1) {
		// Переменная x1
		if (NumberTypePeremennoi == 0 && sizeType == 4) { // dd
			peremennai = val4 + val3 + val2 + val1;
		}
		else if (NumberTypePeremennoi == 1 && sizeType == 2) { // dw
			peremennai = val2 + val1;
		}
		else if (NumberTypePeremennoi == 2 && sizeType == 1) { // db
			peremennai = val1;
		}
	}
	else if (elementNumber >= 2 && elementNumber < 22) {
		// Регистр
		int regIndex = (elementNumber - 2) / 4;
		int regType = (elementNumber - 2) % 4;

		if (regType == 0 && sizeType == 4) { // Полный 4-байтный регистр
			byteregistrs[regIndex][3] = val1;
			byteregistrs[regIndex][2] = val2;
			byteregistrs[regIndex][1] = val3;
			byteregistrs[regIndex][0] = val4;
		}
		else if (regType == 1 && sizeType == 2) { // 2-байтный регистр
			byteregistrs[regIndex][3] = val1;
			byteregistrs[regIndex][2] = val2;
		}
		else if (regType == 2 && sizeType == 1) { // AH
			byteregistrs[regIndex][1] = val1;
		}
		else if (regType == 3 && sizeType == 1) { // AL
			byteregistrs[regIndex][3] = val1;
		}
	}
}
void gui::Render() noexcept
{
    if (NumberRegistr % 4 == 0 or NumberRegistr == 18) bitysInRegistr = 4;
    else if (NumberRegistr % 4 == 1 or NumberRegistr == 19) bitysInRegistr = 2;
    else bitysInRegistr = 1;
    if (NumberRegistr > 17) Registr = 5;
    else Registr = NumberRegistr / 4;
    NumberElements = FakeNumberElements;
    if (FakeNumberElements > 19) NumberElements += 2;
    if (FakeNumberElements > 21) NumberElements += 2;
    if (FakeNumberElements > 39) NumberElements += 2;



    if (NumberCommand >= 2 and NumberCommand <= 4) {
        // LODS* - используем ESI (индекс 4)
        std::string address =
            byteregistrs[4][0] +
            byteregistrs[4][1] +
            byteregistrs[4][2] +
            byteregistrs[4][3];

        // Приводим к верхнему регистру
        std::transform(address.begin(), address.end(), address.begin(),
            [](unsigned char c) { return std::toupper(c); });

        // Выравниваем до 8 символов
        while (address.size() < 8) address = "0" + address;

        // ОБНОВЛЯЕМ ВСЕ ЧАСТИ адреса!
        ElementsCommands[1][0] = address;
        ElementsCommands[1][1] = address.substr(0, 2);
        ElementsCommands[1][2] = address.substr(2, 2);
        ElementsCommands[1][3] = address.substr(4, 2);
        ElementsCommands[1][4] = address.substr(6, 2);
    }
    else if (NumberCommand >= 5 and NumberCommand <= 7) {
        // STOS* - используем EDI (индекс 5)
        std::string address =
            byteregistrs[5][0] +
            byteregistrs[5][1] +
            byteregistrs[5][2] +
            byteregistrs[5][3];

        std::transform(address.begin(), address.end(), address.begin(),
            [](unsigned char c) { return std::toupper(c); });

        while (address.size() < 8) address = "0" + address;

        // ОБНОВЛЯЕМ ВСЕ ЧАСТИ адреса!
        ElementsCommands[1][0] = address;
        ElementsCommands[1][1] = address.substr(0, 2);
        ElementsCommands[1][2] = address.substr(2, 2);
        ElementsCommands[1][3] = address.substr(4, 2);
        ElementsCommands[1][4] = address.substr(6, 2);
    }



    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(red, green, blue, 1.0f));

    ImGui::Begin(
        (const char*)u8"Visual Command Assembler",
        &isRunning,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove
    );
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(red1, green1, blue1, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(redbutton, greenbutton, bluebutton, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(red, green, blue, 1.0f));
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Визуализация команд ассемблера");

    ImGui::SameLine(670, 0);

    // Преобразование строк в верхний регистр
    std::transform(znachbyte1.begin(), znachbyte1.end(), znachbyte1.begin(),
        [](unsigned char c) { return std::toupper(c); });
    std::transform(znachbyte2.begin(), znachbyte2.end(), znachbyte2.begin(),
        [](unsigned char c) { return std::toupper(c); });
    std::transform(znachbyte3.begin(), znachbyte3.end(), znachbyte3.begin(),
        [](unsigned char c) { return std::toupper(c); });
    std::transform(znachbyte4.begin(), znachbyte4.end(), znachbyte4.begin(),
        [](unsigned char c) { return std::toupper(c); });





    if (ImGui::Button((const char*)u8"Сменить тему")) {
        if (green == 1) {
            green = 0;
            blue = 0;
            red = 0;
            green1 = 1;
            blue1 = 1;
            red1 = 1;
            redbutton = 0.1216;
            greenbutton = 0.6745;
            bluebutton = 0.8667;
        }
        else {
            green1 = 0;
            blue1 = 0;
            red1 = 0;
            green = 1;
            blue = 1;
            red = 1;
            redbutton = 0.1216;
            greenbutton = 0.6745;
            bluebutton = 0.8667;
        }
    }
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(red1, green1, blue1, 1.0f));

    ImGui::BeginChild("Левая панель", ImVec2(300, 370), false);

    ImGui::SameLine(0, 0);
    ImGui::BeginChild("Переменная", ImVec2(300, 80), true);
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Переменная x1");
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "x1");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (TypePeremennoi[NumberTypePeremennoi]));
    ImGui::SameLine();
    std::transform(peremennai.begin(), peremennai.end(), peremennai.begin(),
        [](unsigned char c) { return std::toupper(c); });
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), peremennai.c_str());

    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Изменить:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.26f);

    // Изменяем цвет текста в InputText при ошибке 2
    if (error == 2) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Красный
    }
    ImGui::InputText("##hidden", bufznach, IM_ARRAYSIZE(bufznach), ImGuiInputTextFlags_CharsHexadecimal);
    if (error == 2) {
        ImGui::PopStyleColor();
    }

    ImGui::SameLine();
 //   ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(redznach, greenznach, blueznach, 1.0f));
    if (ImGui::Button((const char*)u8"DW"))
    {
        peremennai = bufznach;
        error = 0;
        NumberTypePeremennoi = 0;
        while (size(peremennai) < 8) peremennai = "0" + peremennai;
        if (bitysInRegistr == 4)
        {
            znachbyte4 = peremennai.substr(0, 2);
            znachbyte3 = peremennai.substr(2, 2);
            znachbyte2 = peremennai.substr(4, 2);
            znachbyte1 = peremennai.substr(6, 2);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button((const char*)u8"W"))
    {
        peremennai = bufznach;
        error = 0;
        NumberTypePeremennoi = 1;
        while (size(peremennai) < 4) peremennai = "0" + peremennai;
        if (size(peremennai) <= 4)
        {
            znachbyte2 = peremennai.substr(0, 2);
            znachbyte1 = peremennai.substr(2, 2);
            znachbyte4 = "00";
            znachbyte3 = "00";
        }
        else error = 2;
    }
    ImGui::SameLine();
    if (ImGui::Button((const char*)u8"B"))
    {
        peremennai = bufznach;
        error = 0;
        NumberTypePeremennoi = 2;
        while (size(peremennai) < 2) peremennai = "0" + peremennai;
        if (size(peremennai) <= 2)
        {
            znachbyte1 = peremennai.substr(0, 2);
            znachbyte4 = "00";
            znachbyte3 = "00";
            znachbyte2 = "00";
        }
        else error = 2;
    }
    ImGui::EndChild();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4);
    ImGui::BeginChild("Регистр", ImVec2(300, 65), true);
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Регистр");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.2f);
    ImGui::Combo((const char*)u8"##123", &NumberRegistr, Registres, IM_ARRAYSIZE(Registres));
    registr = Registres[NumberRegistr];
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.043, 0.854, 0.317, 1));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
    if (ImGui::Button((const char*)u8"Очистить регистр"))
    {
        byteregistrs[Registr][0] = "00";
        byteregistrs[Registr][1] = "00";
        byteregistrs[Registr][2] = "00";
        byteregistrs[Registr][3] = "00";
    }
    ImGui::PopStyleColor(2);
    ImGui::EndChild();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4);
    ImGui::BeginChild("Команда", ImVec2(300, 225), true);

    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Команда:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.18f);
    ImGui::Combo("", &NumberCommand, Commads, IM_ARRAYSIZE(Commads));

    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Операнд:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.18f);
    ImGui::Combo((const char*)u8"##Операнд", &FakeNumberElements, FakeElements, IM_ARRAYSIZE(FakeElements));

    if (NumberCommand < 2) NumberElementCommands = 0;
    else NumberElementCommands = 1;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Текущая команда:");
    ImGui::Spacing();

    if (NumberCommand == 0)
    {
        if (redadres != 1) {
            redadres = red1;
            greenadres = green1;
            blueadres = blue1;
        }
        if (chet >= 0 and chet <= 4.5)
        {
            BlinkingText("LEA");
            ImGui::SameLine();
            BlinkingText((registr + ",").c_str());
            ImGui::SameLine();
            BlinkingText((const char*)FakeElements[FakeNumberElements]);
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "LEA");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)registr.c_str());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)FakeElements[FakeNumberElements]);
        }
    }
    else if (NumberCommand == 1)
    {
        if (redadres != 1) {
            redadres = 0.5;
            greenadres = 0.5;
            blueadres = 0.5;
        }
        if (chet >= 0 and chet <= 4.5)
        {
            BlinkingText("MOV");
            ImGui::SameLine();
            BlinkingText((registr + ",").c_str());
            ImGui::SameLine();
            BlinkingText((const char*)FakeElements[FakeNumberElements]);
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "MOV");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)registr.c_str());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)FakeElements[FakeNumberElements]);
        }
    }
    else if (NumberCommand >= 2 and NumberCommand <= 7)
    {
        if (redadres != 1) {
            redadres = red1;
            greenadres = green1;
            blueadres = blue1;
        }
        if (chet >= 0 and chet <= 4.5)
        {
            BlinkingText(Commads[NumberCommand]);
            ImGui::SameLine();
            BlinkingText("");
            ImGui::SameLine();
            BlinkingText("");
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), Commads[NumberCommand]);
        }
    }
    else if (NumberCommand >= 8 and NumberCommand <= 9)
    {
        if (redadres != 1) {
            redadres = 0.5;
            greenadres = 0.5;
            blueadres = 0.5;
        }
        if (chet >= 0 and chet <= 4.5)
        {
            BlinkingText(Commads[NumberCommand]);
            ImGui::SameLine();
            BlinkingText("");
            ImGui::SameLine();
            BlinkingText("");
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), Commads[NumberCommand]);
        }
    }
    else if (NumberCommand == 10)
    {
        if (redadres != 1) {
            redadres = 0.5;
            greenadres = 0.5;
            blueadres = 0.5;
        }
        if (chet >= 0 and chet <= 4.5)
        {
            BlinkingText("XCHG");
            ImGui::SameLine();
            BlinkingText((registr + ",").c_str());
            ImGui::SameLine();
            BlinkingText((const char*)FakeElements[FakeNumberElements]);
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "XCHG");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)registr.c_str());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)FakeElements[FakeNumberElements]);
        }
    }
    else if (NumberCommand == 11)
    {
        if (redadres != 1) {
            redadres = 0.5;
            greenadres = 0.5;
            blueadres = 0.5;
        }
        if (chet >= 0 and chet <= 4.5)
        {
            BlinkingText("MOVZX");
            ImGui::SameLine();
            BlinkingText((registr + ",").c_str());
            ImGui::SameLine();
            BlinkingText((const char*)FakeElements[FakeNumberElements]);
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "MOVZX");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)registr.c_str());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)FakeElements[FakeNumberElements]);
        }
    }
    else if (NumberCommand == 12)
    {
        if (redadres != 1) {
            redadres = 0.5;
            greenadres = 0.5;
            blueadres = 0.5;
        }
        if (chet >= 0 and chet <= 4.5)
        {
            BlinkingText("MOVSX");
            ImGui::SameLine();
            BlinkingText((registr + ",").c_str());
            ImGui::SameLine();
            BlinkingText((const char*)FakeElements[FakeNumberElements]);
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "MOVSX");
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)registr.c_str());
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)FakeElements[FakeNumberElements]);
        }
    }

    // Логика обработки данных для отображения в ОЗУ
    if (!isBlinking)
    {
        if (NumberCommand >= 2 and NumberCommand <= 7)
        {
            NumberElements = -1;
        }
        if (NumberElements > 1 and NumberElements < 26)
        {
            int znachOperativ = (NumberElements - 2) / 4;
            otchet = 0;
            if ((NumberElements - 2) % 4 == 2 and NumberElements < 20) {
                znachbyte1 = byteregistrs[znachOperativ][2];
                znachbyte2 = "00";
                znachbyte3 = "00";
                znachbyte4 = "00";
            }
            else
            {
                znachbyte1 = byteregistrs[znachOperativ][3];
                znachbyte2 = byteregistrs[znachOperativ][2];
                znachbyte3 = byteregistrs[znachOperativ][1];
                znachbyte4 = byteregistrs[znachOperativ][0];
            }
        }
        else if (NumberElements >= 26 or (NumberCommand >= 2 and NumberCommand <= 7))
        {
            if ((NumberTypePeremennoi == 0 and NumberCommand == 1) or NumberCommand == 4 or NumberCommand == 7)
            {
                int KosAddress = (NumberElements - 26) / 4;
                if (NumberCommand == 7) KosAddress = 5;
                else if (NumberCommand == 4) KosAddress = 4;
                if ((NumberCommand != 7 and ElementsCommands[NumberElementCommands][1] == byteregistrs[KosAddress][0] and ElementsCommands[NumberElementCommands][2] == byteregistrs[KosAddress][1] and ElementsCommands[NumberElementCommands][3] == byteregistrs[KosAddress][2] and ElementsCommands[NumberElementCommands][4] == byteregistrs[KosAddress][3])
                    or (NumberCommand == 7 and ElementsCommands[NumberElementCommands][1] == predZnach[0] and ElementsCommands[NumberElementCommands][2] == predZnach[1] and ElementsCommands[NumberElementCommands][3] == predZnach[2] and ElementsCommands[NumberElementCommands][4] == predZnach[3]))
                {
                    otchet = 0;
                    if (size(peremennai) == 8)
                    {
                        znachbyte4 = peremennai.substr(0, 2);
                        znachbyte3 = peremennai.substr(2, 2);
                        znachbyte2 = peremennai.substr(4, 2);
                        znachbyte1 = peremennai.substr(6, 2);
                    }
                    else if (size(peremennai) == 4)
                    {
                        znachbyte2 = peremennai.substr(0, 2);
                        znachbyte1 = peremennai.substr(2, 2);
                        znachbyte4 = "00";
                        znachbyte3 = "00";
                    }
                    else
                    {
                        znachbyte1 = peremennai.substr(0, 2);
                        znachbyte4 = "00";
                        znachbyte3 = "00";
                        znachbyte2 = "00";
                    }
                }
                else
                {
                    glavregistr = byteregistrs[KosAddress][0] + byteregistrs[KosAddress][1] + byteregistrs[KosAddress][2] + byteregistrs[KosAddress][3];
                    if (NumberCommand == 7) glavregistr = predZnach[0] + predZnach[1] + predZnach[2] + predZnach[3];
                    int otchet1;
                    otchet1 = convertStringtoHex(ElementsCommands[NumberElementCommands][0]) - convertStringtoHex(glavregistr);
                    if (otchet1 == 1)
                    {
                        otchet = 1;
                        znachbyte4 = "00";
                        znachbyte3 = peremennai.substr(0, 2);
                        znachbyte2 = peremennai.substr(2, 2);
                        znachbyte1 = peremennai.substr(4, 2);
                    }
                    else if (otchet1 == 2)
                    {
                        otchet = 2;
                        znachbyte4 = "00";
                        znachbyte3 = "00";
                        znachbyte2 = peremennai.substr(0, 2);
                        znachbyte1 = peremennai.substr(2, 2);
                    }
                    else if (otchet1 == 3)
                    {
                        otchet = 3;
                        znachbyte4 = "00";
                        znachbyte3 = "00";
                        znachbyte2 = "00";
                        znachbyte1 = peremennai.substr(0, 2);
                    }
                    else
                    {
                        otchet = 4;
                        znachbyte4 = "00";
                        znachbyte3 = "00";
                        znachbyte2 = "00";
                        znachbyte1 = "00";
                    }
                }
            }
            else if ((NumberTypePeremennoi == 2 and NumberCommand == 1) or NumberCommand == 2 or NumberCommand == 5)
            {
                int KosAddress = (NumberElements - 26) / 4;
                if (NumberCommand == 5) KosAddress = 5;
                else if (NumberCommand == 2) KosAddress = 4;
                if ((NumberCommand != 5 and ElementsCommands[NumberElementCommands][1] == byteregistrs[KosAddress][0] and ElementsCommands[NumberElementCommands][2] == byteregistrs[KosAddress][1] and ElementsCommands[NumberElementCommands][3] == byteregistrs[KosAddress][2] and ElementsCommands[NumberElementCommands][4] == byteregistrs[KosAddress][3])
                    or (NumberCommand == 5 and ElementsCommands[NumberElementCommands][1] == predZnach[0] and ElementsCommands[NumberElementCommands][2] == predZnach[1] and ElementsCommands[NumberElementCommands][3] == predZnach[2] and ElementsCommands[NumberElementCommands][4] == predZnach[3]))
                {
                    otchet = 3;
                    znachbyte4 = "00";
                    znachbyte3 = "00";
                    znachbyte2 = "00";
                    znachbyte1 = peremennai.substr(6, 2);
                }
                else
                {
                    otchet = 4;
                    znachbyte4 = "00";
                    znachbyte3 = "00";
                    znachbyte2 = "00";
                    znachbyte1 = "00";
                }
            }
            else if ((NumberTypePeremennoi == 1 and NumberCommand == 1) or NumberCommand == 3 or NumberCommand == 6)
            {
                int KosAddress = (NumberElements - 26) / 4;
                if (NumberCommand == 6) KosAddress = 5;
                else if (NumberCommand == 3) KosAddress = 4;
                if ((NumberCommand != 6 and ElementsCommands[NumberElementCommands][1] == byteregistrs[KosAddress][0] and ElementsCommands[NumberElementCommands][2] == byteregistrs[KosAddress][1] and ElementsCommands[NumberElementCommands][3] == byteregistrs[KosAddress][2] and ElementsCommands[NumberElementCommands][4] == byteregistrs[KosAddress][3])
                    or (NumberCommand == 6 and ElementsCommands[NumberElementCommands][1] == predZnach[0] and ElementsCommands[NumberElementCommands][2] == predZnach[1] and ElementsCommands[NumberElementCommands][3] == predZnach[2] and ElementsCommands[NumberElementCommands][4] == predZnach[3]))
                {
                    otchet = 2;
                    znachbyte4 = "00";
                    znachbyte3 = "00";
                    znachbyte2 = peremennai.substr(6, 2);
                    znachbyte1 = peremennai.substr(4, 2);
                }
                else
                {
                    glavregistr = byteregistrs[KosAddress][0] + byteregistrs[KosAddress][1] + byteregistrs[KosAddress][2] + byteregistrs[KosAddress][3];
                    if (NumberCommand == 6) glavregistr = predZnach[0] + predZnach[1] + predZnach[2] + predZnach[3];
                    int otchet1;
                    otchet1 = convertStringtoHex(ElementsCommands[NumberElementCommands][0]) - convertStringtoHex(glavregistr);

                    if (otchet1 == 1)
                    {
                        otchet = 3;
                        znachbyte4 = "00";
                        znachbyte3 = "00";
                        znachbyte2 = "00";
                        znachbyte1 = peremennai.substr(4, 2);
                    }
                    else
                    {
                        otchet = 4;
                        znachbyte4 = "00";
                        znachbyte3 = "00";
                        znachbyte2 = "00";
                        znachbyte1 = "00";
                    }
                }
            }
        }
        else
        {
            otchet = 0;
            if (size(peremennai) == 8)
            {
                znachbyte4 = peremennai.substr(0, 2);
                znachbyte3 = peremennai.substr(2, 2);
                znachbyte2 = peremennai.substr(4, 2);
                znachbyte1 = peremennai.substr(6, 2);
            }
            else if (size(peremennai) == 4)
            {
                znachbyte2 = peremennai.substr(0, 2);
                znachbyte1 = peremennai.substr(2, 2);
                znachbyte4 = "00";
                znachbyte3 = "00";
            }
            else
            {
                znachbyte1 = peremennai.substr(0, 2);
                znachbyte4 = "00";
                znachbyte3 = "00";
                znachbyte2 = "00";
            }
        }
    }

    // Кнопка активации команды
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (error != 2) {
        redznach = red1;
        greenznach = green1;
        blueznach = blue1;
    }

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.043, 0.854, 0.317, 1));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));

    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize((const char*)u8"Активировать команду").x - 20) * 0.5f);
    if (ImGui::Button((const char*)u8"Активировать команду", ImVec2(ImGui::CalcTextSize((const char*)u8"Активировать команду").x + 20, 0)) and !isBlinking)
    {
        if ((NumberElements - 2) % 4 != 0 and NumberElements >= 26 and NumberElements != 40) error = 8;
        else
        {
            error = 0;
            if (NumberCommand == -1) error = 6;
            else if (NumberCommand == 0)
            {
                if ((bitysInRegistr == 4 or bitysInRegistr == 2) and (NumberElements <= 1 or NumberElements >= 26)) isBlinking = !isBlinking;
                else if (NumberElements > 1 and NumberElements < 22) error = 7;
                else if (NumberElements >= 26 and (NumberElements - 2) % 4 != 0) error = 8;
                else error = 1;
            }
            else if (NumberCommand == 1 or NumberCommand == 10)
            {// При нажатии кнопки XCHG
                if (NumberCommand == 10) {
                    // Проверяем, что оба операнда совместимы
                    string opVal1, opVal2, opVal3, opVal4;
                    int operandSize = 0;
                    GetOperandValues(FakeNumberElements, opVal1, opVal2, opVal3, opVal4, operandSize);
                    if ((bitysInRegistr == 4 && operandSize == 4) ||
                        (bitysInRegistr == 2 && operandSize == 2) ||
                        (bitysInRegistr == 1 && operandSize == 1)) {
                        isBlinking = !isBlinking;
                        xchg_operand_reg_index = FakeNumberElements;

                        // Сохраняем значения ВТОРОГО операнда
                        xchg_temp_val1 = opVal1;
                        xchg_temp_val2 = opVal2;
                        xchg_temp_val3 = opVal3;
                        xchg_temp_val4 = opVal4;

                        // Сохраняем значения ПЕРВОГО операнда (текущего регистра)
                        int curRegIndex = (NumberRegistr > 17) ? 5 : NumberRegistr / 4;
                        xchg_reg_val1 = byteregistrs[curRegIndex][3];
                        xchg_reg_val2 = byteregistrs[curRegIndex][2];
                        xchg_reg_val3 = byteregistrs[curRegIndex][1];
                        xchg_reg_val4 = byteregistrs[curRegIndex][0];
                    }
                    else {
                        error = 3;
                    }
                }
                else {
                    // Для MOV существующая логика
                    if (otchet == 4 or ((NumberElements >= 26 and otchet != 0) and bitysInRegistr == 4) or ((NumberElements >= 26 and otchet == 3) and NumberRegistr % 4 == 1)) error = 9;
                    else if (bitysInRegistr == 4 and ((NumberTypePeremennoi == 0 and NumberElements <= 1) or ((NumberElements - 2) % 4 == 0) or (NumberElements >= 26 and otchet == 0))) isBlinking = !isBlinking;
                    else if (bitysInRegistr == 4 and ((NumberTypePeremennoi != 0 and NumberElements <= 1) or ((NumberElements - 2) % 4 != 0))) error = 3;
                    else if (bitysInRegistr == 2 and ((NumberTypePeremennoi == 1 and NumberElements <= 1) or ((NumberElements - 2) % 4 == 1) or (NumberElements >= 26 and otchet != 3))) isBlinking = !isBlinking;
                    else if (bitysInRegistr == 2 and ((NumberTypePeremennoi == 0 and NumberElements <= 1) or ((NumberElements - 2) % 4 == 0))) error = 4;
                    else if (bitysInRegistr == 2 and ((NumberTypePeremennoi == 2 and NumberElements <= 1) or ((NumberElements - 2) % 4 >= 2))) error = 3;
                    else if (bitysInRegistr == 1 and ((NumberTypePeremennoi == 2 and NumberElements <= 1) or ((NumberElements - 2) % 4 >= 2) or NumberElements >= 26)) isBlinking = !isBlinking;
                    else error = 4;
                }
            }
            else if (NumberCommand >= 2 and NumberCommand <= 4)
            {
                if (NumberCommand == 4 and znachbyte4 != "00") isBlinking = !isBlinking;
                else if (NumberCommand == 3 and znachbyte2 != "00") isBlinking = !isBlinking;
                else if (NumberCommand == 2 and znachbyte1 != "00") isBlinking = !isBlinking;
                else error = 9;
            }
            else if (NumberCommand >= 5 and NumberCommand <= 9)
            {
                isBlinking = !isBlinking;
            }
            else if (NumberCommand == 11 or NumberCommand == 12)
            {
                if (bitysInRegistr == 1) error = 10;
                else if (bitysInRegistr == 2 and ((NumberTypePeremennoi == 2 and NumberElements <= 1) or ((NumberElements - 2) % 4 >= 2))) isBlinking = !isBlinking;
                else if (bitysInRegistr == 4 and ((NumberTypePeremennoi != 0 and NumberElements <= 1) or ((NumberElements - 2) % 4 != 0 and NumberElements > 1 and NumberElements < 26))) isBlinking = !isBlinking;
                else if (bitysInRegistr == 4 and NumberElements >= 26) error = 11;
                else error = 10;
   
            }
        }
    }
    ImGui::PopStyleColor(2);
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::SameLine(306, 0);
    ImGui::BeginChild("Адрес", ImVec2(125, 370), true);
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Ввести адрeс");
    ImGui::InputText("##hidden", bufaddress, IM_ARRAYSIZE(bufaddress), ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::SameLine();
    if (ImGui::Button((const char*)u8"Ок"))
    {
        ElementsCommands[NumberElementCommands][0] = bufaddress;
        error = 0;
        while (size(ElementsCommands[NumberElementCommands][0]) < 8) ElementsCommands[NumberElementCommands][0] = ElementsCommands[NumberElementCommands][0] + "0";
        ElementsCommands[NumberElementCommands][1] = ElementsCommands[NumberElementCommands][0].substr(0, 2);
        ElementsCommands[NumberElementCommands][2] = ElementsCommands[NumberElementCommands][0].substr(2, 2);
        ElementsCommands[NumberElementCommands][3] = ElementsCommands[NumberElementCommands][0].substr(4, 2);
        ElementsCommands[NumberElementCommands][4] = ElementsCommands[NumberElementCommands][0].substr(6, 2);
    }
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Адрес");
    std::transform(ElementsCommands[NumberElementCommands][1].begin(), ElementsCommands[NumberElementCommands][1].end(), ElementsCommands[NumberElementCommands][1].begin(),
        [](unsigned char c) { return std::toupper(c); });
    std::transform(ElementsCommands[NumberElementCommands][2].begin(), ElementsCommands[NumberElementCommands][2].end(), ElementsCommands[NumberElementCommands][2].begin(),
        [](unsigned char c) { return std::toupper(c); });
    std::transform(ElementsCommands[NumberElementCommands][3].begin(), ElementsCommands[NumberElementCommands][3].end(), ElementsCommands[NumberElementCommands][3].begin(),
        [](unsigned char c) { return std::toupper(c); });
    std::transform(ElementsCommands[NumberElementCommands][4].begin(), ElementsCommands[NumberElementCommands][4].end(), ElementsCommands[NumberElementCommands][4].begin(),
        [](unsigned char c) { return std::toupper(c); });
    if (NumberCommand == 0)
    {
        if (chet >= 4.5 and chet <= 9 and bitysInRegistr == 4 and NumberElements <= 1)
        {

            BlinkingText(ElementsCommands[NumberElementCommands][1].c_str());
            ImGui::SameLine(0, 0);
            BlinkingText(ElementsCommands[NumberElementCommands][2].c_str());
            ImGui::SameLine(0, 0);
            BlinkingText(ElementsCommands[NumberElementCommands][3].c_str());
            ImGui::SameLine(0, 0);
            BlinkingText(ElementsCommands[NumberElementCommands][4].c_str());
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "h==>");
        }
        else if (chet >= 4.5 and chet <= 9 and bitysInRegistr == 2)
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), ElementsCommands[NumberElementCommands][1].c_str());
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), ElementsCommands[NumberElementCommands][2].c_str());
            ImGui::SameLine(0, 0);
            BlinkingText(ElementsCommands[NumberElementCommands][3].c_str());
            ImGui::SameLine(0, 0);
            BlinkingText(ElementsCommands[NumberElementCommands][4].c_str());
            ImGui::SameLine(0,0);
            BlinkingText("");
            ImGui::SameLine(0,0);
            BlinkingText("");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "h==>");
        }
        else
        {
            ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][1].c_str());
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][2].c_str());
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][3].c_str());
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][4].c_str());
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), "h==>");
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][1].c_str());
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][2].c_str());
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][3].c_str());
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][4].c_str());
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), "h==>");
    }
    if (NumberTypePeremennoi == 1)
    {

        unsigned long long hexadres;
        std::stringstream ss;
        ss << std::hex << ElementsCommands[NumberElementCommands][0];
        ss >> hexadres;
        hexadres += 1;
        std::stringstream ss2;
        ss2 << std::hex << hexadres;



        if (size(ss2.str()) == 9) error = 5;
        else {
            resultString = ss2.str();
            std::transform(resultString.begin(), resultString.end(), resultString.begin(),
                [](unsigned char c) { return std::toupper(c); });
            while (size(resultString) < 8) resultString = "0" + resultString;
            resultString += "h==>";
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "");
            ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), resultString.c_str());
        }
    }
    else if (NumberTypePeremennoi == 0)
    {

        unsigned long long hexadres;
        std::stringstream ss;
        ss << std::hex << ElementsCommands[NumberElementCommands][0];
        ss >> hexadres;
        hexadres += 1;
        std::stringstream ss2;
        ss2 << std::hex << hexadres;
        if (size(ss2.str()) == 9) error = 5;
        else {
            resultString = ss2.str();
            std::transform(resultString.begin(), resultString.end(), resultString.begin(),
                [](unsigned char c) { return std::toupper(c); });
            while (size(resultString) < 8) resultString = "0" + resultString;
            resultString += "h==>";
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "");
            ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), resultString.c_str());
        }
        resultString = "";
        unsigned long long hexadres1;
        std::stringstream ss3;
        ss3 << std::hex << ElementsCommands[NumberElementCommands][0];
        ss3 >> hexadres1;
        hexadres1 += 2;
        std::stringstream ss4;
        ss4 << std::hex << hexadres1;
        if (size(ss4.str()) == 9) error = 5;
        else {
            resultString = ss4.str();
            std::transform(resultString.begin(), resultString.end(), resultString.begin(),
                [](unsigned char c) { return std::toupper(c); });
            while (size(resultString) < 8) resultString = "0" + resultString;
            resultString += "h==>";
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "");
            ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), resultString.c_str());
        }
        resultString = "";
        unsigned long long hexadres2;
        std::stringstream ss5;
        ss5 << std::hex << ElementsCommands[NumberElementCommands][0];
        ss5 >> hexadres2;
        hexadres2 += 3;
        std::stringstream ss6;
        ss6 << std::hex << hexadres2;
        if (size(ss6.str()) == 9) error = 5;
        else {
            resultString = ss6.str();
            std::transform(resultString.begin(), resultString.end(), resultString.begin(),
                [](unsigned char c) { return std::toupper(c); });
            while (size(resultString) < 8) resultString = "0" + resultString;
            resultString += "h==>";
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "");
            ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), resultString.c_str());
        }
    }

    ImGui::EndChild();

    ImGui::SameLine(431, 0);
    ImGui::BeginChild("Оперативка", ImVec2(110, 370), ImGuiWindowFlags_NoMove, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"    ОЗУ");
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
    ImGui::BeginChild("Фейк1", ImVec2(87, 40), true);
    ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), "  ??");
    ImGui::EndChild();
    ImGui::BeginChild("Значение1", ImVec2(87, 40), true);
    if (chet >= 4.5f and chet <= 9.0f and NumberCommand > 0 and NumberCommand < 8)
    {
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
        ImGui::SameLine(0, 0);
        BlinkingText(znachbyte1.c_str());
        ImGui::SameLine();
        BlinkingText("");
    }
    else
    {
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
        ImGui::SameLine(0, 0);
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), znachbyte1.c_str());
    }
    ImGui::EndChild();
    if (((NumberTypePeremennoi == 0 and NumberElements < 2) or
        (((NumberElements - 2) % 4) == 0) and
        NumberCommand < 2) or NumberCommand == 4 or NumberCommand == 7)
    {
        ImGui::BeginChild("Значение2", ImVec2(87, 40), true);
        if (chet >= 9.0f and chet <= 13.5f and NumberCommand > 0 and NumberCommand < 8)
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
            ImGui::SameLine(0, 0);
            BlinkingText(znachbyte2.c_str());
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), znachbyte2.c_str());
        }
        ImGui::EndChild();

        ImGui::BeginChild("Значение3", ImVec2(87, 40), true);
        if (chet >= 13.5 and chet <= 18 and NumberCommand > 0 and NumberCommand < 8)
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
            ImGui::SameLine(0, 0);
            BlinkingText(znachbyte3.c_str());
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), znachbyte3.c_str());
        }
        ImGui::EndChild();
        ImGui::BeginChild("Значение4", ImVec2(87, 40), true);
        if (chet >= 18 and chet <= 22.5 and NumberCommand > 0 and NumberCommand < 8)
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
            ImGui::SameLine(0, 0);
            BlinkingText(znachbyte4.c_str());
            ImGui::SameLine();
            BlinkingText("");
        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), znachbyte4.c_str());
        }
        ImGui::EndChild();
    }
    else if (((NumberTypePeremennoi == 1 and NumberElements < 2) or
        ((((NumberElements - 2) % 4) == 1)) and
        NumberCommand < 2) or NumberCommand == 3 or NumberCommand == 6)
    {
        ImGui::BeginChild("Значение2", ImVec2(87, 40), true);
        if (chet >= 9.0f and chet <= 13.5f and NumberCommand > 0 and NumberCommand < 8)
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
            ImGui::SameLine(0, 0);
            BlinkingText(znachbyte2.c_str());
            ImGui::SameLine();
            BlinkingText("");

        }
        else
        {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "  ");
            ImGui::SameLine(0, 0);
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), znachbyte2.c_str());
        }
        ImGui::EndChild();

    }
    ImGui::BeginChild("Фейк2", ImVec2(87, 40), true);
    ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), "  ??");
    ImGui::EndChild();
    ImGui::BeginChild("Фейк3", ImVec2(87, 40), true);
    ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), "  ??");
    ImGui::EndChild();
    ImGui::BeginChild("Фейк4", ImVec2(87, 40), true);
    ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), "  ??");
    ImGui::EndChild();
    ImGui::BeginChild("Фейк5", ImVec2(87, 40), true);
    ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), "  ??");
    ImGui::EndChild();
    ImGui::BeginChild("Фейк6", ImVec2(87, 40), true);
    ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), "  ??");
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::SameLine(541, 0);
    ImGui::BeginChild("Регистры", ImVec2(239, 370), ImGuiWindowFlags_NoMove);
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"  Регистры");

    // Функция для отрисовки байтов регистра
    auto DrawRegisterBytes = [&](int registrIndex, const char* registerName, int blinkCondition = -1) {
        UnderlineText("AX", 70, 20, 1);

        std::string prefix = (registrIndex < 4 ? "E" : "E");
        std::string suffix = (registrIndex < 4 ? "X" : "I");

        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "         %s", registerName);
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "%s%s=", prefix.c_str(), registerName);
        ImGui::SameLine(0, 0);

        // Определяем цвета для каждого байта
        std::string byteColors[4] = { "White", "White", "White", "White" };

        // Проверяем, является ли этот регистр выбранным для мигания
        bool isCurrentRegistr = (Registr == registrIndex);

        // Полная логика из предыдущего кода, но применяем мигание только к выбранному регистру
        if (bitysInRegistr == 4 and NumberCommand < 2)
        {
            if (chet >= 9 && chet <= 13.5 && NumberCommand == 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Blink";
            }
            else if (chet >= 4.5 && chet <= 9 && NumberCommand == 1 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = "White";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && chet <= 13.5 && NumberCommand == 1 && isCurrentRegistr)
            {
                byteregistrs[registrIndex][3] = znachbyte1;
                byteColors[0] = byteColors[1] = "White";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5 && chet <= 18 && NumberCommand == 1 && isCurrentRegistr)
            {
                byteregistrs[registrIndex][2] = znachbyte2;
                byteColors[0] = "White";
                byteColors[1] = "Blink";
                byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 18 && chet <= 22.5 && NumberCommand == 1 && isCurrentRegistr)
            {
                byteregistrs[registrIndex][1] = znachbyte3;
                byteColors[0] = "Blink";
                byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 22.5 && NumberCommand == 1 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][0] = znachbyte4;
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 13.5 && NumberCommand == 0 && NumberElements <= 1 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][0] = ElementsCommands[NumberElementCommands][1];
                byteregistrs[registrIndex][1] = ElementsCommands[NumberElementCommands][2];
                byteregistrs[registrIndex][2] = ElementsCommands[NumberElementCommands][3];
                byteregistrs[registrIndex][3] = ElementsCommands[NumberElementCommands][4];
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 13.5 && NumberCommand == 0 && ((NumberElements >= 26 && (NumberElements - 2) % 4 == 0)) && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][0] = byteregistrs[(NumberElements - 22) / 4][0];
                byteregistrs[registrIndex][1] = byteregistrs[(NumberElements - 22) / 4][1];
                byteregistrs[registrIndex][2] = byteregistrs[(NumberElements - 22) / 4][2];
                byteregistrs[registrIndex][3] = byteregistrs[(NumberElements - 22) / 4][3];
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (error == 3 && NumberTypePeremennoi == 1 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "White";
                byteColors[2] = byteColors[3] = "Red";
            }
            else if (error == 3 && NumberTypePeremennoi == 2 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "White";
                byteColors[2] = byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
        }
        else if (bitysInRegistr == 2 and NumberCommand < 2)
        {
            if (chet >= 9 && chet <= 13.5 && NumberCommand == 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "Blink";
            }
            else if (chet >= 4.5 && chet <= 9 && NumberCommand == 1 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "White";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && chet <= 13.5 && NumberCommand == 1 && isCurrentRegistr)
            {
                byteregistrs[registrIndex][3] = znachbyte1;
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5 && NumberCommand == 1 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][2] = znachbyte2;
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 13.5 && NumberCommand == 0 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][2] = ElementsCommands[NumberElementCommands][3];
                byteregistrs[registrIndex][3] = ElementsCommands[NumberElementCommands][4];
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
            else if (error == 4 && NumberTypePeremennoi == 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            else if (error == 3 && NumberTypePeremennoi == 2 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "White";
                byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
        }
        else if (registrIndex % 4 == 2 && NumberCommand < 2 && bitysInRegistr != 2)
        {
            if (NumberCommand == 0 && error == 1 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            else if (chet >= 4.5 && chet <= 9 && NumberCommand == 1 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "Blink";
                byteColors[3] = "Gray";
            }
            else if (chet >= 9 && NumberCommand == 1 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][2] = znachbyte1;
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "White";
                byteColors[3] = "Gray";
            }
            else if (error == 4 && NumberTypePeremennoi == 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            else if (error == 4 && NumberTypePeremennoi == 1 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "White";
                byteColors[3] = "Gray";
            }
        }
        else if (registrIndex % 4 == 3 && NumberCommand < 2 && bitysInRegistr != 2)
        {
            if (NumberCommand == 0 && error == 1 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            else if (chet >= 4.5 && chet <= 9 && NumberCommand == 1 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && NumberCommand == 1 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][3] = znachbyte1;
                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "White";
            }
            else if (error == 4 && NumberTypePeremennoi == 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            else if (error == 4 && NumberTypePeremennoi == 1 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "White";
            }
        }
        else if (NumberCommand == 4 && isCurrentRegistr) // STOSD
        {
            if (chet >= 4.5 && chet <= 9)
            {
                NumberRegistr = 0;
                byteColors[0] = byteColors[1] = byteColors[2] = "White";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && chet <= 13.5)
            {
                byteregistrs[registrIndex][3] = znachbyte1;
                byteColors[0] = byteColors[1] = "White";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5 && chet <= 18)
            {
                byteregistrs[registrIndex][2] = znachbyte2;
                byteColors[0] = "White";
                byteColors[1] = "Blink";
                byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 18 && chet <= 22.5)
            {
                byteregistrs[registrIndex][1] = znachbyte3;
                byteColors[0] = "Blink";
                byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 22.5) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][0] = znachbyte4;

                if (!DF) AdresHex = convertStringtoHex((byteregistrs[4][0] + byteregistrs[4][1] + byteregistrs[4][2] + byteregistrs[4][3])) + 4;
                else AdresHex = convertStringtoHex((byteregistrs[4][0] + byteregistrs[4][1] + byteregistrs[4][2] + byteregistrs[4][3])) - 4;
                resultString = convertHextoString(AdresHex);
                if (!DF && resultString.size() > 8) resultString = convertHextoString(convertStringtoHex(resultString) - convertStringtoHex("FFFFFFFF") - 1);
                else if (DF && resultString.size() > 8) resultString = convertHextoString(convertStringtoHex(resultString) + convertStringtoHex("FFFFFFFF") + 1);
                while (resultString.size() < 8) resultString = "0" + resultString;
                byteregistrs[4][0] = resultString.substr(0, 2);
                byteregistrs[4][1] = resultString.substr(2, 2);
                byteregistrs[4][2] = resultString.substr(4, 2);
                byteregistrs[4][3] = resultString.substr(6, 2);

                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
        }
        else if (NumberCommand == 3 && isCurrentRegistr) // STOSW
        {
            if (chet >= 4.5 && chet <= 9)
            {
                NumberRegistr = 0;
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "White";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && chet <= 13.5)
            {
                byteregistrs[registrIndex][3] = znachbyte1;
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5)
            {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][2] = znachbyte2;

                if (!DF) AdresHex = convertStringtoHex((byteregistrs[4][0] + byteregistrs[4][1] + byteregistrs[4][2] + byteregistrs[4][3])) + 2;
                else AdresHex = convertStringtoHex((byteregistrs[4][0] + byteregistrs[4][1] + byteregistrs[4][2] + byteregistrs[4][3])) - 2;
                resultString = convertHextoString(AdresHex);
                if (!DF && resultString.size() > 8) resultString = convertHextoString(convertStringtoHex(resultString) - convertStringtoHex("FFFFFFFF") - 1);
                else if (DF && resultString.size() > 8) resultString = convertHextoString(convertStringtoHex(resultString) + convertStringtoHex("FFFFFFFF") + 1);
                while (resultString.size() < 8) resultString = "0" + resultString;
                byteregistrs[4][0] = resultString.substr(0, 2);
                byteregistrs[4][1] = resultString.substr(2, 2);
                byteregistrs[4][2] = resultString.substr(4, 2);
                byteregistrs[4][3] = resultString.substr(6, 2);

                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
            else
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
        }
        else if (NumberCommand == 2 && isCurrentRegistr) // STOSB
        {
            if (chet >= 4.5 && chet <= 9)
            {
                NumberRegistr = 0;
                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9)
            {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][3] = znachbyte1;

                if (!DF) AdresHex = convertStringtoHex((byteregistrs[4][0] + byteregistrs[4][1] + byteregistrs[4][2] + byteregistrs[4][3])) + 1;
                else AdresHex = convertStringtoHex((byteregistrs[4][0] + byteregistrs[4][1] + byteregistrs[4][2] + byteregistrs[4][3])) - 1;
                resultString = convertHextoString(AdresHex);
                if (!DF && resultString.size() > 8) resultString = "00000000";
                else if (DF && resultString.size() > 8) resultString = "FFFFFFFF";
                while (resultString.size() < 8) resultString = "0" + resultString;
                byteregistrs[4][0] = resultString.substr(0, 2);
                byteregistrs[4][1] = resultString.substr(2, 2);
                byteregistrs[4][2] = resultString.substr(4, 2);
                byteregistrs[4][3] = resultString.substr(6, 2);

                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "White";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "White";
            }
        }
        else if (NumberCommand == 7 && isCurrentRegistr) // LODSD
        {
            if (chet >= 4.5 && chet <= 9)
            {
                NumberRegistr = 0;
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 9 && chet <= 13.5)
            {
                znachbyte1 = byteregistrs[registrIndex][3];
                byteColors[0] = byteColors[1] = "White";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5 && chet <= 18)
            {
                znachbyte2 = byteregistrs[registrIndex][2];
                byteColors[0] = "White";
                byteColors[1] = "Blink";
                byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 18 && chet <= 22.5)
            {
                znachbyte3 = byteregistrs[registrIndex][1];
                byteColors[0] = "Blink";
                byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 22.5) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                NumberTypePeremennoi = 0;
                znachbyte4 = byteregistrs[registrIndex][0];
                peremennai = znachbyte4 + znachbyte3 + znachbyte2 + znachbyte1;
                predZnach[0] = byteregistrs[5][0];
                predZnach[1] = byteregistrs[5][1];
                predZnach[2] = byteregistrs[5][2];
                predZnach[3] = byteregistrs[5][3];
                if (!DF) AdresHex = convertStringtoHex((byteregistrs[5][0] + byteregistrs[5][1] + byteregistrs[5][2] + byteregistrs[5][3])) + 4;
                else AdresHex = convertStringtoHex((byteregistrs[5][0] + byteregistrs[5][1] + byteregistrs[5][2] + byteregistrs[5][3])) - 4;
                resultString = convertHextoString(AdresHex);
                if (!DF && resultString.size() > 8) resultString = convertHextoString(convertStringtoHex(resultString) - convertStringtoHex("FFFFFFFF") - 1);
                else if (DF && resultString.size() > 8) resultString = convertHextoString(convertStringtoHex(resultString) + convertStringtoHex("FFFFFFFF") + 1);
                while (resultString.size() < 8) resultString = "0" + resultString;
                byteregistrs[5][0] = resultString.substr(0, 2);
                byteregistrs[5][1] = resultString.substr(2, 2);
                byteregistrs[5][2] = resultString.substr(4, 2);
                byteregistrs[5][3] = resultString.substr(6, 2);

                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
        }
        else if (NumberCommand == 6 && isCurrentRegistr) // LODSW
        {
            if (chet >= 4.5 && chet <= 9)
            {
                NumberRegistr = 0;
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "White";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && chet <= 13.5)
            {
                znachbyte1 = byteregistrs[registrIndex][3];
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5)
            {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                NumberTypePeremennoi = 1;
                znachbyte2 = byteregistrs[registrIndex][2];
                znachbyte3 = ""; znachbyte4 = "";
                peremennai = znachbyte2 + znachbyte1;
                predZnach[0] = byteregistrs[5][0];
                predZnach[1] = byteregistrs[5][1];
                predZnach[2] = byteregistrs[5][2];
                predZnach[3] = byteregistrs[5][3];
                if (!DF) AdresHex = convertStringtoHex((byteregistrs[5][0] + byteregistrs[5][1] + byteregistrs[5][2] + byteregistrs[5][3])) + 2;
                else AdresHex = convertStringtoHex((byteregistrs[5][0] + byteregistrs[5][1] + byteregistrs[5][2] + byteregistrs[5][3])) - 2;
                resultString = convertHextoString(AdresHex);
                if (!DF && resultString.size() > 8) resultString = convertHextoString(convertStringtoHex(resultString) - convertStringtoHex("FFFFFFFF") - 1);
                else if (DF && resultString.size() > 8) resultString = convertHextoString(convertStringtoHex(resultString) + convertStringtoHex("FFFFFFFF") + 1);
                while (resultString.size() < 8) resultString = "0" + resultString;
                byteregistrs[5][0] = resultString.substr(0, 2);
                byteregistrs[5][1] = resultString.substr(2, 2);
                byteregistrs[5][2] = resultString.substr(4, 2);
                byteregistrs[5][3] = resultString.substr(6, 2);

                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
            else
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
        }
        else if (NumberCommand == 5 && isCurrentRegistr) // LODSB
        {
            if (chet >= 4.5 && chet <= 9)
            {
                NumberRegistr = 0;
                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9)
            {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                NumberTypePeremennoi = 2;
                znachbyte2 = ""; znachbyte3 = ""; znachbyte4 = "";
                znachbyte1 = byteregistrs[registrIndex][3];
                peremennai = znachbyte1;
                predZnach[0] = byteregistrs[5][0];
                predZnach[1] = byteregistrs[5][1];
                predZnach[2] = byteregistrs[5][2];
                predZnach[3] = byteregistrs[5][3];
                if (!DF) AdresHex = convertStringtoHex((byteregistrs[5][0] + byteregistrs[5][1] + byteregistrs[5][2] + byteregistrs[5][3])) + 1;
                else AdresHex = convertStringtoHex((byteregistrs[5][0] + byteregistrs[5][1] + byteregistrs[5][2] + byteregistrs[5][3])) - 1;
                resultString = convertHextoString(AdresHex);
                if (!DF && resultString.size() > 8) resultString = "00000000";
                else if (DF && resultString.size() > 8) resultString = "FFFFFFFF";
                while (resultString.size() < 8) resultString = "0" + resultString;
                byteregistrs[5][0] = resultString.substr(0, 2);
                byteregistrs[5][1] = resultString.substr(2, 2);
                byteregistrs[5][2] = resultString.substr(4, 2);
                byteregistrs[5][3] = resultString.substr(6, 2);

                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "White";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "White";
            }
        }
        else if (NumberCommand == 8 || NumberCommand == 9)
        {
            byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Gray";
        }
        else if (NumberCommand == 10 && isCurrentRegistr) // XCHG
        {
            // ИСПОЛЬЗУЕМ СОХРАНЁННЫЕ ЗНАЧЕНИЯ вместо повторного GetOperandValues
            string opVal1 = xchg_temp_val1;
            string opVal2 = xchg_temp_val2;
            string opVal3 = xchg_temp_val3;
            string opVal4 = xchg_temp_val4;

            // Определяем размер операнда из сохранённых данных
            int opSize = 0;
            if (bitysInRegistr == 4) opSize = 4;
            else if (bitysInRegistr == 2) opSize = 2;
            else opSize = 1;


            // Сохраняем ТЕКУЩИЕ значения регистра ПЕРЕД началом обмена
            string regVal1 = xchg_reg_val1; // Младший байт (AL)
            string regVal2 = xchg_reg_val2; // AH / второй байт
            string regVal3 = xchg_reg_val3; // третий байт
            string regVal4 = xchg_reg_val4; // Старший байт

            // ========== 4-БАЙТНЫЙ РЕЖИМ (EAX, EBX, ECX, EDX) ==========
            if (bitysInRegistr == 4 && opSize == 4)
            {
                if (chet >= 4.5f && chet <= 9.0f)
                {
                    // Этап 1: мигание младшего байта
                    byteColors[0] = byteColors[1] = byteColors[2] = "White";
                    byteColors[3] = "Blink";
                }
                else if (chet >= 9.0f && chet <= 13.5f)
                {
                    // Этап 2: запись младшего байта из второго операнда
                    byteregistrs[registrIndex][3] = opVal1;
                    byteColors[0] = byteColors[1] = "White";
                    byteColors[2] = "Blink";
                    byteColors[3] = "White";
                }
                else if (chet >= 13.5f && chet <= 18.0f)
                {
                    // Этап 3: запись второго байта
                    byteregistrs[registrIndex][2] = opVal2;
                    byteColors[0] = "White";
                    byteColors[1] = "Blink";
                    byteColors[2] = byteColors[3] = "White";
                }
                else if (chet >= 18.0f && chet <= 22.5f)
                {
                    // Этап 4: запись третьего байта
                    byteregistrs[registrIndex][1] = opVal3;
                    byteColors[0] = "Blink";
                    byteColors[1] = byteColors[2] = byteColors[3] = "White";
                }
                else if (chet >= 22.5f)
                {
                    // Этап 5: запись старшего байта + установка значений ВТОРОМУ операнду
                    byteregistrs[registrIndex][0] = opVal4;

                    // КЛЮЧЕВОЙ МОМЕНТ: устанавливаем второму операнду СОХРАНЁННЫЕ значения регистра
                    SetOperandValues(xchg_operand_reg_index,
                        regVal1, regVal2, regVal3, regVal4, opSize);

                    // Сброс анимации
                    chet = 0.0f;
                    timer = 0.0f;
                    isBlinking = !isBlinking;
                    byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
                }
            }
            // ========== 2-БАЙТНЫЙ РЕЖИМ (AX, BX, CX, DX) ==========
            else if (bitysInRegistr == 2 && opSize == 2)
            {
                if (chet >= 4.5f && chet <= 9.0f)
                {
                    byteColors[0] = byteColors[1] = "Gray";  // старшие байты неактивны
                    byteColors[2] = "White";
                    byteColors[3] = "Blink";                  // мигает младший байт
                }
                else if (chet >= 9.0f && chet <= 13.5f)
                {
                    // Запись младшего байта (AL)
                    byteregistrs[registrIndex][3] = opVal1;
                    byteColors[0] = byteColors[1] = "Gray";
                    byteColors[2] = "Blink";                  // мигает AH
                    byteColors[3] = "White";
                }
                else if (chet >= 13.5f)
                {
                    // Запись старшего байта (AH) + установка второму операнду
                    byteregistrs[registrIndex][2] = opVal2;

                    // Устанавливаем второму операнду оба байта из сохранённых значений
                    SetOperandValues(xchg_operand_reg_index,
                        regVal1, regVal2, "00", "00", opSize);

                    // Сброс анимации
                    chet = 0.0f;
                    timer = 0.0f;
                    isBlinking = !isBlinking;
                    byteColors[0] = byteColors[1] = "Gray";
                    byteColors[2] = byteColors[3] = "White";
                }
            }
            // ========== 1-БАЙТНЫЙ РЕЖИМ (AH, AL, BH, BL и т.д.) ==========
            else if (bitysInRegistr == 1 && opSize == 1)
            {
                if (NumberRegistr % 4 == 2) // AH, BH, CH, DH
                {
                    if (chet >= 4.5f && chet <= 9.0f)
                    {
                        byteColors[0] = byteColors[1] = "Gray";
                        byteColors[2] = "Blink";              // мигает AH
                        byteColors[3] = "Gray";
                    }
                    else if (chet >= 9.0f)
                    {
                        // Запись значения в AH + установка второму операнду
                        byteregistrs[registrIndex][1] = opVal1;

                        SetOperandValues(xchg_operand_reg_index,
                            regVal2, "00", "00", "00", opSize);

                        // Сброс анимации
                        chet = 0.0f;
                        timer = 0.0f;
                        isBlinking = !isBlinking;
                        byteColors[0] = byteColors[1] = "Gray";
                        byteColors[2] = "White";
                        byteColors[3] = "Gray";
                    }
                }
                else if (NumberRegistr % 4 == 3) // AL, BL, CL, DL
                {
                    if (chet >= 4.5f && chet <= 9.0f)
                    {
                        byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                        byteColors[3] = "Blink";              // мигает AL
                    }
                    else if (chet >= 9.0f)
                    {
                        // Запись значения в AL + установка второму операнду
                        byteregistrs[registrIndex][3] = opVal1;

                        SetOperandValues(xchg_operand_reg_index,
                            regVal1, "00", "00", "00", opSize);

                        // Сброс анимации
                        chet = 0.0f;
                        timer = 0.0f;
                        isBlinking = !isBlinking;
                        byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                        byteColors[3] = "White";
                    }
                }
            }
            // ========== ОШИБКА: несовместимые размеры операндов ==========
            else
            {
                // Подсветка ошибки красным
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            }
        else if (bitysInRegistr == 4 and NumberCommand == 12)
        {
            if (chet >= 4.5 && chet <= 9 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = "White";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && chet <= 13.5 && isCurrentRegistr)
            {
                byteregistrs[registrIndex][3] = znachbyte1;
                byteColors[0] = byteColors[1] = "White";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5 && chet <= 18 && isCurrentRegistr)
            {
                if (NumberTypePeremennoi == 2)
                    if (convertStringtoHex(znachbyte1) < 128) byteregistrs[registrIndex][2] = "00";
                    else byteregistrs[registrIndex][2] = "FF";
                else byteregistrs[registrIndex][2] = znachbyte2;
                byteColors[0] = "White";
                byteColors[1] = "Blink";
                byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 18 && chet <= 22.5 && isCurrentRegistr)
            {
                if (NumberTypePeremennoi == 2)
                    if (convertStringtoHex(znachbyte1) < 128) byteregistrs[registrIndex][1] = "00";
                    else byteregistrs[registrIndex][1] = "FF";
                else 
                    if(convertStringtoHex(znachbyte2+znachbyte1) < 32768) byteregistrs[registrIndex][1] = "00";
                    else byteregistrs[registrIndex][1] = "FF";
                byteColors[0] = "Blink";
                byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 22.5 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                if (NumberTypePeremennoi == 2)
                    if (convertStringtoHex(znachbyte1) < 128) byteregistrs[registrIndex][0] = "00";
                    else byteregistrs[registrIndex][0] = "FF";
                else
                    if (convertStringtoHex(znachbyte2 + znachbyte1) < 32768) byteregistrs[registrIndex][0] = "00";
                    else byteregistrs[registrIndex][0] = "FF";
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (error == 10 && NumberTypePeremennoi == 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Red";
                byteColors[2] = byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
        }
        else if (bitysInRegistr == 2 and NumberCommand == 11)
        {
            if (chet >= 4.5 && chet <= 9 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "White";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && chet <= 13.5 && isCurrentRegistr)
            {
                byteregistrs[registrIndex][3] = znachbyte1;
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                if (convertStringtoHex(znachbyte1) < 128) byteregistrs[registrIndex][2] = "00";
                else byteregistrs[registrIndex][2] = "FF";
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
            else if (error == 10 && NumberTypePeremennoi != 2 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
        }
        else if (bitysInRegistr == 1 && NumberCommand == 11)
        {

            if (error == 10 && NumberTypePeremennoi == 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            else if (error == 10 && NumberTypePeremennoi != 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "White";
            }

        }
        else if (bitysInRegistr == 4 and NumberCommand == 11)
        {
            if (chet >= 4.5 && chet <= 9 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = "White";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && chet <= 13.5 && isCurrentRegistr)
            {
                byteregistrs[registrIndex][3] = znachbyte1;
                byteColors[0] = byteColors[1] = "White";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5 && chet <= 18 && isCurrentRegistr)
            {
                if (NumberTypePeremennoi == 2) byteregistrs[registrIndex][2] = "00";
                else byteregistrs[registrIndex][2] = znachbyte2;
                byteColors[0] = "White";
                byteColors[1] = "Blink";
                byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 18 && chet <= 22.5 && isCurrentRegistr)
            {
                byteregistrs[registrIndex][1] = "00";
                byteColors[0] = "Blink";
                byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (chet >= 22.5 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][0] = "00";
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            else if (error == 10 && NumberTypePeremennoi == 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Red";
                byteColors[2] = byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "White";
            }
            }
        else if (bitysInRegistr == 2 and NumberCommand == 11)
        {
            if (chet >= 4.5 && chet <= 9 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "White";
                byteColors[3] = "Blink";
            }
            else if (chet >= 9 && chet <= 13.5 && isCurrentRegistr)
            {
                byteregistrs[registrIndex][3] = znachbyte1;
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = "Blink";
                byteColors[3] = "White";
            }
            else if (chet >= 13.5 && isCurrentRegistr) {
                chet = 0.0f;
                timer = 0.0f;
                isBlinking = !isBlinking;
                byteregistrs[registrIndex][2] = "00";
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
            else if (error == 10 && NumberTypePeremennoi != 2 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "White";
            }
            }
        else if (bitysInRegistr == 1 && NumberCommand == 11)
        {

            if (error == 10 && NumberTypePeremennoi == 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = byteColors[2] = byteColors[3] = "Red";
            }
            else if (error == 10 && NumberTypePeremennoi != 0 && isCurrentRegistr)
            {
                byteColors[0] = byteColors[1] = "Gray";
                byteColors[2] = byteColors[3] = "Red";
            }
            else
            {
                byteColors[0] = byteColors[1] = byteColors[2] = "Gray";
                byteColors[3] = "White";
            }

            }
        // Отрисовываем байты
        for (int i = 0; i < 4; i++) {
            if (i > 1) UnderlineText(byteregistrs[registrIndex][i].c_str(), -1, -2, 1);

            // Отрисовываем текст с нужным цветом
            bool shouldBlink = (byteColors[i] == "Blink");

            if (shouldBlink) {
                BlinkingText(byteregistrs[registrIndex][i].c_str());
            }
            else if (byteColors[i] == "Red") {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), byteregistrs[registrIndex][i].c_str());
            }
            else if (byteColors[i] == "Gray") {
                ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), byteregistrs[registrIndex][i].c_str());
            }
            else {
                ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[registrIndex][i].c_str());
            }

            if (i != 3) ImGui::SameLine(0, 0);
        }

        // Добавляем подписи для AH/AL, BH/BL и т.д. для первых 4 регистров
        if (registrIndex < 4) {
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "        %cH", registerName[0]);
            ImGui::SameLine(0, 1);
            ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "%cL", registerName[0]);
        }
        };

    // Отрисовываем все регистры
    DrawRegisterBytes(0, "AX", 5);  // EAX
    DrawRegisterBytes(1, "BX", 6);  // EBX  
    DrawRegisterBytes(2, "CX", 7);  // ECX
    DrawRegisterBytes(3, "DX", 8);  // EDX
    DrawRegisterBytes(4, "SI", -1); // ESI
    DrawRegisterBytes(5, "DI", -1); // EDI

    ImGui::EndChild();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 4);
    ImGui::BeginChild("Откладка", ImVec2(633, 150), true);

    if (NumberCommand == 4)
    {
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "[ESI] ==> EAX");
    }
    else if (NumberCommand == 3)
    {
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "[ESI] ==> AX");
    }
    else if (NumberCommand == 2)
    {
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "[ESI] ==> AL");
    }
    else if (NumberCommand == 7)
    {
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "EAX ==> [EDI]");
    }
    else if (NumberCommand == 6)
    {
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "AX ==> [EDI]");
    }
    else if (NumberCommand == 5)
    {
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "AL ==> [EDI]");
    }

    if (error == 0)
    {
        greenznach = green1;
        blueznach = blue1;
        redznach = red1;
        greenadres = green1;
        blueadres = blue1;
        redadres = red1;
    }
    else if (error == 1)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Этот регистр не может быть использован для команды lea.");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно использовать регистр размера x16 или x32");
    }
    else if (error == 2)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Числовое значение переменной больше положенного");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно увеличить тип переменной или уменьшить ее числовое значение");
    //    greenznach = 0;
   //     blueznach = 0;
    //    redznach = 1;
    }
    else if (error == 3)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Основной регистр больше переменной x1 или другого регистра");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно увеличить переменную или уменьшить регистр");
    }
    else if (error == 4)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Переменная x1 или другой регистр больше основного регистра ");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно увеличить регистр или уменьшить переменную");

    }
    else if (error == 5)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Выход за диапазон адресов ");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно уменьшить адрес ");
       greenadres = 0;
       blueadres = 0;
       redadres = 1;
    }
    else if (error == 6)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Выберите команду");

    }
    else if (error == 7)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const  char*)u8"Ошибка: Нельзя найти адрес регистра");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно выбрать переменную");
    }
    else if (error == 8)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const  char*)u8"Ошибка: Указатель должен быть с базовым регистром");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно выбрать указатель с четырехбайтным регистром");
    }
    else if (error == 9)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const  char*)u8"Ошибка: Выход за пределы значения переменной");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно увеличить адрес или уменьшить значение указателя");
    }
    else if (error == 10)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const  char*)u8"Ошибка: Для команд MOVZX и MOVSX переменная или другой регистр");
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"должны быть меньше основного регистра");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно увеличить регистр или уменьшить переменную");
    }
    else if (error == 11)
    {
        ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const  char*)u8"Неопределенность");
        ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Нужно уменьшить главный операнд");
    }
    ImGui::EndChild();
    ImGui::SameLine(0, 0);

    ImGui::BeginChild("Флаги", ImVec2(140, 150), true);
    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "DF =");
    ImGui::SameLine();
    if (chet >= 4.5 and chet <= 9 and NumberCommand == 8)
    {
        if (DF) BlinkingText("1");
        else    BlinkingText("0");
        ImGui::SameLine();
        BlinkingText("");
        ImGui::SameLine();
        BlinkingText("");
        ImGui::SameLine();
        BlinkingText("");
    }
    else if (chet >= 4.5 and chet <= 9 and NumberCommand == 9)
    {
        if (DF) BlinkingText("1");
        else    BlinkingText("0");
        ImGui::SameLine();
        BlinkingText("");
        ImGui::SameLine();
        BlinkingText("");
        ImGui::SameLine();
        BlinkingText("");
   
    }
    else
    {
        if (DF)     ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "1");
        else    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "0");
    }
    if (chet > 9 and NumberCommand == 8) DF = false;
    else if (chet > 9 and NumberCommand == 9) DF = true;
    if (ImGui::Button((const char*)u8"0")) DF = false;
    ImGui::SameLine();
    if (ImGui::Button((const char*)u8"1")) DF = true;
    ImGui::EndChild();
    ImGui::Separator();

    ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"Мои ссылки");
    if (ImGui::Button("YouTube")) {
        ShellExecute(NULL, "open", "https://www.youtube.com/c/@king174rus", 0, 0, SW_SHOWNORMAL);
    }
    ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
    if (ImGui::Button((const char*)u8"GitHub")) {
        ShellExecute(NULL, "open", "https://github.com/king174rus", 0, 0, SW_SHOWNORMAL);
    }
    ImGui::PopStyleColor(4);
    ImGui::PopStyleColor(1);
    ImGui::End();
}