#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>

/*
	HDC = Handle to Device Context

	Device context, as defined on MS docs:
		
		A device context is a structure that defines a set of graphic objects and
		their associated attributes, as well as the graphic modes that affect output.
*/
HDC hdc_main; // represents our physical device
HDC hdc_comp; // A virtual device which we will use to draw "behind the scenes"

// pointer to the buffer that we will be using to play with pixels 
DWORD* window_bmp_p;

// Windows API specific bitmap
HBITMAP whole_screen; // handle to a bitmap
BITMAPINFO bmi; // a data struct holding info necessary to create a bitmap

// Windows requires this paintstruct to draw anything
PAINTSTRUCT ps;

int screenw = GetSystemMetrics(SM_CXSCREEN);
int screenh = GetSystemMetrics(SM_CYSCREEN);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void MakeBitMap(HWND hwnd, HBITMAP* Bitmap, BITMAPINFO Bmi, DWORD** window_p, int Width, int Height)
{
	// Create the bitmap info header
	Bmi.bmiHeader.biSize = sizeof(Bmi.bmiHeader);
	Bmi.bmiHeader.biWidth = Width;
	Bmi.bmiHeader.biHeight = -Height;
	Bmi.bmiHeader.biPlanes = 1;
	Bmi.bmiHeader.biBitCount = GetDeviceCaps(GetDC(hwnd), BITSPIXEL);
	Bmi.bmiHeader.biCompression = BI_RGB;
	Bmi.bmiHeader.biSizeImage = 0;
	Bmi.bmiHeader.biXPelsPerMeter = 0;
	Bmi.bmiHeader.biYPelsPerMeter = 0;
	Bmi.bmiHeader.biClrUsed = 0;
	Bmi.bmiHeader.biClrImportant = 0;

	// DIB = Device Independent Bitmap, a bitmap we can use regardless of device
	*Bitmap = CreateDIBSection
	(
		GetDC(hwnd),
		&Bmi,
		DIB_RGB_COLORS,
		(void**)window_p,
		0,
		0
	);

	// Get a handle (pointer) to the bits of the bitmap
	GetDIBits
	(
		GetDC(hwnd),
		*Bitmap,
		0,
		Height,
		*window_p,
		&Bmi,
		DIB_RGB_COLORS
	);

	// Write some color value to the buffer
	DWORD* ws_cpy = *window_p; // creates a local copy of our bitmap pointer
	int MapSize = Width * Height; // explicitly define size of map, compiler likely already optimizes to this
	for (int c = 0; c < MapSize; c++)
	{
		ws_cpy[c] = 0x0000aaff; // aRGB (alpha RGB) in hexadecimal format
	}

	// TRYING TO PUT IN A LINE IN THE MIDDLE OF SCREEN
	int line_width = 50;
	int line_height = 400;

	// calculate starting position
	int start_x = (screenw / 2) - (line_width / 2);
	int start_y = (screenh / 2) - (line_height / 2);

	// draw the line
	for (int y = 0; y < line_height; y++)
	{
		for (int x = 0; x < line_width; x++)
		{
			int pixel_index = (start_y + y) * screenw + (start_x + x);
			ws_cpy[pixel_index] = 0xffff0000; // bright red in aRGB
		}
	}

}

void Prepare_Screen(HWND hwnd)
{
	MakeBitMap(hwnd, &whole_screen, bmi, &window_bmp_p, screenw, screenh);
	hdc_main = GetDC(hwnd);
	hdc_comp = CreateCompatibleDC(hdc_main);
	SelectObject(hdc_comp, whole_screen);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow)
{
	// Register the window class
	const wchar_t CLASS_NAME[] = L"TheWindowClass";

	WNDCLASS wc = { 0 };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// ShowCursor(false); // switch on or off at your leisure buddy

	// create the window
	HWND hwnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW, //WS_EX_TRANSPARENT, WS_EX_OVERLAPPEDWINDOW (other option to overlap window instead of see through it)
		CLASS_NAME,									// Window class
		L"draw_pixels",								// Window text
		WS_POPUP | WS_VISIBLE,						// Window Style

		// POSITION and size
		0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),

		NULL,							// Parent window
		NULL,							// Menu
		hInstance,						// Instance handle
		NULL							// additional application data
	);

	Prepare_Screen(hwnd);

	ShowWindow(hwnd, nCmdShow);

	// Run the message loop
	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BitBlt(BeginPaint(hwnd, &ps), 0, 0, screenw, screenh, hdc_comp, 0, 0, SRCCOPY); // fist magic num pair represents coords inside window
	EndPaint(hwnd, &ps);

	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}