// geometryDraw.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <osgViewer\Viewer>
#include <osg\Node>
#include <osg\Geode>
#include <osg\Group>
#include <osg\PositionAttitudeTransform>
#include <osgDB\ReadFile>
#include <osgDB\WriteFile>
#include <osgUtil\Optimizer>

class MouseHandler : public osgGA::GUIEventHandler
{
public:
	MouseHandler(osg::observer_ptr<osg::Geometry> geom)
	{
		x = 0.0f;
		y = 0.0f;
		OK = false;
		geometry = geom;
	}

	osg::observer_ptr<osg::Geometry> geometry;

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
	{
		bool doit = false;

		osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);

		if (!viewer)
		{
			return false;
		}

		if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH)
		{
			x = ea.getXnormalized();
			y = ea.getYnormalized();
			x_pick = ea.getX();
			y_pick = ea.getY();

			OK = true;
		}

		if (ea.getEventType() == osgGA::GUIEventAdapter::DRAG)
		{
			if (OK)
			{
				float end_x = ea.getXnormalized();
				float end_y = ea.getYnormalized();

				if (geometry.valid())
				{
					osg::Vec3Array* vertex = new osg::Vec3Array(4);
					(*vertex)[0] = osg::Vec3(x, 0, y);
					(*vertex)[1] = osg::Vec3(x, 0, end_y);
					(*vertex)[2] = osg::Vec3(end_x, 0, end_y);
					(*vertex)[3] = osg::Vec3(end_x, 0, y);
					geometry->setVertexArray(vertex);
					geometry->dirtyDisplayList();
				}

				if (ea.getModKeyMask()&osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT)
				{
					doit = true;
				}
			}
		}

		if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE)
		{
			OK = false;

			float pick_x = ea.getX();
			float pick_y = ea.getY();
			float xMin, xMax, yMin, yMax;
			xMin = osg::minimum(x_pick, pick_x);
			xMax = osg::maximum(x_pick, pick_x);
			yMin = osg::minimum(y_pick, pick_y);
			yMax = osg::maximum(y_pick, pick_y);

			osg::ref_ptr<osgUtil::PolytopeIntersector> intersector =

				new osgUtil::PolytopeIntersector(osgUtil::Intersector::WINDOW, xMin, yMin, xMax, yMax);

			osgUtil::IntersectionVisitor iv(intersector.get());

			viewer->getCamera()->accept(iv);

			if (intersector->containsIntersections())
			{
				std::cout << "OK" << std::endl;

			}

			if (geometry.valid())
			{
				osg::Vec3Array* vertex = new osg::Vec3Array(4);
				(*vertex)[0] = osg::Vec3(0, 0, 0);
				(*vertex)[1] = osg::Vec3(0, 0, 0);
				(*vertex)[2] = osg::Vec3(0, 0, 0);
				(*vertex)[3] = osg::Vec3(0, 0, 0);
				geometry->setVertexArray(vertex);
				geometry->dirtyDisplayList();
			}
		}
		return doit;
	}

	float x, y;
	float x_pick, y_pick;
	bool OK;
};

//创建一个四边形节点
osg::ref_ptr<osg::Node> createQuad(osg::ref_ptr<osg::Geode> geode, osg::ref_ptr<osg::Geometry> geom)
{
	//创建顶点数组，注意顶点的添加顺序是逆时针的
	osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array();

	//添加数据
	v->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
	v->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
	v->push_back(osg::Vec3(1.0f, 0.0f, 1.0f));
	v->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
	//设置顶点数据
	geom->setVertexArray(v.get());

	//创建纹理坐标
	osg::ref_ptr<osg::Vec2Array> vt = new osg::Vec2Array();
	//添加数据
	vt->push_back(osg::Vec2(0.0f, 0.0f));
	vt->push_back(osg::Vec2(1.0f, 0.0f));
	vt->push_back(osg::Vec2(1.0f, 1.0f));
	vt->push_back(osg::Vec2(0.0f, 1.0f));
	//设置纹理坐标
	geom->setTexCoordArray(0, vt.get());

	//创建颜色数组
	osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array();
	//添加数据
	vc->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
	vc->push_back(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
	vc->push_back(osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f));
	vc->push_back(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
	//设置颜色数组
	geom->setColorArray(vc.get());
	//设置颜色的绑定方式为单个顶点
	geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	//创建法线数组
	osg::ref_ptr<osg::Vec3Array> nc = new osg::Vec3Array();
	//添加法线
	nc->push_back(osg::Vec3(0.0f, -1.0f, 0.0f));
	//设置法线数组
	geom->setNormalArray(nc.get());
	//设置法线的绑定方式为全部顶点
	geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

	//添加图元，绘图基元为四边形
	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, 4));

	//添加到叶节点
	geode->addDrawable(geom.get());

	return geode.get();
}

int main()
{
	//创建Viewer对象，场景浏览器
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer();
	osg::ref_ptr<osg::Group> root = new osg::Group();
	//创建一个叶节点对象
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();

	//添加事件处理
	osg::ref_ptr<MouseHandler> picker = new MouseHandler(geom);
	viewer->addEventHandler(picker.get());

	//设置绘制列表
	geom->setDataVariance(osg::Object::DYNAMIC);
	geom->setUseDisplayList(false);

	//添加到场景
	root->addChild(createQuad(geode,geom));

	//优化场景数据
	osgUtil::Optimizer optimizer;
	optimizer.optimize(root.get());

	viewer->setSceneData(root.get());

	viewer->realize();

	viewer->run();

	return 0;

}
