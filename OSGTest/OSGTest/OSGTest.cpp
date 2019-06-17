// OSGTest.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <osgViewer\Viewer>
#include <osg\Node>
#include <osg\Geode>
#include <osg\Group>
#include <osg\PositionAttitudeTransform>
#include <osg\PolygonMode>
#include <osg\MatrixTransform>
#include <osgDB\ReadFile>
#include <osgDB\WriteFile>
#include <osgUtil\Optimizer>
#include <osg\Geometry>
#include <osgGA\OrbitManipulator>
#include <iostream>

class BoxPicker : public osgGA::GUIEventHandler
{
public:
	BoxPicker()
	{
		x = 0.0f;
		y = 0.0f;
		OK = false;
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



int main(int argc, char* argv[])
{
	osg::ArgumentParser argument(&argc, argv);

	osg::Node* model1 = osgDB::readNodeFile("cow.osg");

	model1->setName("COW1");

	osg::MatrixTransform* mt1 = new osg::MatrixTransform;

	mt1->setMatrix(osg::Matrix::translate(osg::Vec3(10, 0, 0)));

	mt1->addChild(model1);

	osg::Node* model2 = osgDB::readNodeFile("cow.osg");

	model2->setName("COW2");

	osgViewer::Viewer viewer;

	osg::ref_ptr<BoxPicker> picker = new BoxPicker;

	viewer.addEventHandler(picker.get());

	osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;

	geo->setDataVariance(osg::Object::DYNAMIC);

	geo->setUseDisplayList(false);

	osg::Vec3Array* vertex = new osg::Vec3Array(4);
	(*vertex)[0] = osg::Vec3(-0.5, 0.0, -0.5);
	(*vertex)[1] = osg::Vec3(0.5, 0.0, -0.5);
	(*vertex)[2] = osg::Vec3(0.5, 0.0, 0.5);
	(*vertex)[3] = osg::Vec3(-0.5, 0.0, 0.5);
	geo->setVertexArray(vertex);

	osg::Vec4Array* colors = new osg::Vec4Array;
	colors->push_back(osg::Vec4(1.0, 1.0, 0.0, 1.0));
	geo->setColorArray(colors);
	geo->setColorBinding(osg::Geometry::BIND_OVERALL);

	osg::Vec3Array* normal = new osg::Vec3Array(1);
	(*normal)[0] = osg::Vec3(0, -1, 0);
	geo->setNormalArray(normal);
	geo->setNormalBinding(osg::Geometry::BIND_OVERALL);

	osg::ref_ptr<osg::DrawArrays> pri = new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, 4);

	geo->addPrimitiveSet(pri.get());

	osg::ref_ptr<osg::PolygonMode> polyMode = new osg::PolygonMode;

	polyMode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);

	geo->getOrCreateStateSet()->setAttributeAndModes(polyMode.get());

	geo->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

	picker->geometry = geo;

	osg::Camera* camera = new osg::Camera;

	camera->setProjectionMatrix(osg::Matrix::ortho2D(-1.0, 1.0, -1.0, 1.0));

	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

	camera->setViewMatrixAsLookAt(osg::Vec3(0, -1, 0), osg::Vec3(0, 0, 0), osg::Vec3(0, 0, 1));

	camera->setClearMask(GL_DEPTH_BUFFER_BIT);

	camera->setRenderOrder(osg::Camera::POST_RENDER);

	camera->setAllowEventFocus(false);

	osg::ref_ptr<osg::Geode> geode = new osg::Geode;

	geode->addDrawable(geo.get());

	camera->addChild(geode.get());

	osg::ref_ptr<osg::Group> root = new osg::Group;

	root->addChild(camera);

	root->addChild(mt1);

	root->addChild(model2);

	viewer.setSceneData(root.get());

	viewer.setCameraManipulator(new osgGA::OrbitManipulator);

	viewer.run();

	return 0;
}