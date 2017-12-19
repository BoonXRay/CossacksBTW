#include <cassert>

#include "CDirSnd.h"
#include "GameSound.h"
#include "Interface.h"
#include "main.h"
#include "Multi.h"
#include "Nation.h"
#include "Nature.h"
#include "NewMon.h"
#include "NewUpgrade.h"
#include "StringHash.h"
#include "Weapon.h"

#include "stRecog.h"


Weapon * WPLIST[1024];
static int NUCLUSE[4];
static char* mbm[1024];
static int	nmbm = 0;

static void Errx(LPCSTR s)
{
	MessageBox(hwnd, s, "Nation loading failed...", MB_ICONWARNING | MB_OK);
	assert(false);
}

static int GetCIndex(char* s)
{
	char gx[128];
	if (!strcmp(s, "NUCLUSE"))
		return 1;
	sprintf(gx, "Unknown constant : %s", s);
	Errx(gx);
	return -1;
}

static const char* CurFile;
static StrHash MODS;

static void InitModes()
{
	MODS.AddString("??");
	MODS.AddString("[MEMBERS]");
	MODS.AddString("[TILEWEAPON]");
	MODS.AddString("[WEAPON]");
	MODS.AddString("[BUILDINGS]");
	MODS.AddString("[PRODUCE]");
	MODS.AddString("[COUNTRY]");
	MODS.AddString("[UPGRADE]");
	MODS.AddString("[AUTOUPGRADE]");
	MODS.AddString("[UPGRADELINKS]");
	MODS.AddString("[FIXED_PRODUCE]");
	MODS.AddString("[WEAPONCENTER]");
	MODS.AddString("[CHILDWEAPON]");
	MODS.AddString("[RADIUS&DAMAGE]");
	MODS.AddString("[CONSTANT]");
	MODS.AddString("[CHILDMONSTER]");
	MODS.AddString("[VISIBILITY]");
	MODS.AddString("[HILAYER]");
	MODS.AddString("[DANGERZONE]");
	MODS.AddString("[NOSEARCHVICTIM]");
	MODS.AddString("[NOANSWER]");
	MODS.AddString("[NEEDNOHELP]");
	MODS.AddString("[LIFESHOTLOST]");
	MODS.AddString("??");
	MODS.AddString("[ONWATER]");
	MODS.AddString("[ONCOST]");
	MODS.AddString("[ONOIL]");
	MODS.AddString("[CANBUILD]");
	MODS.AddString("[USEFUL_FOR_DEFENSE]");
	MODS.AddString("[USEFUL_FOR_ATTACK]");
	MODS.AddString("[USEFUL_FOR_SCIENCE]");
	MODS.AddString("[USEFUL_FOR_NARXOZ]");
	MODS.AddString("[ABSOLUTELYNEED]");
	MODS.AddString("[BRANCHPERCENT]");
	MODS.AddString("[AICOEFFICIENTS]");
	MODS.AddString("[MAXAUTOAMOUNT]");
	MODS.AddString("[UPGRADEENABLE]");
	MODS.AddString("[PRIVATE]");
	MODS.AddString("[UPGRADEPLACE]");
	MODS.AddString("[TELEPORT]");
	MODS.AddString("[ENABLED]");
	MODS.AddString("[AUTO_PERFORM_UPGRADE]");
	MODS.AddString("[ICONS]");
	MODS.AddString("[UNITS_INTERFACE]");
	MODS.AddString("[MOREWEAPON]");
	MODS.AddString("[IDWEAPON]");
	MODS.AddString("[TRANSPORT]");
	MODS.AddString("[COMMONMENU]");
	MODS.AddString("[FUNCTIONALITY]");
	MODS.AddString("[MAGICREQUEST]");
	MODS.AddString("[ACCESSCONTROL]");
	MODS.AddString("[MAGICCONTROL]");
	MODS.AddString("[RESOURCEABSORBERS]");
	MODS.AddString("[OFFICERS]");
	MODS.AddString("[WALLKILLER]");
	MODS.AddString("[FEAR]");
	MODS.AddString("[USEMAGIC]");
	MODS.AddString("[NUCLEAR]");
	MODS.AddString("[ANTINUCLEAR]");
	MODS.AddString("[UFO]");
	MODS.AddString("[UFOTRANSFORM]");
	MODS.AddString("[SYNCWEAPON]");
	MODS.AddString("[ON_HIT_SOUND]");
	MODS.AddString("[ON_CLICK_SOUND]");
	MODS.AddString("[SOUND]");
	MODS.AddString("[ON_ORDER_SOUND]");
	MODS.AddString("[ON_ATTACK_SOUND]");
	MODS.AddString("[ON_DEATH_SOUND]");
	MODS.AddString("[ON_BORN_SOUND]");
	MODS.AddString("[ON_TREE_SOUND]");
	MODS.AddString("[ON_GOLD_SOUND]");
	MODS.AddString("[SOUNDEFFECTS]");
	MODS.AddString("[CANREPAIR]");
	MODS.AddString("[CANSETDEST]");
	MODS.AddString("[EXTMENU]");
	MODS.AddString("[RESOURCEUPGRADE]");
	MODS.AddString("[UNITLOCK]");
	MODS.AddString("[FULLPARENT]");
	MODS.AddString("[CUSTOMEXPLOSION]");
	MODS.AddString("[SHADOWWEAPON]");
	MODS.AddString("[FIRES]");
	MODS.AddString("[DISABLED_UPGRADES]");
	MODS.AddString("[SPECIAL_UNIT]");
	MODS.AddString("[SPECIAL_UPGRADE]");
	MODS.AddString("[UPGRADESTAGE]");
}

static void GetMode(char* s, int* mode, int line)
{
	if (!MODS.LastIndex)InitModes();
	if (s[0] != '[')return;
	int v = MODS.SearchString(s);
	if (v == -1) {
		if (!strcmp(s, "[END]")) {
			*mode = 255;
			return;
        }
		char gx[128];
		sprintf(gx, "%s : Unrecognised command in line %d", CurFile, line);
		Errx(gx);
	}
	else *mode = v;
}

int SearchStr(char** Res, const char* s, int count)
{
	for (int i = 0; i < count; i++)
		if (!strcmp(Res[i], s))return i;
	return -1;
}

int GetWeaponIndex( const char* str)
{
	return SearchStr(mbm, str, nmbm);
}

void LoadWeapon()
{
	char gg[128];
	char gx[128], gx1[128], gx2[128], gx3[128];
	char gy[128];

    const char* fn = "weapon.nds";
	CurFile = fn;

	int	mode = 0;
	int line = 1;
	int wid = 0;
	int zz1, zz2;
	int p1, p2, p3, p4;

	memset(&WPLIST, 0, sizeof WPLIST);

	GFILE* f = Gopen("weapon.nds", "rt");
	if (!int(f))
	{
		Errx("Could not find WEAPON.NDS");
		return;
	}

	int z = 0;

	do
	{
		switch (mode)
		{
		case 0:
			z = Gscanf(f, "%s", gg);
			NLine(f);
			line++;
			GetMode(gg, &mode, line);
			break;
		case 1:
			//[MEMBERS]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 1 && gg[0] != '/'&&gg[0] != 0) {
				z = Gscanf(f, "%s%d%d%s%s%s%d%d", gx3, &p1, &p2, gx2, gx1, gx, &p3, &p4);
				mbm[nmbm] = new char[strlen(gg) + 1];
				strcpy(mbm[nmbm], gg);
				WPLIST[nmbm] = new Weapon;
				Weapon* WPL = WPLIST[nmbm];
				WPL->MyIndex = nmbm;
				WPL->Default.InitChild();
				WPL->NCustomEx = 0;
				WPL->CustomEx = NULL;
				WPL->ShadowWeapon = NULL;
				WPL->NewAnm = GetNewAnimationByName(gx3);
				if (!WPL->NewAnm) {
					sprintf(gx, "Weapon.nds, %d, Unknown animation:%s", line, gx3);
					Errx(gx);
                }
				//WPLIST[nmbm]->CreateWeapon(&WEP,nmbm,wid,0);
				WPL->Damage = p1;
				WPL->Radius = p2 << 4;
				WPL->HiLayer = 0;
				WPL->SoundID = -1;
				//Transparency loading
				if (!strcmp(gx2, "NONE"))WPL->Transparency = 0; else
					if (!strcmp(gx2, "DARK"))WPL->Transparency = 1; else
						if (!strcmp(gx2, "WHITE"))WPL->Transparency = 2; else
							if (!strcmp(gx2, "RED"))WPL->Transparency = 3; else
								if (!strcmp(gx2, "BRIGHT"))WPL->Transparency = 4; else
									if (!strcmp(gx2, "ALPHAR"))WPL->Transparency = 5; else
										if (!strcmp(gx2, "ALPHAW"))WPL->Transparency = 7; else
											if (!strcmp(gx2, "ALPHAGB"))WPL->Transparency = 7; else
												if (!strcmp(gx2, "YELLOW"))WPL->Transparency = 0; else {
													sprintf(gy, "%s,line %d :Unknown transparency ID: %s", fn, line, gx2);
													Errx(gy);
                                                }
				//Gravity loading
				if (!strcmp(gx1, "NO_GRAVITY"))WPL->Gravity = 0; else
					if (!strcmp(gx1, "LO_GRAVITY"))WPL->Gravity = 1; else
						if (!strcmp(gx1, "HI_GRAVITY"))WPL->Gravity = 2; else
							if (!strcmp(gx1, "HI_GRAVITY1"))WPL->Gravity = 3; else
								if (!strcmp(gx1, "HI_GRAVITY2"))WPL->Gravity = 4; else
									if (!strcmp(gx1, "HI_GRAVITY3"))WPL->Gravity = 5; else {
										sprintf(gy, "%s,line %d :Unknown gravity type: %s", fn, line, gx1);
										Errx(gy);
                                    }
				//Propagation type
				if (!strcmp(gx, "STAND"))WPL->Propagation = 0; else
					if (!strcmp(gx, "SLIGHTUP"))WPL->Propagation = 1; else
						if (!strcmp(gx, "RANDOM"))WPL->Propagation = 2; else
							if (!strcmp(gx, "RANDOM1"))WPL->Propagation = 6; else
								if (!strcmp(gx, "FLY"))WPL->Propagation = 3; else
									if (!strcmp(gx, "REFLECT	"))WPL->Propagation = 7; else
										if (!strcmp(gx, "ANGLE"))WPL->Propagation = 5; else
											if (!strcmp(gx, "IMMEDIATE"))WPL->Propagation = 4; else {
												sprintf(gy, "%s,line %d :Unknown propagation type: %s", fn, line, gx1);
												Errx(gy);
                                            }
				WPL->Speed = p3;
				WPL->Times = p4;
				WPL->NTileWeapon = 0;
				WPL->TileProbability = 0;
				WPL->MaxChild = 0;
				WPL->MinChild = 0;
				WPL->SoundID = -1;
				WPL->FullParent = false;
				WPL->NSyncWeapon = 0;
				WPL->HotFrame = 0;
				nmbm++;
            }
			NLine(f);
			line++;
			break;
		case 80://[FIRES]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 80 && gg[0] != '/'&&gg[0] != 0) {
				z = Gscanf(f, "%d", &p1);
				NFiresAnm[0] = p1;
				FiresAnm[0] = new lpNewAnimation[p1];
				PreFires[0] = new lpNewAnimation[p1];
				PostFires[0] = new lpNewAnimation[p1];
				for (int i = 0; i < p1; i++) {
					z = Gscanf(f, "%s", gy);
					PreFires[0][i] = GetNewAnimationByName(gy);
					if (!PreFires[0][i]) {
						sprintf(gx, "Weapon.nds, %d, Unknown animation:%s", line, gy);
						Errx(gx);
                    }
                }
				NLine(f);
				line++;
				for (int i = 0; i < p1; i++) {
					z = Gscanf(f, "%s", gy);
					FiresAnm[0][i] = GetNewAnimationByName(gy);
					if (!FiresAnm[0][i]) {
						sprintf(gx, "Weapon.nds, %d, Unknown animation:%s", line, gy);
						Errx(gx);
                    }
                }
				NLine(f);
				line++;
				for (int i = 0; i < p1; i++) {
					z = Gscanf(f, "%s", gy);
					PostFires[0][i] = GetNewAnimationByName(gy);
					if (!PostFires[0][i]) {
						sprintf(gx, "Weapon.nds, %d, Unknown animation:%s", line, gy);
						Errx(gx);
                    }
                }
				NLine(f);
				line++;
				z = Gscanf(f, "%s%d", gy, &p1);
				NFiresAnm[1] = p1;
				FiresAnm[1] = new lpNewAnimation[p1];
				PreFires[1] = new lpNewAnimation[p1];
				PostFires[1] = new lpNewAnimation[p1];
				for (int i = 0; i < p1; i++) {
					z = Gscanf(f, "%s", gy);
					PreFires[1][i] = GetNewAnimationByName(gy);
					if (!PreFires[1][i]) {
						sprintf(gx, "Weapon.nds, %d, Unknown animation:%s", line, gy);
						Errx(gx);
                    }
                }
				NLine(f);
				line++;
				for (int i = 0; i < p1; i++) {
					z = Gscanf(f, "%s", gy);
					FiresAnm[1][i] = GetNewAnimationByName(gy);
					if (!FiresAnm[1][i]) {
						sprintf(gx, "Weapon.nds, %d, Unknown animation:%s", line, gy);
						Errx(gx);
                    }
                }
				NLine(f);
				line++;
				for (int i = 0; i < p1; i++) {
					z = Gscanf(f, "%s", gy);
					PostFires[1][i] = GetNewAnimationByName(gy);
					if (!PostFires[1][i]) {
						sprintf(gx, "Weapon.nds, %d, Unknown animation:%s", line, gy);
						Errx(gx);
                    }
                }
				NLine(f);
				line++;
            }
			break;
		case 77://[FULLPARENT]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 77 && gg[0] != '/'&&gg[0] != 0) {
				zz1 = SearchStr(mbm, gg, nmbm);
				if (zz1 == -1) {
					sprintf(gy, "%s,line %d :[FULLPARENT]:Parent weapon %s not found.", fn, line, gg);
					Errx(gy);
                }
				WPLIST[zz1]->FullParent = true;
				NLine(f);
				line++;
            }
			break;
		case 17://[HILAYER]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 17 && gg[0] != '/'&&gg[0] != 0) {
				zz1 = SearchStr(mbm, gg, nmbm);
				if (zz1 == -1) {
					sprintf(gy, "%s,line %d :[HILAYER]:weapon %s not found.", fn, line, gg);
					Errx(gy);
                }
				WPLIST[zz1]->HiLayer = true;
				NLine(f);
				line++;
            }
			break;
		case 2:
			//[TILEWEAPON]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 2 && gg[0] != '/'&&gg[0] != 0) {
				z = Gscanf(f, "%d%d", &p1, &p2);
				zz1 = SearchStr(mbm, gg, nmbm);
				if (zz1 == -1) {
					sprintf(gy, "%s,line %d :[TILEWEAPON]:Parent weapon %s not found.", fn, line, gg);
					Errx(gy);
                }
				WPLIST[zz1]->NTileWeapon = p2;
				WPLIST[zz1]->TileProbability = p1;
				if (p2 > 4) {
					sprintf(gy, "%d (WEAPON.NDS) :[TILEWEAPON]:Too mutch tile weapon IDs : %d", line, p1);
					Errx(gy);
                }
				for (int j = 0; j < p2; j++) {
					z = Gscanf(f, "%s", gx);
					int zz2 = SearchStr(mbm, gx, nmbm);
					if (zz2 == -1) {
						sprintf(gy, "Identifier %s not found.", gx);
						Errx(gy);
                    }
					WPLIST[zz1]->TileWeapon[j] = WPLIST[zz2];
                }
            }
			NLine(f);
			line++;
			break;
		case 12://[CHILDWEAPON]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 12 && gg[0] != '/'&&gg[0] != 0) {
				z = Gscanf(f, "%d%d%d%d", &p1, &p2, &p3, &wid);
				zz1 = SearchStr(mbm, gg, nmbm);
				if (zz1 == -1) {
					sprintf(gy, "%d (WEAPON.NDS) : Unknown parent weapon : %s", line, gg);
					Errx(gy);
                }

				mbm[nmbm] = new char[strlen(gg) + 1];
				strcpy(mbm[nmbm], gg);
				Weapon* WP = WPLIST[zz1];
				ChildWeapon* CWP = &WP->Default;
				CWP->MinChild = p1;
				CWP->MaxChild = p2;
				WP->HotFrame = p3;
				CWP->NChildWeapon = wid;
				if (wid > 4) {
					sprintf(gy, "%d (WEAPON.NDS) : [CHILDWEAPON] Too mutch child weapon IDs : %d", line, wid);
					Errx(gy);
                }
				for (int j = 0; j < wid; j++) {
					z = Gscanf(f, "%s", gx1);
					zz2 = SearchStr(mbm, gx1, nmbm);
					if (zz2 == -1) {
						sprintf(gy, "%d (WEAPON.NDS) : Unknown child weapon ID : %s", line, gx1);
						Errx(gy);
                    }
					CWP->Child[j] = WPLIST[zz2];
                }

            }
			NLine(f);
			line++;
			break;
		case 61://[SYNCWEAPON]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 61 && gg[0] != '/'&&gg[0] != 0) {
				z = Gscanf(f, "%d", &p1);
				zz1 = SearchStr(mbm, gg, nmbm);
				if (zz1 == -1) {
					sprintf(gy, "%d (WEAPON.NDS) : Unknown parent weapon : %s", line, gg);
					Errx(gy);
                }

				mbm[nmbm] = new char[strlen(gg) + 1];
				strcpy(mbm[nmbm], gg);
				Weapon* WP = WPLIST[zz1];
				if (p1 > 4) {
					sprintf(gy, "%d (WEAPON.NDS) : %s Too mutch child weapon IDs : %d", line, gg, wid);
					Errx(gy);
                }
				WP->NSyncWeapon = p1;
				for (int j = 0; j < p1; j++) {
					z = Gscanf(f, "%s", gx1);
					zz2 = SearchStr(mbm, gx1, nmbm);
					if (zz1 == -1) {
						sprintf(gy, "%d (WEAPON.NDS) : Unknown child weapon ID : %s", line, gx1);
						Errx(gy);
                    }
					WP->SyncWeapon[j] = WPLIST[zz2];
                }
				WP->HotFrame = 0;
            }
			NLine(f);
			line++;
			break;
		case 78://[CUSTOMEXPLOSION]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 78 && gg[0] != '/'&&gg[0] != 0) {
				char mtype[16];
				z = Gscanf(f, "%s%d%d%d%d", mtype, &p1, &p2, &p3, &wid);
				zz1 = SearchStr(mbm, gg, nmbm);
				if (zz1 == -1) {
					sprintf(gy, "%d (WEAPON.NDS) : Unknown parent weapon : %s", line, gg);
					Errx(gy);
                }
				mbm[nmbm] = new char[strlen(gg) + 1];
				strcpy(mbm[nmbm], gg);
				Weapon* WP = WPLIST[zz1];
				ChildWeapon* CWP = (ChildWeapon*)realloc(WP->CustomEx, int(WP->NCustomEx + 1) * sizeof(ChildWeapon));
				WP->CustomEx = CWP;
				CWP += WP->NCustomEx;
				WP->NCustomEx++;
				int typ = GetExMedia(mtype);
				if (typ == -1) {
					sprintf(gy, "%d (Weapon.nds) : Unknown explosion media type : %s", line, mtype);
					Errx(gy);
                }
				CWP->Type = byte(typ);
				CWP->MinChild = p1;
				CWP->MaxChild = p2;
				WP->HotFrame = p3;
				CWP->NChildWeapon = wid;
				if (wid > 4) {
					sprintf(gy, "%d (WEAPON.NDS) : [CUSTOMEXPLOSION] Too mutch child weapon IDs : %d", line, wid);
					Errx(gy);
                }
				for (int j = 0; j < wid; j++) {
					z = Gscanf(f, "%s", gx1);
					zz2 = SearchStr(mbm, gx1, nmbm);
					if (zz2 == -1) {
						sprintf(gy, "%d (WEAPON.NDS) : Unknown child weapon ID : %s", line, gx1);
						Errx(gy);
                    }
					CWP->Child[j] = WPLIST[zz2];
                }
            }
			NLine(f);
			line++;
			break;
		case 79:
			//[SHADOWWEAPON]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 79 && gg[0] != '/'&&gg[0] != 0) {
				zz1 = SearchStr(mbm, gg, nmbm);
				if (zz1 == -1) {
					sprintf(gy, "%s,line %d :[SHADOWWEAPON]:Parent weapon %s not found.", fn, line, gg);
					Errx(gy);
                }
				z = Gscanf(f, "%s", gx);
				int zz2 = SearchStr(mbm, gx, nmbm);
				if (zz2 == -1) {
					sprintf(gy, "Identifier %s not found.", gx);
					Errx(gy);
                }
				WPLIST[zz1]->ShadowWeapon = WPLIST[zz2];
            }
			NLine(f);
			line++;
			break;
		case 13://[RADIUS&DAMAGE]
			z = Gscanf(f, "%s%d%d", gg, &p1, &p2);
			GetMode(gg, &mode, line);
			if (mode == 12 && gg[0] != '/'&&gg[0] != 0) {
				z = Gscanf(f, "%d%d%d%d", &p1, &p2, &p3, &wid);
				NLine(f);
				line++;
				zz1 = SearchStr(mbm, gg, nmbm);
				if (zz1 == -1) {
					sprintf(gy, "%d (WEAPON.NDS) : Unknown weapon ID: %s", line, gg);
					Errx(gy);
                }
				WPLIST[zz1]->Radius = p1;
				WPLIST[zz1]->Damage = p2;
            }
			break;
		case 14://[CONSTANT]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 14)
				z = Gscanf(f, "%s%s%s%s", gx, gx1, gx2, gx3);
			NLine(f);
			line++;
			if (mode == 14 && gg[0] != '/'&&gg[0] != 0) {
				zz1 = GetCIndex(gg);
				if (zz1 == 1) {
					zz1 = SearchStr(mbm, gx, nmbm);
					if (zz1 == -1) {
						sprintf(gy, "Illegal value for NUCLUSE : %s", gx);
						Errx(gy);
                    }
					NUCLUSE[0] = zz1;
					zz1 = SearchStr(mbm, gx1, nmbm);
					if (zz1 == -1) {
						sprintf(gy, "Illegal value for NUCLUSE : %s", gx1);
						Errx(gy);
                    }
					NUCLUSE[1] = zz1;
					zz1 = SearchStr(mbm, gx2, nmbm);
					if (zz1 == -1) {
						sprintf(gy, "Illegal value for NUCLUSE : %s", gx2);
						Errx(gy);
                    }
					NUCLUSE[2] = zz1;
					zz1 = SearchStr(mbm, gx3, nmbm);
					if (zz1 == -1) {
						sprintf(gy, "Illegal value for NUCLUSE : %s", gx);
						Errx(gy);
                    }
					NUCLUSE[3] = zz1;
                }
            }
			break;
		case 45://[IDWEAPON]
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 45)
			{
				z = Gscanf(f, "%s", gx);
			}

			NLine(f);
			line++;

			if (mode == 45 && gg[0] != '/'&&gg[0] != 0)
			{
				zz1 = SearchStr(mbm, gx, nmbm);
				if (zz1 == -1)
				{
					sprintf(gy, "Identifier %s not found.", gg);
					Errx(gy);
				}
                // BoonXRay 26.11.2017 Commented because setted variables not used (now deleted)
                //if (!strcmp(gg, "ID_FIRE"))ID_FIRE = zz1; else
                    //if (!strcmp(gg, "ID_FIRE1"))ID_FIRE1 = zz1; else
                        //if (!strcmp(gg, "ID_MAGW"))ID_MAGW = zz1; else
                            //if (!strcmp(gg, "ID_EXPL1"))ID_EXPL1 = zz1; else
                                //if (!strcmp(gg, "ID_EXPL2"))ID_EXPL2 = zz1; else
                                    //if (!strcmp(gg, "ID_GLASSFLY"))ID_GLASSFLY = zz1; else
                                        //if (!strcmp(gg, "ID_GLASSBROKEN"))ID_GLASSBROKEN = zz1; else
                                            //if (!strcmp(gg, "ID_MAGEXP"))ID_MAGEXP = zz1; else
                                                //if (!strcmp(gg, "ID_FOG"))ID_FOG = zz1; else
                                                    //if (!strcmp(gg, "ID_FOG1"))ID_FOG1 = zz1; else
                                                        //if (!strcmp(gg, "ID_FIREX"))ID_FIREX = zz1; else
														{
															sprintf(gx, "[IDWEAPON] %d :Unknown constant: %s", line, gg);
															Errx(gx);
														}
			}
			break;
		case 64://[SOUND}
			z = Gscanf(f, "%s", gg);
			GetMode(gg, &mode, line);

			if (mode == 64)
			{
				z = Gscanf(f, "%s", gx);
			}

			NLine(f);
			line++;
			if (mode == 64 && gg[0] != '/'&&gg[0] != 0)
			{
				zz1 = SearchStr(mbm, gg, nmbm);

				if (zz1 == -1)
				{
					sprintf(gy, "Unknown weapon ID: %s", gg);
					Errx(gy);
				}

				zz2 = SearchStr(SoundID, gx, NSounds);

				if (zz2 == -1)
				{
					sprintf(gy, "Unknown Sound ID: %s", gx);
					Errx(gy);
				}

				WPLIST[zz1]->SoundID = zz2;
			}
			break;
		case 255:
			z = 0;
			break;
		}
	} while (z);

	if (mode == 0)
	{
		Errx("[MEMBERS] not found in WEAPON.NDS");
	}
}

void normstr(char* str)
{
	for (int i = 0; str[i] != 0; i++)
	{
		if (str[i] == '_')
		{
			str[i] = ' ';
		}
	}
}

void InitFlags(GeneralObject* GO)
{
	GO->delay = 8;
	GO->NoSearchVictim = false;
	GO->LifeShotLost = 0;
	GO->Enabled = false;
	GO->WepSpeed = 64;
	GO->WepDelay = 16;
	GO->AGold = false;
	GO->AWood = false;
	for (int i = 0; i < NBRANCH; i++)
	{
		GO->Useful[i] = 0;
		GO->SpUsef[i] = 0;
	}
	GO->AbsNeedPrio = 0;
	GO->AbsNeedCount = 0;
	GO->MaxAutoAmount = 2048;
	GO->NIcons = 0;
	GO->NWeap = 0;
	GO->Transport = false;
	GO->FWEAP = false;
	GO->RefreshLife = false;
	GO->CanFly = false;
	GO->CanAttWall = false;
	GO->CanFear = false;
	GO->AntiNuc = false;
	GO->canNucAttack = false;
	GO->UFO = false;
	GO->UFOTrans = false;
	for (int i = 0; i < 12; i++)
	{
		GO->MWeap[i] = nullptr;
		GO->SWPIndex[i] = 255;
	}
	GO->MagAgainst = 0;
	GO->MagWith = 0;
	GO->HitSound = -1;
	GO->ClickSound = -1;
	GO->DeathSound = -1;
	GO->OrderSound = -1;
	GO->AttackSound = -1;
	GO->TreeSound = -1;
	GO->GoldSound = -1;
	GO->BornSound = -1;
	GO->CanRepair = false;
	GO->ExtMenu = false;
	GO->CanDest = false;
	GO->LockID = 0xFFFF;
	GO->NLockUnits = 0;
	GO->Upg = nullptr;
	GO->NUpgrades = 0;
	GO->Country = 0;
	memset(GO->Wdx, 0, 8);
	memset(GO->Wdy, 0, 8);
}

typedef word* lpWORD;

static void InitNation(byte msk, byte NIndex)
{
	Nation* nat = &NATIONS[NIndex];
    memset(nat, 0, sizeof( Nation));
	nat->CreateNation(msk, NIndex);
	nat->NNUM = NIndex;
	nat->DangerSound = -1;
	nat->VictorySound = -1;
	nat->ConstructSound = -1;
	nat->BuildDieSound = -1;
	nat->UnitDieSound = -1;
	nat->LastAttackTime = 0;
	nat->NMon = 0;
	nat->NUpgrades = 0;
	nat->UnitNames = new lplpCHAR[NNations];
	nat->NUnits = new int[NNations];
	nat->UnitsIDS = new lpWORD[NNations];
	nat->Harch = 0;
	memset(nat->NUnits, 0, NNations * 4);
	memset(nat->UnitNames, 0, NNations * 4);
	memset(nat->UnitsIDS, 0, NNations * 4);
	nat->FoodEff = 1010;
	nat->WoodEff = 100;
	nat->StoneEff = 100;
	nat->NHistory = 0;
	nat->History = nullptr;
}

static StrHash mnm;
static StrHash upg;
static StrHash ico;

uint32_t LOADNATMASK = 0;

void LoadAllNations(uint8_t NIndex)
{
	if (NIndex)
	{
		return;
	}

	for (int cp = 0; cp < 8; cp++)
	{
		InitNation(1 << cp, cp);
	}

	int m0 = 1;

	for (int i = 0; i < NNations; i++)
	{
		if (strcmp(NatScripts[i], "-") && (LOADNATMASK & m0))
		{
			LoadNation(NatScripts[i], NIndex, i);
			ProcessMessages();
		}
		m0 <<= 1;
	}

	mnm.Clear();
	upg.Clear();
	ico.Clear();

	for (NIndex = 0; NIndex < 8; NIndex++)
	{
		Nation* NT = NATIONS + NIndex;
		NT->CITY = &CITY[NIndex];
		NT->MINE_CAPTURE_RADIUS = 250;
		NT->MINE_UPGRADE1_RADIUS = 180;
		NT->MINE_UPGRADE2_RADIUS = 90;
		NT->DEFAULT_MAX_WORKERS = 120;
		NT->FormUnitID = 1;
		CITY[NIndex].CloseCity();
		CITY[NIndex].CreateCity(NIndex);
		CITY[NIndex].CreateCity(NIndex);
		memset(NT->NKilled, 0, sizeof NT->NKilled);
		memset(NT->NProduced, 0, sizeof NT->NProduced);
		memset(NT->ResTotal, 0, 32);
		memset(NT->ResOnMines, 0, 32);
		memset(NT->ResOnBuildings, 0, 32);
		memset(NT->ResOnUpgrade, 0, 32);
		memset(NT->ResOnUnits, 0, 32);
		memset(NT->ResOnLife, 0, 32);
		ProcessMessages();
	}
}

void LoadNation(char* fn, uint8_t NIndex, uint8_t NatID)
{
	if (NIndex)
	{
		return;
	}

	CurFile = fn;
	char gg[128];
	char gx[128];
	char gy[128];
	char gz[128];
	int	mode = 0;
	int line = 0;
	int wid = 0;
	int	zz1, zz2;
	int parm1, parm2, parm3;

	GFILE* f1 = Gopen(fn, "r");
	if (!int(f1))
	{
		sprintf(gg, "Could not open %s", fn);
		Errx(gg);
		return;
	}

	Nation* nat = &NATIONS[NIndex];
	memset(nat->ResRem, 0, sizeof nat->ResRem);
	memset(nat->ResSpeed, 0, sizeof nat->ResSpeed);
	memset(nat->SoundMask, 0, sizeof nat->SoundMask);
	nat->GoldBunt = false;
	int z = 0;
	int cp;
	do
	{
		switch (mode)
		{
		case 0:
			z = Gscanf(f1, "%s", gg);
			NLine(f1);
			line++;
			GetMode(gg, &mode, line);
			break;
		case 1:
			//[MEMBERS]
			z = Gscanf(f1, "%s", gx);
			GetMode(gx, &mode, line);
			if (mode == 1 && gx[0] != '/'&&gx[0] != 0)
			{
				z = Gscanf(f1, "%s", gg);

				for (int cp = 0; cp < 8; cp++) {
					nat[cp].Mon[nat[cp].NMon] = (GeneralObject*)(new Visuals);
					CreateGOByName(nat[cp].Mon[nat[cp].NMon], gg, gx);
					nat[cp].Mon[nat[cp].NMon]->NatID = NatID;
					nat[cp].Mon[nat[cp].NMon]->Options = 0;
					nat[cp].NMon++;
                }
				mnm.AddString(gx);
            }
			NLine(f1);
			line++;
			break;
		case 5:
			//[PRODUCE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			if (gg[0] == '/' || gg[0] == 0) {
				NLine(f1);
				line++;
				break;
            }
			if (mode == 5)
				z = Gscanf(f1, "%d", &wid);
			NLine(f1);
			line++;
			if (mode == 5) {
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1) {
					sprintf(gy, "%s : Undefined identifier %s at line %d", fn, gg, line);
					Errx(gy);
                }
				for (cp = 0; cp < 8; cp++) {
					nat[cp].PACount[zz1] = wid;
					nat[cp].PAble[zz1] = new word[wid];
					nat[cp].AIndex[zz1] = new char[wid];
					nat[cp].AHotKey[zz1] = new char[wid];
                }
				for (int j = 0; j < wid; j++) {
					z = Gscanf(f1, "%s", gg);
					int	zz2 = mnm.SearchString(gg);
					if (zz2 == -1) {
						sprintf(gy, "%s : Undefined identifier %s at line %d", fn, gg, line);
						Errx(gy);
                    }
					for (cp = 0; cp < 8; cp++) {
						nat[cp].PAble[zz1][j] = zz2;
						nat[cp].AIndex[zz1][j] = -1;
						nat[cp].AHotKey[zz1][j] = 0;
                    }
					line++;
                }
            }
			break;
		case 6://COUNTRY
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 6)
			{
				zz1 = GetNationByID(gg);
				if (zz1 == -1)
				{
					sprintf(gz, "%s Line %d:[COUNTRY]: Invalid country ID(see Nations.lst):%s", fn, line, gg);
					Errx(gz);
				}
				z = Gscanf(f1, "%d", &parm1);
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].UnitNames[zz1] = new lpCHAR[parm1];
					nat[cp].UnitsIDS[zz1] = new word[parm1];
					nat[cp].NUnits[zz1] = parm1;
				}
				if (z != 1)
				{
					sprintf(gy, "%s Line %d :[COUNTRY]: Invalid amount of units.", fn, line);
					Errx(gy);
				}
				NLine(f1);
				for (int i = 0; i < parm1; i++)
				{
					z = Gscanf(f1, "%s", gy);
					if (gy[0] == '#')
					{
						for (cp = 0; cp < 8; cp++)
						{
							nat[cp].UnitNames[zz1][i] = new char[strlen(gy)];
						}
						normstr(gy);
						DosToWin(gy);
						strcpy(nat->UnitNames[zz1][i], gy + 1);
						for (cp = 0; cp < 8; cp++)
						{
							nat[cp].UnitsIDS[zz1][i] = 0xFFFF;
						}
					}
					else
					{
						zz2 = mnm.SearchString(gy);
						if (zz2 == -1)
						{
							sprintf(gz, "%s Line %d :[COUNTRY]: Invalid monster ID:%s", fn, line, gy);
							Errx(gz);
						}
						for (cp = 0; cp < 8; cp++)
						{
							nat[cp].UnitsIDS[zz1][i] = zz2;
							nat[cp].UnitNames[zz1][i] = nullptr;
						}
					}
					line++;
					NLine(f1);
				}
			}
			break;
		case 7://[UPGRADE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			if (mode == 7)
			{
				if (gg[0] == '/' || gg[0] == 0)
				{
					NLine(f1);
					line++;
				}
				else
				{
					if (!UnderstandUpgrade(f1, gg, nat, fn, &line, NatID))
					{
						sprintf(gy, "%s, Line %d :%s: Incorrect upgrade.", fn, line, gg);
						Errx(gy);
					}
				}
			}
			else
			{
				NLine(f1);
				line++;
			}
			break;
		case 8://[AUTOUPGRADE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			//NLine(f1);
			if (mode != 8)
			{
				NLine(f1);
				break;
			}
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			parm2 = GetUpgradeID(nat, gg);
			if (parm2 == -1)
			{
				sprintf(gz, "%s, Line %d :[AUTOUPGRADE]: Unknown parent upgrade:%s", fn, line, gg);
				Errx(gz);
			}
			z = Gscanf(f1, "%d", &parm1);
			if (z != 1)
			{
				sprintf(gz, "%s, Line %d :[AUTOUPGRADE]: Invalid amount of upgrades.", fn, line);
				Errx(gz);
			}
			for (cp = 0; cp < 8; cp++)
			{
				nat[cp].UPGRADE[parm2]->AutoPerform = new word[parm1];
				nat[cp].UPGRADE[parm2]->NAutoPerform = parm1;
			}
			for (parm3 = 0; parm3 < parm1; parm3++)
			{
				z = Gscanf(f1, "%s", gy);
				int zz = GetUpgradeID(nat, gy);
				if (zz == -1)
				{
					sprintf(gz, "%s, Line %d :[AUTOUPGRADE]: Unknown upgrade:%s", fn, line, gy);
					Errx(gz);
				}
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].UPGRADE[parm2]->AutoPerform[parm3] = zz;
				}
			}
			NLine(f1);
			break;
		case 9://[UPGRADELINKS]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			//NLine(f1);
			if (mode != 9)
			{
				NLine(f1);
				break;
			}
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			parm2 = GetUpgradeID(nat, gg);
			if (parm2 == -1)
			{
				sprintf(gz, "%s, Line %d :[UPGRADELINKS]: Unknown parent upgrade:%s", fn, line, gg);
				Errx(gz);
			}
			z = Gscanf(f1, "%d", &parm1);
			if (z != 1)
			{
				sprintf(gz, "%s, Line %d :[UPGRADELINKS]: Invalid amount of upgrades.", fn, line);
				Errx(gz);
			}
			for (cp = 0; cp < 8; cp++)
			{
				nat[cp].UPGRADE[parm2]->AutoEnable = new word[parm1];
				nat[cp].UPGRADE[parm2]->NAutoEnable = parm1;
			}
			for (parm3 = 0; parm3 < parm1; parm3++)
			{
				z = Gscanf(f1, "%s", gy);
				int zz = GetUpgradeID(nat, gy);
				if (zz == -1)
				{
					sprintf(gz, "%s, Line %d :[UPGRADELINKS]: Unknown upgrade:%s", fn, line, gy);
					Errx(gz);
				}
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].UPGRADE[parm2]->AutoEnable[parm3] = zz;
				}
			}
			NLine(f1);
			break;
		case 10://[FIXED_PRODUCE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				line++;
				break;
			}
			if (mode == 10)
			{
				z = Gscanf(f1, "%d", &wid);
				NLine(f1);
				line++;
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1)
				{
					sprintf(gy, "%s : Undefined identifier %s at line %d", fn, gg, line);
					Errx(gy);
				}
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].PACount[zz1] = wid;
					nat[cp].PAble[zz1] = new word[wid];
					nat[cp].AIndex[zz1] = new char[wid];
					nat[cp].AHotKey[zz1] = new char[wid];
                }
				for (int j = 0; j < wid; j++)
				{
					int xi, yi;
					z = Gscanf(f1, "%s%d%d%s", gg, &xi, &yi, gy);
					if (z != 4)
					{
						sprintf(gy, "%s : Not enough parameters at line %d", fn, line);
						Errx(gy);
					}
					int	zz2 = mnm.SearchString(gg);
					if (zz2 == -1)
					{
						sprintf(gy, "%s : Undefined identifier %s at line %d", fn, gg, line);
						Errx(gy);
					}
					for (cp = 0; cp < 8; cp++)nat[cp].PAble[zz1][j] = zz2;
					if (xi >= 12 || yi > 2)
					{
						sprintf(gy, "%s,Line %d :[FIXED_PRODUCE]: Invalid icon coordinates", fn, line);
						Errx(gy);
					}
					for (cp = 0; cp < 8; cp++)
					{
						nat[cp].AIndex[zz1][j] = xi + yi * 12;
						if ((!strcmp(gy, "NONE")) || (!strcmp(gy, "----")))nat[cp].AHotKey[zz1][j] = 0;
						else nat[cp].AHotKey[zz1][j] = gy[0];
					}
					line++;
				}
			}
			else
			{
				NLine(f1);
				line++;
			}
			break;
		case 19://[NOSEARCHVICTIM]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 19)
			{
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1)
				{
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
				}
				NLine(f1);
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].Mon[zz1]->NoSearchVictim = true;
				}
			}
			break;
		case 22://[LIFESHOTLOST]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 22)
			{
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1)
				{
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
				}
				z = Gscanf(f1, "%d", &parm1);
				NLine(f1);
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].Mon[zz1]->LifeShotLost = parm1;
				}
			}
			break;
		case 23://[STARTACCESS]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 23)
			{
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1)
				{
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
				}
				NLine(f1);
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].Mon[zz1]->Enabled = true;
				}
			}
			break;


		case 27://[CANBUILD]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 27)
			{
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1)
				{
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
				}
				NLine(f1);
			}
			break;

		case 36://[UPGRADEENABLE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 36)
			{
				int zz = GetUpgradeID(nat, gg);
				if (zz == -1)
				{
					sprintf(gz, "%s, Line %d:[UPGRADEENABLE]: Unknown upgrade ID: %s", fn, line, gg);
					Errx(gz);
				}
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].UPGRADE[zz]->Enabled = true;
					nat[cp].UPGRADE[zz]->PermanentEnabled = true;
				}
				NLine(f1);
			}
			break;
		case 81://[DISABLED_UPGRADE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 81)
			{
				int zz = GetUpgradeID(nat, gg);
				if (zz == -1)
				{
					sprintf(gz, "%s, Line %d:[DISABLED_UPGRADE]: Unknown upgrade ID: %s", fn, line, gg);
					Errx(gz);
				}
				for (cp = 0; cp < 8; cp++)nat[cp].UPGRADE[zz]->Gray = true;
				NLine(f1);
			}
			break;
		case 37://[PRIVATE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 37)
			{
				int zz = GetUpgradeID(nat, gg);

				if (zz == -1)
				{
					sprintf(gz, "%s, Line %d:[UPGRADEENABLE]: Unknown upgrade ID: %s", fn, line, gg);
					Errx(gz);
				}

				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].UPGRADE[zz]->Individual = true;
				}

				NLine(f1);
			}
			break;

		case 38://[UPGRADEPLACE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}

			if (mode == 38)
			{
				z = Gscanf(f1, "%d", &zz1);
				if (z != 1)
				{
					sprintf(gz, "%s, Line %d :[UPGRADEPLACE]: Invalid number of upgrades.", fn, line);
					Errx(gz);
				}
				zz2 = mnm.SearchString(gg);
				if (zz2 == -1)
				{
					sprintf(gz, "%s, Line %d :[UPGRADEPLACE]: Unknown parent monster ID: %s", fn, line, gg);
					Errx(gz);
				}
				line--;
				for (cp = 0; cp < 8; cp++)
				{
					GeneralObject* GO = nat[cp].Mon[zz2];
					GO->NUpgrades = zz1;
					GO->Upg = new word[zz1];
				}
				for (int i = 0; i < zz1; i++)
				{
					z = Gscanf(f1, "%s", gy);
					NLine(f1);
					line++;
					if (z != 1)
					{
						sprintf(gz, "%s, Line %d :[UPGRADEPLACE]: Could not read the name of the upgrade.", fn, line);
						Errx(gz);
					}
					int zz = GetUpgradeID(nat, gy);
					if (zz == -1)
					{
						sprintf(gz, "%s, Line %d :[UPGRADEPLACE]: Unknown upgade ID: %s", fn, line, gy);
						Errx(gz);
					}

					for (cp = 0; cp < 8; cp++)nat[cp].Mon[zz2]->Upg[i] = zz;
				}
			}
			break;

		case 40://[ENABLED]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 40)
			{
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1)
				{
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
				}
				NLine(f1);
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].Mon[zz1]->Enabled = true;
					nat[cp].Mon[zz1]->CondEnabled = true;
				}
			}
			break;
		case 41://[AUTO_PERFORM_UPGRADE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (mode != 41)
			{
				NLine(f1);
				break;
			}
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			NLine(f1);
			break;
		case 50://[ACCESSCONTROL]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 50)
			{
				if (nat->NCOND >= 4096)
				{
					sprintf(gx, "%d (%s) : [ASSESCONTROL] : Too many lines", line, fn);
					Errx(gx);
				}
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1)
				{
					zz1 = GetUpgradeID(nat, gg);
					if (zz1 == -1)
					{
						sprintf(gx, "%d ( %s ) : Unknown monster/upgrade identifief in [ACCESSCONTROL]: %s", line, fn, gg);
						Errx(gx);
					}
					else
					{
						zz1 += 8192;
					}
				}
				z = Gscanf(f1, "%d", &parm1);
				if (z != 1)
				{
					sprintf(gx, "%d (%s) : Invalid [ASSESCONTROL] for %s", line, fn, gg);
					Errx(gx);
				}
				word nc = nat->NCOND;
				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].NCOND++;
					nat[cp].CLSize[nc] = parm1;
					nat[cp].CLRef[nc] = new word[parm1 + 1];
					word* CRF = nat[cp].CLRef[nc];
					CRF[0] = zz1;
				}
				for (int i = 0; i < parm1; i++)
				{
					z = Gscanf(f1, "%s", gy);
					zz1 = mnm.SearchString(gy);
					if (zz1 == -1)
					{
						//upgrade?
						zz1 = GetUpgradeID(nat, gy);
						if (zz1 == -1)
						{
							sprintf(gx, "%d ( %s ) : Unknown monster/upgrade ID in [ACCESSCONTROL] : %s", line, fn, gy);
							Errx(gx);
						}
						else
						{
							for (cp = 0; cp < 8; cp++)nat[cp].CLRef[nc][i + 1] = zz1 + 8192;
						}
					}
					else
					{
						for (cp = 0; cp < 8; cp++)nat[cp].CLRef[nc][i + 1] = zz1;
					}
				}
				NLine(f1);
			}
			break;
		case 53://[OFFICERS]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}
			if (mode == 53)
			{
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1)
				{
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
				}
				int ns;
				z = Gscanf(f1, "%s%d", gx, &ns);
				int bid = mnm.SearchString(gx);
				if (bid == -1)
				{
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gx);
					Errx(gz);
				}
				NLine(f1);
				line++;
				for (cp = 0; cp < 8; cp++)
				{
					GeneralObject* GO = nat[cp].Mon[zz1];
					GO->OFCR = new OfficerRecord;
					GO->OFCR->BarabanID = bid;
					GO->OFCR->NStroi = ns;
				}
				for (int j = 0; j < ns; j++)
				{
					int na;
					z = Gscanf(f1, "%s%d", gy, &na);

					int z2 = 0;
					for (z2 = 0; z2 < NOClasses && strcmp(gy, OrderDesc[z2].ID); z2++);//BUGFIX: s/z/z2/
					//FUNNY: wtf officer formation buttons logic all fucked up
					//WHY U NAME variables like z or z2 and mix them up?

					if (z2 < NOClasses)
					{
						for (cp = 0; cp < 8; cp++)
						{
							StroiDescription* SDI = nat[cp].Mon[zz1]->OFCR->SDES + j;
							SDI->ID = z2;
						}
					}
					else
					{
						sprintf(gz, "( %s ) %d :Unknown order ID for[OFFICERS]:%s", fn, line, gy);
						Errx(gz);
					}

					for (cp = 0; cp < 8; cp++)
					{
						StroiDescription* SDI = nat[cp].Mon[zz1]->OFCR->SDES + j;
						SDI->NAmount = na;
						SDI->Amount = new word[na];
						SDI->LocalID = new word[na];
					}

					for (int p = 0; p < na; p++)
					{
						char cc1[128];
						z2 = Gscanf(f1, "%s", cc1);
						int t;
						for (t = 0; t < NEOrders&&strcmp(ElementaryOrders[t].ID, cc1); t++);
						if (t >= NEOrders)
						{
							sprintf(gz, "( %s ) %d :[OFFICERS]->%s->%s Invalid oder ID: %s.", fn, line, gg, gy, cc1);
							Errx(gz);
						}
						for (cp = 0; cp < 8; cp++)
						{
							StroiDescription* SDI = nat[cp].Mon[zz1]->OFCR->SDES + j;
							SDI->Amount[p] = ElementaryOrders[t].NUnits;
							SDI->LocalID[p] = t;
						}
					}
					z2 = Gscanf(f1, "%d", &na);
					if (z2 != 1)
					{
						sprintf(gz, "( %s ) %d :[OFFICERS]->%s->%s Invalid value.", fn, line, gg, gy);
						Errx(gz);
					}
					for (cp = 0; cp < 8; cp++)
					{
						StroiDescription* SDI = nat[cp].Mon[zz1]->OFCR->SDES + j;
						SDI->NUnits = na;
						SDI->Units = new word[na];
					}
					for (int p = 0; p < na; p++)
					{
						z2 = Gscanf(f1, "%s", gx);
						int ui = mnm.SearchString(gx);
						if (ui == -1)
						{
							sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gx);
							Errx(gz);
						}
						for (cp = 0; cp < 8; cp++)
						{
							StroiDescription* SDI = nat[cp].Mon[zz1]->OFCR->SDES + j;
							SDI->Units[p] = ui;
						}
					}
					NLine(f1);
					line++;
				}
			}
			break;

		case 73://[CANSETDEST]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0) {
				NLine(f1);
				break;
            }
			if (mode == 73) {
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1) {
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
                }
				NLine(f1);
				for (cp = 0; cp < 8; cp++) {
					nat[cp].Mon[zz1]->CanDest = true;
                }
            }
			break;
		case 74://[EXTMENU]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0) {
				NLine(f1);
				break;
            }
			if (mode == 74) {
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1) {
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
                }
				NLine(f1);
				nat->Mon[zz1]->ExtMenu = true;
            }
			break;

		case 76://[LOCKUNIT]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0) {
				NLine(f1);
				break;
            }
			if (mode == 76) {
				z = Gscanf(f1, "%d%s", &parm1, gx);
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1) {
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
                }
				parm3 = mnm.SearchString(gx);
				if (parm3 == -1) {
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gx);
					Errx(gz);
                }
				NLine(f1);
				for (cp = 0; cp < 8; cp++) {
					nat[cp].Mon[zz1]->LockID = parm3;
					nat[cp].Mon[zz1]->NLockUnits = parm1;
                }
            }
			break;
		case 82://[SPECIAL_UNIT]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0) {
				NLine(f1);
				break;
            }
			if (mode == 82) {
				int z1, z2, z3, z4, z5, z6, z7, z8;
				z1 = 0; z2 = 0; z3 = 0; z4 = 0; z5 = 0; z6 = 0; z7 = 0; z8 = 0;
				z = Gscanf(f1, "%d%d%d%d%d%d%d%d", &z1, &z2, &z3, &z4, &z5, &z6, &z7, &z8);
				zz1 = mnm.SearchString(gg);
				if (zz1 == -1) {
					sprintf(gz, "( %s ) %d :Undefined monster : %s", fn, line, gg);
					Errx(gz);
                }
				for (cp = 0; cp < 8; cp++) {
					nat[cp].Mon[zz1]->Options = 0 != (z1 + (z2 << 1) + (z3 << 2) + (z4 << 3) + (z5 << 3) + (z6 << 3) + (z7 << 3) + (z8 << 3));
                }
            }
			break;
		case 83://[SPECIAL_UPGRADE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0) {
				NLine(f1);
				break;
            }
			if (mode == 83) {
				int z1, z2, z3, z4, z5, z6, z7, z8;
				z1 = 0; z2 = 0; z3 = 0; z4 = 0; z5 = 0; z6 = 0; z7 = 0; z8 = 0;
				z = Gscanf(f1, "%d%d%d%d%d%d%d%d", &z1, &z2, &z3, &z4, &z5, &z6, &z7, &z8);
				zz1 = GetUpgradeID(nat, gg);
				if (zz1 == -1) {
					sprintf(gz, "( %s ) %d :Undefined upgrade : %s", fn, line, gg);
					Errx(gz);
                }
				for (cp = 0; cp < 8; cp++) {
					NewUpgrade* NU = nat[cp].UPGRADE[zz1];
					NU->Options = z1 + (z2 << 1) + (z3 << 2) + (z4 << 3) + (z5 << 3) + (z6 << 3) + (z7 << 3) + (z8 << 3);
                }
            }
			break;

		case 84://[UPGRADESTAGE]
			z = Gscanf(f1, "%s", gg);
			GetMode(gg, &mode, line);
			line++;
			if (gg[0] == '/' || gg[0] == 0)
			{
				NLine(f1);
				break;
			}

			if (mode == 84)
			{
				int zz = GetUpgradeID(nat, gg);
				if (zz == -1)
				{
					sprintf(gz, "%s, Line %d:[UPGRADESTAGE]: Unknown upgrade ID: %s", fn, line, gg);
					Errx(gz);
				}

				for (cp = 0; cp < 8; cp++)
				{
					nat[cp].UPGRADE[zz]->StageUp = true;
				}

				NLine(f1);
			}
			break;

		case 255://[END]
			z = 0;
			break;
		}
	} while (z);
	Gclose(f1);
}
