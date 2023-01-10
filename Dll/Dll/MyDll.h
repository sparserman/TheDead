#pragma once

#pragma comment(lib, "msimg32.lib")
#pragma comment (lib, "winmm.lib")

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <mmsystem.h>//PlaySound, mciSendCommand
#include <Digitalv.h>//mciSendCommand
using namespace std;

extern "C" _declspec(dllimport)
void TransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask);
extern "C" _declspec(dllimport)
void MoveTransBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask, int frm, int maxfrm);
extern "C" _declspec(dllimport)
void DrawBitmap(HDC hdc, int x, int y, HBITMAP hBit);
extern "C" _declspec(dllimport)
BOOL CheckPtInRectCustom(HBITMAP hBit, int Obitx, int Obity, int Obitx2, int Obity2, HBITMAP Pbit, int Pbitx, int Pbity, int Pbitx2, int Pbity2);
extern "C" _declspec(dllimport)
BOOL CheckPtInRect(HBITMAP hBit, int Obitx, int Obity, HBITMAP Pbit, int Pbitx, int Pbity);