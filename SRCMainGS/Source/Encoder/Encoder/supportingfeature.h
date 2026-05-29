#pragma once
#define KIND_PLAYER							1
#define KIND_MONSTER						2
#define KIND_NPC							4
#define KIND_TRAP							8
#define KIND_OPERATE						16
#define KIND_EDIT							32
#define KIND_PET							64
#define KIND_TMP							128


#define RENDER_MESH_OBJ_ITEM				0
#define RENDER_MESH_OBJ_MONSTER				1

class CUSTOM_ITEM_INFO
{
public:
	int Itemindex;
	DWORD RenderColor;
	char filename[32];
public:
	CUSTOM_ITEM_INFO() {
		release();
	};

	void release() {
		Itemindex = -1;
		filename[0] = '\0';
		RenderColor = -1;
	};
};

class CUSTOM_MONSTER_INFO
{
public:
	int MonsterIndex;
	int RenderIndex;
	int Kind;
	float fSize;
	char Name[32];
	char directory[64];
	char filename[32];
public:
	CUSTOM_MONSTER_INFO() {
		release();
	};

	void release() {
		MonsterIndex = -1;
		directory[0] = '\0';
		filename[0] = '\0';
	};
};

class CUSTOM_MAPE_INFO
{
public:
	int mapIndex;
	int checkIndex;
	int textIndex;
	int TerrainExt;
	char ImgName[32];
	char lpszMp3[32];
public:
	CUSTOM_MAPE_INFO() {
		release();
	};

	void release() {
		mapIndex = -1;
		checkIndex = -1;
		ImgName[0] = '\0';
		lpszMp3[0] = '\0';
	};
};

class CUSTOM_IMAGE_INFO
{
public:
	int ImgIndex;
	char ImgName[32];
	int Wrap;
	int Type;
public:
	CUSTOM_IMAGE_INFO() {
		release();
	};

	void release() {
		ImgIndex = -1;
		ImgName[0] = '\0';
		Wrap = 0x2901;
		Type = 0x2600;
	};
};

class CUSTOM_WING_INFO
{
public:
	int ItemIndex;
	int DefenseConstA;
	int IncDamageConstA;
	int IncDamageConstB;
	int DecDamageConstA;
	int DecDamageConstB;
	int OptionIndex1;
	int OptionValue1;
	int OptionIndex2;
	int OptionValue2;
	int OptionIndex3;
	int OptionValue3;
	int NewOptionIndex1;
	int NewOptionValue1;
	int NewOptionIndex2;
	int NewOptionValue2;
	int NewOptionIndex3;
	int NewOptionValue3;
	int NewOptionIndex4;
	int NewOptionValue4;
	int ModelLinkType;
	char filename[32];
public:
	CUSTOM_WING_INFO() {
		release();
	};
	void release() {
		ItemIndex = -1;
		filename[0] = '\0';
	};
};

class CUSTOM_ITEM_STACK
{
public:
	int ItemIndex;
	int Level;
	int MaxStack;
	int ItemConvert;
	int break_up;
public:
	CUSTOM_ITEM_STACK() {
		release();
	};

	void release() {
		ItemIndex = -1;
		MaxStack = 0;
	};
};

class CUSTOM_SMOKE_STACK
{
public:
	int ItemIndex;
	DWORD EffectColor;
public:
	CUSTOM_SMOKE_STACK() {
		release();
	};

	void release() {
		ItemIndex = -1;
		EffectColor = -1;
	};
};

class CUSTOM_PET_STACK
{
public:
	int ItemIndex;
	int RenderType;
	int MovementType;
	int RenderEffect;
	float RenderScale;
	char filename[32];
public:
	CUSTOM_PET_STACK() {
		release();
	};

	void release() {
		ItemIndex = -1;
		RenderType = -1;
		MovementType = 0;
		filename[0] = '\0';
	};
};

class CUSTOM_EFFECT_STACK
{
public:
	int ItemIndex;
	int Texture;
	int Join;
	int SubType;
	float Scale;
	float Angle;
	DWORD EffectColor;
public:
	CUSTOM_EFFECT_STACK() {
		release();
	};

	void release() {
		ItemIndex = -1;
		EffectColor = -1;
	};
};

class CUSTOM_NPC_NAME
{
public:
	int IndexNpc;
	int PositionX;
	int PositionY;
	char Name[32];
	int WorldIndex;
public:
	CUSTOM_NPC_NAME() {
		release();
	};

	void release() {
		IndexNpc = -1;
		Name[0] = '\0';
		PositionX = -1;
		PositionY = -1;
		WorldIndex = -1;
	};
};

struct TEMPLATE_MEMBERSHIP
{

	int Index;
	int CntExp;
	int CntDrop;
	int CntDays;
	int AccountLevel;
	int Coin[3];
	char text_name[32];

	TEMPLATE_MEMBERSHIP() {
		release();
	}

	void release() {
		Index = 0;
	}
};


struct CUSTOM_COMMAND_INFO
{
	int m_Index;
	BYTE m_bColor;
	BYTE m_AccountLevel[4];
	BYTE m_bGameMaster;
	char Command[32];
	char Description[100];
	char Information[100];

	CUSTOM_COMMAND_INFO() {
		release();
	};

	void release() {
		m_Index = -1;
		memset(Command, 0, sizeof(Command));
		memset(Description, 0, sizeof(Description));
		memset(Information, 0, sizeof(Information));
	}
};

struct CHARACTER_NAME_EFFECT
{
	int index;
	char Name[32];

	CHARACTER_NAME_EFFECT() {
		release();
	}

	void release() {
		index = -1;
		memset(Name, 0, sizeof(Name));
	}
};

class RENDER_MESH_EFFECT
{
public:
	int Index;
	int GroupId; //-- meshId
	int RenderFlag;
	int Texture;
	DWORD BodyLight;
	int RenderType;
	float BlendLight;
	float TimeEffect;
	float TimeTexture;
	BOOL RenderLevel;
public:
	RENDER_MESH_EFFECT() {
		this->release();
	};

	void release() {
		Index = -1;
	};
};

class RENDER_MESH_EFFECT_DEFAULT
{
public:
	int ItemIndex;
	int EffectIndex;
public:
	RENDER_MESH_EFFECT_DEFAULT() {
		release();
	}
	void release() {
		ItemIndex = -1;
	};
};

class CUSTOM_JEWEL_INFO
{
public:
	int ItemIndex;
	int MinItemLevel;
	int MaxItemLevel;
	int MaxItemOption1;
	int MaxItemOption2;
	int MinItemOption3;
	int MaxItemOption3;
	int MinItemNewOption;
	int MaxItemNewOption;
	int MaxItemSetOption;
	int MinItemSocketOption;
	int MaxItemSocketOption;
	int EnableSlotWeapon;
	int EnableSlotArmor;
	int EnableSlotWing;
	int SuccessRate[4];
	int SalePrice;
	char Filename[32];
public:
	CUSTOM_JEWEL_INFO() {
		release();
	}
	void release() {
		ItemIndex = -1;
		memset(Filename, 0, sizeof(Filename));
	};
};

class WINGS_PHYSICS_INFO
{
public:
	int ItemIndex;
	BYTE RenderType;
	BYTE iStrip;
	BYTE iBoneLayer;
	BYTE iBoneStrip;
	WORD textureIndex[4];
	char textureLayer[32];
	char textureStrip[32];
	char textureLayer_R[32];
	char textureStrip_R[32];
public:
	WINGS_PHYSICS_INFO() {
		release();
	};

	void release() {
		ItemIndex = -1;
		textureIndex[0] = 0;
		textureIndex[1] = 0;
		textureIndex[2] = 0;
		textureIndex[3] = 0;
		memset(textureLayer, 0, sizeof(textureLayer));
		memset(textureStrip, 0, sizeof(textureStrip));
		memset(textureLayer_R, 0, sizeof(textureLayer_R));
		memset(textureStrip_R, 0, sizeof(textureStrip_R));
	};
};


extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_ITEM_INFO>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_MAPE_INFO>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_IMAGE_INFO>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_MONSTER_INFO>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_WING_INFO>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_ITEM_STACK>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_PET_STACK>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_EFFECT_STACK>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_SMOKE_STACK>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_NPC_NAME>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<TEMPLATE_MEMBERSHIP>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_COMMAND_INFO>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<CHARACTER_NAME_EFFECT>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<RENDER_MESH_EFFECT>& item_info, size_t size, BYTE Type);
extern void runtime_unique_document_file(std::string filename, std::vector<RENDER_MESH_EFFECT_DEFAULT>& item_info, size_t size);

extern void runtime_unique_document_file(std::string filename, std::vector<CUSTOM_JEWEL_INFO>& item_info, size_t size);
extern void runtime_unique_document_file(std::string filename, std::vector<WINGS_PHYSICS_INFO>& item_info, size_t size);


extern void runtime_kapocha_document_file(std::string filename, std::vector<RENDER_MESH_EFFECT>& item_info, size_t size);
