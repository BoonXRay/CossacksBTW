#include "ActiveScenary.h"
#include "ConstStr.h"
#include "DrawForm.h"
#include "Economy.h"
#include "FastDraw.h"
#include "GameSound.h"
#include "IconTool.h"
#include "main.h"
#include "Mapa.h"
#include "MapSprites.h"
#include "Mouse_X.h"
#include "Multi.h"
#include "Nation.h"
#include "Nature.h"
#include "NewMon.h"
#include "SelProp.h"
#include "Transport.h"
#include "UnSyncro.h"
#include "Walls.h"


#include "Mine.h"


bool OneObject::GoToMine( word ID, byte Prio )
{
	return GoToMine( ID, Prio, 0 );
}

bool OneObject::GoToMine( word ID, byte Prio, byte Type )
{
	if (UnlimitedMotion)return false;
	if (CheckOrderAbility())return false;
	NewMonster* NM = newMons;
	OneObject* OB = Group[ID];
	if (!OB)return false;
	if (OB->Sdoxlo || !OB->Ready)return false;

	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	NewMonster* MiNM = OB->newMons;

	if (!( newMons->Peasant || newMons->Usage == PeasantID || newMons->CanStorm ))return false;

	if (!( NewMonst && ( MiNM->UnitAbsorber ||
		( MiNM->PeasantAbsorber&&NM->Peasant ) ) ))return false;
	if (OB->NInside >= ADC->MaxInside + OB->AddInside)return false;
	if (!( MiNM->NConcPt&&MiNM->NBornPt ))return false;
	if (PrioryLevel > Prio)return false;
	if (LocalOrder&&LocalOrder->OrderType == 242
		&& LocalOrder->info.BuildObj.ObjIndex == ID)return true;
	Order1* Or1 = CreateOrder( Type );
	if (!int( Or1 ))return false;
	Or1->PrioryLevel = Prio & 127;
	Or1->OrderType = 242;
	Or1->OrderTime = 0;
	Or1->DoLink = &GoToMineLink;
	int xm, ym;
	OB->GetCornerXY( &xm, &ym );
	Or1->info.BuildObj.ObjIndex = ID;
	Or1->info.BuildObj.SN = OB->Serial;
	Or1->info.BuildObj.ObjX = 0;
	//Or1->info.BuildObj.ObjY=ym+MiNM->ConcPtY[0];
	//PrioryLevel=Prio&127;
	UnBlockUnit();
	MakeOrderSound( this, 2 );
	return true;
}

void GoToMineLink( OneObject* OBJ )
{
	//int xx=OBJ->LocalOrder->info.BuildObj.ObjX;
	//int yy=OBJ->LocalOrder->info.BuildObj.ObjY;
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	int n = OBJ->LocalOrder->info.BuildObj.ObjX;
	word OID = OBJ->LocalOrder->info.BuildObj.ObjIndex;
	word OSN = OBJ->LocalOrder->info.BuildObj.SN;
	OneObject* OB = Group[OID];
	if (!OB)
	{
		if (OBJ->UnlimitedMotion)OBJ->UnlimitedMotion = false;
		OBJ->DeleteLastOrder();
		return;
    }
	int xx, yy, xm, ym;
	OB->GetCornerXY( &xm, &ym );
    NewMonster* MiNM = OB->newMons;
	xx = xm + MiNM->ConcPtX[n];
	yy = ym + MiNM->ConcPtY[n];
	AdvCharacter* ADC = OB->Ref.General->MoreCharacter;
	if (( !OB->Sdoxlo ) && ( OB->Serial & 0xFFFE ) == ( OSN & 0xFFFE ) && OB->NInside < ADC->MaxInside + OB->AddInside)
	{
		int dst = Norma( OBJ->x - xx, OBJ->y - yy );
		if (dst < 2)
		{
			OBJ->PrioryLevel = 127;
			if (n + 1 >= MiNM->NConcPt)
			{
				//checking for siege
				if (OBJ->NMask&OB->NMask)
				{
					//Hiding
GGG1:
					word* Ins = new word[OB->NInside + 1];
					if (OB->NInside)
					{
						memcpy( Ins, OB->Inside, OB->NInside << 1 );
						free( OB->Inside );
                    }
					OB->Inside = Ins;
					Ins[OB->NInside] = OBJ->Index;
					OBJ->HideMe();
					OBJ->ClearOrders();
					OB->NInside++;
				}
				else
				{
					if (!OB->NInside)
					{
						byte OldNat = OB->NNUM;
						DelObject( OB );
						OB->Nat->CITY->UnRegisterNewUnit( OB );
						int oldst = OBJ->Stage;
						int oldmax = OBJ->Ref.General->MoreCharacter->ProduceStages;
						OB->Ref.General = NATIONS[OBJ->NNUM].Mon[OB->NIndex];
						OB->Nat = &NATIONS[OBJ->NNUM];
						OB->NMask = OBJ->Nat->NMask;
						OB->Serial ^= 1;
						OB->Selected = 0;
						OB->ImSelected = 0;
						OB->Zombi = false;
						if (OB->NewBuilding&&OldNat == MyNation)
						{
							LastAttackDelay = 900;
							if (AlarmSoundID != -1)
							{
								AddEffect( ( mapx << 5 ) + 500, ( mapy << 4 ) + 300, AlarmSoundID );
								LastActionX = OBJ->RealX;
								LastActionY = OBJ->RealY;
								AlarmDelay = 60;
                            }
                        }
						OB->NNUM = OBJ->NNUM;
						OB->Nat->CITY->RegisterNewUnit( OB );
						AddObject( OB );
						if (OB->NewBuilding)
							OBJ->Stage = ( oldst*int( OBJ->Ref.General->MoreCharacter->ProduceStages ) ) / oldmax;
						goto GGG1;
					}
					else
					{
						if (OBJ->NewAnm == OBJ->newMons->Attack)
						{
							if (OBJ->NewCurSprite >= OBJ->NewAnm->NFrames - FrmDec)
							{
								if (OBJ->newMons->StormForce)
								{
									int N = OB->NInside;
									N = ( int( rando() )*N ) >> 15;
									word MID = OB->Inside[N];
									if (MID != 0xFFFF)
									{
										OneObject* IOB = Group[MID];
										if (IOB)
										{
											int sf = IOB->newMons->StormForce;
											int min = OB->newMons->MinOposit;
											int max = OB->newMons->MaxOposit;
											int ins = OB->NInside;
											int mins = OB->Ref.General->MoreCharacter->MaxInside;
											int VV = ( ( sf*( min + ( ( max - min )*ins ) / mins ) )*int( rando() ) ) >> 15;
											if (VV < OBJ->newMons->StormForce)IOB->Die();
                                        }
                                    }
                                }
								OBJ->Die();
                            }
							return;
						}
						else
						{
							OBJ->InMotion = 0;
							OBJ->NewAnm = OBJ->newMons->Attack;
                            OBJ->NewCurSprite = 0;
							return;
                        }
                    }
                }
			}
			else
			{
				OBJ->UnlimitedMotion = true;
				OBJ->LocalOrder->info.BuildObj.ObjX++;
				n++;
				xx = xm + MiNM->ConcPtX[n];
				yy = ym + MiNM->ConcPtY[n];
				OBJ->CreatePath( xx, yy );
            }
		}
		else OBJ->CreatePath( xx, yy );
	}
	else
	{
		if (OBJ->UnlimitedMotion)
		{
			OBJ->ClearOrders();
			OBJ->UnlimitedMotion = false;
			OBJ->SetOrderedUnlimitedMotion( 2 );
			xx = xm + MiNM->ConcPtX[0];
			yy = ym + MiNM->ConcPtY[0];
			OBJ->NewMonsterSendTo( xx << 8, yy << 8, 16, 2 );
			OBJ->ClearOrderedUnlimitedMotion( 2 );
		}
		else
		{
			OBJ->DeleteLastOrder();
			OBJ->UnlimitedMotion = false;
        }
    }
}

void OneObject::LeaveMine( word Type )
{
	if (UnlimitedMotion)return;
	if (!NInside)return;
	if (newMons->Transport)
	{
		LeaveTransport( Type );
		return;
    }
	Order1* OR1 = CreateOrder( 1 );
	OR1->info.BuildObj.ObjIndex = Type;
	OR1->info.BuildObj.ObjX = 32;
	OR1->DoLink = &LeaveMineLink;
	OR1->PrioryLevel = 0;
}

void LeaveMineLink( OneObject* OBJ )
{
	OBJ->PrioryLevel = OBJ->LocalOrder->PrioryLevel;
	OBJ->LocalOrder->info.BuildObj.ObjX--;
	if (OBJ->LocalOrder->info.BuildObj.ObjX)return;
	if (!OBJ->NInside)
	{
		OBJ->DeleteLastOrder();
		return;
    }
	OneObject* OB = NULL;
	word p;
	word Type = OBJ->LocalOrder->info.BuildObj.ObjIndex;
	for (int i = 0; i < OBJ->NInside && !OB; i++)
	{
		p = OBJ->Inside[i];
		OB = Group[p];
		if (OB&&OB->NIndex == Type)
		{
			if (i + 1 < OBJ->NInside)memcpy( OBJ->Inside + i, OBJ->Inside + i + 1, ( OBJ->NInside - i - 1 ) << 1 );
			OBJ->NInside--;
		}
		else OB = NULL;
    }
	if (!OB)return;
	if (!OBJ->NInside)
	{
		free( OBJ->Inside );
		OBJ->Inside = NULL;
    }
	OB->ShowMe();
	NewMonster* MiNM = OBJ->newMons;
	int nc = MiNM->NConcPt;
    int xx, yy;
	OB->UnlimitedMotion = 0;
	OB->SetOrderedUnlimitedMotion( 0 );
	OBJ->GetCornerXY( &xx, &yy );
	for (int i = 1; i < nc; i++)
	{
		OB->NewMonsterSendTo( ( xx + MiNM->ConcPtX[nc - i - 1] + 1 ) << 8, ( yy + MiNM->ConcPtY[nc - i - 1] + 1 ) << 8, 16, 2 );
    }
	OB->ClearOrderedUnlimitedMotion( 2 );
	OB->UnlimitedMotion = true;

	OBJ->DeleteLastOrder();

}

static void DeleteFromSelection( OneObject* OB )
{
	if (!OB)return;
	for (int i = 0; i < 8; i++)
	{
		if (OB->ImSelected&( 1 << i ))
		{
			int Nsel = ImNSL[i];
			word* SMon = ImSelm[i];
			word ID = OB->Index;
			for (int f = 0; f < Nsel; f++)if (SMon[f] == ID)SMon[f] = 0xFFFF;
			OB->ImSelected &= ~GM( i );
			CmdUnSelUnitsSet( i, &ID, 1 );
        }
    }
}

void OneObject::HideMe()
{
	Hidden = true;
	Sdoxlo = 1;
	ClearOrders();
	DeleteFromSelection( this );
	DestX = -1;
	DestY = -1;
	NewAnm = &newMons->Stand;
	NewCurSprite = 0;
	InMotion = false;
	DeletePath();
}

void OneObject::ShowMe()
{
	Hidden = false;
	Sdoxlo = 0;
}

static void PushUnitOutOfMine( int i )
{
	if (GetKeyState( VK_SHIFT ) & 0x8000)
	{
		CmdLeaveMine( MyNation, i );
		CmdLeaveMine( MyNation, i );
		CmdLeaveMine( MyNation, i );
		CmdLeaveMine( MyNation, i );
		CmdLeaveMine( MyNation, i );
	}
	else
	{
		CmdLeaveMine( MyNation, i );
	}
}

void CmdCreateGates( byte NI );

static void DoGates( int /*i*/ )//typedef void HandlePro(int);
{
	CmdCreateGates( MyNation );
}

static void OpGates( int /*i*/ )//typedef void HandlePro(int);
{
	CmdOpenGates( MyNation );
}

static void ClGates( int /*i*/ )//typedef void HandlePro(int);
{
	CmdCloseGates( MyNation );
}

static uint16_t LastOFCR = 0xFFFF;
static uint16_t create_formation_type = 0;

static void OfficerCallback( int i )//typedef void HandlePro(int);
{
	create_formation_type = i;
}

static void CrBrig( int i )//typedef void HandlePro(int);
{
	CmdCrBig( MyNation, i );
}

bool SetDestMode = false;

static void SDS_Pro( int /*i*/ )//typedef void HandlePro(int);
{
	SetDestMode = true;
}


static void ZAGLUXA( int /*i*/ )//typedef void HandlePro(int);
{}

//Calls CmdStopUpgrade()
//int i = OneObject->Index of the building, in which the upgrade is running
static void StopUpgrade( int /*i*/ )//typedef void HandlePro(int);
{
	CmdStopUpgrade( MyNation );
}

static void UNLOAD( int /*i*/ ) //typedef void HandlePro(int);
{
	CmdUnloadAll( MyNation );
}

static void CHGUARD( int i )
{
	if (i)
	{
		GUARDMODE = 1;
	}
	else
	{
		CmdSetGuardState( MyNation, 0xFFFF );
	}
}

static void SELECT1( int i )
{
	if (i == 0)
	{
		CmdSelectIdlePeasants( MyNation );
	}
	else if (i == 1)
	{
		CmdSelectIdleMines( MyNation );
	}
}

static void ChOrdN( int i )
{
	i -= 2;
	if (create_formation_type)
	{
		if (i < 0)
		{
			if (create_formation_type > 256)
			{
				create_formation_type -= 256;
			}
		}
		else
		{
			create_formation_type += 256;
		}
	}
	Lpressed = 0;
}

int DecOrdID = 43;
int IncOrdID = 47;
bool AttGrMode = 0;

void ATTGR_PRO( int /*p*/ )//typedef void HandlePro(int);
{
	AttGrMode = 1;
}

//Displays special abilities for various units or buildings
//Curious: handles officers' abilities, but not for existing formations
bool CreateInsideList( IconSet* IS, uint8_t NI )
{
	if (PlayGameMode == 1)
	{
		return false;
	}

	word Nmons[1024];
	memset( Nmons, 0, 2048 );

	int Nsel = ImNSL[MyNation];
	word* SMon = ImSelm[NI];
	if (!Nsel)
	{
		return false;
	}

	word MID;
	bool OneType = 1;
	word TypeID = 0xFFFF;

	if (Nsel)
	{
		//checking for gates
		bool OpnGate = false;
		bool ClsGate = false;
		bool OnlyWall = true;
		bool OnlyDest = true;
		bool Usual = 0;
		bool Guards = 0;
		bool HaveArtpodg = 0;
		bool HaveNoArtpodg = 0;

		WallCharacter* WCR = nullptr;
		for (int i = 0; i < Nsel; i++)
		{
			MID = SMon[i];
			if (MID != 0xFFFF)
			{
				OneObject* OB = Group[MID];
				if (OB)
				{
					if (OB->Wall)
					{
						if (CheckOpenGate( OB ))
						{
							OpnGate = true;
						}
						if (CheckCloseGate( OB ))
						{
							ClsGate = true;
						}
						WCR = GetWChar( OB );
					}
					else
					{
						OnlyWall = false;
					}

					if (!OB->Ref.General->CanDest)
					{
						OnlyDest = false;
					}

					int ID = OB->NIndex;

					if (TypeID == 0xFFFF)
					{
						TypeID = ID;
					}
					else
					{
						if (TypeID != ID)
						{
							OneType = 0;
						}
					}

					if (!( OB->Sdoxlo || OB->LockType || OB->newMons->Capture || OB->NewBuilding ))
					{
						if (OB->Guard == 0xFFFF)
						{
							Usual = 1;
						}
						else
						{
							Guards = 1;
						}
					}

					if (!OB->Sdoxlo)
					{
						if (OB->newMons->Artpodgotovka)
						{
							HaveArtpodg = 1;
						}
						else
						{
							HaveNoArtpodg = 1;
						}
					}
				}
			}
		}

		if (OnlyDest)
		{
			OneIcon* OI = IS->AddIconFixed( 0, SET_DEST_ICON, SDS_X + SDS_Y * 12 );
			OI->CreateHint( SDS_Str );
			OI->AssignLeft( &SDS_Pro, 0 );
		}

		if (OnlyWall)
		{
			if (OpnGate)
			{
				OneIcon* OI = IS->AddIcon( 0, WCR->OpenGateIcon );
				OI->CreateHint( OPENGAT );//"Open gates."
				OI->AssignLeft( &OpGates, 0 );
			}

			if (ClsGate)
			{
				OneIcon* OI = IS->AddIcon( 0, WCR->CloseGateIcon );
				OI->CreateHint( CLOSGAT );//"Close gates."
				OI->AssignLeft( &ClGates, 0 );
			}
		}

		MID = SMon[0];
		if (MID == 0xFFFF)
		{
			return false;
		}

		OneObject* OB = Group[MID];
		if (!OB)
		{
			return false;
		}

		GeneralObject* GO = OB->Ref.General;
		if (OneType && !( OB->newMons->Capture || OB->NewBuilding
			|| OB->Transport || OB->Wall || OB->newMons->Usage == PeasantID ))
		{
			if (Guards && !Usual)
			{
				OneIcon* OI = IS->AddIconFixed( 0, NOGUARD_ICON, 13 );
				OI->CreateHint( HNOGUARD );
				OI->AssignLeft( &CHGUARD, 0 );
			}

			if (!Guards)
			{
				OneIcon* OI = IS->AddIconFixed( 0, DOGUARD_ICON, 13 );
				OI->CreateHint( HDOGUARD );
				OI->AssignLeft( &CHGUARD, 1 );
			}
		}

		if (HaveArtpodg && !HaveNoArtpodg && !SCENINF.hLib)
		{
			OneIcon* OI = IS->AddIconFixed( 0, ATTGR_ICON, AttGrPos );
			OI->CreateHint( ATGHINT );
			OI->AssignLeft( &ATTGR_PRO, 0 );
		}

		if (Nsel == 1 || ( GO->OFCR && OneType ))
		{
			byte Usage = OB->newMons->Usage;
			if (Usage == CenterID && OB->Ready)
			{
				OneIcon* OI = IS->AddIconFixed( 0, IDLE_PICON, IDLE_PX + IDLE_PY * 12 );
				OI->CreateHint( IDLE_P );
				OI->AssignLeft( &SELECT1, 0 );
				OI = IS->AddIconFixed( 0, IDLE_MICON, IDLE_MX + IDLE_MY * 12 );
				OI->CreateHint( IDLE_M );
				OI->AssignLeft( &SELECT1, 1 );
			}

			if (( OB->NewBuilding || OB->LockType )
				&& OB->NUstages && ( !OB->Ready )
				&& OB->LocalOrder && OB->LocalOrder->DoLink == &PerformUpgradeLink)
			{//Selected building, upgrade in progress
				NewUpgrade* NU = OB->Nat->UPGRADE[OB->LocalOrder->info.PUpgrade.NewUpgrade];
				OneIcon* OI = IS->AddIconFixed( 0, NU->IconSpriteID, 0 );
				OI->MoreSprite = STOP_ICON;
				OI->CreateHint( STOPUPG );
				OI->AssignLeft( &StopUpgrade, OB->Index );
				OI->AssignColor( 0xD0 + MyNation * 4 );
				OI = IS->AddIconFixed( 0, NU->IconSpriteID, 4 );
				OI->CreateHint( NU->Message );
				OI->AssignLeft( &ZAGLUXA, 0 );
				OI->AssignLevel( NU->Level );
				OI->AssignColor( 0xD0 + MyNation * 4 );
                int y0 = AblY + 10;
				int LX = 113;
                int L = ( (LX) *OB->Ustage ) / OB->NUstages;
				TempWindow TW;
				PushWindow( &TW );
				GPS.ShowGP( AblX + 44, AblY, BordGP, 83, 0 );
				IntersectWindows( AblX, AblY, AblX + 44 + 4 + L, AblY + 40 );
				GPS.ShowGP( AblX + 44, AblY, BordGP, 84, 0 );
				PopWindow( &TW );
				char cc[20];
				sprintf( cc, "%d%%", ( OB->Ustage * 100 ) / OB->NUstages );
				ShowString( AblX + 60 + 44 - GetRLCStrWidth( cc, &SmallYellowFont ) / 2, y0 + 4, cc, &SmallYellowFont );
			}

			//Officer abilities (cavalry too)
			if (GO->OFCR)
			{
				if (OB->Index != LastOFCR)
				{
					LastOFCR = OB->Index;
					create_formation_type = 0;
				}

				OfficerRecord* OFCR = GO->OFCR;
				bool are_drummers_nearby = 0 != EnumUnitsInRound( OB->RealX, OB->RealY, MobilR * 16, OFCR->BarabanID, NatRefTBL[MyNation] );
				for (int i = 0; i < OFCR->NStroi; i++)
				{
					int id = OFCR->SDES[i].ID;
					if (create_formation_type && !i)
					{//Puls & minus buttons for changing the size of formation to be created
						OneIcon* OID = IS->AddIconFixed( 0, DecOrdID, OrderDesc[id].IconPos - 2 );
						OID->AssignLeft( &ChOrdN, 3 );
						OID->CreateHint( GetTextByID( "DECFORM" ) );

						OneIcon* OII = IS->AddIconFixed( 0, IncOrdID, OrderDesc[id].IconPos - 1 );
						OII->AssignLeft( &ChOrdN, 1 );
						OII->CreateHint( GetTextByID( "INCFORM" ) );
					}

					//Formation icon
					OneIcon* OI = IS->AddIconFixed( 0, OrderDesc[id].IconID, OrderDesc[id].IconPos );

					if (are_drummers_nearby)
					{
						OI->AssignLeft( &OfficerCallback, i + 1 );
						OI->CreateHint( OrderDesc[id].Message );
					}
					else
					{//Disable formation abilities if there are no drummers nearby
						OI->Disable();
						OI->CreateHint( OrderDesc[id].Message );
						create_formation_type = 0;
					}
				}

				if (create_formation_type)
				{//Add unit types / quantities icons next to formation type icon
					StroiDescription* SDS = GO->OFCR->SDES + ( ( create_formation_type - 1 ) & 255 );
					int DD = create_formation_type >> 8;
					bool AtLeastOne = 0;

					for (int j = 0; j < SDS->NUnits; j++)
					{
						int NNear = EnumUnitsInRound( OB->RealX, OB->RealY, MobilR * 16, SDS->Units[j], NatRefTBL[MyNation] );

						int p;
						for (p = 0; p < SDS->NAmount && SDS->Amount[p] <= NNear; p++);

						OneIcon* OI = IS->AddIconFixed( 0,
							NATIONS[MyNation].Mon[SDS->Units[j]]->newMons->IconID,
							OrderDesc[SDS->ID].IconPos + j + 1 );

						p -= DD;
						if (p > 0)
						{
							OI->AssignIntVal( SDS->Amount[p - 1] );

							OI->AssignLeft( &CrBrig, int( DWORD( OB->Index & 8191 )
								+ DWORD( SDS->LocalID[p - 1] ) * 8192
								+ DWORD( SDS->Units[j] ) * 8192 * 256 ) );

							OI->CreateHint( NATIONS[MyNation].Mon[SDS->Units[j]]->Message );

							AtLeastOne = 1;
						}
						else
						{
							OI->Disable();
						}
					}

					if (DD && !AtLeastOne)
					{
						DD--;
					}

					create_formation_type = ( create_formation_type & 255 ) + ( DD << 8 );
				}
			}
			else
			{
				LastOFCR = 0xFFFF;
			}
		}
		else
		{
			LastOFCR = 0xFFFF;
		}

		if (GO->newMons->Rinok&&OB->Ready)
		{
			EconomyInterface( IS );
		}

		if (OB->Wall)
		{
			WallCharacter* WCR = GetWChar( OB );
			if (CheckGateUpgrade( OB ))
			{
				if (WCR)
				{
					OneIcon* OI = IS->AddIcon( 0, WCR->UpgradeGateIcon );
                    const char* Text = GetTextByID( "CreateGates." );
					char ccc[128];
					strcpy( ccc, Text );
					for (int k = 0; k < 8; k++)
					{
						if (WCR->GateCost[k])
						{
							char cc1[64];
							sprintf( cc1, "%s %d ", RDS[k].Name, WCR->GateCost[k] );
							strcat( ccc, cc1 );
						}
					}

					OI->CreateHint( ccc );
					OI->AssignLeft( &DoGates, 0 );
					return true;
				}
			}
		}
	}

	int Per = false;
	for (int i = 0; i < Nsel; i++)
	{
		MID = SMon[i];
		if (MID != 0xFFFF)
		{
			OneObject* OB = Group[MID];
			if (OB&&OB->NInside&&OB->Ready)
			{
				if (OB->Transport)Per = true;
                int Nins = OB->NInside;
				word* INS = OB->Inside;
				for (int j = 0; j < Nins; j++)
				{
					word InMID = INS[j];
					if (InMID != 0xFFFF)
					{
						OneObject* INO = Group[InMID];
						if (INO)
						{
							Nmons[INO->NIndex]++;
							//assert(INO->NIndex<1024);
                        }
                    }
                }
				Order1* OR1 = OB->LocalOrder;
				while (OR1)
				{
					if (OR1->DoLink == &LeaveMineLink || OR1->DoLink == &LeaveTransportLink)
					{
						word IID = OR1->info.BuildObj.ObjIndex;
						//assert(IID<1024);
						if (Nmons[IID])Nmons[IID]--;
                    }
					OR1 = OR1->NextOrder;
                }
            }
        }
    }
	Nation* NT = &NATIONS[NI];
	bool Pres = 0;
	for (int i = 0; i < 1024; i++)
	{
		if (Nmons[i])
		{
			GeneralObject* GO = NT->Mon[i];
			NewMonster* NM = GO->newMons;
			OneIcon* OI = IS->AddIcon( NM->IconFileID, NM->IconID, 0, NM->Message );
			OI->AssignIntVal( Nmons[i] );
			OI->AssignLeft( &PushUnitOutOfMine, i );
			Pres = 1;
        }
    }
	if (Pres&&Per)
	{
		OneIcon* OI = IS->AddIconFixed( 0, UNLOAD_ICON, 12 * 2 );
		OI->CreateHint( UNLOADALL );
		OI->AssignLeft( &UNLOAD, 0 );
    }
	return true;
}

void OneObject::TakeResourceFromSprite( int SID )
{
	if (SID == INITBEST || SID == -1)
	{
		return;
	}

	if (UnlimitedMotion)
	{
		return;
	}

	if (CheckOrderAbility())
	{
		return;
	}

	NewMonster* NM = newMons;

	OneSprite* OS = &Sprites[SID];
	if (!OS->Enabled)
	{
		return;
	}

	ObjCharacter* OC = OS->OC;
	if (OC->IntResType > 8)
	{
		return;
	}

	byte msk = 1 << OC->IntResType;
	if (msk & NM->ProdType)
	{
		RType = OC->IntResType;
		RAmount = 0;
		Order1* OR1 = CreateOrder( 0 );
		OR1->DoLink = &TakeResourceFromSpriteLink;
		PrioryLevel = 0;
		OR1->info.BuildObj.ObjIndex = SID;
		OR1->info.BuildObj.SN = 0;
	}
}

void TakeResourceFromSpriteLink( OneObject* OBJ )
{
	int SID = OBJ->LocalOrder->info.BuildObj.ObjIndex;

	OneSprite* OS = &Sprites[SID];
	if (!OS->Enabled)
	{
		OBJ->DeleteLastOrder();
		return;
	}

	ObjCharacter* OC = OS->OC;
	if (OBJ->RType != OC->IntResType)
	{
		OBJ->DeleteLastOrder();
		return;
	}

	int NT = OBJ->NNUM;
	int IRES = OBJ->LocalOrder->info.BuildObj.SN;

	NewMonster* NM = OBJ->newMons;

	int work = ( NM->FreeAdd + OBJ->NInside*NM->PeasantAdd ) << ( 2 + SpeedSh );

	byte RTP = OBJ->RType;
	if (RTP == GoldID)
	{
		NInGold[NT] += OBJ->NInside;
	}
	else
	{
		if (RTP == IronID)
		{
			NInIron[NT] += OBJ->NInside;
		}
		else
		{
			if (RTP == CoalID)
			{
				NInCoal[NT] += OBJ->NInside;
			}
		}
	}

	IRES += work;
	OBJ->RAmount += work;
	if (IRES > 1000)
	{
		div_t qq = div( IRES, 1000 );
        /*int work =*/ OS->PerformIntWork( qq.quot );
		AddXRESRC( OBJ->NNUM, OBJ->RType, qq.quot );
		Nation* NT = NATIONS + OBJ->NNUM;
		NT->AddResource( OBJ->RType, qq.quot );
		NT->ResTotal[OBJ->RType] += qq.quot;
		IRES = qq.rem;
	}

	OBJ->LocalOrder->info.BuildObj.SN = IRES;
	if (rando() < 250)
	{
		if (!OBJ->Nat->Harch)
		{
			if (OBJ->NInside)
			{
				word Last = OBJ->Inside[OBJ->NInside - 1];
				if (Last != 0xFFFF)
				{
					OneObject* OB = Group[Last];
					if (OB)
					{
						OB->Nat->CITY->UnRegisterNewUnit( OB );
						OB->CloseObject();
						OBJ->NInside--;
						DelObject( OB );
						Group[OB->Index] = nullptr;
					}
				}
			}
		}
	}
}

char* GetSprResourceName( OneObject* OB )
{
	Order1* ORD = OB->LocalOrder;
	if (ORD&&ORD->DoLink == &TakeResourceFromSpriteLink)
	{
		int rt = OB->RType;
		if (rt >= 8)
		{
			return nullptr;
		}
		return RDS[rt].Name;
	}
	return nullptr;
}

//Order the acquisition of resources from mines
void HandleMines()
{
	if (0 == tmtmt % 256)
	{
		for (int i = 0; i < MAXOBJECT; i++)
		{
			OneObject* OB = Group[i];
			if (OB && OB->newMons->ProdType)
			{
				if (!OB->LocalOrder)
				{
					int xx = OB->RealX;
					int yy = OB->RealY;
					int ID = CheckMinePosition( OB->newMons, &xx, &yy, 64 );
					if (ID != -1)
					{
						OB->TakeResourceFromSprite( ID );
					}
				}
			}
		}
	}
}
