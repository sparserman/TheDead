#include "MyDll.h"

#include "resource.h"

#define MinX 75			// 왼쪽 벽
#define MaxX 1745			// 오른쪽 벽
#define MinY 0				// 위 벽
#define MaxY 1030			// 아래 벽		(최대 이동 가능 범위입니다)

#define SOUND 150		// 기본 소리 범위
#define MINSOUND 10		// 작은 소리 범위
#define MAXSOUND 5000		// 큰 소리 범위

#define FOGSPEED 10		// 안개 속도

#define CLEARSTAGE 20	// 클리어 스테이지

// 기본 워커 상태
#define DEFAULT_WALKER walker[i].GetState() == RIGHT || walker[i].GetState() == LEFT || walker[i].GetState() == RIGHTWALKING || walker[i].GetState() == LEFTWALKING

// 좌, 우 전투중 클렘 상태
#define RIGHT_FIGHTING_CLEM clem.GetState() == CLEMRIGHTCAUGHT || clem.GetState() == CLEMRIGHTCOUNTERATTACK || clem.GetState() == CLEMRIGHTSUDDENATTACK
#define LEFT_FIGHTING_CLEM clem.GetState() == CLEMLEFTCAUGHT || clem.GetState() == CLEMLEFTCOUNTERATTACK || clem.GetState() == CLEMLEFTSUDDENATTACK

// 비전투 클렘 상태
#define NOTFIGHTING_CLEM clem.GetState() == CLEMRIGHT || clem.GetState() == CLEMLEFT || clem.GetState() == CLEMRIGHTWALKING || clem.GetState() == CLEMLEFTWALKING || clem.GetState() == CLEMRIGHTSLOWWALKING || clem.GetState() == CLEMLEFTSLOWWALKING || clem.GetState() == CLEMRIGHTJUMP || clem.GetState() == CLEMLEFTJUMP || clem.GetState() == CLEMOPEN
// 비전투 블루 상태
#define NOTFIGHTING_BLUE blue.GetState() == BRIGHT || blue.GetState() == BLEFT || blue.GetState() == BRIGHTWALKING || blue.GetState() == BLEFTWALKING || blue.GetState() == BRIGHTJUMP || blue.GetState() == BLEFTJUMP

// dll
void (*TransBltPt)(HDC, int, int, HBITMAP, COLORREF) = NULL;
void (*MoveTransBltPt)(HDC, int, int, HBITMAP, COLORREF, int, int) = NULL;
void (*DrawBitmapPt)(HDC, int, int, HBITMAP) = NULL;
BOOL(*CheckPtInRectCustomPt)(HBITMAP, int, int, int, int, HBITMAP, int, int, int, int) = NULL;
BOOL(*CheckPtInRectPt)(HBITMAP, int, int, HBITMAP, int, int) = NULL;

MCI_PLAY_PARMS playBgm;
MCI_OPEN_PARMS openBgm;
int dwID;

float SPEED = 3;				// 플레이어 이동속도입니다
float WalkerSpeed = 0.8;			// 워커의 이동속도입니다

bool AllStop = true;			// 기본 인게임 정지
bool stop = false;				// 클렘 정지용
bool die = false;				// 클렘 죽었는 지
bool dead = false;				// 재시작 가능 상태

const TCHAR* text = TEXT("내용");

HINSTANCE hInst;

HWND hWndMain;

HBITMAP hBit;
//EVOLVEONECS0727
// 모든 비트맵 선언
HBITMAP ClemBit[19], BackGroundBit[5], FogBit, ObjectBit[30], WalkerBit[10], BackPackBit, PackItemBit[6], TitleBit[5], TextBoxBit[2], ArrowBit, BulletBit, LifeBit[5]
		, BlueBit[10], InterfaceBit[2], FoodSelectBit[4], SmallFogBit, EnergyBit;

float fx = -2070, fy = -1330;	// 안개 위치
bool showfog = true;		// 안개 활성화
bool movefog = false;		// 안개 이동 활성화

int objindex = 0;		// 오브젝트 번호
int walkerindex = 0;	// 워커 번호
int bulletindex = 0;	// 총알 번호

float sound = 10;	// 소리 범위

bool key = true;		// 버튼 꾹 누르기 방지

bool openbackpack = false;	// 가방 열었는 지

int stage = 0;			// 게임 스테이지

int page = 0;		// 텍스트 박스용
bool showtext = false;	// 텍스트 박스가 켜져있는 지

int sel = 0;		// 로비에서 메뉴 선택

bool musicOn = true;	// 배경음악 On/Off

int background = 0;		// 배경그림

bool walkerstop = false;	// 워커 정지

// 스테이지 저장
int g_bullet = 0;
bool g_item[4];
int g_energy = 0;
int g_blueenergy = 0;
int g_food = 0;

float mousex = 0, mousey = 0;

bool blueattackstate = true;	// 공격태세

// 함수 선언
void Talk();
void Save();
void FoodUp();
void FoodSelect();
void LifeBlt(HDC hMemDC, int x, int y, HBITMAP bitmap, COLORREF clrMask, int num);
void BulletControl(HDC hMemDC);
void OnBGM(int num);
void TextGo();
void TextStop();
void ShowText(HDC hMemDC);
void FontSet2(HDC hMemDC);
void FontSet(HDC hdc);
void SoundSelect(int m);
void BackPackOpen(HDC hMemDC);
void CreateWalker(float mx, float my, int state);
void DrawWalker(HDC hMemDC);
void WalkerControl();
void Stage();
void CreateObject(HBITMAP bitmap, float mx, float my, float mx2, float my2, int kind);
void DrawObject(HDC hMemDC);
void Collision();
void FogControl(HDC hMemDC);
void OnTimer();
void Create();
//



enum ClemState {
	CLEMRIGHT = 0, CLEMLEFT, CLEMRIGHTWALKING, CLEMLEFTWALKING, CLEMRIGHTSLOWWALKING, CLEMLEFTSLOWWALKING,
	CLEMRIGHTJUMP, CLEMLEFTJUMP, CLEMRIGHTSUDDENATTACK, CLEMLEFTSUDDENATTACK, CLEMRIGHTCAUGHT, CLEMLEFTCAUGHT,
	CLEMRIGHTCOUNTERATTACK, CLEMLEFTCOUNTERATTACK, CLEMRIGHTDIE, CLEMLEFTDIE, CLEMOPEN, CLEMRIGHTSHOT, CLEMLEFTSHOT
};

enum WalkerState {
	LEFT = 0, RIGHT, LEFTWALKING ,RIGHTWALKING, LEFTDIE, RIGHTDIE, LEFTDIE2, RIGHTDIE2, LEFTGUNDIE, RIGHTGUNDIE
};

enum ObjectName {
	LAND = 0, STICK, KEY, GUN, FOOD, BULLET, WALKER, DOOR, OPENDOOR, DARKLAND, PILLAR, STONE, ROAD, ROOFTOP, SWITCH, CLOSEBOX, OPENBOX, NEWDOOR, NEWOPENDOOR
	, WARDROBE, OPENWARDROBE, ITEMBOX
};

enum TitleName {
	LOBBY = 0, INGAME, CLEAR, DEAD
};

enum BlueState {
	BRIGHT, BLEFT, BRIGHTWALKING, BLEFTWALKING, BRIGHTJUMP, BLEFTJUMP, BRIGHTATTACK, BLEFTATTACK, BRIGHTDIE, BLEFTDIE
};

enum SoundName {
	BACKPACKSOUND = 100, ITEMGETSOUND, BREAKINGSOUND, BLUEATTACKSOUND, BLUEATTACK2SOUND, BLUEDIESOUND, FOODSOUND, SWITCHSOUND
};

struct FoodSel
{
	RECT rt = { 0,0,0,0 };
	bool create = false;
	HBITMAP bitmap;
};

FoodSel foodsel[2];

struct Bullet
{
	int x = 0;
	int y = 0;
	int sx = 0;
	int sy = 0;
	bool create = false;
};

Bullet bull[5];

class Walker
{
public:
	void MXSetting()
	{
		switch (state)
		{
		case RIGHT:
		case RIGHTWALKING:
			mx = 50;
			sight = 50;
			break;
		case LEFT:
		case LEFTWALKING:
			mx = 30;
			sight = -550;
			break;
		}
	}
	void FrmControl()
	{
		// 총맞을 시
		if (create)
		{
			if (state == RIGHTGUNDIE)
			{
				if (frm == 4)
				{
					create = false;
					CreateObject(WalkerBit[LEFTDIE], x, y, 0, 0, WALKER);
				}
			}
			else if (state == LEFTGUNDIE)
			{
				if (frm == 4)
				{
					create = false;
					CreateObject(WalkerBit[RIGHTDIE], x, y, 0, 0, WALKER);
				}
			}
		}
		if (frm == maxfrm - 1)
		{
			frm = 0;
		}
		else
		{
			frm++;
		}
	}
	void Move(float m)
	{
		if (!walkerstop)
		{
			x += m;
			if (y > MaxY)
			{
				y = -100;
			}
			if (x < MinX)
			{
				state = RIGHTWALKING;
			}
			if (x > MaxX)
			{
				state = LEFTWALKING;
			}
		}
	}
	void Gravity()
	{
		if (air)
		{
			if (gravity < 10)
			{
				gravity += 0.2;
				air = true;
			}
		}
		else
		{
			gravity = 0;
		}

		y += gravity;
	}

	// 생성자 소멸자
	Walker() : x(0), y(0), mx(0), sight(0), state(0), frm(0), maxfrm(10), create(false), gravity(0), air(false), landcheck(false) {}
	~Walker() {}
#pragma region GetSet
	float GetX() { return x; }
	void SetX(float m) { x = m; }
	float GetY() { return y; }
	void SetY(float m) { y = m; }
	float GetMX() { return mx; }
	void SetMX(float m) { mx = m; }
	float GetSight() { return sight; }
	void SetSight(float m) { sight = m; }
	int GetState() { return state; }
	void SetState(int m) { state = m; }
	int GetFrm() { return frm; }
	void SetFrm(int m) { frm = m; }
	int GetMaxfrm() { return maxfrm; }
	void SetMaxfrm(int m) { maxfrm = m; }
	bool GetCreate() { return create; }
	void SetCreate(bool m) { create = m; }
	float GetGravity() { return gravity; }
	void SetGravity(float m) { gravity = m; }
	bool GetAir() { return air; }
	void SetAir(bool m) { air = m; }
	bool GetLandCheck() { return landcheck; }
	void SetLandCheck(bool m) { landcheck = m; }
#pragma endregion
private:
	float x;
	float y;
	float mx;		// 잡기 범위
	float sight;	// 시야
	int state;
	int frm;
	int maxfrm;
	bool create;
	float gravity;
	bool air;
	bool landcheck;
};

Walker walker[30];

struct Object
{
	float x = 0;
	float y = 0;
	float mx = 0;
	float my = 0;
	HBITMAP bitmap;
	int kind = -1;		// 종류
	bool create = false;
};

Object obj[50];

class Clem
{
public:
	// 초기화
	void ClemReset()
	{
		hp = 100;
		
		for (int i = 0; i < 4; i++)
		{
			item[i] = g_item[i];
		}
	}

	// 사격
	void Shot()
	{
		if (item[GUN] && bullet > 0 && !stop)
		{
			if (state == CLEMRIGHT)
			{
				state = CLEMRIGHTSHOT;
				bullet--;
				NewFrm(6);
				stop = true;
			}
			else if (state == CLEMLEFT)
			{
				state = CLEMLEFTSHOT;
				bullet--;
				NewFrm(6);
				stop = true;
			}
		}
	}

	// 문 열기 & 들어가기 & 상호작용
	void OpenDoor()
	{
		for (int i = 0; i < 50; i++)
		{
			if (obj[i].create)
			{
				switch (obj[i].kind)
				{
				case DOOR:
					if (item[STICK])
					{
						if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], x + 20, y, 40, 100, obj[i].bitmap, obj[i].x, obj[i].y, 70, 120))
						{
							if (page == 64)
							{
								page = 70;
								return;
							}
							state = CLEMOPEN;
							x = obj[i].x - 50;
							NewFrm(13);
							obj[i].kind = OPENDOOR;
							stop = true;
							SoundSelect(CLEMOPEN);
						}
					}
					else if (item[KEY])
					{
						if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], x + 20, y, 40, 100, obj[i].bitmap, obj[i].x, obj[i].y, 70, 120))
						{
							if (page == 64)
							{
								page = 70;
								return;
							}
							obj[i].kind = OPENDOOR;
							obj[i].bitmap = ObjectBit[OPENDOOR];
							SoundSelect(KEY);
							item[KEY] = false;	// 열쇠 파괴
						}
					}
					break;
				case OPENDOOR:
					if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], x + 20, y, 40, 100, obj[i].bitmap, obj[i].x, obj[i].y, 70, 120))
					{
						// 실내등을 끄고 다음으로 넘어가려 할 때
						if (showfog && stage == 13 && page == 69)
						{
							page = 72;
							return;
						}
						Save();
						Stage();
						if (page == 6 || page == 9)
						{
							page++;
						}
						if (page == 29 && stage == 9)
						{
							page++;
						}
						if (page == 36)
						{
							page = 55;
						}
					}
					break;
				}
			}
		}
	}

	// 체력 깎기 관련
	void HpDown()
	{
		if (state == CLEMLEFTCAUGHT || state == CLEMRIGHTCAUGHT)
		{
			hp -= 2;
			if (energy <= 0)	// 에너지가 없을 경우 바로 사망
			{
				hp = 0;
			}
		}
		if (hp <= 0 && !die)
		{
			NewFrm(14);
			SoundSelect(CLEMRIGHTDIE);
			if (state == CLEMLEFTCAUGHT)
			{
				state = CLEMLEFTDIE;
			}
			else if (state == CLEMRIGHTCAUGHT)
			{
				state = CLEMRIGHTDIE;
			}
			die = true;
		}
		if (hp >= 200)
		{
			hp = 50;
			NewFrm(20);
			SoundSelect(CLEMRIGHTCOUNTERATTACK);
			if (state == CLEMRIGHTCAUGHT)
			{
				state = CLEMRIGHTCOUNTERATTACK;
			}
			else if (state == CLEMLEFTCAUGHT)
			{
				state = CLEMLEFTCOUNTERATTACK;
			}
		}
	}

	// 엔드 프레임
	void EndFrm()
	{
		// 급습
		if (state == CLEMRIGHTSUDDENATTACK)
		{
			if (frm == 23)
			{
				state = CLEMRIGHT;
				NewFrm(10);
				x -= 20;
				stop = false;
				CreateObject(WalkerBit[RIGHTDIE], x + 20, y, 0, 0, WALKER);
			}
		}
		if (state == CLEMLEFTSUDDENATTACK)
		{
			if (frm == 23)
			{
				state = CLEMLEFT;
				NewFrm(10);
				x += 20;
				stop = false;
				CreateObject(WalkerBit[LEFTDIE], x - 20, y, 0, 0, WALKER);
			}
		}

		// 죽음
		if (state == CLEMLEFTDIE || state == CLEMRIGHTDIE)
		{
			if (frm == 13)
			{
				AllStop = true;
				
				SetTimer(hWndMain, 2, 1000, NULL);
			}
		}

		// 반격
		if (state == CLEMRIGHTCOUNTERATTACK)
		{
			if (frm == 19)
			{
				hp = 100;
				state = CLEMRIGHT;
				NewFrm(10);
				x -= 20;
				stop = false;
				CreateObject(WalkerBit[RIGHTDIE2], x + 20, y, 0, 0, WALKER);
				energy -= 20;
			}
		}
		if (state == CLEMLEFTCOUNTERATTACK)
		{
			if (frm == 19)
			{
				hp = 100;
				state = CLEMLEFT;
				NewFrm(10);
				x += 20;
				stop = false;
				CreateObject(WalkerBit[LEFTDIE2], x - 20, y, 0, 0, WALKER);
				energy -= 20;
			}
		}

		// 문따기
		if (state == CLEMOPEN)
		{
			if (frm == 12)
			{
				state = CLEMRIGHT;
				NewFrm(10);
				for (int i = 0; i < 50; i++)
				{
					if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], x + 20, y, 40, 100, obj[i].bitmap, obj[i].x, obj[i].y, 70, 120))
					{
						if (obj[i].kind == OPENDOOR)
						{
							obj[i].bitmap = ObjectBit[OPENDOOR];
						}
					}
				}
				stop = false;
			}
		}
		// 사격
		if (state == CLEMRIGHTSHOT)
		{
			if (frm == 5)
			{
				bull[bulletindex].create = true;
				bull[bulletindex].x = x + 90;
				bull[bulletindex].y = y + 30;
				bull[bulletindex].sx = 10;
				state = CLEMRIGHT;
				NewFrm(10);
				SoundSelect(CLEMRIGHTSHOT);
				stop = false;
				if (bulletindex != 4)
				{
					bulletindex++;
				}
				else
				{
					bulletindex = 0;
				}
			}
		}
		else if (state == CLEMLEFTSHOT)
		{
			if (frm == 5)
			{
				bull[bulletindex].create = true;
				bull[bulletindex].x = x + 10;
				bull[bulletindex].y = y + 30;
				bull[bulletindex].sx = -10;
				state = CLEMLEFT;
				NewFrm(10);
				SoundSelect(CLEMRIGHTSHOT);
				stop = false;
				if (bulletindex != 4)
				{
					bulletindex++;
				}
				else
				{
					bulletindex = 0;
				}
			}
		}

		if (state == CLEMRIGHT || state == CLEMLEFT)
		{
			sound = MINSOUND;
		}
	}

	// 급습
	void SuddenAttack()
	{
		for (int i = 0; i < 15; i++)
		{
			if (walker[i].GetCreate())
			{
				if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], x + 10, y, 80, 100, WalkerBit[RIGHT], walker[i].GetX(), walker[i].GetY(), 100, 100))
				{
					switch (state)
					{
					case CLEMRIGHT:
					case CLEMRIGHTWALKING:
					case CLEMRIGHTSLOWWALKING:
						if (walker[i].GetState() == RIGHT)
						{
							x = walker[i].GetX() - 20;
							y = walker[i].GetY();
							state = CLEMRIGHTSUDDENATTACK;
							NewFrm(24);
							stop = true;
							walker[i].SetCreate(false);
							SoundSelect(CLEMRIGHTSUDDENATTACK);
						}
						break;
					case CLEMLEFT:
					case CLEMLEFTWALKING:
					case CLEMLEFTSLOWWALKING:
						if (walker[i].GetState() == LEFT)
						{
							x = walker[i].GetX() + 20;
							y = walker[i].GetY();
							state = CLEMLEFTSUDDENATTACK;
							NewFrm(24);
							stop = true;
							walker[i].SetCreate(false);
							SoundSelect(CLEMRIGHTSUDDENATTACK);
						}
						break;
					}
				}
			}
		}
	}

	// 프레임 새로 설정
	void NewFrm(int m)
	{
		frm = 0;
		maxfrm = m;
	}

	//클렘 이동
	void ClemMove()
	{
		if (!AllStop)
		{
			if (x > MinX)
			{
				if (GetKeyState(VK_LEFT) & 0x8000)				// 왼쪽
				{
					if (!movefog)
					{
						// 이동 속도 관련
						if (state == CLEMLEFTSLOWWALKING)
						{
							if (!((frm >= 6 && frm <= 9) || (frm >= 15 && frm <= 19)))
							{
								x -= SPEED;
							}
						}
						else
						{
							x -= SPEED;
						}

						// 이동 중 천천히 걸을 경우
						if (!air && state != CLEMLEFTSLOWWALKING)
						{
							if (slow)
							{
								state = CLEMLEFTSLOWWALKING;
								if (maxfrm != 20)
								{
									NewFrm(20);
								}
							}
						}

						// 정지 중 천천히 걸을 경우
						if (!air && state != CLEMLEFTWALKING)
						{
							if (slow)
							{
								state = CLEMLEFTSLOWWALKING;
								if (maxfrm != 20)
								{
									NewFrm(20);
								}
							}
							else
							{
								state = CLEMLEFTWALKING;
								if (maxfrm != 6)
								{
									NewFrm(6);
								}
							}
						}
						else if (state == CLEMRIGHTJUMP)
						{
							state = CLEMLEFTJUMP;
						}
					}
					else if (fx + 1920 > MinX)
					{
						fx -= FOGSPEED;
					}
				}
			}

			if (x < MaxX)
			{
				if (GetKeyState(VK_RIGHT) & 0x8000)            // 오른쪽
				{
					if (!movefog)
					{
						// 이동 속도 관련
						if (state == CLEMRIGHTSLOWWALKING)
						{
							if (!((frm >= 6 && frm <= 9) || (frm >= 15 && frm <= 19)))
							{
								x += SPEED;
							}
						}
						else
						{
							x += SPEED;
						}

						// 이동 중 천천히 걸을 경우
						if (!air && state != CLEMLEFTSLOWWALKING)
						{
							if (slow)
							{
								state = CLEMLEFTSLOWWALKING;
								if (maxfrm != 20)
								{
									NewFrm(20);
								}
							}
						}

						// 정지 중 천천히 걸을 경우
						if (!air && state != CLEMRIGHTWALKING)
						{
							if (slow)
							{
								state = CLEMRIGHTSLOWWALKING;
								if (maxfrm != 20)
								{
									NewFrm(20);
								}
							}
							else
							{
								state = CLEMRIGHTWALKING;
								if (maxfrm != 6)
								{
									NewFrm(6);
								}
							}
						}
						else if (state == CLEMLEFTJUMP)
						{
							state = CLEMRIGHTJUMP;
						}
					}
					else if (fx + 1920 < MaxX)
					{
						fx += FOGSPEED;
					}
				}
			}

			if (fy + 1080 > MinY && movefog)
			{
				if (GetKeyState(VK_UP) & 0x8000)				// 위
				{
					fy -= FOGSPEED;
				}
			}

			if (fy + 1180 < MaxY && movefog)
			{
				if (GetKeyState(VK_DOWN) & 0x8000)				// 아래
				{
					fy += FOGSPEED;
				}
			}
		}
	}

	// 프레임 관리
	void FrmControl()
	{
		if (frm == maxfrm - 1)
		{
			frm = 0;
		}
		else
		{
			frm++;
		}

		switch (state)
		{
		case CLEMRIGHT:
		case CLEMLEFT:
		case CLEMRIGHTSLOWWALKING:
		case CLEMLEFTSLOWWALKING:
			sound = MINSOUND;
			break;
		case CLEMRIGHTSHOT:
		case CLEMLEFTSHOT:
			sound = MAXSOUND;
			break;
		default:
			sound = SOUND;
			break;
		}
	}

	// 점프
	void Jump()
	{
		if (jump > 0)
		{
			y -= jump;
			jump -= 0.2;
			air = true;
			//NewFrm(1);
		}
	}

	// 중력
	void Gravity()
	{
		if (air)
		{
			if (gravity < 10)
			{
				gravity += 0.2;
				air = true;
				if (state == CLEMRIGHTWALKING || state == CLEMRIGHT || state == CLEMRIGHTSLOWWALKING)
				{
					state = CLEMRIGHTJUMP;
					NewFrm(1);
				}
				else if (state == CLEMLEFTWALKING || state == CLEMLEFT || state == CLEMLEFTSLOWWALKING)
				{
					state = CLEMLEFTJUMP;
					NewFrm(1);
				}
			}
		}
		else
		{
			gravity = 0;
			if (state == CLEMRIGHTJUMP)
			{
				state = CLEMRIGHT;
			}
			else if (state == CLEMLEFTJUMP)
			{
				state = CLEMLEFT;
			}
		}

		y += gravity;
	}
	// 생성자 소멸자
	Clem() :hp(50), x(800), y(100), frm(0), maxfrm(10), gravity(0), air(true), jump(0), state(0), slow(false), landcheck(false), bullet(0), energy(100), food(0)
	{
		for (int i = 0; i < 4; i++)
		{
			item[i] = false;
		}
	}
	~Clem() {}
	// getter, setter
#pragma region GetSet
	float GetHp() { return hp; }
	void SetHp(float m) { hp = m; }
	float GetX() { return x; }
	void SetX(float m) { x = m; }
	float GetY() { return y; }
	void SetY(float m) { y = m; }
	int GetFrm() { return frm; }
	void SetFrm(int m) { frm = m; }
	int GetMaxfrm() { return maxfrm; }
	void SetMaxfrm(int m) { maxfrm = m; }
	float GetGravity() { return gravity; }
	void SetGravity(float m) { gravity = m; }
	bool GetAir() { return air; }
	void SetAir(bool m) { air = m; }
	float GetJump() { return jump; }
	void SetJump(float m) { jump = m; }
	int GetState() { return state; }
	void SetState(int m) { state = m; }
	bool GetSlow() { return slow; }
	void SetSlow(bool m) { slow = m; }
	bool GetLandCheck() { return landcheck; }
	void SetLandCheck(bool m) { landcheck = m; }
	bool GetItem(int m) { return item[m]; }
	void SetItem(int m, bool m2) { item[m] = m2; }
	int GetBullet() { return bullet; }
	void SetBullet(int m) { bullet = m; }
	int GetEnergy() { return energy; }
	void SetEnergy(int m) { energy = m; }
	int GetFood() { return food; }
	void SetFood(int m) { food = m; }
#pragma endregion
private:
	int hp;		// 체력
	float x;	// 위치
	float y;
	int frm;	// 애니메이션 프레임
	int maxfrm;	// 애니메이션 최대 프레임
	float gravity;	// 중력
	bool air;	// 공중상태
	float jump;	// 점프량
	int state;	// 상태
	bool slow;	// 천천히 걷기
	bool landcheck;		// 땅 확인
	bool item[5];		// 아이템 확인
	int bullet;			// 총알
	int energy;			// 실제 체력
	int food;		// 음식량
};

Clem clem;

class NPC
{
public:
	void EndFrm()
	{
		if (state == BRIGHTATTACK)
		{
			if (frm == 7)
			{
				state = BRIGHT;
				NewFrm(6);
				for (int i = 0; i < 15; i++)
				{
					if (CheckPtInRectCustomPt(BlueBit[BRIGHTJUMP], x, y, 100, 100, WalkerBit[RIGHT], walker[i].GetX(), walker[i].GetY(), 100, 100))
					{
						if (walker[i].GetState() == RIGHT || walker[i].GetState() == RIGHTWALKING)
						{
							walker[i].SetState(RIGHTGUNDIE);
							walker[i].SetFrm(0);
							walker[i].SetMaxfrm(5);
							
						}
						else if (walker[i].GetState() == LEFT || walker[i].GetState() == LEFTWALKING)
						{
							walker[i].SetState(LEFTGUNDIE);
							walker[i].SetFrm(0);
							walker[i].SetMaxfrm(5);
						}
						hp -= 20;
						break;
					}
				}
			}
		}
		else if (state == BLEFTATTACK)
		{
			if (frm == 7)
			{
				state = BLEFT;
				NewFrm(6);
				for (int i = 0; i < 15; i++)
				{
					if (CheckPtInRectCustomPt(BlueBit[BRIGHTJUMP], x, y, 100, 100, WalkerBit[RIGHT], walker[i].GetX(), walker[i].GetY(), 100, 100))
					{
						if (walker[i].GetState() == RIGHT || walker[i].GetState() == RIGHTWALKING)
						{
							walker[i].SetState(RIGHTGUNDIE);
							walker[i].SetFrm(0);
							walker[i].SetMaxfrm(5);
						}
						else if (walker[i].GetState() == LEFT || walker[i].GetState() == LEFTWALKING)
						{
							walker[i].SetState(LEFTGUNDIE);
							walker[i].SetFrm(0);
							walker[i].SetMaxfrm(5);
						}
						hp -= 20;
						break;
					}
				}
			}
		}
		// 사망
		if (state == BRIGHTDIE || state == BLEFTDIE)
		{
			if (frm == 9)
			{
				die = true;
			}
		}
	}

	void Move()
	{
		if (team && (state != BRIGHTATTACK && state != BLEFTATTACK) && !scan && !die)
		{
			// 클렘 방향 쳐다보기
			if (clem.GetX() > x)
			{
				if (state == BLEFT)
				{
					state = BRIGHT;
				}
			}
			else
			{
				if (state == BRIGHT)
				{
					state = BLEFT;
				}
			}
			// 점프하기
			if (clem.GetY() < y - 10)
			{
				if (!air && !clem.GetAir())
				{
					jump = 10;
					air = true;
				}
			}
			// 클렘따라 이동하기
			if (clem.GetX() > x + 150)
			{
				if (state != BRIGHTWALKING && state != BRIGHTATTACK && state != BRIGHTJUMP && !air)
				{
					state = BRIGHTWALKING;
					NewFrm(6);
				}
			}
			else if (clem.GetX() < x - 150)
			{
				if (state != BLEFTWALKING && state != BLEFTATTACK && state != BLEFTJUMP && !air)
				{
					state = BLEFTWALKING;
					NewFrm(6);
				}
			}
			else if (clem.GetY() > y + 50)
			{
				if (clem.GetX() > x + 10 && state != BRIGHTWALKING)
				{
					state = BRIGHTWALKING;
					NewFrm(6);
				}
				else if (clem.GetX() < x - 10 && state != BLEFTWALKING)
				{
					state = BLEFTWALKING;
					NewFrm(6);
				}
			}
			else	// 가만히 있을 때
			{
				if (state == BLEFTWALKING)
				{
					state = BLEFT;
					NewFrm(6);
				}
				else if (state == BRIGHTWALKING)
				{
					state = BRIGHT;
					NewFrm(6);
				}
			}
		}

		switch (state)
		{
		case BRIGHTWALKING:
			x += 3;
			break;
		case BLEFTWALKING:
			x -= 3;
			break;
		case BRIGHTJUMP:
			if (clem.GetX() > x + 20)
			{
				x += 3;
			}
			break;
		case BLEFTJUMP:
			if (clem.GetX() < x - 20)
			{
				x -= 3;
			}
			break;
		}
	}

	void NewFrm(int m)
	{
		frm = 0;
		maxfrm = m;
	}

	void FrmControl()
	{
		if (!die)
		{
			if (frm == maxfrm - 1)
			{
				frm = 0;
			}
			else
			{
				frm++;
			}
		}
	}

	void Jump()
	{
		if (jump > 0)
		{
			y -= jump;
			jump -= 0.2;
			air = true;
		}
	}

	void Gravity()
	{
		if (air)
		{
			if (gravity < 10)
			{
				gravity += 0.2;
				air = true;
				if (state == BRIGHTWALKING || state == BRIGHT)
				{
					state = BRIGHTJUMP;
					NewFrm(1);
				}
				else if (state == BLEFTWALKING || state == BLEFT)
				{
					state = BLEFTJUMP;
					NewFrm(1);
				}
			}
		}
		else
		{
			gravity = 0;
			if (state == BRIGHTJUMP)
			{
				state = BRIGHT;
			}
			else if (state == BLEFTJUMP)
			{
				state = BLEFT;
			}
		}

		y += gravity;
	}

	NPC() :hp(40), x(-100), y(0), frm(0), maxfrm(6), gravity(0), air(true), jump(0), state(RIGHT), landcheck(false), team(false), scan(false), die(false) {}
	~NPC() {}

#pragma region GetSet
	float GetHp() { return hp; }
	void SetHp(float m) { hp = m; }
	float GetX() { return x; }
	void SetX(float m) { x = m; }
	float GetY() { return y; }
	void SetY(float m) { y = m; }
	int GetFrm() { return frm; }
	void SetFrm(int m) { frm = m; }
	int GetMaxfrm() { return maxfrm; }
	void SetMaxfrm(int m) { maxfrm = m; }
	float GetGravity() { return gravity; }
	void SetGravity(float m) { gravity = m; }
	bool GetAir() { return air; }
	void SetAir(bool m) { air = m; }
	float GetJump() { return jump; }
	void SetJump(float m) { jump = m; }
	int GetState() { return state; }
	void SetState(int m) { state = m; }
	bool GetLandCheck() { return landcheck; }
	void SetLandCheck(bool m) { landcheck = m; }
	bool GetTeam() { return team; }
	void SetTeam(bool m) { team = m; }
	bool GetScan() { return scan; }
	void SetScan(bool m) { scan = m; }
	bool GetDie() { return die; }
	void SetDie(bool m) { die = m; }
#pragma endregion
private:
	int hp;
	float x;
	float y;
	int frm;
	int maxfrm;
	float gravity;
	bool air;
	float jump;
	int state;
	bool landcheck;
	bool team = true;	// 팀 상태
	bool scan = false;	// 적 감지
	bool die = false;	// 죽음
};

NPC blue;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("The Dead");

// WINDOWS API
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;	//MAKEINTRESOURCE(IDR_MENU1);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPED | WS_SYSMENU,
		//CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, 1920, 1030,
		NULL, (HMENU)NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);

	hWndMain = hWnd;

	while (GetMessage(&Message, NULL, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return (int)Message.wParam;
}

// 에너지 표현
void Energy(HDC hMemDC)
{
	if (clem.GetState() == CLEMLEFTCAUGHT || clem.GetState() == CLEMRIGHTCAUGHT)
	{
		LifeBlt(hMemDC, clem.GetX(), clem.GetY(), EnergyBit, RGB(255, 0, 255), 2);
	}
}

// 대화 및 상호작용
void Talk()
{
	// 블루와 대화
	if ((page == 36 || page == 45) && blue.GetState() != BRIGHTDIE && blue.GetState() != BLEFTDIE)
	{
		if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], clem.GetX(), clem.GetY(), 100, 100, BlueBit[BRIGHTJUMP], blue.GetX(), blue.GetY(), 100, 100))
		{
			page++;
		}
	}
	else if (page == 36 || page == 45)
	{
		page = 52;
		clem.SetFood(clem.GetFood() + 1);
	}

	// 스위치 상호작용
	for (int i = 0; i < 50; i++)
	{
		if (obj[i].create && obj[i].kind == SWITCH)
		{
			if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], clem.GetX(), clem.GetY(), 100, 100, ObjectBit[SWITCH], obj[i].x, obj[i].y, 10, 70))
			{
				if (page == 64)
				{
					page++;
				}
				else
				{
					if (showfog)
					{
						showfog = false;
					}
					else
					{
						showfog = true;
					}
					SoundSelect(SWITCHSOUND);
				}
			}
		}
	}

	// 상자 상호작용
	for (int i = 0; i < 50; i++)
	{
		if (obj[i].create && obj[i].kind == CLOSEBOX)
		{
			if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], clem.GetX(), clem.GetY(), 100, 100, ObjectBit[SWITCH], obj[i].x, obj[i].y, 10, 70))
			{
				obj[i].kind = OPENBOX;
				obj[i].bitmap = ObjectBit[OPENBOX];
			}
		}
	}
}

// 현재 상태 저장
void Save()
{
	g_bullet = clem.GetBullet();
	g_energy = clem.GetEnergy();
	g_food = clem.GetFood();
	for (int i = 0; i < 4; i++)
	{
		g_item[i] = clem.GetItem(i);
	}
	g_blueenergy = blue.GetHp();
}

// 선택지 위
void FoodUp()
{
	POINT pt;

	pt.x = mousex;
	pt.y = mousey;

	if (clem.GetFood() > 0 && openbackpack)
	{
		for (int i = 0; i < 2; i++)
		{
			if (PtInRect(&foodsel[i].rt, pt))
			{
				switch (i)
				{
				case 0:
					foodsel[i].bitmap = FoodSelectBit[1];
					break;
				case 1:
					foodsel[i].bitmap = FoodSelectBit[3];
					break;
				}
			}
			else
			{
				switch (i)
				{
				case 0:
					foodsel[i].bitmap = FoodSelectBit[0];
					break;
				case 1:
					foodsel[i].bitmap = FoodSelectBit[2];
					break;
				}
			}
		}
	}
}

// 통조림 선택
void FoodSelect()
{
	POINT pt;

	pt.x = mousex;
	pt.y = mousey;

	RECT canrt = { 950,50, 1345, 495 };

	if (clem.GetFood() > 0 && openbackpack)
	{
		if (PtInRect(&canrt, pt) && !foodsel[1].create)
		{
			for (int i = 0; i < 2; i++)
			{
				foodsel[i].rt.left = pt.x;
				foodsel[i].rt.right = pt.x + 200;
				foodsel[i].rt.top = pt.y + (i * 50);
				foodsel[i].rt.bottom = pt.y + 50 + (i * 50);
			}
			if (blue.GetTeam())
			{
				foodsel[0].create = true;
			}
			foodsel[1].create = true;
		}
		else
		{
			for (int i = 0; i < 2; i++)
			{
				if (PtInRect(&foodsel[i].rt, pt) && foodsel[i].create)
				{
					foodsel[i].create = false;
					switch (i)
					{
					case 0:
						if (blue.GetHp() >= 80)
						{
							blue.SetHp(100);
						}
						else
						{
							blue.SetHp(blue.GetHp() + 40);
						}
						break;
					case 1:
						if (clem.GetEnergy() >= 80)
						{
							clem.SetEnergy(100);
						}
						else
						{
							clem.SetEnergy(clem.GetEnergy() + 40);
						}
						break;
					}
					clem.SetFood(clem.GetFood() - 1);
					SoundSelect(106);
				}
				else
				{
					foodsel[i].create = false;
				}
			}
		}
	}
}

// 체력 그리기
void LifeBlt(HDC hdc, int x, int y, HBITMAP hbitmap, COLORREF clrMask, int num)
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

	switch (num)
	{
	case 0:
		TransparentBlt(hdc, x, y, clem.GetEnergy(), 20, hMemDC, 0, 0, ptSize.x, ptSize.y, clrMask);
		break;
	case 1:
		TransparentBlt(hdc, x, y, blue.GetHp(), 20, hMemDC, 0, 0, ptSize.x, ptSize.y, clrMask);
		break;
	case 2:
		TransparentBlt(hdc, x, y, clem.GetHp() / 2, 5, hMemDC, 0, 0, ptSize.x, ptSize.y, clrMask);
		break;
	}

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
}

// 발사총알 관리
void BulletControl(HDC hMemDC)
{
	for (int i = 0; i < 5; i++)
	{
		if (bull[i].create)
		{
			bull[i].x += bull[i].sx;
			TransBltPt(hMemDC, bull[i].x, bull[i].y, BulletBit, RGB(255, 0, 255));
		}

	}
}

// 배경음악
void OnBGM(int num)
{
	if (!musicOn)
	{
		return;
	}

	mciSendCommand(dwID, MCI_CLOSE, MCI_NOTIFY, (DWORD)(LPVOID)&openBgm); // 일시정지 PAUSE, 완전 제거 CLOSE

	openBgm.lpstrDeviceType = "mpegvideo";// wave , "mpegvideo"; //mp3 형식
	switch (num)
	{
	case LOBBY:
		openBgm.lpstrElementName = "Sound//Lobby.mp3";
		break;
	case INGAME:
		openBgm.lpstrElementName = "Sound//InGame.mp3";
		break;
	case CLEAR:
		openBgm.lpstrElementName = "Sound//Clear.mp3";
		break;
	}
	mciSendCommand(0, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE, (DWORD)(LPVOID)&openBgm);
	dwID = openBgm.wDeviceID;
	mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, (DWORD)(LPVOID)&openBgm);
}

// 텍스트 시작
void TextGo()
{
	stop = true;
	walkerstop = true;
	showtext = true;
}

// 텍스트 끝
void TextStop()
{
	if (showtext)
	{
		stop = false;
		walkerstop = false;
		showtext = false;
	}
}

// 대화 내용
void ShowText(HDC hMemDC)
{
	switch (page)
	{
	case 0:
		TextGo();
		text = TEXT("안녕? 난 클레멘타인이야 내가 이곳에서 살아가는 법을 알려줄게");
		FontSet(hMemDC);
		break;
	case 1:
		text = TEXT("[X]키로 점프해봐");
		FontSet(hMemDC);
		break;
	case 2:
		TextStop();
		break;
	case 3:
		TextGo();
		text = TEXT("좋아, 이동은 [방향키]로 가능해");
		FontSet(hMemDC);
		break;
	case 4:
		TextStop();
		break;
	case 5:
		TextGo();
		text = TEXT("그리고 모든 상호작용은 [Z]키로 가능하지");
		FontSet(hMemDC);
		break;
	case 6:
		TextStop();
		break;
	case 7:
		TextGo();
		text = TEXT("매우 어두운 곳이야");
		FontSet(hMemDC);
		break;
	case 8:
		TextGo();
		text = TEXT("[V]키를 꾹 누르고 시야를 옮겨봐");
		FontSet(hMemDC);
		break;
	case 9:
		TextStop();
		break;
	case 10:
		TextGo();
		text = TEXT("워커야!");
		FontSet(hMemDC);
		break;
	case 11:
		text = TEXT("[SHIFT]키를 꾹 누르고 이동하면 소리를 적게내며 이동 할 수 있어");
		FontSet(hMemDC);
		break;
	case 12:
		text = TEXT("워커의 등 뒤로 적당한 거리가 되면 [Z]키로 암살 할 수 있지");
		FontSet(hMemDC);
		break;
	case 13:
		text = TEXT("혹시나 잡히더라도 [Z]키를 반복해서 누르면 에너지를 사용하여 반격 할 수 있으니까 걱정하지마");
		FontSet(hMemDC);
		break;
	case 14:
		TextStop();
		break;
	case 15:
		TextGo();
		text = TEXT("문이 잠겨있어");
		FontSet(hMemDC);
		break;
	case 16:
		text = TEXT("주변에서 열쇠를 찾아봐");
		FontSet(hMemDC);
		break;
	case 17:
		TextStop();
		break;
	case 18:
		TextGo();
		text = TEXT("얻은 아이템은 [C]키로 가방을 열어 확인해 볼 수 있어");
		FontSet(hMemDC);
		break;
	case 19:
		text = TEXT("열쇠는 어차피 다른 문에는 안 맞으니까 열고나면 버릴 거야");
		FontSet(hMemDC);
		break;
	case 20:
		TextStop();
		break;
	case 21:
		TextGo();
		text = TEXT("총알이 3발씩이나 있네?");
		FontSet(hMemDC);
		break;
	case 22:
		text = TEXT("총은 [A]키로 발사 할 수 있어");
		FontSet(hMemDC);
		break;
	case 23:
		text = TEXT("그러나 소음이 심해서 주변 워커들을 불러 모으니 신중하게 쏘는 게 좋을 거야");
		FontSet(hMemDC);
		break;
	case 24:
		TextStop();
		break;
	case 25:
		TextGo();
		text = TEXT("큰 소리를 내버린 거 같은데..");
		FontSet(hMemDC);
		break;
	case 26:
		TextStop();
		break;
	case 27:
		TextGo();
		if (clem.GetEnergy() >= 80)
		{
			text = TEXT("워커를 상대하지 않으니 에너지가 남아도는걸?");
		}
		else if (clem.GetEnergy() >= 20)
		{
			text = TEXT("으.. 배고팠는데 잘 됐네");
		}
		else
		{
			text = TEXT("지금 당장 먹지 않으면 좋지 않은 일이 생길 거야..");
		}
		FontSet(hMemDC);
		break;
	case 28:
		text = TEXT("음식은 가방에서 통조림을 [클릭]해서 먹을 수 있어");
		FontSet(hMemDC);
		break;
	case 29:
		TextStop();
		break;
	case 30:
		TextGo();
		text = TEXT("!");
		FontSet(hMemDC);
		break;
	case 31:
		TextGo();
		text = TEXT("생존자야");
		FontSet(hMemDC);
		break;
	case 32:
		TextGo();
		text = TEXT("어떻게 할지는 니 몫이야");
		FontSet(hMemDC);
		break;
	case 33:
		TextGo();
		text = TEXT("구해주고 말을 걸어 볼지");
		FontSet(hMemDC);
		break;
	case 34:
		TextGo();
		text = TEXT("총으로 죽이고 약탈을 하던지");
		FontSet(hMemDC);
		break;
	case 35:
		TextGo();
		text = TEXT("그냥 두고 가는 것도 너의 선택이야");
		FontSet(hMemDC);
		break;
	case 36:
		TextStop();
		break;
	case 37:
		TextGo();
		text = TEXT("저기.. 괜찮니?");
		FontSet(hMemDC);
		break;
	case 38:
		text = TEXT("구해주셔서 감사해요!");
		FontSet2(hMemDC);
		break;
	case 39:
		text = TEXT("혹시.. 저를 데려가 주시면 안 될까요?");
		FontSet2(hMemDC);
		break;
	case 40:
		text = TEXT("?");
		FontSet(hMemDC);
		break;
	case 41:
		text = TEXT("저 싸움도 잘해요! 데려가 주신다면 워커들을 제가 다 처리해 드릴게요!");
		FontSet2(hMemDC);
		break;
	case 42:
		text = TEXT("...");
		FontSet(hMemDC);
		break;
	case 43:
		text = TEXT("자. 데려가고 싶으면 다시 말을 걸고");
		FontSet(hMemDC);
		break;
	case 44:
		text = TEXT("그렇지 않다면 그냥 떠나면 돼");
		FontSet(hMemDC);
		break;
	case 45:
		TextStop();
		break;
	case 46:
		TextGo();
		text = TEXT("좋아, 따라와");
		FontSet(hMemDC);
		break;
	case 47:
		text = TEXT("혹시 먹을 거 좀 있어?");
		FontSet(hMemDC);
		break;
	case 48:
		text = TEXT("네, 배고파도 하나 아껴두고 있었어요..");
		FontSet2(hMemDC);
		break;
	case 49:
		text = TEXT("알았어. 가자");
		FontSet(hMemDC);
		break;
	case 50:
		text = TEXT("[S]키로 블루가 워커를 공격하지 않게 할 수 있어");
		FontSet(hMemDC);
		break;
	case 51:
		TextStop();
		blue.SetTeam(true);
		clem.SetFood(clem.GetFood() + 1);
		page = 55;
		break;
	case 52:
		TextGo();
		text = TEXT("...");
		FontSet(hMemDC);
		break;
	case 53:
		text = TEXT("식량 하나때문에..");
		FontSet(hMemDC);
		break;
	case 54:
		text = TEXT("이런 짓까지 하게 될 줄이야..");
		FontSet(hMemDC);
		break;
	case 55:
		if (!blue.GetTeam())
		{
			blue.SetX(-100);
		}
		TextStop();
		break;
	case 56:
		TextGo();
		text = TEXT("드디어 지하 밖으로 나온거같네");
		if (blue.GetTeam())
		{
			text = TEXT("후.. 드디어 밖이야");
		}
		FontSet(hMemDC);
		break;
	case 57:
		if (blue.GetTeam())
		{
			text = TEXT("이젠 안전할까요?");
			FontSet2(hMemDC);
		}
		else
		{
			text = TEXT("안전한 곳을 찾아봐야겠어");
			FontSet(hMemDC);
		}
		break;
	case 58:
		if (blue.GetTeam())
		{
			text = TEXT("아니, 계속 찾아봐야해");
			FontSet(hMemDC);
		}
		else
		{
			page++;
		}
		break;
	case 59:
		TextStop();
		break;
	case 60:
		TextGo();
		text = TEXT("하아.. 이제야 어두운 곳에서 탈출 한 줄 알았더니만..");
		if (blue.GetTeam())
		{
			text = TEXT("일단 건물로 들어왔는데..");
		}
		FontSet(hMemDC);
		break;
	case 61:
		text = TEXT("너무 어두워");
		FontSet(hMemDC);
		break;
	case 62:
		if (blue.GetTeam())
		{
			text = TEXT("건물인데 전등 스위치 같은 건 없을까요..?");
			FontSet2(hMemDC);
		}
		else
		{
			text = TEXT("분명 이 건물엔 전등 스위치같은게 있을 거야..");
			FontSet(hMemDC);
		}
		break;
	case 63:
		text = TEXT("흠..");
		FontSet(hMemDC);
		break;
	case 64:
		TextStop();
		break;
	case 65:
		TextGo();
		text = TEXT("제발.. 켜져라..!");
		FontSet(hMemDC);
		break;
	case 66:
		SoundSelect(SWITCHSOUND);
		showfog = false;
		page++;
		break;
	case 67:
		if (blue.GetTeam())
		{
			text = TEXT("오! 불이 켜졌어요!");
			FontSet2(hMemDC);
		}
		else
		{
			text = TEXT("좋아!");
			FontSet(hMemDC);
		}
		break;
	case 68:
		text = TEXT("다행히 여긴 일단 안전해보여");
		FontSet(hMemDC);
		break;
	case 69:
		TextStop();
		break;
	case 70:
		TextGo();
		text = TEXT("불부터 켜야 해");
		FontSet(hMemDC);
		break;
	case 71:
		TextStop();
		page = 64;
		break;
	case 72:
		TextGo();
		if (blue.GetTeam())
		{
			text = TEXT("불을 끄고 가시면 위험해요..!");
			FontSet2(hMemDC);
		}
		else
		{
			text = TEXT("불을 켜지 않고 가는건 미친짓이야..");
			FontSet(hMemDC);
		}
		break;
	case 73:
		TextStop();
		page = 69;
		break;
	}
}

// 대화창 블루
void FontSet2(HDC hdc)
{
	RECT grt;
	SetRect(&grt, 480, 850, 1120, 1000);

	TransBltPt(hdc, 0, 0, TextBoxBit[1], RGB(255, 0, 255));		// 텍스트 박스

	HFONT font, oldfont;

	// 내용
	SetBkMode(hdc, TRANSPARENT);
	font = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
		VARIABLE_PITCH | FF_ROMAN, "맑은 고딕");
	oldfont = (HFONT)SelectObject(hdc, font);
	SetTextColor(hdc, RGB(255, 255, 255));
	DrawText(hdc, text, -1, &grt, DT_WORDBREAK);
	DeleteObject(SelectObject(hdc, oldfont));

	text = TEXT("");
}

// 대화창 그리기 (클렘)
void FontSet(HDC hdc)
{
	RECT grt;
	SetRect(&grt, 480, 850, 1120, 1000);

	TransBltPt(hdc, 0, 0, TextBoxBit[0], RGB(255, 0, 255));		// 텍스트 박스

	HFONT font, oldfont;

	// 내용
	SetBkMode(hdc, TRANSPARENT);
	font = CreateFont(30, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
		VARIABLE_PITCH | FF_ROMAN, "맑은 고딕");
	oldfont = (HFONT)SelectObject(hdc, font);
	SetTextColor(hdc, RGB(255, 255, 255));
	DrawText(hdc, text, -1, &grt, DT_WORDBREAK);
	DeleteObject(SelectObject(hdc, oldfont));

	text = TEXT("");
}

// 효과음 선택
void SoundSelect(int m)
{
	switch (m)
	{
	case CLEMRIGHTSUDDENATTACK:
	case CLEMLEFTSUDDENATTACK:
		PlaySound("Sound//SuddenAttack.wav", NULL, SND_ASYNC);
		break;
	case CLEMRIGHTCOUNTERATTACK:
	case CLEMLEFTCOUNTERATTACK:
		PlaySound("Sound//CounterAttack.wav", NULL, SND_ASYNC);
		break;
	case CLEMRIGHTDIE:
	case CLEMLEFTDIE:
		PlaySound("Sound//Die.wav", NULL, SND_ASYNC);
		break;
	case CLEMRIGHTCAUGHT:
	case CLEMLEFTCAUGHT:
		PlaySound("Sound//Caught.wav", NULL, SND_ASYNC);
		break;
	case CLEMOPEN:
		PlaySound("Sound//OpenDoor.wav", NULL, SND_ASYNC);
		break;
	case CLEMRIGHTSHOT:
	case CLEMLEFTSHOT:
		PlaySound("Sound//Gun.wav", NULL, SND_ASYNC);
		break;
	case BACKPACKSOUND:
		PlaySound("Sound//BackPack.wav", NULL, SND_ASYNC);
		break;
	case KEY:
		PlaySound("Sound//Key.wav", NULL, SND_ASYNC);
		break;
	case ITEMGETSOUND:
		PlaySound("Sound//ItemGet.wav", NULL, SND_ASYNC);
		break;
	case BREAKINGSOUND:
		PlaySound("Sound//Breaking.wav", NULL, SND_ASYNC);
		break;
	case BLUEATTACKSOUND:
		PlaySound("Sound//BlueAttack.wav", NULL, SND_ASYNC);
		break;
	case BLUEATTACK2SOUND:
		PlaySound("Sound//BlueAttack2.wav", NULL, SND_ASYNC);
		break;
	case BLUEDIESOUND:
		PlaySound("Sound//BlueDie.wav", NULL, SND_ASYNC);
		break;
	case FOODSOUND:
		PlaySound("Sound//Food.wav", NULL, SND_ASYNC);
		break;
	case SWITCHSOUND:
		PlaySound("Sound//Switch.wav", NULL, SND_ASYNC);
		break;
	}
}

// 가방 열기
void BackPackOpen(HDC hMemDC)
{
	if (openbackpack)
	{
		TransBltPt(hMemDC, 0, 0, BackPackBit, RGB(0, 0, 0));
		for (int i = 1; i < 4; i++)
		{
			if (clem.GetItem(i))
			{
				TransBltPt(hMemDC, 400 + (i * 150), 500, PackItemBit[i], RGB(255, 0, 255));
			}
		}

		if (clem.GetBullet() > 0)
		{
			TransBltPt(hMemDC, 700, 400, PackItemBit[BULLET], RGB(255, 0, 255));
			RECT grt;
			TCHAR str[50];
			SetRect(&grt, 700, 400, 1120, 1000);

			HFONT font, oldfont;
			wsprintf(str, "남은 총알 : %d", clem.GetBullet());

			// 내용
			SetBkMode(hMemDC, TRANSPARENT);
			font = CreateFont(40, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
				VARIABLE_PITCH | FF_ROMAN, "맑은 고딕");
			oldfont = (HFONT)SelectObject(hMemDC, font);
			SetTextColor(hMemDC, RGB(255, 255, 255));
			DrawText(hMemDC, str, -1, &grt, DT_WORDBREAK);
			DeleteObject(SelectObject(hMemDC, oldfont));
		}

		if (clem.GetFood() > 0)
		{
			TransBltPt(hMemDC, 950, 50, PackItemBit[FOOD], RGB(255, 0, 255));
			RECT grt;
			TCHAR str[50];
			SetRect(&grt, 950, 50, 1500, 1000);

			HFONT font, oldfont;
			wsprintf(str, "남은 음식 : %d", clem.GetFood());

			// 내용
			SetBkMode(hMemDC, TRANSPARENT);
			font = CreateFont(40, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 3, 2, 1,
				VARIABLE_PITCH | FF_ROMAN, "맑은 고딕");
			oldfont = (HFONT)SelectObject(hMemDC, font);
			SetTextColor(hMemDC, RGB(255, 255, 255));
			DrawText(hMemDC, str, -1, &grt, DT_WORDBREAK);
			DeleteObject(SelectObject(hMemDC, oldfont));
		}
	}
}

// 워커 생성
void CreateWalker(float mx, float my, int state)
{
	walker[walkerindex].SetX(mx);
	walker[walkerindex].SetY(my);
	walker[walkerindex].SetFrm(0);
	walker[walkerindex].SetState(state);
	walker[walkerindex].SetMaxfrm(10);
	walker[walkerindex].SetCreate(true);
	walker[walkerindex].SetGravity(0);
	walker[walkerindex].SetAir(true);
	if (walkerindex != 29)
	{
		walkerindex++;
	}
	else
	{
		walkerindex = 0;
	}
}

// 워커 그리기
void DrawWalker(HDC hMemDC)
{
	for (int i = 0; i < 30; i++)
	{
		if (walker[i].GetCreate())
		{
			MoveTransBltPt(hMemDC, walker[i].GetX(), walker[i].GetY(), WalkerBit[walker[i].GetState()], RGB(255, 0, 255), walker[i].GetFrm(), walker[i].GetMaxfrm());
		}
	}
}

// 워커 이동
void WalkerControl()
{
	for (int i = 0; i < 30; i++)
	{
		if (walker[i].GetCreate())	// 워커 이동
		{
			if (!walkerstop)
			{
				switch (walker[i].GetState())
				{
				case RIGHTWALKING:
					walker[i].Move(WalkerSpeed);
					break;
				case LEFTWALKING:
					walker[i].Move(-WalkerSpeed);
					break;
				}
			}
		}
		walker[i].MXSetting();		// 워커 범위 설정
		walker[i].Gravity();		// 워커 중력
	}
}

// 스테이지
void Stage()
{
	// 스테이지 이동 전 오브젝트 초기화
	for (int i = 0; i < 50; i++)
	{
		obj[i].create = false;
	}
	// 워커 초기화
	for (int i = 0; i < 30; i++)
	{
		walker[i].SetCreate(false);
	}
	stage++;

	// 불러오기
	clem.SetBullet(g_bullet);
	clem.SetEnergy(g_energy);
	clem.SetFood(g_food);
	for (int i = 0; i < 4; i++)
	{
		g_item[i] = clem.GetItem(i);
	}
	blue.SetHp(g_blueenergy);

	// 배경
	if (stage > 12)
	{
		background = 4;
	}
	else if (stage > 10)
	{
		background = 2;
	}
	else if (stage > 4)
	{
		background = 1;
	}

	// 스테이지 세팅
	switch (stage)
	{
	case 1:
		clem.SetX(150);
		clem.SetY(800);
		// 땅
		CreateObject(ObjectBit[LAND], 120, 925, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 120, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 420, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 720, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1020, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1320, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1520, 800, 300, 10, LAND);
		// 문
		CreateObject(ObjectBit[OPENDOOR], 900, 680, 70, 120, OPENDOOR);
		break;
	case 2:
		clem.SetX(150);
		clem.SetY(650);
		// 땅
		CreateObject(ObjectBit[LAND], 120, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 420, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1050, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 720, 700, 300, 10, LAND);
		// 문
		CreateObject(ObjectBit[OPENDOOR], 1200, 680, 70, 120, OPENDOOR);
		break;
	case 3:
		clem.SetX(450);
		clem.SetY(650);
		// 땅
		CreateObject(ObjectBit[LAND], 420, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 720, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1050, 800, 300, 10, LAND);
		// 문
		CreateObject(ObjectBit[OPENDOOR], 1200, 680, 70, 120, OPENDOOR);
		// 워커
		CreateWalker(950, 650, RIGHT);
		break;
	case 4:
		clem.SetX(150);
		clem.SetY(550);
		clem.SetItem(KEY, false);
		// 땅
		CreateObject(ObjectBit[LAND], 120, 700, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 420, 700, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 420, 600, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 420, 500, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 120, 400, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 650, 500, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 800, 500, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1000, 500, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1200, 500, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1200, 600, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1200, 700, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1200, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1520, 400, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 120, 900, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 420, 900, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 900, 900, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1100, 900, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1300, 900, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1500, 900, 300, 10, LAND);
		// 문
		CreateObject(ObjectBit[DOOR], 900, 380, 70, 120, DOOR);
		// 워커들
		CreateWalker(370, 250, RIGHT);
		CreateWalker(1740, 250, LEFT);
		CreateWalker(1680, 600, RIGHT);
		// 열쇠
		CreateObject(ObjectBit[KEY], 1730, 830, 50, 70, KEY);
		break;
	case 5:
		clem.SetX(150);
		clem.SetY(550);
		clem.SetItem(KEY, false);
		// 땅
		CreateObject(ObjectBit[LAND], 120, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 420, 850, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 720, 850, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1020, 850, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1320, 800, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1500, 750, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1500, 650, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1150, 550, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 850, 550, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 550, 550, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 250, 550, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 120, 450, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 470, 320, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 900, 250, 300, 10, LAND);
		CreateObject(ObjectBit[LAND], 1300, 200, 300, 10, LAND);
		// 문
		CreateObject(ObjectBit[DOOR], 850, 730, 70, 120, DOOR);
		// 워커들
		CreateWalker(200, 250, RIGHT);
		CreateWalker(300, 250, RIGHT);
		CreateWalker(400, 250, RIGHT);
		CreateWalker(500, 250, RIGHT);
		// 열쇠
		CreateObject(ObjectBit[KEY], 1600, 150, 50, 70, KEY);
		// 총알
		CreateObject(ObjectBit[BULLET], 1700, 600, 50, 70, BULLET);
		break;
	case 6:
		clem.SetX(150);
		clem.SetY(100);
		// 땅
		CreateObject(ObjectBit[DARKLAND], 120, 200, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 430, 300, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 730, 300, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1030, 300, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1330, 300, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 120, 400, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1500, 400, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 510, 500, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 810, 500, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1110, 500, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 350, 600, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1325, 600, 300, 10, LAND);
		// 소리 오브젝트
		CreateObject(ObjectBit[PILLAR], 1480, 550, 40, 70, PILLAR);
		// 문
		CreateObject(ObjectBit[DOOR], 300, 80, 70, 120, DOOR);
		// 워커들
		CreateWalker(900, 200, RIGHT);
		CreateWalker(1200, 200, LEFT);
		CreateWalker(130, 300, LEFT);
		CreateWalker(400, 500, LEFT);
		CreateWalker(1350, 400, RIGHT);
		// 쇠 지렛대
		CreateObject(ObjectBit[STICK], 1550, 550, 50, 70, STICK);
		// 총알
		CreateObject(ObjectBit[BULLET], 1000, 250, 50, 70, BULLET);
		break;
	case 7:
		clem.SetX(1700);
		clem.SetY(600);
		// 땅
		CreateObject(ObjectBit[DARKLAND], 1500, 700, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1200, 600, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1500, 500, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1200, 400, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1500, 300, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1200, 160, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 900, 150, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 600, 150, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 120, 300, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 120, 800, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 420, 800, 300, 10, LAND);
		// 소리 오브젝트
		CreateObject(ObjectBit[STONE], 1450, 150, 20, 30, PILLAR);
		// 문
		CreateObject(ObjectBit[DOOR], 300, 680, 70, 120, DOOR);
		// 워커들
		CreateWalker(1700, 350, RIGHT);
		CreateWalker(1200, 250, LEFT);
		CreateWalker(1700, 150, RIGHT);
		CreateWalker(700, 50, LEFT);
		CreateWalker(150, 200, RIGHT);
		CreateWalker(150, 700, RIGHT);
		CreateWalker(550, 700, RIGHT);
		// 음식
		CreateObject(ObjectBit[FOOD], 1180, 350, 40, 70, FOOD);
		CreateObject(ObjectBit[FOOD], 1780, 450, 40, 70, FOOD);
		break;
	case 8:
		clem.SetX(950);
		clem.SetY(100);
		// 땅
		CreateObject(ObjectBit[DARKLAND], 120, 250, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 420, 250, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 720, 250, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1020, 250, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1500, 250, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 120, 400, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 420, 400, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 900, 400, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1200, 400, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1500, 400, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 160, 550, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 420, 550, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 720, 550, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1020, 550, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1320, 550, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 120, 700, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 600, 700, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 900, 700, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1200, 700, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1500, 700, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 420, 700, 300, 10, LAND);
		// 소리 오브젝트
		CreateObject(ObjectBit[PILLAR], 950, 150, 40, 70, PILLAR);
		// 문
		CreateObject(ObjectBit[DOOR], 470, 580, 70, 120, DOOR);
		// 워커들
		CreateWalker(150, 150, RIGHT);
		CreateWalker(250, 150, RIGHT);
		CreateWalker(1750, 150, LEFT);
		CreateWalker(1550, 150, LEFT);

		CreateWalker(170, 300, RIGHT);
		CreateWalker(250, 300, RIGHT);
		CreateWalker(1730, 300, LEFT);
		CreateWalker(1680, 300, LEFT);

		CreateWalker(210, 450, RIGHT);
		CreateWalker(280, 450, RIGHT);

		CreateWalker(120, 600, RIGHT);
		CreateWalker(500, 600, RIGHT);
		CreateWalker(900, 600, RIGHT);
		break;
	case 9:
		clem.SetX(150);
		clem.SetY(600);
		blue.SetX(200);
		blue.SetY(200);
		// 땅
		CreateObject(ObjectBit[DARKLAND], 120, 750, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 420, 650, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 720, 650, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1020, 650, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 1320, 550, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 1500, 450, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 120, 350, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 420, 350, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 720, 350, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1020, 350, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1320, 350, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 1500, 250, 300, 10, LAND);
		// 문
		CreateObject(ObjectBit[DOOR], 1700, 130, 70, 120, DOOR);
		// 워커들
		CreateWalker(400, 150, RIGHT);
		CreateWalker(750, 150, LEFT);
		// 음식
		CreateObject(ObjectBit[FOOD], 1000, 600, 40, 70, FOOD);
		// 총알
		CreateObject(ObjectBit[BULLET], 1400, 500, 40, 70, BULLET);
		break;
	case 10:
		clem.SetX(1700);
		clem.SetY(650);
		if (blue.GetTeam())
		{
			blue.SetX(1750);
			blue.SetY(650);
		}
		// 땅
		CreateObject(ObjectBit[DARKLAND], 1500, 800, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 900, 700, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1200, 700, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 300, 600, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 600, 600, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 120, 465, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 420, 350, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 720, 350, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1020, 350, 300, 10, LAND);

		CreateObject(ObjectBit[DARKLAND], 1320, 300, 300, 10, LAND);
		CreateObject(ObjectBit[DARKLAND], 1500, 250, 300, 10, LAND);
		// 문
		CreateObject(ObjectBit[DOOR], 1700, 130, 70, 120, DOOR);
		// 워커들
		CreateWalker(1300, 550, LEFT);
		CreateWalker(950, 550, RIGHT);

		CreateWalker(550, 460, LEFT);
		CreateWalker(300, 460, LEFT);

		CreateWalker(200, 300, LEFT);

		CreateWalker(1000, 200, RIGHT);
		break;
	case 11:
		clem.SetX(120);
		clem.SetY(600);
		if (blue.GetTeam())
		{
			blue.SetX(120);
			blue.SetY(600);
		}
		if (page == 55)
		{
			page++;
		}
		// 땅
		CreateObject(ObjectBit[ROAD], 120, 750, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 320, 725, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 520, 700, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 720, 675, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 960, 650, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 1290, 625, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1490, 600, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 1600, 550, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 1490, 500, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1290, 500, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 1090, 475, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 890, 475, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 690, 475, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 490, 475, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 490, 425, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 290, 425, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 120, 400, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 120, 350, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 120, 300, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 120, 250, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 690, 375, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 890, 325, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1090, 275, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 1290, 225, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1490, 225, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1600, 225, 200, 10, LAND);
		// 문
		CreateObject(ObjectBit[DOOR], 1650, 105, 70, 120, DOOR);
		// 워커들
		CreateWalker(1350, 300, RIGHT);
		CreateWalker(850, 300, RIGHT);

		CreateWalker(1550, 50, RIGHT);
		CreateWalker(250, 50, RIGHT);
		// 음식
		CreateObject(ObjectBit[FOOD], 150, 200, 40, 70, FOOD);
		break;
	case 12:
		clem.SetX(800);
		clem.SetY(680);
		if (blue.GetTeam())
		{
			blue.SetX(800);
			blue.SetY(680);
		}
		// 땅
		CreateObject(ObjectBit[ROAD], 120, 800, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 400, 800, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 520, 800, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 720, 800, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 920, 800, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1120, 800, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1320, 800, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1520, 800, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1600, 780, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 120, 625, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 400, 625, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 520, 625, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 720, 625, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 920, 625, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1120, 625, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1300, 625, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 120, 300, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 420, 350, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 720, 400, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1020, 450, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1300, 500, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 420, 250, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 720, 200, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1020, 150, 200, 10, LAND);
		CreateObject(ObjectBit[ROAD], 1300, 120, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 1600, 660, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 1725, 100, 200, 10, LAND);

		CreateObject(ObjectBit[ROAD], 1600, 100, 200, 10, LAND);
		// 문
		CreateObject(ObjectBit[DOOR], 1720, -20, 70, 120, DOOR);
		// 워커들
		CreateWalker(420, 680, RIGHT);
		CreateWalker(460, 680, RIGHT);

		CreateWalker(150, 680, RIGHT);
		CreateWalker(180, 680, RIGHT);

		CreateWalker(150, 680, RIGHT);
		CreateWalker(180, 680, RIGHT);

		CreateWalker(500, 500, RIGHT);
		CreateWalker(600, 500, RIGHT);
		CreateWalker(650, 500, RIGHT);
		CreateWalker(750, 500, RIGHT);
		CreateWalker(850, 500, RIGHT);
		CreateWalker(950, 500, RIGHT);

		CreateWalker(1100, 500, LEFT);
		CreateWalker(1200, 500, LEFT);
		CreateWalker(1300, 500, LEFT);

		CreateWalker(1700, 0, LEFT);
		// 소리 오브젝트
		CreateObject(ObjectBit[STONE], 900, 790, 40, 70, PILLAR);
		break;
	case 13:
		if (page == 59)
		{
			page++;
		}
		clem.SetX(120);
		clem.SetY(700);
		if (blue.GetTeam())
		{
			blue.SetX(120);
			blue.SetY(700);
		}
		// 땅
		CreateObject(ObjectBit[STONE], 0, 825, 1920, 20, LAND);
		// 문
		CreateObject(ObjectBit[DOOR], 1720, 700, 70, 120, DOOR);
		// 스위치
		CreateObject(ObjectBit[SWITCH], 950, 750, 50, 120, SWITCH);

		CreateObject(ObjectBit[CLOSEBOX], 1050, 750, 50, 120, CLOSEBOX);
		break;
	case CLEARSTAGE:
		OnBGM(CLEAR);
		break;
	}
}

// 오브젝트 생성
void CreateObject(HBITMAP bitmap, float mx, float my, float mx2, float my2, int kind)
{
	obj[objindex].bitmap = bitmap;
	obj[objindex].x = mx;
	obj[objindex].y = my;
	obj[objindex].mx = mx2;
	obj[objindex].my = my2;
	obj[objindex].create = true;
	obj[objindex].kind = kind;
	if (objindex != 49)
	{
		objindex++;
	}
	else
	{
		objindex = 0;
	}
}

// 오브젝트 그리기
void DrawObject(HDC hMemDC)
{
	for (int i = 0; i < 50; i++)
	{
		if (obj[i].create)
		{
			TransBltPt(hMemDC, obj[i].x, obj[i].y, obj[i].bitmap, RGB(255, 0, 255));
		}
	}
}

// 충돌 확인
void Collision()
{
	// 클렘 <-> 오브젝트
	for (int i = 0; i < 50; i++)
	{
		if (obj[i].create)
		{
			if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], clem.GetX() + 25, clem.GetY() + 80, 40, 20, obj[i].bitmap, obj[i].x, obj[i].y, obj[i].mx, obj[i].my))
			{
				switch (obj[i].kind)
				{
				case LAND:
					if (clem.GetJump() - clem.GetGravity() <= 0)
					{
						clem.SetAir(false);
						clem.SetJump(0);
						clem.SetLandCheck(true);		// 땅에 있다
						if (clem.GetState() == CLEMRIGHTJUMP || clem.GetState() == CLEMLEFTJUMP)
						{
							clem.NewFrm(10);
							if (page == 2)
							{
								page++;
							}
						}
					}
					break;
				case STICK:
					if (page == 20)
					{
						page++;
					}
					obj[i].create = false;
					clem.SetItem(STICK, true);
					SoundSelect(101);
					break;
				case DOOR:
					if (page == 14)
					{
						page++;
					}
					break;
				case OPENDOOR:
					if (page == 4)
					{
						page++;
					}
					break;
				case KEY:
					clem.SetItem(KEY, true);
					obj[i].create = false;
					SoundSelect(ITEMGETSOUND);
					if (page == 17)
					{
						page++;
					}
					if (page == 14)
					{
						page = 18;
					}
					break;
				case BULLET:
					clem.SetBullet(clem.GetBullet() + 1);
					if (page == 20)
					{
						clem.SetBullet(clem.GetBullet() + 2);
						page++;
					}
					else if (stage == 5)
					{
						clem.SetBullet(clem.GetBullet() + 2);
					}
					obj[i].create = false;
					SoundSelect(ITEMGETSOUND);
					break;
				case PILLAR:
					obj[i].create = false;
					SoundSelect(BREAKINGSOUND);
					sound = MAXSOUND;
					if (page == 24)
					{
						page++;
					}
					break;
				case FOOD:
					obj[i].create = false;
					clem.SetFood(clem.GetFood() + 1);
					SoundSelect(ITEMGETSOUND);
					if (page == 26)
					{
						page++;
					}
					break;
				}
			}
		}
	}
	if (!clem.GetLandCheck())
	{
		clem.SetAir(true);
	}
	else
	{
		clem.SetLandCheck(false);
	}

	// 블루 <-> 오브젝트
	for (int i = 0; i < 50; i++)
	{
		if (obj[i].create)
		{
			if (CheckPtInRectCustomPt(BlueBit[BRIGHTJUMP], blue.GetX() + 25, blue.GetY() + 80, 40, 20, obj[i].bitmap, obj[i].x, obj[i].y, obj[i].mx, obj[i].my))
			{
				switch (obj[i].kind)
				{
				case LAND:
					if (blue.GetJump() - blue.GetGravity() <= 0)
					{
						blue.SetAir(false);
						blue.SetJump(0);
						blue.SetLandCheck(true);		// 땅에 있다
						if (blue.GetState() == BRIGHTJUMP || blue.GetState() == BLEFTJUMP)
						{
							blue.NewFrm(6);
						}
					}
					break;
				}
			}
		}
	}
	if (!blue.GetLandCheck())
	{
		blue.SetAir(true);
	}
	else
	{
		blue.SetLandCheck(false);
	}

	// 워커 <-> 오브젝트
	for (int i = 0; i < 30; i++)
	{
		if (walker[i].GetCreate())
		{
			for (int j = 0; j < 50; j++)
			{
				if (obj[j].kind == LAND && obj[j].create)
				{
					if (CheckPtInRectCustomPt(WalkerBit[0], walker[i].GetX() + 25, walker[i].GetY() + 90, 40, 10, obj[j].bitmap, obj[j].x, obj[j].y, obj[j].mx, obj[j].my))
					{
						walker[i].SetAir(false);
						walker[i].SetGravity(0);
						walker[i].SetLandCheck(true);
					}
				}
			}
		}
		if (!walker[i].GetLandCheck())
		{
			walker[i].SetAir(true);
		}
		else
		{
			walker[i].SetLandCheck(false);
		}
	}

	// 워커 <-> 사운드
	for (int i = 0; i < 30; i++)
	{
		if (walker[i].GetCreate())
		{
			// 워커 <-> 사운드
			if (CheckPtInRectCustomPt(WalkerBit[0], walker[i].GetX(), walker[i].GetY(), 100, 100,
				ClemBit[0], clem.GetX() + 50 - sound, clem.GetY() + 50 + (-sound / 4), sound * 2, (sound / 4) * 2))
			{
				if (walker[i].GetX() < clem.GetX() && (DEFAULT_WALKER))
				{
					walker[i].SetState(RIGHTWALKING);
				}
				else if (DEFAULT_WALKER)
				{
					walker[i].SetState(LEFTWALKING);
				}
			}

			// 워커 시야 <-> 사운드
			if (CheckPtInRectCustomPt(WalkerBit[0], walker[i].GetX() + walker[i].GetSight(), walker[i].GetY(), 500, 100,
				ClemBit[0], clem.GetX(), clem.GetY(), 100, 100))
			{
				if (walker[i].GetX() < clem.GetX() && (walker[i].GetState() == RIGHT || walker[i].GetState() == LEFT || walker[i].GetState() == RIGHTWALKING
					|| walker[i].GetState() == LEFTWALKING))
				{
					walker[i].SetState(RIGHTWALKING);
				}
				else if (walker[i].GetState() == RIGHT || walker[i].GetState() == LEFT || walker[i].GetState() == RIGHTWALKING
					|| walker[i].GetState() == LEFTWALKING)
				{
					walker[i].SetState(LEFTWALKING);
				}
			}

		}
	}

	// 클렘 <-> 워커
	for (int i = 0; i < 30; i++)
	{
		if (walker[i].GetCreate() && !die && !dead)
		{
			if (CheckPtInRectCustomPt(ClemBit[CLEMRIGHTJUMP], clem.GetX() + 40, clem.GetY() + 30, 20, 70,
				WalkerBit[0], walker[i].GetX() + walker[i].GetMX(), walker[i].GetY() + 20, 20, 80))
			{
				movefog = false;
				// 잡히지 않은상태 (왼)
				if (walker[i].GetX() < clem.GetX() && (DEFAULT_WALKER) && (NOTFIGHTING_CLEM))
				{
					clem.SetX(walker[i].GetX() + 20);
					clem.NewFrm(6);
					clem.SetState(CLEMLEFTCAUGHT);
					walker[i].SetCreate(false);
					stop = true;
					openbackpack = false;
					SoundSelect(CLEMRIGHTCAUGHT);
				}	// (오른)
				else if (walker[i].GetX() > clem.GetX() && (DEFAULT_WALKER) && (NOTFIGHTING_CLEM))
				{
					clem.SetX(walker[i].GetX() - 20);
					clem.NewFrm(6);
					clem.SetState(CLEMRIGHTCAUGHT);
					walker[i].SetCreate(false);
					stop = true;
					openbackpack = false;
					SoundSelect(CLEMRIGHTCAUGHT);
				}	// 잡힌 상태 (오)
				else if (RIGHT_FIGHTING_CLEM)
				{
					clem.NewFrm(14);
					clem.SetState(CLEMRIGHTDIE);
					clem.SetHp(100);
					walker[i].SetCreate(false);
					SoundSelect(CLEMRIGHTDIE);
				}	// 잡힌 상태 (왼)
				else if (LEFT_FIGHTING_CLEM)
				{
					clem.NewFrm(14);
					clem.SetState(CLEMLEFTDIE);
					clem.SetHp(100);
					walker[i].SetCreate(false);
					SoundSelect(CLEMRIGHTDIE);
				}
				KillTimer(hWndMain, 2);
			}
		}
	}

	// 블루 감지 제거 후
	blue.SetScan(false);
	// 블루 감지범위 <-> 워커
	for (int i = 0; i < 30; i++)
	{
		if (walker[i].GetCreate() && blue.GetTeam() && blueattackstate)
		{
			if (CheckPtInRectCustomPt(BlueBit[BRIGHTJUMP], blue.GetX() - 300, blue.GetY() + 30, 600, 70,
				WalkerBit[0], walker[i].GetX() + walker[i].GetMX(), walker[i].GetY() + 20, 20, 80))
			{
				if (blue.GetState() != BRIGHTJUMP && blue.GetState() != BLEFTJUMP && blue.GetHp() > 0)
				{
					if (!blue.GetScan() && (DEFAULT_WALKER) && (NOTFIGHTING_BLUE))
					{
						blue.SetScan(true);
					}
					if ((walker[i].GetX() > blue.GetX() || !blue.GetScan()) && (DEFAULT_WALKER) && (NOTFIGHTING_BLUE))
					{
						blue.SetState(BRIGHTWALKING);
					}
					else if ((walker[i].GetX() < blue.GetX() || !blue.GetScan()) && (DEFAULT_WALKER) && (NOTFIGHTING_BLUE))
					{
						blue.SetState(BLEFTWALKING);
					}
				}
			}
		}
	}

	// 블루 <-> 워커
	for (int i = 0; i < 30; i++)
	{
		if (walker[i].GetCreate() && blue.GetTeam() && blueattackstate)
		{
			if (CheckPtInRectCustomPt(BlueBit[BRIGHTJUMP], blue.GetX(), blue.GetY() + 30, 100, 70,
				WalkerBit[0], walker[i].GetX() + walker[i].GetMX(), walker[i].GetY() + 20, 20, 80) && !blue.GetAir())
			{
				int rnd = rand() % 2;
				if (walker[i].GetX() < blue.GetX() && (DEFAULT_WALKER) && (NOTFIGHTING_BLUE) && blue.GetState() != BLEFTDIE)
				{
					if (blue.GetHp() <= 0)
					{
						blue.NewFrm(10);
						blue.SetState(BLEFTDIE);
						SoundSelect(105);
						break;
					}
					blue.NewFrm(8);
					blue.SetState(BLEFTATTACK);
					switch (rnd)
					{
					case 0:
						SoundSelect(103);
						break;
					case 1:
						SoundSelect(104);
						break;
					}
				}
				else if (walker[i].GetX() > blue.GetX() && (DEFAULT_WALKER) && (NOTFIGHTING_BLUE) && blue.GetState() != RIGHTDIE)
				{
					if (blue.GetHp() <= 0 && blue.GetState() != BRIGHT)
					{
						blue.NewFrm(10);
						blue.SetState(BRIGHTDIE);
						SoundSelect(105);
						break;
					}
					blue.NewFrm(8);
					blue.SetState(BRIGHTATTACK);
					switch (rnd)
					{
					case 0:
						SoundSelect(103);
						break;
					case 1:
						SoundSelect(104);
						break;
					}
				}
			}
		}
	}

	// 총알 <-> 워커
	for (int i = 0; i < 5; i++)
	{
		if (bull[i].create)
		{
			for (int j = 0; j < 30; j++)
			{
				if (walker[j].GetCreate())
				{
					if (CheckPtInRectCustomPt(BulletBit, bull[i].x, bull[i].y, 10, 10,
						WalkerBit[0], walker[j].GetX() + 30, walker[j].GetY() + 10, 50, 90))
					{
						bull[i].create = false;
						walker[j].SetFrm(0);
						walker[j].SetMaxfrm(5);
						if (walker[j].GetState() == RIGHT || walker[j].GetState() == RIGHTWALKING)
						{
							walker[j].SetState(RIGHTGUNDIE);
						}
						else if (walker[j].GetState() == LEFT || walker[j].GetState() == LEFTWALKING)
						{
							walker[j].SetState(LEFTGUNDIE);
						}
					}
				}
			}
		}
	}

	// 총알 <-> 블루
	for (int i = 0; i < 5; i++)
	{
		if (bull[i].create)
		{
			for (int j = 0; j < 30; j++)
			{
				if (!blue.GetTeam())
				{
					if (CheckPtInRectCustomPt(BulletBit, bull[i].x, bull[i].y, 10, 10,
						BlueBit[BRIGHTJUMP], blue.GetX() + 30, blue.GetY() + 30, 50, 70))
					{
						bull[i].create = false;
						blue.NewFrm(10);
						SoundSelect(105);
						if (blue.GetState() == RIGHT)
						{
							blue.SetState(BRIGHTDIE);
						}
						else if (walker[j].GetState() == LEFT)
						{
							blue.SetState(BLEFTDIE);
						}
					}
				}
			}
		}
	}
}

// 안개 관리
void FogControl(HDC hMemDC)
{
	if (showfog && (stage == 2 || stage == 4))
	{
		TransBltPt(hMemDC, fx, fy, FogBit, RGB(255, 0, 255));
	}
	else if (stage == 13 && showfog)
	{
		TransBltPt(hMemDC, fx, fy, SmallFogBit, RGB(255, 0, 255));
	}

	if (showfog && !movefog && stage != 13)
	{
		switch (clem.GetState())
		{
		case CLEMRIGHT:
		case CLEMRIGHTJUMP:
		case CLEMRIGHTWALKING:
		case CLEMRIGHTSLOWWALKING:
		case CLEMRIGHTCAUGHT:
		case CLEMRIGHTDIE:
		case CLEMRIGHTSUDDENATTACK:
		case CLEMRIGHTCOUNTERATTACK:
			if (fx - 300 > clem.GetX() - 1750)
			{
				fx = clem.GetX() - 1750;
			}
			else if (fx + 550 < clem.GetX() - 1750)
			{
				fx = clem.GetX() - 1750;
			}

			if (fx > clem.GetX() - 1750)
			{
				fx -= 10;
			}
			else if (fx < clem.GetX() - 1760)
			{
				fx += 10;
			}

			if (fy - 100 > clem.GetY() - 1150)
			{
				fy = clem.GetY() - 1150;
			}
			else if (fy + 100 < clem.GetY() - 1160)
			{
				fy = clem.GetY() - 1160;
			}

			if (fy > clem.GetY() - 1150)
			{
				fy -= 10;
			}
			else if (fy < clem.GetY() - 1160)
			{
				fy += 10;
			}
			break;
		case CLEMLEFT:
		case CLEMLEFTJUMP:
		case CLEMLEFTWALKING:
		case CLEMLEFTSLOWWALKING:
		case CLEMLEFTCAUGHT:
		case CLEMLEFTDIE:
		case CLEMLEFTSUDDENATTACK:
		case CLEMLEFTCOUNTERATTACK:
			if (fx - 550 > clem.GetX() - 2250)
			{
				fx = clem.GetX() - 2250;
			}
			else if (fx + 300 < clem.GetX() - 2250)
			{
				fx = clem.GetX() - 2250;
			}

			if (fx > clem.GetX() - 2250)
			{
				fx -= 10;
			}
			else if (fx < clem.GetX() - 2260)
			{
				fx += 10;
			}

			if (fy - 100 < clem.GetY() - 1150)
			{
				fy = clem.GetY() - 1150;
			}
			else if (fy + 100 > clem.GetY() - 1160)
			{
				fy = clem.GetY() - 1160;
			}

			if (fy > clem.GetY() - 1150)
			{
				fy -= 10;
			}
			else if (fy < clem.GetY() - 1160)
			{
				fy += 10;
			}
			break;
		}
	}
	else if(stage == 13)
	{
		fx = clem.GetX() - 1990;
		fy = clem.GetY() - 1100;
	}
}

// 더블버퍼링
void OnTimer()
{
	RECT crt;
	HDC hdc, hMemDC;
	HBITMAP OldBit;

	GetClientRect(hWndMain, &crt);
	hdc = GetDC(hWndMain);

	if (!hBit)
	{
		hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
	}

	hMemDC = CreateCompatibleDC(hdc);
	OldBit = (HBITMAP)SelectObject(hMemDC, hBit);

	if (!AllStop)		// 게임이 정지상태가 아닐 시에만
	{

		// 배경 그리기
		DrawBitmapPt(hMemDC, 0, 0, BackGroundBit[background]);

		// 사운드 충돌 박스
		//Rectangle(hMemDC, clem.GetX() + 50 - sound, clem.GetY() + 50 + (-sound / 4), clem.GetX() + 50 - sound + (sound * 2), clem.GetY() + 50 + (sound / 4));

		// 워커 충돌 박스
		/*
		for (int i = 0; i < 15; i++)
		{
			if (walker[i].GetCreate())
			{
				Rectangle(hMemDC, walker[i].GetX(), walker[i].GetY(), walker[i].GetX() + 100, walker[i].GetY() + 100);
			}
		}
		*/

		// 오브젝트
		DrawObject(hMemDC);


		// 클렘 그리기	(상태에 따라 변화)
		MoveTransBltPt(hMemDC, clem.GetX(), clem.GetY(), ClemBit[clem.GetState()], RGB(255, 0, 255), clem.GetFrm(), clem.GetMaxfrm());

		// 블루 그리기
		MoveTransBltPt(hMemDC, blue.GetX(), blue.GetY(), BlueBit[blue.GetState()], RGB(255, 0, 255), blue.GetFrm(), blue.GetMaxfrm());

		// 워커 그리기
		DrawWalker(hMemDC);

		if (clem.GetY() > MaxY)	// 클렘 낙사
		{
			die = true;
			dead = true;
			clem.ClemReset();
		}

		/*if (clem.GetY() > MaxY)	// 블루 낙사 (설마)
		{
			blue.SetTeam(false);
		}*/

		if (!stop)
		{
			clem.ClemMove();	// 클렘 이동
		}
		clem.Jump();		// 클렘 점프

		clem.Gravity();		// 클렘 중력
		clem.EndFrm();		// 클렘 공격 후
		clem.HpDown();		// 클렘 공격받을 경우

		blue.Gravity();		// 블루 중력
		blue.Move();		// 블루 이동
		blue.Jump();		// 블루 점프
		blue.EndFrm();		// 블루 공격 후

		WalkerControl();		// 워커들 관리

		BulletControl(hMemDC);

		// 안개 관리
		FogControl(hMemDC);

		// 충돌 확인
		Collision();

		// 가방 그리기
		BackPackOpen(hMemDC);

		// 텍스트 표시
		ShowText(hMemDC);

		// 인터페이스
		if (blue.GetTeam())
		{
			DrawBitmapPt(hMemDC, 0, 880, InterfaceBit[0]);
			LifeBlt(hMemDC, 150, 935, LifeBit[1], RGB(255, 0, 255), 1);
		}
		else
		{
			DrawBitmapPt(hMemDC, 0, 880, InterfaceBit[1]);
		}

		// 체력 표시
		LifeBlt(hMemDC, 30, 935, LifeBit[0], RGB(255, 0, 255), 0);

		// 에너지 표시
		Energy(hMemDC);

		// 음식 선택
		for (int i = 0; i < 2; i++)
		{
			if (foodsel[i].create)
			{
				DrawBitmapPt(hMemDC, foodsel[i].rt.left, foodsel[i].rt.top, foodsel[i].bitmap);
			}
		}
	}

	// 대화중 걷기 멈춤
	if (stop)
	{
		if (clem.GetState() == CLEMRIGHTWALKING)
		{
			clem.SetState(CLEMRIGHT);
			clem.NewFrm(10);
		}
		else if (clem.GetState() == CLEMLEFTWALKING)
		{
			clem.SetState(CLEMLEFT);
			clem.NewFrm(10);
		}
	}

	if (!blue.GetDie() && stage > 9 && !blue.GetTeam())
	{
		blue.SetDie(false);
	}

	if (stage == 0)
	{
		DrawBitmapPt(hMemDC, 0, 0, TitleBit[LOBBY]);
		TransBltPt(hMemDC, 300, 710 + (sel * 130), ArrowBit, RGB(255, 0, 255));
	}
	else if (stage == CLEARSTAGE)
	{
		DrawBitmapPt(hMemDC, 0, 0, TitleBit[CLEAR]);
		AllStop = true;
	}

	if (dead)
	{
		DrawBitmapPt(hMemDC, 0, 0, TitleBit[DEAD]);
	}

	SelectObject(hMemDC, OldBit);
	DeleteDC(hMemDC);
	ReleaseDC(hWndMain, hdc);
	InvalidateRect(hWndMain, NULL, false);
}


// 테스트용 함수
void Test()
{
	// 테스트용
	stage = 11;					// 스테이지
	page = 59;					// 대화 진행도
	clem.SetItem(STICK, true);	// 스틱 유무
	blue.SetTeam(false);		// 블루 유무
	Stage();					// 한 스테이지 넘기기
	AllStop = false;			// 모두 정지
	stop = false;				// 클렘 정지
}

// 시작
void Create()				// 모든 비트맵을 여기서 삽입했습니다
{
	hInst = LoadLibrary("Dll.dll");

	TransBltPt = (void (*)(HDC, int, int, HBITMAP, COLORREF))GetProcAddress(hInst, "TransBlt");
	MoveTransBltPt = (void (*)(HDC, int, int, HBITMAP, COLORREF, int, int))GetProcAddress(hInst, "MoveTransBlt");
	DrawBitmapPt = (void (*)(HDC, int, int, HBITMAP))GetProcAddress(hInst, "DrawBitmap");
	CheckPtInRectCustomPt = (BOOL(*)(HBITMAP, int, int, int, int, HBITMAP, int, int, int, int))GetProcAddress(hInst, "CheckPtInRectCustom");
	CheckPtInRectPt = (BOOL(*)(HBITMAP, int, int, HBITMAP, int, int))GetProcAddress(hInst, "CheckPtInRect");

	// 클레멘타인
	ClemBit[CLEMRIGHT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));		// 오른 대기
	ClemBit[CLEMLEFT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP4));		// 왼 대기
	ClemBit[CLEMRIGHTWALKING] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP5));		// 오른 이동
	ClemBit[CLEMLEFTWALKING] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP6));		// 왼 이동
	ClemBit[CLEMRIGHTJUMP] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP7));		// 오른 점프
	ClemBit[CLEMLEFTJUMP] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP8));		// 왼 점프
	ClemBit[CLEMRIGHTSLOWWALKING] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP11));		// 오른 천천히 이동
	ClemBit[CLEMLEFTSLOWWALKING] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP12));		// 왼 천천히 이동
	ClemBit[CLEMRIGHTSUDDENATTACK] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP17));		// 오른 급습
	ClemBit[CLEMLEFTSUDDENATTACK] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP18));		// 왼 급습
	ClemBit[CLEMRIGHTCAUGHT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP19));		// 오른 잡힘
	ClemBit[CLEMLEFTCAUGHT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP20));		// 왼 잡힘
	ClemBit[CLEMRIGHTCOUNTERATTACK] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP21));		// 오른 반격
	ClemBit[CLEMLEFTCOUNTERATTACK] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP22));		// 왼 반격
	ClemBit[CLEMRIGHTDIE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP23));		// 오른 사망
	ClemBit[CLEMLEFTDIE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP24));		// 왼 사망
	ClemBit[CLEMOPEN] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP33));		// 문따기
	ClemBit[CLEMRIGHTSHOT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP34));		// 오른 사격
	ClemBit[CLEMLEFTSHOT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP35));		// 왼 사격

	// 블루
	BlueBit[BRIGHT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP58));		// 블루 오른 대기
	BlueBit[BLEFT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP59));		// 블루 왼 대기
	BlueBit[BRIGHTWALKING] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP62));		// 블루 오른 이동
	BlueBit[BLEFTWALKING] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP63));		// 블루 왼 이동
	BlueBit[BRIGHTJUMP] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP60));		// 블루 오른 점프
	BlueBit[BLEFTJUMP] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP61));		// 블루 왼 점프
	BlueBit[BRIGHTATTACK] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP56));		// 블루 오른 공격
	BlueBit[BLEFTATTACK] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP57));		// 블루 왼 공격
	BlueBit[BRIGHTDIE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP67));		// 블루 오른 사망
	BlueBit[BLEFTDIE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP68));		// 블루 왼 사망

	// 배경
	BackGroundBit[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP2));		// 시골
	BackGroundBit[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP42));		// 창고
	BackGroundBit[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP76));		// 도로
	BackGroundBit[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP77));		// 옥상
	BackGroundBit[4] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP83));		// 건물내부

	// 안개
	FogBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP9));		// 기본 안개
	SmallFogBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP88));	// 작은 안개

	// 오브젝트
	ObjectBit[LAND] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP10));		// 땅 300 x 50
	ObjectBit[STICK] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP25));		// 쇠 지렛대
	ObjectBit[DOOR] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP32));		// 문
	ObjectBit[OPENDOOR] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP36));		// 열린 문
	ObjectBit[KEY] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP39));		// 열쇠
	ObjectBit[GUN] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP43));		// 권총
	ObjectBit[BULLET] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP45));		// 총알
	ObjectBit[DARKLAND] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP50));		// 검은 땅 300 x 50
	ObjectBit[PILLAR] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP53));		// 쇠 기둥
	ObjectBit[STONE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP54));		// 돌멩이
	ObjectBit[FOOD] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP69));		// 음식
	ObjectBit[ROAD] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP78));		// 도로 땅 200 x 20
	ObjectBit[ROOFTOP] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP79));	// 옥상 땅 100 x 20
	ObjectBit[SWITCH] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP80));		// 전등 스위치
	ObjectBit[CLOSEBOX] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP81));		// 닫힌 박스
	ObjectBit[OPENBOX] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP82));		// 열린 박스
	ObjectBit[WARDROBE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP87));		// 닫힌 옷장
	ObjectBit[OPENWARDROBE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP86));		// 열린 옷장

	// 워커
	WalkerBit[LEFT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP13));		// 왼 워커
	WalkerBit[RIGHT] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP14));		// 오른 워커
	WalkerBit[LEFTWALKING] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP15));		// 왼 워커 이동
	WalkerBit[RIGHTWALKING] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP16));		// 오른 워커 이동
	WalkerBit[RIGHTDIE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP28));		// 오른 시체
	WalkerBit[LEFTDIE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP31));		// 왼 시체
	WalkerBit[RIGHTDIE2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP29));		// 오른 시체2
	WalkerBit[LEFTDIE2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP30));		// 왼 시체2
	WalkerBit[LEFTGUNDIE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP47));		// 왼 총사망
	WalkerBit[RIGHTGUNDIE] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP48));		// 오른 총사망

	// 가방 & 내용물
	BackPackBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP26));			// 가방
	PackItemBit[STICK] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP27));	// 가방 속 쇠 지렛대
	PackItemBit[KEY] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP40));	// 가방 속 열쇠
	PackItemBit[GUN] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP44));	// 가방 속 권총
	PackItemBit[BULLET] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP46));	// 가방 속 총알
	PackItemBit[FOOD] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP70));	// 가방 속 음식

	// 타이틀 등
	TitleBit[LOBBY] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP37));	// 로비
	TitleBit[CLEAR] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP51));	// 클리어
	TitleBit[DEAD] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP52));	// 죽음

	// 기타
	TextBoxBit[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP38));	// 텍스트 박스 (클렘)
	TextBoxBit[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP75));	// 텍스트 박스 (블루)
	ArrowBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP41));	// 화살표
	BulletBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP49));	// 발사총알
	LifeBit[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP55));	// 클렘 체력
	LifeBit[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP64));	// 블루 체력
	InterfaceBit[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP65));	// 인터페이스
	InterfaceBit[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP66));	// 솔로 인터페이스
	FoodSelectBit[0] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP71));	// 블루 음식선택버튼
	FoodSelectBit[1] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP72));	// 블루 음식선택버튼 (마우스 올려둔 상태)
	FoodSelectBit[2] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP73));	// 클렘 음식선택버튼
	FoodSelectBit[3] = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP74));	// 클렘 음식선택버튼 (마우스 올려둔 상태)
	EnergyBit = LoadBitmap(g_hInst, MAKEINTRESOURCE(IDB_BITMAP89));	// 에너지 바

	OnBGM(LOBBY);
	// 총은 들고 시작
	clem.SetItem(GUN, true);
	
	// 음식 주기 선택 비트맵
	foodsel[0].bitmap = FoodSelectBit[0];
	foodsel[1].bitmap = FoodSelectBit[2];

	// 시작 정보
	Save();

	// 테스트용
	//Test();

}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

	switch (iMessage)
	{
	case WM_CREATE:
		srand(time(NULL));

		Create();		//  시작 시

		SetTimer(hWnd, 0, 10, NULL);		// OnTimer, GetKeyState
		SetTimer(hWnd, 1, 80, NULL);		// 애니메이션 프레임 관리
		return 0;
	case WM_TIMER:
		switch (wParam)
		{
		case 0:	// OnTimer용 타이머
			OnTimer();
			break;
		case 1:	// 프레임 관리 타이머
			clem.FrmControl();		// 클렘 프레임 관리
			blue.FrmControl();		// 블루 프레임 관리
			for (int i = 0; i < 30; i++)
			{
				walker[i].FrmControl();		// 워커 프레임 관리
			}
			break;
		case 2:
			dead = true;
			KillTimer(hWnd, 2);
			break;
		}
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'Z':
			if (stage == LOBBY)
			{
				if (sel == 0)
				{
					AllStop = false;
					Stage();
					OnBGM(INGAME);
				}
				else
				{
					PostQuitMessage(0);
				}
			}
			else if (stage == CLEARSTAGE)
			{
				stage = 0;
				clem.ClemReset();
			}
			else if (!stop)
			{
				clem.SuddenAttack();
				clem.OpenDoor();
				Talk();
				
			}
			else if (key && (clem.GetState() == CLEMRIGHTCAUGHT || clem.GetState() == CLEMLEFTCAUGHT))
			{
				clem.SetHp(clem.GetHp() + 20);
				key = false;
			}
			// 대화용
			else if (showtext)
			{
				page++;
			}
			break;
		case 'X':
			if (!clem.GetAir() && !stop && key && !movefog)
			{
				clem.SetJump(10);		// 점프력
				key = false;
			}
			break;
		case 'C':
			if (!AllStop)
			{
				if (stop && (clem.GetState() == CLEMRIGHT || clem.GetState() == CLEMLEFT) && key && !showtext)
				{
					stop = false;
					openbackpack = false;
					SoundSelect(100);
					key = false;
					if (blue.GetTeam())
					{
						foodsel[0].create = false;
					}
					foodsel[1].create = false;
				}
				else if (!stop && (clem.GetState() == CLEMRIGHT || clem.GetState() == CLEMLEFT) && key && !showtext)
				{
					stop = true;
					openbackpack = true;
					SoundSelect(100);
					key = false;
				}
			}
			break;
		case 'A':
			clem.Shot();
			break;
		case 'R':
			if (dead)
			{
				AllStop = false;
				stop = false;
				clem.SetHp(100);
				stage--;
				die = false;
				dead = false;
				blue.SetDie(false);
				blue.SetState(BLEFT);
				blue.NewFrm(6);
				clem.ClemReset();
				if (!clem.GetAir())
				{
					clem.SetState(CLEMRIGHT);
				}
				Stage();
			}
			break;
		case 'V':
			if (!movefog)
			{
				movefog = true;
			}
			break;
		case 'T':
			if (showfog)
			{
				showfog = false;
			}
			else
			{
				showfog = true;
			}
			break;
		case 'S':
			if (blueattackstate)
			{
				blueattackstate = false;
			}
			else
			{
				blueattackstate = true;
			}
			break;
		case VK_SHIFT:
			if (!clem.GetSlow() && !stop)
			{
				SPEED = 2;
				clem.SetSlow(true);
			}
			break;
		case VK_UP:
			if (sel == 1)
			{
				sel = 0;
			}
			break;
		case VK_DOWN:
			if (sel == 0)
			{
				sel = 1;
			}
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_LEFT:
			if (!clem.GetAir() && !stop && !movefog)
			{
				clem.NewFrm(10);
				clem.SetState(CLEMLEFT);
			}
			break;
		case VK_RIGHT:
			if (!clem.GetAir() && !stop && !movefog)
			{
				clem.NewFrm(10);
				clem.SetState(CLEMRIGHT);
			}
			break;
		case VK_SHIFT:
			if (clem.GetSlow())
			{
				SPEED = 3;
				clem.SetSlow(false);
			}
			break;
		case 'Z':
		case 'X':
		case 'C':
			if (!key)
			{
				key = true;
			}
			break;
		case 'V':
			if (movefog)
			{
				movefog = false;
			}
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		FoodSelect();
		break;
	case WM_MOUSEMOVE:
		mousex = LOWORD(lParam);
		mousey = HIWORD(lParam);
		FoodUp();
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);	// 도구를 빌리고

		DrawBitmapPt(hdc, 0, 0, hBit);		// 메모리에 미리 그려둔 그림들 한번에 그리기

		EndPaint(hWnd, &ps);		// 반납했습니다
		return 0;
	case WM_DESTROY:
		for (int i = 0; i < 3; i++)
		{
			KillTimer(hWnd, i);				// 모든 타이머 정지
		}
		FreeLibrary(hInst);
		PostQuitMessage(0);
		return 0;
	}

	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}