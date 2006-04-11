/*
 * VRJuggler
 *   Copyright (C) 1997,1998,1999,2000
 *   Iowa State University Research Foundation, Inc.
 *   All Rights Reserved
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
 */

#ifndef _PERFORMER_JUGGLER_SOUND_REPLACE_TRAV_H_
#define _PERFORMER_JUGGLER_SOUND_REPLACE_TRAV_H_

#include <iostream.h>
#include <Performer/pr.h>
#include <Performer/pf.h>
#include <Performer/pfutil.h>
#include <Performer/pf/pfGeode.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfScene.h>
#include <Performer/pr/pfMemory.h>
#include <Performer/pf/pfSwitch.h>
#include <unistd.h>
#include <string>
#include <assert.h>
#include <Sound/pf/pfSoundNode.h>
#include <Sound/vjSoundManager.h>
#include <Kernel/vjDebug.h>

//: This is a collection of Performer traversers...
//
// REQUIRED!!! You must call pfSoundTraverser::preForkInit() in your juggler app's
//              preForkInit() member function.  Your app is likely to crash otherwise!!
class pfSoundTraverser
{
public:
   // This function must be called pfSoundTraverser::preForkInit() in your juggler app's
   // preForkInit() member function.  Your app is likely to crash otherwise!!
   static void preForkInit()
   {
      vjDEBUG(vjDBG_ALL,1)<<"[pfSoundTraverser] pfSoundTraverser::preForkInit(): Initializing sound node type with performer\n"<<vjDEBUG_FLUSH;
      pfSoundNode::init();
   }

   // This is a Performer traverser that will replace any node ending in keyName
   //  in your scene graph with a pfSoundNode.
   //  For example, if mKeyName == _Sound_, and your nodename == Gong_Sound_
   //  then that node will be replaced with a new pfSoundNode of a sound
   //  called "Gong"
   //
   static void replace( pfNode* node, const std::string& keyName = "_Sound_" )
   {
      //vjSoundManager::instance().engine()->setPosition( 0.0f, 0.0f, 0.0f );
      vjDEBUG(vjDBG_ALL,1)<<clrOutNORM(clrGREEN,"pfSoundTraverser: ") <<"Checking graph for soundnodes (nodes with the "<<keyName.c_str()<<" extension...\n"<<vjDEBUG_FLUSH;

      // use the performer traversal mechanism
       pfuTraverser trav;
       pfuInitTraverser( &trav );

       trav.preFunc = pfSoundTraverser::nodesReplaceWithSound;
       trav.data = const_cast<std::string*>( &keyName );

       pfuTraverse( node, &trav );
   }

   // enable the sound, keeping the state whether it is triggered or stopped.
   static void enable( pfNode* node )
   {
      //vjSoundManager::instance().engine()->setPosition( 0.0f, 0.0f, 0.0f );
      vjDEBUG(vjDBG_ALL,1)<<clrOutNORM(clrGREEN,"pfSoundTraverser: [enable]")<<" Enabling sounds in subgraph.\n"<<vjDEBUG_FLUSH;

      // use the performer traversal mechanism
       pfuTraverser trav;
       pfuInitTraverser( &trav );

       trav.preFunc = pfSoundTraverser::soundNodesEnable;
       trav.data = NULL;

       pfuTraverse( node, &trav );
   }

   // disable the sound, keeping the state whether it is triggered or stopped.
   // after this, an enable would respect the previous triggered/stopped state.
   static void disable( pfNode* node )
   {
      //vjSoundManager::instance().engine()->setPosition( 0.0f, 0.0f, 0.0f );
      vjDEBUG(vjDBG_ALL,1)<< clrOutNORM(clrGREEN,"pfSoundTraverser: [disable]") <<" Disabling sounds in subgraph.\n"<<vjDEBUG_FLUSH;

      // use the performer traversal mechanism
       pfuTraverser trav;
       pfuInitTraverser( &trav );

       trav.preFunc = pfSoundTraverser::soundNodesDisable;
       trav.data = NULL;

       pfuTraverse( node, &trav );
   }
   
   // trigger the sound
   static void trigger( pfNode* node )
   {
      //vjSoundManager::instance().engine()->setPosition( 0.0f, 0.0f, 0.0f );
      vjDEBUG(vjDBG_ALL,1)<<clrOutNORM(clrGREEN,"pfSoundTraverser: [soundOn]")<<" Triggering sounds in subgraph.\n"<<vjDEBUG_FLUSH;

      // use the performer traversal mechanism
       pfuTraverser trav;
       pfuInitTraverser( &trav );

       trav.preFunc = pfSoundTraverser::soundNodesTrigger;
       trav.data = NULL;

       pfuTraverse( node, &trav );
   }

   // stop the sound
   static void stop( pfNode* node )
   {
      //vjSoundManager::instance().engine()->setPosition( 0.0f, 0.0f, 0.0f );
      vjDEBUG(vjDBG_ALL,1)<< clrOutNORM(clrGREEN,"pfSoundTraverser: [soundOff]") <<" Stopping sounds in subgraph.\n"<<vjDEBUG_FLUSH;

      // use the performer traversal mechanism
       pfuTraverser trav;
       pfuInitTraverser( &trav );

       trav.preFunc = pfSoundTraverser::soundNodesStop;
       trav.data = NULL;

       pfuTraverse( node, &trav );
   }


protected:
   // triggers every vjSound in the graph.
   static int soundNodesTrigger( pfuTraverser* trav )
   {
      pfNode* currentNode = trav->node;
      if (currentNode->isOfType( pfSoundNode::getClassType() ))
      {
         pfSoundNode* soundNode = static_cast<pfSoundNode*>( currentNode );
         soundNode->sound().trigger();
         vjDEBUG(vjDBG_ALL,0)<<clrOutNORM(clrYELLOW,"[SoundTrigger] ")<<"Setting the "<<soundNode->sound().getName()<<" sound node to on\n"<<vjDEBUG_FLUSH;
      }
      else
      {
         //cout<<"Not a sound Node: "<<currentNode->getName()<<" classtype="<<currentNode->getClassType()<<"!="<<pfSoundNode::getClassType()<<"\n"<<flush;
      }

      return PFTRAV_CONT;      // Return continue
   }

   // stops every vjSound in the graph.
   static int soundNodesStop( pfuTraverser* trav )
   {
      pfNode* currentNode = trav->node;
      if (currentNode->isOfType( pfSoundNode::getClassType() ))
      {
         pfSoundNode* soundNode = static_cast<pfSoundNode*>( currentNode );
         soundNode->sound().stop();
         vjDEBUG(vjDBG_ALL,0)<<clrOutNORM(clrYELLOW,"[SoundStop] ")<<"Setting the "<<soundNode->sound().getName()<<" sound node to off\n"<<vjDEBUG_FLUSH;
      }
      else
      {
         //cout<<"Not a sound Node: "<<currentNode->getName()<<" classtype="<<currentNode->getClassType()<<"!="<<pfSoundNode::getClassType()<<"\n"<<flush;
      }

      return PFTRAV_CONT;      // Return continue
   }

   // enables every vjSound in the graph.
   static int soundNodesEnable( pfuTraverser* trav )
   {
      pfNode* currentNode = trav->node;
      if (currentNode->isOfType( pfSoundNode::getClassType() ))
      {
         pfSoundNode* soundNode = static_cast<pfSoundNode*>( currentNode );
         soundNode->sound().enable( true );
         vjDEBUG(vjDBG_ALL,0)<<clrOutNORM(clrYELLOW,"[SoundEnable] ")<<"Setting the "<<soundNode->sound().getName()<<" sound node to enabled\n"<<vjDEBUG_FLUSH;
      }
      else
      {
         //cout<<"Not a sound Node: "<<currentNode->getName()<<" classtype="<<currentNode->getClassType()<<"!="<<pfSoundNode::getClassType()<<"\n"<<flush;
      }

      return PFTRAV_CONT;      // Return continue
   }

   // disables every vjSound in the graph.
   static int soundNodesDisable( pfuTraverser* trav )
   {
      pfNode* currentNode = trav->node;
      if (currentNode->isOfType( pfSoundNode::getClassType() ))
      {
         pfSoundNode* soundNode = static_cast<pfSoundNode*>( currentNode );
         soundNode->sound().enable( false );
         vjDEBUG(vjDBG_ALL,0)<<clrOutNORM(clrYELLOW,"[SoundDisable] ")<<"Setting the "<<soundNode->sound().getName()<<" sound node to disabled\n"<<vjDEBUG_FLUSH;
      }
      else
      {
         //cout<<"Not a sound Node: "<<currentNode->getName()<<" classtype="<<currentNode->getClassType()<<"!="<<pfSoundNode::getClassType()<<"\n"<<flush;
      }

      return PFTRAV_CONT;      // Return continue
   }
   
   // used to traverse - don't call
   static int nodesReplaceWithSound( pfuTraverser* trav )
   {
      // get the keyname, usually it is "_Sound_"
      std::string keyName = *reinterpret_cast<std::string*>(trav->data);
      pfNode* currentNode = trav->node;
      if (currentNode == NULL)
      {
         return PFTRAV_CONT;      // Return continue
      }

      std::string nodeName = currentNode->getName();
      if (nodeName.size() <= keyName.size())
      {
         // name was too short to even be a match, discard it.
         return PFTRAV_CONT;      // Return continue
      }

      // for verbose output (outputs every node's name.)
      vjDEBUG(vjDBG_ALL, vjDBG_STATE_LVL)<<"[SoundReplacer] Examining node in graph named \""<<nodeName.c_str()<<"\":\n"<<vjDEBUG_FLUSH;

      int startOfKeyWord = nodeName.size() - keyName.size();
      int endOfKeyWord = nodeName.size() - 1;
      std::string isThisOurKeyWord = nodeName.substr( startOfKeyWord, endOfKeyWord );
      if (isThisOurKeyWord == keyName)   // If node is not a Geode
      {
         vjDEBUG( vjDBG_ALL, vjDBG_STATE_LVL )<<"[SoundReplacer] Found node in graph named \""<<nodeName.c_str()<<"\":\n"<<vjDEBUG_FLUSH;
         vjDEBUG( vjDBG_ALL, vjDBG_STATE_LVL )<<"[SoundReplacer]     Substring "<<keyName<<" matched, "<<vjDEBUG_FLUSH;
         pfGroup* parent = currentNode->getParent( 0 ); // FIXME?? will 0 work for all cases (instanced nodes?)
         if (parent != NULL)
         {
            std::string soundName = nodeName.substr( 0, startOfKeyWord );
            vjDEBUG_CONT( vjDBG_ALL, vjDBG_STATE_LVL )<<"extracted sound named \""<<soundName<<"\"\n"<<vjDEBUG_FLUSH;
            vjSound* sound = vjSoundManager::instance()->getHandle( soundName.c_str() );

            // replace the found node with a sound node.
            // note: only replace if the sound is valid.
            //        this way, the users can see where the sounds are.
            if (sound != NULL)
            {
               parent->removeChild( currentNode );
               bool positional_sound_true( true );
               pfSoundNode* sn = new pfSoundNode( sound, positional_sound_true );
               int resuln = sn->setName( nodeName.c_str() );
               if (resuln == 0)
               {
                  std::string newName = nodeName;
                  newName += "__sound";
                  sn->setName( newName.c_str() );
               }
               parent->addChild( sn );
               sound->trigger();
               vjDEBUG( vjDBG_ALL, vjDBG_CONFIG_LVL )<<clrOutNORM( clrGREEN,"[SoundReplacer]     " )<<"Replaced "<<clrOutNORM(clrGREEN, sn->getName())<<" node with a pfSoundNode referencing the "<<soundName<<" sound."<<vjDEBUG_FLUSH;
            }
            else
            {
               vjDEBUG( vjDBG_ALL, vjDBG_CRITICAL_LVL )<<clrOutNORM( clrRED,"[SoundReplacer] !!! WARNING !!! " ) <<"SOUND NOT FOUND: "<<soundName.c_str()<<"\n"<<vjDEBUG_FLUSH;
               vjDEBUG( vjDBG_ALL, vjDBG_CRITICAL_LVL )<<clrOutNORM( clrRED,"[SoundReplacer] !!!         !!! " ) <<"You need to enter \""<<soundName.c_str()<<"\" into your sound config file(s)\n"<<vjDEBUG_FLUSH;
               return PFTRAV_CONT;      // Return continue
            }
         }
         else
         {
            vjDEBUG_CONT( vjDBG_ALL, vjDBG_CRITICAL_LVL )<<"but Parent is NULL (nowhere to hang the pfSoundNode!)\n"<<vjDEBUG_FLUSH;
         }
      }
      // for very verbose output...
      else
      {
         vjDEBUG( vjDBG_ALL,vjDBG_STATE_LVL )<<"[SoundReplacer]     Substring not matched: \""<<isThisOurKeyWord.c_str()<<"\" != \""<<keyName<<"\"\n"<<vjDEBUG_FLUSH;
      }

      return PFTRAV_CONT;      // Return continue
   }
};

#endif   /* _PERFORMER_JUGGLER_SOUND_REPLACE_TRAV_H_ */
