/**
 * @file    UpdatableSynonymContainer.h
 * @author  Vernkin
 * @date    Feb 3, 2010
 * @details
 *  Bridge class to add SynonymContainer into Updatable Dictionary model
 */

#ifndef UPDATABLESYNONYMCONTAINER_H_
#define UPDATABLESYNONYMCONTAINER_H_

#include <la/dict/UpdatableDict.h>

#include <am/vsynonym/VSynonym.h>

#include <string>

namespace la
{

class UpdatableSynonymContainer : public UpdatableDict
{
public:

    /**
     *
     */
    UpdatableSynonymContainer(
            izenelib::am::VSynonymContainer* pSynonymContainer,
            const std::string& knowledgePath)
            : pSynonymContainer_( pSynonymContainer ),
              knowledgePath_( knowledgePath ),
              lastModifiedTime_( -1 ),
              curModifiedTime_( -1 )
    {
    }

    ~UpdatableSynonymContainer()
    {
        delete pSynonymContainer_;
    }

    /**
     * @bried Perform updating
     * @param path the dictionary path
     * @param lastModifiedTime last modified time
     * @return 0 indicates success and others indicates fails
     */
    virtual int update( const char* path, unsigned int lastModifiedTime );

    /**
     * Get the pointer of WK_SynonymContainer, Don't delete this
     * pointer directly
     */
    inline izenelib::am::VSynonymContainer* getSynonymContainer()
    {
        if( lastModifiedTime_ != curModifiedTime_ )
        {
            // perform updating
            pSynonymContainer_->clear( false );
            pSynonymContainer_->loadSynonym( knowledgePath_.c_str() );
            // update the lastModified time
            lastModifiedTime_ = curModifiedTime_;
        }
        return pSynonymContainer_;
    }

private:
    izenelib::am::VSynonymContainer* pSynonymContainer_;

    /** The latest destination path */
    std::string knowledgePath_;

    /** last modified time */
    unsigned int lastModifiedTime_;

    /** current modified time */
    unsigned int curModifiedTime_;

};

}

#endif /* UPDATABLESYNONYMCONTAINER_H_ */
