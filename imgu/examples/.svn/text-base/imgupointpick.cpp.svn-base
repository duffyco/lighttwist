/**
 * point pick dans un modele
 *
*/



#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osg/CoordinateSystemNode>

#include <osg/Switch>
#include <osgText/Text>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/PolygonStipple>

#include <osg/Material>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>

#include <iostream>

// distance entre une ligne p-q et un point t
// dist[p_,q_,t_]:=Norm[(t-p)-((t-p).Normalize[q-p])*Normalize[q-p]]
// nx,ny = Normalize[q-p]
// Norm[(t-p)-((t-p).n)*n]
double lineDist(osg::Vec3 p,osg::Vec3 q,osg::Vec3 t) {
	osg::Vec3d n = q-p;
	osg::Vec3d z = t-p;
	n.normalize();

	double dot=z*n;
	osg::Vec3d blub=n*dot;

	osg::Vec3d g=(z-blub);
	return(g.length());
}


// pour le qsort
osg::Vec3Array* global_vertices;

int cmp(void *a,void *b)
{
int ai=*((int *)a);
int bi=*((int *)b);
osg::Vec3 pa=(*global_vertices)[ai];
osg::Vec3 pb=(*global_vertices)[bi];

	if( pa.x()<pb.x() ) return(-1);
	if( pa.x()>pb.x() ) return(1);
	return(0);
}


// trouve la geometrie
// coustruit un index, tri en X
struct GeometryFinder : public osg::NodeVisitor
{
    osg::ref_ptr<osg::Geometry> _geom;
    osg::Vec3Array* _vertices;
    int _nb; // nb de points =_vertices->size();
    //int* _index; // [nb]
    GeometryFinder() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    void apply(osg::Geode& geode)
    {
	// arrete au premier geom qu'on trouve
        if (_geom.valid()) return;
        for (unsigned int i = 0; i < geode.getNumDrawables(); i++)
        {
            osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
            if (geom) {
	 	_vertices = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
		printf("trouve une geometrie!!! (nb points = %d)\n",(int)_vertices->size());

		printf("Indexing...\n");
		_nb=_vertices->size();
		//_index=new int[_nb];
		//int i;
		//for(i=0;i<_nb;i++) _index[i]=i;
		// une variable globale temporaire...
		//global_vertices=_vertices;

		/*
		for(i=0;i<_nb;i++) {
			osg::Vec3 p=(*_vertices)[_index[i]];
			printf("%5d : (%12.6f,%12.6f,%12.6f)\n",_index[i],p.x(),p.y(),p.z());
		}
		*/

		// ne sert a rien parce que la recherche est compliquee
		//qsort(_index,_nb,sizeof(int),(__compar_fn_t)cmp);

		/*
		for(i=0;i<_nb;i++) {
			osg::Vec3 p=(*_vertices)[_index[i]];
			printf("%5d : (%12.6f,%12.6f,%12.6f)\n",_index[i],p.x(),p.y(),p.z());
		}
		*/
                _geom = geom;
		break;
            }
        }
    }

    osg::Vec3d closest(osg::Vec3d p,osg::Vec3d q)
    {
	if( !_geom.valid() ) return(osg::Vec3d(0.,0.,0.));
	osg::Vec3d psol;
	double bestd,d;
	//osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(_geom->getVertexArray());
	bestd=-1.0;
            for(int i=0;i<_nb;i++)
            {
		if( i%100000==0 ) printf("%d...\n",i);
		osg::Vec3 t= (*_vertices)[i];
		//d=(p-t).length();
		d=lineDist(p,q,t);
		//printf("%5d : (%12.6f,%12.6f,%12.6f) dist=%12.6f\n",i,p.x(),p.y(),p.z(),d);
		if( d<bestd || bestd<0 ) {
			bestd=d;
			psol=t;
		}
            }
	return(psol);
    }
};

// ajoute les lignes p-q, p-t, q-t
osg::Geometry* addLines(osg::Vec3d p,osg::Vec3d q,osg::Vec3d t,osg::Geode *geode,osg::Geometry *linesGeom)
    {
        osg::Vec3Array* vertices = new osg::Vec3Array(6);
        (*vertices)[0].set(p.x(),p.y(),p.z());
        (*vertices)[1].set(q.x(),q.y(),q.z());
        (*vertices)[2].set(p.x(),p.y(),p.z());
        (*vertices)[3].set(t.x(),t.y(),t.z());
        (*vertices)[4].set(q.x(),q.y(),q.z());
        (*vertices)[5].set(t.x(),t.y(),t.z());

	if( linesGeom!=NULL ) {
        	linesGeom->setVertexArray(vertices);
		return(linesGeom);
	}

        // create Geometry object to store all the vertices and lines primitive.
        linesGeom = new osg::Geometry();

        // pass the created vertex array to the points geometry object.
        linesGeom->setVertexArray(vertices);

        // set the colors as before, plus using the above
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
        linesGeom->setColorArray(colors);
        linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);


        // set the normal in the same way color.
        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
        linesGeom->setNormalArray(normals);
        linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);


        // This time we simply use primitive, and hardwire the number of coords to use 
        // since we know up front,
        linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,6));

        // add the points geometry to the geode.
        geode->addDrawable(linesGeom);
	return(linesGeom);
    }

void addLine(osg::Vec3d p,osg::Vec3d q,osg::Geode *geode)
    {
        // create Geometry object to store all the vertices and lines primitive.
        osg::Geometry* linesGeom = new osg::Geometry();

        // this time we'll preallocate the vertex array to the size we
        // need and then simple set them as array elements, 8 points
        // makes 4 line segments.
        osg::Vec3Array* vertices = new osg::Vec3Array(8);
        (*vertices)[0].set(p.x(),p.y(),p.z());
        (*vertices)[1].set(q.x(),q.y(),q.z());


        // pass the created vertex array to the points geometry object.
        linesGeom->setVertexArray(vertices);

        // set the colors as before, plus using the above
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
        linesGeom->setColorArray(colors);
        linesGeom->setColorBinding(osg::Geometry::BIND_OVERALL);


        // set the normal in the same way color.
        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
        linesGeom->setNormalArray(normals);
        linesGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);


        // This time we simply use primitive, and hardwire the number of coords to use 
        // since we know up front,
        linesGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

        // add the points geometry to the geode.
        geode->addDrawable(linesGeom);
    }




osg::Geometry *addTriangle(osg::Geode *geode,osg::Vec3 p[3],osg::Geometry* polyGeom)
{
	printf("tri!\n");
       	osg::Vec3Array* vertices = new osg::Vec3Array(3,p);

	if( polyGeom!=NULL ) {
		// update geometry
       		polyGeom->setVertexArray(vertices);
		return polyGeom;
	}
	polyGeom = new osg::Geometry();
       	polyGeom->setVertexArray(vertices);

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));

        //osg::Vec3Array* normals = new osg::Vec3Array;
        //normals->push_back(osg::Vec3(1.0f,-1.0f,0.0f));

        polyGeom->setColorArray(colors);
        polyGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

        // use the shared normal array.
        //polyGeom->setNormalArray(normals);
        //polyGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

        polyGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,0,3));

        // polygon stipple
        osg::StateSet* stateSet = new osg::StateSet();
        polyGeom->setStateSet(stateSet);

	osg::PolygonStipple* polygonStipple = new osg::PolygonStipple;
        stateSet->setAttributeAndModes(polygonStipple,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

	stateSet->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);

	//osg::Vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
	//osg::Material* material = new osg::Material;
        //material->setDiffuse(osg::Material::FRONT_AND_BACK, color);
        //stateSet->setAttribute(material);

	//geode->setCullingActive(false);

        geode->addDrawable(polyGeom);
	return(polyGeom);
}



class ImageViewsEventHandler : public osgGA::GUIEventHandler
{
public:
    osgViewer::Viewer *_viewer;
    osg::Geode *_geode;
    GeometryFinder *_finder;
    double _xref,_yref; // only set when starting a move.
    osg::Vec3 _pos[3]; // les 3 points
    int _done; // 0=no, 7 = yes
    osg::Geometry *_polyGeom; // le triangle (NULL au depart)
    osg::Geometry *_lineGeom[3]; // les 3 lignes a afficher
    osgGA::TrackballManipulator* _tbm;

    ImageViewsEventHandler(osgViewer::Viewer *v,osg::Geode *g,GeometryFinder *finder,
	osgGA::TrackballManipulator* tbm) {
	_viewer=v;
	_geode=g;
	_finder=finder;
	_tbm=tbm;
	_done=0;
	_polyGeom=NULL;
	for(int i=0;i<3;i++) _lineGeom[i]=NULL;
    }

    ~ImageViewsEventHandler() {
    }

    void checkout() {
		if( _done!=7 ) return;
		printf("les 3 points:\n");
		printf("( %12.6f, %12.6f, %12.6f)\n",_pos[0].x(),_pos[0].y(),_pos[0].z());
		printf("( %12.6f, %12.6f, %12.6f)\n",_pos[1].x(),_pos[1].y(),_pos[1].z());
		printf("( %12.6f, %12.6f, %12.6f)\n",_pos[2].x(),_pos[2].y(),_pos[2].z());

		// ajoute un triangle
		_polyGeom=addTriangle(_geode,_pos,_polyGeom);
    }


    osg::Vec3d intersect(int n,float x,float y) {
	    osg::Matrixd matrix;
	    osg::Camera *camera=_viewer->getCamera();

	    matrix.postMult(camera->getViewMatrix());
	    matrix.postMult(camera->getProjectionMatrix());

	    double zNear = -1.0;
	    double zFar = 1.0;
	    if (camera->getViewport())
	    {
		matrix.postMult(camera->getViewport()->computeWindowMatrix());
		zNear = 0.0;
		zFar = 1.0;
	    }

	    osg::Matrixd inverse;
	    inverse.invert(matrix);

	    osg::Vec3d p = osg::Vec3d(x,y,zNear) * inverse;
	    osg::Vec3d q = osg::Vec3d(x,y,zFar) * inverse;

	    printf("3D: (%12.6f,%12.6f,%12.6f) - (%12.6f,%12.6f,%12.6f)\n",
		p.x(),p.y(),p.z(),q.x(),q.y(),q.z());

		osg::Vec3d psol=_finder->closest(p,q);
		if( n>=0 ) {
			_lineGeom[n]=addLines(p,q,psol,_geode,_lineGeom[n]);
			_pos[n]=psol;
		}
		return psol;
    }

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa) {
	double x=ea.getX();
	double y=ea.getY();
	//double x=ea.getXnormalized()/2.0+0.5; // 0 .. 1
	//double y=ea.getYnormalized()/2.0+0.5;
	
	switch(ea.getEventType()) {
	  case(osgGA::GUIEventAdapter::KEYDOWN):
		printf("KEYDOWN ::::: %4d,  (%12.6f,%12.6f)\n",ea.getKey(),x,y);
		//osgViewer::Viewer *v=dynamic_cast<osgViewer::Viewer *>(&aa);
		if( ea.getKey()=='a' ) {
			printf("a\n");
			intersect(0,x,y);
			_done|=1;
			checkout();
			return(1);
		}else if( ea.getKey()=='b' ) {
			printf("b\n");
			intersect(1,x,y);
			_done|=2;
			checkout();
			return(1);
		}else if( ea.getKey()=='c' ) {
			printf("c\n");
			intersect(2,x,y);
			_done|=4;
			checkout();
			return(1);
		}else if( ea.getKey()=='d' ) {
			osg::Vec3d pos = intersect(-1,x,y);
			_tbm->setCenter(pos);
			printf("center set to (%f,%f,%f)\n",pos.x(),pos.y(),pos.z());
		}
	}
	return(0); // not handled
    }
};





int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("--login <url> <username> <password>","Provide authentication information for http file access.");

    osgViewer::Viewer viewer(arguments);

    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    std::string url, username, password;
    while(arguments.read("--login",url, username, password))
    {
        if (!osgDB::Registry::instance()->getAuthenticationMap())
        {
            osgDB::Registry::instance()->setAuthenticationMap(new osgDB::AuthenticationMap);
            osgDB::Registry::instance()->getAuthenticationMap()->addAuthenticationDetails(
                url,
                new osgDB::AuthenticationDetails(username, password)
            );
        }
    }

    // set up the camera manipulators.
#ifdef SKIP
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
        keyswitchManipulator->addMatrixManipulator( '5', "Orbit", new osgGA::OrbitManipulator() );
        keyswitchManipulator->addMatrixManipulator( '6', "FirstPerson", new osgGA::FirstPersonManipulator() );
        keyswitchManipulator->addMatrixManipulator( '7', "Spherical", new osgGA::SphericalManipulator() );

        std::string pathfile;
        double animationSpeed = 1.0;
        while(arguments.read("--speed",animationSpeed) ) {}
        char keyForAnimationPath = '8';
        while (arguments.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm || !apm->valid())
            {
                apm->setTimeScale(animationSpeed);

                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                keyswitchManipulator->selectMatrixManipulator(num);
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }
#endif

        osgGA::TrackballManipulator* tbm =  new osgGA::TrackballManipulator();
        viewer.setCameraManipulator( tbm );




    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the thread model handler
    //viewer.addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // add the stats handler
    //viewer.addEventHandler(new osgViewer::StatsHandler);

    // add the help handler
    //viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // add the record camera path handler
    //viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

    // add the LOD Scale handler
    //viewer.addEventHandler(new osgViewer::LODScaleHandler);

    // add the screen capture handler
    viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);


    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }


    // optimize the scene graph, remove redundant nodes and state etc.
    //osgUtil::Optimizer optimizer;
    //optimizer.optimize(loadedModel.get());

    osg::Group *root = new osg::Group();

    osg::Geode *geode = new osg::Geode();

	root->addChild(loadedModel.get());
	root->addChild(geode);

    viewer.setSceneData( root );


    GeometryFinder finder;
    loadedModel->accept(finder);

    viewer.addEventHandler(new ImageViewsEventHandler(&viewer,geode,&finder,tbm));

    viewer.realize();

    return viewer.run();

}

