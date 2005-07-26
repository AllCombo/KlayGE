// App3D.cpp
// KlayGE App3D类 实现文件
// Ver 2.7.0
// 版权所有(C) 龚敏敏, 2003--2005
// Homepage: http://klayge.sourceforge.net
//
// 2.7.0
// 增加了Quit (2005.6.28)
//
// 2.0.0
// 初次建立 (2003.7.10)
//
// 修改记录
/////////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

#include <KlayGE/KlayGE.hpp>
#include <KlayGE/ThrowErr.hpp>
#include <KlayGE/Math.hpp>
#include <KlayGE/Context.hpp>
#include <KlayGE/RenderWindow.hpp>
#include <KlayGE/RenderEngine.hpp>
#include <KlayGE/RenderFactory.hpp>

#include <cassert>
#include <windows.h>

#include <KlayGE/App3D.hpp>

namespace KlayGE
{
	// 构造函数
	/////////////////////////////////////////////////////////////////////////////////
	App3DFramework::App3DFramework()
	{
		Context::Instance().AppInstance(*this);
	}

	App3DFramework::~App3DFramework()
	{
		this->DelObjects();
	}

	// 建立应用程序窗口和D3D接口
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::Create(std::string const & name, RenderSettings const & settings)
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().CreateRenderWindow(name, settings);

		this->InitObjects();
	}

	void App3DFramework::Run()
	{
		Context::Instance().RenderFactoryInstance().RenderEngineInstance().StartRendering();

		this->DelObjects();
	}

	// 获取当前摄像机
	/////////////////////////////////////////////////////////////////////////////////
	Camera& App3DFramework::ActiveCamera()
	{
		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderTarget& activeRenderTarget(*renderEngine.ActiveRenderTarget(0));

		return activeRenderTarget.GetViewport().camera;
	}

	// 设置观察矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::LookAt(Vector3 const & vEye, Vector3 const & vLookAt,
												Vector3 const & vUp)
	{
		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());

		this->ActiveCamera().ViewParams(vEye, vLookAt, vUp);
		renderEngine.ViewMatrix(this->ActiveCamera().ViewMatrix());
	}

	// 设置投射矩阵
	/////////////////////////////////////////////////////////////////////////////////
	void App3DFramework::Proj(float nearPlane, float farPlane)
	{
		assert(nearPlane != 0);
		assert(farPlane != 0);
		assert(farPlane > nearPlane);

		RenderEngine& renderEngine(Context::Instance().RenderFactoryInstance().RenderEngineInstance());
		RenderTarget& activeRenderTarget(*renderEngine.ActiveRenderTarget(0));
		Camera& camera(this->ActiveCamera());

		camera.ProjParams(PI / 4, static_cast<float>(activeRenderTarget.Width()) / activeRenderTarget.Height(),
			nearPlane, farPlane);
		renderEngine.ProjectionMatrix(camera.ProjMatrix());
 	}

	void App3DFramework::Quit()
	{
#ifdef WIN32
		::PostQuitMessage(0);
#else
		exit(0);
#endif
	}
}
