#include "SceneDev1.h"
#include "SceneMgr.h"
#include "../Framework/Framework.h"
#include "../Framework/InputMgr.h"
#include "../Framework/ResourceMgr.h"
#include "../GameObject/SpriteObj.h"
#include "../GameObject/TextObj.h"
#include "../Framework/SoundMgr.h"
#include "../GameObject/Player.h"
#include "../GameObject/Zombie.h"
#include "../GameObject/Bullet.h"
#include "../GameObject/Pickup.h"
#include "../GameObject/VertexArrayObj.h"
#include "../GameObject/ItemGenerator.h"
#include "../GameObject/Pistol.h"
#include "../GameObject/SM.h"
#include "../GameObject/Sword.h"
#include "../GameObject/Slash.h"
#include "../UI/UIDev1Mgr.h"

void OnCreateBullet(Bullet* bullet)
{
	SceneDev1* scene = (SceneDev1*)SCENE_MGR->GetScene(Scenes::Dev1);
	bullet->SetTexture(*RESOURCE_MGR->GetTexture("graphics/bullet.png"));
	bullet->SetZombieList(scene->GetZombieList());
	bullet->Init();
}

SceneDev1::SceneDev1() : Scene(Scenes::Dev1)
{

}

SceneDev1::~SceneDev1()
{

}

void SceneDev1::Init()
{
	Release();
	
	uiMgr = new UIDev1Mgr(this);
	uiMgr->Init();

	CreateBackground(15, 26, 50.f, 50.f);
	//CreateZombies(10);

	Vector2i centerPos = FRAMEWORK->GetWindowSize() / 2;
	background->SetPos({ 0,0 });
	background->SetOrigin(Origins::MC);
	
	//player
	player = new Player();
	player->SetName("Player");
	player->SetTexture(*RESOURCE_MGR->GetTexture("graphics/player.png"));
	player->SetBulletPool(&bullets);
	player->SetBackground(background);
	objList.push_back(player);

	bullets.OnCreate = OnCreateBullet;
	bullets.Init();

	//item
	ItemGenerator* itemGen = new ItemGenerator();
	itemGen->SetName("ItemGenerator");
	AddGameObj(itemGen);

	//weapon
	pistol = new Pistol();
	pistol->Init(player);

	sm = new SM();
	sm->Init(player);

	sword = new Sword();
	sword->Init(player);
	
	//objList.push_back(uiMgr);
	for ( auto obj : objList )
	{
		obj->Init();
	}

	
	//player->SetActive(false);
}

void SceneDev1::Release()
{
	if ( uiMgr != nullptr )
	{
		uiMgr->Release();
		delete uiMgr;
		uiMgr = nullptr;
	}
	bullets.Release();

	Scene::Release();
	player = nullptr;
	pistol = nullptr;
	sm = nullptr;
	sword = nullptr;
}

void SceneDev1::Enter()
{
	//마우스 커서
	FRAMEWORK->GetWindow().setMouseCursorVisible(false);
	FRAMEWORK->GetWindow().setMouseCursorGrabbed(true);
	Vector2i size = FRAMEWORK->GetWindowSize();
	//Vector2f centerPos(size.x * 0.5f, size.y * 0.5f);

	worldView.setSize(size.x, size.y);
	worldView.setCenter(0.f, 0.f);
	
	uiView.setSize(size.x, size.y);
	uiView.setCenter(size.x * 0.5f, size.y * 0.5f);
	
	player->SetPos({ 0,0 });

	//zombie
	CreateZombies(100);

}

void SceneDev1::Exit()
{
	FRAMEWORK->GetWindow().setMouseCursorVisible(true);
	
	auto it = zombies.begin();
	while ( it != zombies.end() )
	{
		objList.remove(*it);
		delete* it;

		it = zombies.erase(it);
	}

	player->Reset();
	bullets.Reset();
	//bullet->Reset();
	FindGameObj("ItemGenerator")->Reset();
	 
	uiMgr->Reset();
}

void SceneDev1::Update(float dt)
{

	Scene::Update(dt);

	worldView.setCenter(player->GetPos());

	if ( InputMgr::GetKeyDown(Keyboard::Space) )
	{
		SCENE_MGR->ChangeScene(Scenes::Dev2);
		return;
	}
	bullets.Update(dt);
	//bullet->Update(dt);
	switch ( player->GetFireMode() )
	{
	case FireModes::PISTOL:
		pistol->Update(dt);
		break;
	case FireModes::SUBMACHINE:
		sm->Update(dt);
		break;
	case FireModes::SWORD:
		
		sword->Update(dt);
		break;
	default:
		break;
	}
	
	uiMgr->Update(dt);
	
	//cursor->SetPos(ScreenToWorldPos((Vector2i)InputMgr::GetMousePos()));
}

void SceneDev1::Draw(RenderWindow& window)
{
	Scene::Draw(window);

	window.setView(worldView);
	const auto& bulletsList = bullets.GetUseList();
	for ( auto bullet : bulletsList )
	{
		bullet->Draw(window);
	}
	uiMgr->Draw(window);
}

void SceneDev1::CreateBackground(int width, int height, float quadWidth, float quadHeight)
{
	if ( background == nullptr )
	{
		background = new VertexArrayObj();
		background->SetTexture(GetTexture("graphics/background_sheet.png"));
		objList.push_back(background);
	}

	Vector2f startPos = background->GetPos();
	VertexArray& va = background->GetVA();
	va.clear();
	va.setPrimitiveType(Quads);
	va.resize(width * height * 4);
	Vector2f currPos = startPos;

	Vector2f offsets[4] = {
		{0, 0},
		{quadWidth, 0},
		{quadWidth, quadHeight},
		{0, quadHeight},
	};

	for ( int i = 0; i < width; ++i )
	{
		for ( int j = 0; j < height; ++j )
		{
			int texIndex = Utils::RandomRange(0, 3);
			if ( (i == 0 || i == width - 1) || (j == 0 || j == height - 1) )
			{
				texIndex = 3;
			}
			int quadIndex = i * height + j;
			for ( int k = 0; k < 4; ++k )
			{
				int vertexIndex = quadIndex * 4 + k;
				va[vertexIndex].position = currPos + offsets[k];
				va[vertexIndex].texCoords = offsets[k];
				va[vertexIndex].texCoords.y += quadHeight * texIndex;
			}
			currPos.x += 50;
		}
		currPos.x = startPos.x;
		currPos.y += 50;
	}
	
	
}

void SceneDev1::CreateZombies(int count)
{
	Vector2f point;

	for ( int i = 0; i < count; i++ )
	{
		Zombie* zombie = new Zombie();
		zombie->SetType((Zombie::Types)Utils::RandomRange(0, Zombie::TotalTypes));
		zombie->Init(player);
		//std::cout << (int)zombie->GetType() << std::endl;
		point.x = Utils::RandomRange(-1.0f, 1.0f);
		point.y = Utils::RandomRange(-1.0f, 1.0f);
		point = Utils::Normalize(point);
		point *= Utils::RandomRange(100.f, 500.f);
		
		zombie->SetBackground(background);
		zombie->SetPos(point);
		
		
		objList.push_back(zombie);
		zombies.push_back(zombie);
	}
}

