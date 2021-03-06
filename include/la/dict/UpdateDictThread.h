/**
 * @file    UpdatableDictThread.h
 * @author  Vernkin
 * @date    Feb 1, 2009
 * @details
 *  Thread to update dictionary from time to time
 */
#ifndef UPDATEDICTTHREAD_H_
#define UPDATEDICTTHREAD_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/function.hpp>
#include <boost/thread/condition.hpp>

#include <la/dict/UpdatableDict.h>
#include <la/dict/PlainDictionary.h>
#include <map>
#include <util/ThreadModel.h>

namespace la
{

/**
 * @brief get the last modified time of specific file, represented in seconds
 * FIXME only support linux/unix now
 */
long getFileLastModifiedTime(const std::string& path);

/**
 * \brief Thread to update dictionary from time to time
 */
class UpdateDictThread
{
public:
    typedef boost::function<void(const std::vector<std::string>&)> UpdateCBType; 
    typedef struct {
        long lastModifiedTime_;
        boost::shared_ptr<UpdatableDict> relatedDict_;
    } DictSource;

    typedef std::map<std::string, DictSource> MapType;

    UpdateDictThread();

    virtual ~UpdateDictThread();

    /**
     * Add related dictionary with specific path
     * \param path the dictionary path
     * \param dict the related dictionary
     */
    boost::shared_ptr<UpdatableDict> addRelatedDict(
            const std::string& path,
            const boost::shared_ptr<UpdatableDict>& dict = boost::shared_ptr<UpdatableDict>()
    );

    void addUpdateCallback(const std::string& unique_str, UpdateCBType cb);
    void removeUpdateCallback(const std::string& unique_str);
    /**
     * Utility function to create plain dictionary. These Dictionary is read-only as
     * updating from dictionary source occurs from time to time
     * \param path the dictionary path
     * \param encoding the encoding of this dictionary
     * \param ignoreNoExistFile if true, the files not exists won't cause error
     */
    boost::shared_ptr<PlainDictionary> createPlainDictionary(
            const string& path,
            izenelib::util::UString::EncodingType encoding,
            bool ignoreNoExistFile = false
    );

    /**
     * Get the check interval ( in seconds )
     */
    unsigned int getCheckInterval()
    {
        return checkInterval_;
    }

    /**
     * Set the check interval ( in seconds )
     */
    void setCheckInterval(unsigned int checkInterval)
    {
        checkInterval_ = checkInterval;
    }

    /**
     * Whether the thread has started
     */
    bool isStarted()
    {
        return thread_.joinable();
    }

    /**
     * start the thread
     * \return whether start successfully. Return false if has started already
     */
    bool start();

    /**
     * Stop the thread's execution, and wait for its complete.
     */
    void stop();

public:
    static UpdateDictThread staticUDT;

private:
    /**
     * Perform updating
     */
    int update_();

    /**
     * Infinite loop for the thread
     */
    void run_();

private:
    /** check interval */
    unsigned int checkInterval_;

    /** path -> DictSource */
    MapType map_;

    std::map<std::string, UpdateCBType> callback_list_;
    /** Read Write Lock */
    boost::mutex lock_;
    boost::condition cond_;
    bool updating_;


    boost::thread thread_;
};

}
#endif /* UPDATEDICTTHREAD_H_ */
