/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
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


#include <Config/vjChunkFactory.h>


//: Creates a Chunk using the given description
vjConfigChunk* vjChunkFactory::createChunk (vjChunkDesc* d) {
    if (d) {
        d->assertValid();
        return new vjConfigChunk (d);
    }
    else
        return 0;
}



// Setup the intial environment needed for creating chunks
void vjChunkFactory::setupInitialEnvironment() {
    // ------ OPEN chunksDesc file ----- //
    char* vj_base_dir = getenv("VJ_BASE_DIR");
    if(vj_base_dir == NULL) {
        vjDEBUG(vjDBG_ALL,vjDBG_VERB_LVL) << "vjChunkFactory::setupInitialEnvironment:\n" << vjDEBUG_FLUSH;
        vjDEBUG(vjDBG_ERROR,vjDBG_CRITICAL_LVL) <<  "Env var VJ_BASE_DIR not defined." << std::endl << vjDEBUG_FLUSH;
        exit(1);
    }

    std::string chunk_desc_file = vj_base_dir;
    chunk_desc_file += "/";
    chunk_desc_file += VJ_SHARE_DIR;
    chunk_desc_file += "/Data/chunksDesc";
    vjDEBUG(vjDBG_ALL,vjDBG_CONFIG_LVL) << "Loading chunk desc file: ["
                           << chunk_desc_file << "]\n" << vjDEBUG_FLUSH;

    this->loadDescs(chunk_desc_file);
    
//     vjChunkDescDB* cfg_desc = new vjChunkDescDB;
//     if (!cfg_desc->load(chunk_desc_file)) {
//             vjDEBUG(vjDBG_ERROR,0) << clrOutNORM(clrRED, "ERROR:") << " vjChunkFactory::setupInitialEnvironment: Config Desc failed to load file: " << endl << vjDEBUG_FLUSH;
//             exit(1);
//     }
//     this->addDescs(cfg_desc);
     

}


/*
vjChunkFactory* vjChunkFactory::_instance = NULL;
vjMutex  vjChunkFactory::_inst_lock;
*/
vjSingletonImp(vjChunkFactory);
