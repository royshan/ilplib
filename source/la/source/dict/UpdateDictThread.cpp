/**
 * @file    UpdatableDictThread.h
 * @author  Vernkin
 * @date    Feb 1, 2009
 * @details
 *  Thread to update dictionary from time to time
 */

#include <la/dict/UpdateDictThread.h>

// for getLastModifiedTime
#include <sys/stat.h>

//#define DEBUG_UDT

#ifdef DEBUG_UDT
    #include <iostream>
    using namespace std;

    #define DEFAULT_CHECK_INTERVAL 2
#else
    #define DEFAULT_CHECK_INTERVAL 600
#endif

using izenelib::util::UString;

namespace la
{

/**
 * @brief get the last modified time of specific file, represented in seconds
 * FIXME only support linux/unix now
 */
long getFileLastModifiedTime(const std::string& path)
{
    struct stat attrib;   // create a file attribute structure
    stat(path.c_str() , &attrib);  // get the attributes of specific file
    return static_cast<long>(attrib.st_mtime);
}

UpdateDictThread::UpdateDictThread()
    : checkInterval_(DEFAULT_CHECK_INTERVAL)
{
}

UpdateDictThread::~UpdateDictThread()
{
    stop();
}

UpdateDictThread UpdateDictThread::staticUDT;

boost::shared_ptr<UpdatableDict> UpdateDictThread::addRelatedDict(const std::string& path,
        const boost::shared_ptr< UpdatableDict >& dict)
{
    izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> swl(lock_);
    MapType::iterator itr = map_.find(path);
    if (itr == map_.end())
    {
        if (dict.get())
        {
            DictSource& ds = map_[path];
            ds.lastModifiedTime_ = getFileLastModifiedTime(path);
            ds.relatedDict_ = dict;
        }
        return dict;
    }
    else
        return itr->second.relatedDict_;
}

boost::shared_ptr<PlainDictionary> UpdateDictThread::createPlainDictionary(
        const std::string& path,
        UString::EncodingType encoding,
        bool ignoreNoExistFile)
{
    boost::shared_ptr<PlainDictionary> pdPtr;
    pdPtr.reset(new PlainDictionary(encoding));
    pdPtr->loadDict(path.c_str(), ignoreNoExistFile);
    addRelatedDict(path, pdPtr);
    return pdPtr;
}

int UpdateDictThread::update_()
{
    izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> swl(lock_);
    int failedCount = 0;
    for (MapType::iterator itr = map_.begin(); itr != map_.end(); ++itr)
    {
        long curModifiedTime = getFileLastModifiedTime(itr->first);
        if (curModifiedTime == itr->second.lastModifiedTime_)
            continue;

        //update all dictionary
        if (itr->second.relatedDict_.get()->update(itr->first.c_str(), curModifiedTime))
            ++failedCount;

#ifdef DEBUG_UDT
        cout << "Update " << itr->first << " failed." << endl;
#endif
        itr->second.lastModifiedTime_ = curModifiedTime;
    }
    return failedCount;
}

void UpdateDictThread::run_()
{
    try
    {
        while (true)
        {
            boost::this_thread::sleep(boost::posix_time::seconds(checkInterval_));
            update_();
        }
    }
    catch (boost::thread_interrupted& e)
    {
        return;
    }
}

bool UpdateDictThread::start()
{
    izenelib::util::ScopedWriteLock<izenelib::util::ReadWriteLock> swl(lock_);
    if (isStarted())
        return false;

    thread_ = boost::thread(&UpdateDictThread::run_, this);
    return true;
}

void UpdateDictThread::stop()
{
    thread_.interrupt();
    thread_.join();
}

}
