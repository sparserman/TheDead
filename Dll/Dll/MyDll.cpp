#include "MyDll.h"


// 움직이는 비트맵 그리기 함수
extern "C" _declspec(dllexport)
void MoveTransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask, int frm, int maxfrm)
{
	BITMAP bm;
	HDC hMemDC;
	POINT ptSize;

	HBITMAP hOldBitmap;

	hMemDC = CreateCompatibleDC(hdc);
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hbitmap);
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;


	TransparentBlt(hdc, x, y, (ptSize.x / maxfrm), ptSize.y, hMemDC, frm * (ptSize.x / maxfrm), 0, ptSize.x / maxfrm, ptSize.y, clrMask);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
}

// 정해진 색을 지워서 그려주는 함수입니다
extern "C" _declspec(dllexport)
void TransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask)
{

	BITMAP bm;
	HDC hMemDC;
	POINT ptSize;
	HBITMAP hOldBitmap;

	hMemDC = CreateCompatibleDC(hdc);
	hOldBitmap = (HBITMAP)SelectObject(hMemDC, hbitmap);
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;

	TransparentBlt(hdc, x, y, ptSize.x, ptSize.y, hMemDC, 0, 0, ptSize.x, ptSize.y, clrMask);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
}

// 그림을 그려주는 함수입니다
extern "C" _declspec(dllexport)
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit)
{
	HDC MemDC;
	HBITMAP OldBitmap;
	int bx, by;
	BITMAP bit;

	MemDC = CreateCompatibleDC(hdc);
	OldBitmap = (HBITMAP)SelectObject(MemDC, hBit);

	GetObject(hBit, sizeof(BITMAP), &bit);
	bx = bit.bmWidth;
	by = bit.bmHeight;

	BitBlt(hdc, x, y, bx, by, MemDC, 0, 0, SRCCOPY);

	SelectObject(MemDC, OldBitmap);
	DeleteDC(MemDC);
}

extern "C" _declspec(dllexport)
BOOL CheckPtInRect(HBITMAP hBit, int Obitx, int Obity, HBITMAP Pbit, int Pbitx, int Pbity) //저장할 좌표, 위치, 비트맵, 비트맵 위치
{
	int ox, oy;
	int ppx, ppy;
	BITMAP Obit, Pl_bit;
	RECT Temp, O_bit, P_bit;


	GetObject(hBit, sizeof(BITMAP), &Obit);


	ox = Obit.bmWidth;
	oy = Obit.bmHeight;

	O_bit.top = Obity;
	O_bit.left = Obitx;
	O_bit.right = Obitx + ox;
	O_bit.bottom = Obity + oy;


	GetObject(Pbit, sizeof(BITMAP), &Pl_bit);

	ppx = Pl_bit.bmWidth;
	ppy = Pl_bit.bmHeight;

	P_bit.top = Pbity;
	P_bit.left = Pbitx;
	P_bit.right = Pbitx + ppx;
	P_bit.bottom = Pbity + ppy;


	if (IntersectRect(&Temp, &O_bit, &P_bit))
	{
		return true;
	}
	else
	{
		return false;
	}

}

extern "C" _declspec(dllexport)
BOOL CheckPtInRectCustom(HBITMAP hBit, int Obitx, int Obity, int Obitx2, int Obity2, HBITMAP Pbit, int Pbitx, int Pbity, int Pbitx2, int Pbity2)
{
	BITMAP Obit, Pl_bit;
	RECT Temp, O_bit, P_bit;


	GetObject(hBit, sizeof(BITMAP), &Obit);


	O_bit.top = Obity;
	O_bit.left = Obitx;
	O_bit.right = Obitx + Obitx2;
	O_bit.bottom = Obity + Obity2;


	GetObject(Pbit, sizeof(BITMAP), &Pl_bit);

	P_bit.top = Pbity;
	P_bit.left = Pbitx;
	P_bit.right = Pbitx + Pbitx2;
	P_bit.bottom = Pbity + Pbity2;


	if (IntersectRect(&Temp, &O_bit, &P_bit))
	{
		return true;
	}
	else
	{
		return false;
	}

}