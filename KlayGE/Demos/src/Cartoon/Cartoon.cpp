#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/VertexBuffer.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Font.hpp>
#include <KlayGE/Renderable.hpp>
#include <KlayGE/RenderableHelper.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderEffect.hpp>
#include <KlayGE/SceneManager.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/ResLoader.hpp>
#include <KlayGE/RenderSettings.hpp>

#include <KlayGE/D3D9/D3D9RenderFactory.hpp>

#include <KlayGE/OCTree/OCTree.hpp>

#include <KlayGE/Input.hpp>
#include <KlayGE/DInput/DInputFactory.hpp>

#include <sstream>
#include <ctime>

#include "Torus.hpp"
#include "Cartoon.hpp"

using namespace std;
using namespace KlayGE;

namespace
{
	struct RenderTorus : public RenderableHelper
	{
		RenderTorus(TexturePtr const & toonTex, TexturePtr const & edgeTex)
		{
			effect_ = Context::Instance().RenderFactoryInstance().LoadEffect("Cartoon.fx");
			*(effect_->ParameterByName("toon")) = toonTex;
			*(effect_->ParameterByName("edge")) = edgeTex;
			effect_->SetTechnique("cartoonTec");

			box_ = MathLib::ComputeBoundingBox<float>(reinterpret_cast<Vector3*>(&Pos[0]),
				reinterpret_cast<Vector3*>(&Pos[0] + sizeof(Pos) / sizeof(float)));

			vb_.reset(new VertexBuffer(VertexBuffer::BT_TriangleList));

			vb_->AddVertexStream(VST_Positions, sizeof(float), 3, true);
			vb_->AddVertexStream(VST_Normals, sizeof(float), 3, true);

			vb_->GetVertexStream(VST_Positions)->Assign(Pos, sizeof(Pos) / sizeof(float) / 3);
			vb_->GetVertexStream(VST_Normals)->Assign(Normal, sizeof(Normal) / sizeof(float) / 3);

			vb_->AddIndexStream(true);
			vb_->GetIndexStream()->Assign(Index, sizeof(Index) / sizeof(uint16_t));
		}

		std::wstring const & Name() const
		{
			static const std::wstring name(L"Torus");
			return name;
		}
	};


	enum
	{
		Exit,
	};

	InputAction actions[] = 
	{
		InputAction(Exit, KS_Escape),
	};

	bool ConfirmDevice(RenderDeviceCaps const & caps)
	{
		if (caps.max_shader_model < 1)
		{
			return false;
		}
		return true;
	}
}

int main()
{
	OCTree sceneMgr(Box(Vector3(-10, -10, -10), Vector3(10, 10, 10)), 3);

	Context::Instance().RenderFactoryInstance(D3D9RenderFactoryInstance());
	Context::Instance().SceneManagerInstance(sceneMgr);

	Context::Instance().InputFactoryInstance(DInputFactoryInstance());

	RenderSettings settings;
	settings.width = 800;
	settings.height = 600;
	settings.colorDepth = 32;
	settings.fullScreen = false;
	settings.ConfirmDevice = ConfirmDevice;

	Cartoon app;
	app.Create("��ͨ��Ⱦ����", settings);
	app.Run();

	return 0;
}

Cartoon::Cartoon()
{
	ResLoader::Instance().AddPath("../media/Common");
	ResLoader::Instance().AddPath("../media/Cartoon");
}

void Cartoon::InitObjects()
{
	font_ = Context::Instance().RenderFactoryInstance().MakeFont("gkai00mp.ttf", 16);

	TexturePtr toonTex = LoadTexture("Toon.dds");
	TexturePtr edgeTex = LoadTexture("Edge.dds");

	renderTorus_.reset(new RenderTorus(toonTex, edgeTex));
	renderTorus_->AddToSceneManager();

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	renderEngine.ClearColor(Color(0.2f, 0.4f, 0.6f, 1));

	this->LookAt(Vector3(0, 0, -6), Vector3(0, 0, 0));
	this->Proj(0.1f, 20.0f);

	fpcController_.AttachCamera(this->ActiveCamera());
	fpcController_.Scalers(0.005f, 0.1f);

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	KlayGE::InputActionMap actionMap;
	actionMap.AddActions(actions, actions + sizeof(actions) / sizeof(actions[0]));
	action_map_id_ = inputEngine.ActionMap(actionMap, true);
}

void Cartoon::Update()
{
	fpcController_.Update();

	InputEngine& inputEngine(Context::Instance().InputFactoryInstance().InputEngineInstance());
	InputActionsType actions(inputEngine.Update(action_map_id_));
	for (InputActionsType::iterator iter = actions.begin(); iter != actions.end(); ++ iter)
	{
		switch (iter->first)
		{
		case Exit:
			this->Quit();
			break;
		}
	}

	Matrix4 view = this->ActiveCamera().ViewMatrix();
	Matrix4 proj = this->ActiveCamera().ProjMatrix();
	Vector3 eyePos = this->ActiveCamera().EyePos();

	*(renderTorus_->GetRenderEffect()->ParameterByName("proj")) = proj;
	*(renderTorus_->GetRenderEffect()->ParameterByName("lightPos")) = Vector4(2, 2, -3, 1);
	*(renderTorus_->GetRenderEffect()->ParameterByName("eyePos")) = Vector4(eyePos.x(), eyePos.y(), eyePos.z(), 1);

	RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

	float rotX(std::clock() / 700.0f);
	float rotY(std::clock() / 700.0f);

	Matrix4 mat(MathLib::RotationX(rotX));
	Matrix4 matY(MathLib::RotationY(rotY));
	mat *= matY;
	mat *= view;

	*(renderTorus_->GetRenderEffect()->ParameterByName("worldview")) = mat;
	*(renderTorus_->GetRenderEffect()->ParameterByName("worldviewIT")) = MathLib::Transpose(MathLib::Inverse(mat));

	std::wostringstream stream;
	stream << renderEngine.ActiveRenderTarget(0)->FPS();

	RenderWindow* rw = static_cast<RenderWindow*>(renderEngine.ActiveRenderTarget(0).get());

	font_->RenderText(0, 0, Color(1, 1, 0, 1), L"��ͨ��Ⱦ����");
	font_->RenderText(0, 18, Color(1, 1, 0, 1), rw->Description());
	font_->RenderText(0, 36, Color(1, 1, 0, 1), stream.str().c_str());

	SceneManager& sceneMgr(Context::Instance().SceneManagerInstance());
	stream.str(L"");
	stream << sceneMgr.NumObjectsRendered() << " Renderables "
		<< sceneMgr.NumPrimitivesRendered() << " Primitives "
		<< sceneMgr.NumVerticesRendered() << " Vertices";
	font_->RenderText(0, 54, Color(1, 1, 1, 1), stream.str().c_str());
}
