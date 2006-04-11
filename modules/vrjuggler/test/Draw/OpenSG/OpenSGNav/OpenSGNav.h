/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2002 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _OSG_NAV_
#define _OSG_NAV_

#include <iostream>
#include <iomanip>

/*-----------------------------Juggler includes-------------------------------*/
#include <vrj/vrjConfig.h>
#include <vrj/Draw/OGL/GlApp.h>
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Generate.h>

#include <gadget/Type/PositionInterface.h>
#include <gadget/Type/AnalogInterface.h>
#include <gadget/Type/DigitalInterface.h>

#include <vrj/Draw/OGL/GlContextData.h>

/*-----------------------------OpenSG includes--------------------------------*/
#include <OpenSG/OSGBaseTypes.h>
#include <OpenSG/OSGWindow.h>

#include <OpenSG/OSGDrawAction.h>
#include <OpenSG/OSGRenderAction.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGWindow.h>
#include <OpenSG/OSGImageForeground.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGGeoPropPtrs.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGMatrixCamera.h>
#include <OpenSG/OSGDirectionalLight.h>
#include <OpenSG/OSGGroup.h>

/*----------------------------------------------------------------------------*/

#include <vrj/Draw/OpenSG/OpenSGApp.h>



class OpenSGNav : public vrj::OpenSGApp
{
public:
    OpenSGNav(vrj::Kernel* kern)
       : vrj::OpenSGApp(kern)
    {
        std::cout << "OpenSGNav::OpenSGNav called\n";
        mFileToLoad = std::string("");
    }

    virtual ~OpenSGNav (void)
    {
        std::cout << "OpenSGNav::~OpenSGNav called\n";
    }

    // Handle any initialization needed before API
    virtual void init();


    virtual void initScene();
    virtual OSG::NodePtr getSceneRoot()
    { return mSceneRoot; }

    void initRenderer();

    virtual void draw();

    virtual void contextInit();

    virtual void preFrame();

    void setModelFileName(std::string filename)
    {
        std::cout << "OpenSGNav::setModelFileName called\n";
        mFileToLoad = filename;
    }

  private:
    void initGLState();

  private:
    std::string   mFileToLoad;

    OSG::NodePtr        mSceneRoot;       /**< The root of the scene */
    OSG::TransformPtr   mSceneTransform;  /**< Transform core */
    OSG::NodePtr        mModelRoot;       /**< Root of the loaded model */

    OSG::NodePtr  mLightNode;       /**< The light node */
    OSG::NodePtr  mLightCart;       /**< The cart for the light */

  public:
    gadget::PositionInterface   mWand;
    gadget::DigitalInterface   mButton0;
    gadget::DigitalInterface   mButton1;
    gadget::DigitalInterface   mButton2;
    float  velocity;
};


#endif
